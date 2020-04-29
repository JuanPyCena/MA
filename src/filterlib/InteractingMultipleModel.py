import numpy as np

from src.utils.Decorators import *
from src.utils.ConfigParser import ParserLib

# Global Variables
EmptyArray = np.array([])
EmptyList = []

# This filter is designed after the formulas described in https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw

class InteractingMultipleModel(object):

    @typecheck(list, np.ndarray, np.ndarray)
    def __init__(self, filters, initial_mode_probabilities, markov_transition_matrix):
        """
        Initializes a InteractingMultipleModel object. For description of formulae, see: https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw
        :param filters: list - a list which holds the filter objects used by the IMM
        :param initial_mode_probabilities: np.ndarray - a vector which holds the initial probilities of the various filter/modes
        :param markov_transition_matrix: np.ndarray - a matrix which defines the probabilities of a filter/mode transition
        """
        self.filters                  = filters  # a list off all filters
        self.mode_probabilities       = initial_mode_probabilities  # u(t) a vector holding the probabilities of all modes, gets updated each timestep
        self.markov_transition_matrix = markov_transition_matrix  # P_ij a matrix holding the transitioning probailities of all modes

        self.mixed_state      = []  # a list of numpy.array which holds the mixed state of all filters within the IMM
        self.mixed_covariance = []  # a list numpy.array which holds the mixed covariance matrix of all filters within the IMM

        self.state      = np.zeros(self.filters[0].state.shape)  # the state of the IMM filter, x
        self.covariance = np.zeros(self.filters[0].covariance.shape)  # the covariance of the IMM filter, P

        # Mode probability matrix, U
        self.mode_probability_matrix = np.divide(np.ones(self.markov_transition_matrix.shape),
                                                 np.size(self.markov_transition_matrix))

    ##############################################################################

    def _calc_mode_probability_matrix_matrix(self):
        for col_num in range(0, np.size(self.markov_transition_matrix, 0)):  # col_num: j
            c = 0
            for mode_num, mode_probabilty in enumerate(self.mode_probabilities):  # mode_num: i
                markov_transition_probability = self.markov_transition_matrix[mode_num, col_num]
                c += markov_transition_probability * mode_probabilty

            for mode_num, mode_probabilty in enumerate(self.mode_probabilities):  # mode_num: i
                markov_transition_probability = self.markov_transition_matrix[mode_num, col_num]
                self.mode_probability_matrix[mode_num][col_num] = markov_transition_probability * mode_probabilty / c

    ##############################################################################

    def _calc_mixed_state(self):
        self._calc_mode_probability_matrix_matrix()

        states      = []
        covariances = []
        for j, filter in enumerate(self.filters):
            state      = np.zeros(filter.state.shape)
            covariance = np.zeros(filter.covariance.shape)

            for i, mixed_state in enumerate(self.mixed_state):
                state += mixed_state * self.mode_probability_matrix[i,j]

            # this need to be done in a separate loop because the state has to be calculated before hand
            for i, mixed_covariance in enumerate(self.mixed_covariance):
                covariance += self.mode_probability_matrix[i,j] * (mixed_covariance +
                                                                   np.dot((self.mixed_state[i] - state),
                                                                          (self.mixed_state[i] - state).T))

            # need to save this in a temporary list since we do not want to alter the mixed_state yet
            states.append(state)
            covariances.append(covariance)

        # Save the new mixed states now after all have been calculated
        self.mixed_state      = []
        self.mixed_covariance = []
        for state in states:
            self.mixed_state.append(state)

        for covariance in covariances:
            self.mixed_covariance.append(covariance)

    ##############################################################################
    @typecheck(np.ndarray)
    def _calc_mode_probability(self, measurement):
        lambdas = []

        # Calculate likelihood functions
        for j, filter in enumerate(self.filters):
            # S = HPH' + R
            HP    = np.dot(filter.measurement_function, filter.covariance)
            S     = np.dot(HP, filter.measurement_function.T) + filter.state_uncertainty
            S_inv = np.linalg.inv(S)

            # y = Z - Hx
            y     = measurement - np.dot(filter.measurement_function, filter.state)

            # d^2 = y'S^-1y
            y_TS     = np.dot(y.T, S_inv)
            d_square = np.dot(y_TS, y)

            # lambda_j = (exp((-d^2)/2)/(square(2*pi*det(S))))
            e = np.exp((-d_square)/2)
            lambda_j = e / (np.sqrt(2 * np.pi * np.linalg.det(S)))
            lambdas.append(lambda_j)

        # Recalculate mode probabilities based on previous mode probabilities and current likelihood functions
        # u_j(t) = (u_j(t-1)*lambda_j(t)) / (sum(u_j(t-1)*lambda_j(t))
        new_mode_probabilities = []
        for j, mode_probability in enumerate(self.mode_probabilities):

            c = 0
            for lambda_i in lambdas:
                c += mode_probability * lambda_i

            prob = mode_probability * lambdas[j] / c

            # Catch numerical instability
            if prob <= 1e-9:
                prob = 1e-9

            new_mode_probabilities.append(prob)

        # update mode probabilities
        self.mode_probabilities = np.array(new_mode_probabilities)

    ##############################################################################

    def _calculate_IMM_state_covariance(self):
        state = np.zeros(self.filters[0].state.shape)
        covariance = np.zeros(self.filters[0].covariance.shape)
        for idx, filter in enumerate(self.filters):
            state += self.mode_probabilities[idx] * filter.state

        # this need to be done in a separate loop because the state has to be calculated before hand
        for idx, filter in enumerate(self.filters):
            covariance += self.mode_probabilities[idx] * (filter.covariance +
                                                        np.dot((filter.state - state), (filter.state - state).T))

        self.state      = state
        self.covariance = covariance

    ##############################################################################

    @typecheck(np.ndarray)
    def predict_update(self, measurement, input=None, **update_kwds):
        """
        This function makes a prediction of each filter using their respective predict function and updates their states
        and covariances aswell
        :param measurement: np.ndarray - holds the measurement of the sensor
        :param input: np.ndarray - holds the input into to directly interact with the states of each filter, default(None)
        :param update_kwds: dict - holds a list which can be used to parse several additional inputs to the update
                                   function of the various filters
                                   (e.g.: "kwds = [{}, {"HJacobian": self.H_of, "Hx": self.hx}]",
                                   This example uses a Kalmanfilter as the first filter and an
                                   ExtendedKalmanFilter as the seconds filter)
        """
        if input is None:
            input = EmptyArray

        # Set mixed state to initial filter states
        if self.mixed_state == EmptyList:
            for idx, filter in enumerate(self.filters):
                self.mixed_state.append(filter.state)

        # Set mixed covariance to initial filter covariance
        if self.mixed_covariance == EmptyList:
            for idx, filter in enumerate(self.filters):
                self.mixed_covariance.append(filter.covariance)

        # Get mixed state before prediction
        self._calc_mixed_state()

        for idx, filter in enumerate(self.filters):
            kwds = dict()
            if "update_kwds" in update_kwds.keys():
                kwds = update_kwds["update_kwds"][idx]

            # update filter state and covariance to current mixed values
            filter.state      = self.mixed_state[idx]
            filter.covariance = self.mixed_covariance[idx]

            filter.predict(input)
            filter.update(measurement, **kwds)

            # update mixed_state and mixed_covariance to the updated filter values
            self.mixed_state[idx]      = filter.state
            self.mixed_covariance[idx] = filter.covariance


        # Calculate the mode probability for the state and covariance update
        self._calc_mode_probability(measurement)

        # Calculate te state and covariance of the filter
        self._calculate_IMM_state_covariance()

    ##############################################################################

    @typecheck(float)
    def calculate_time_depended_matrices_of_filters(self, time_delta):
        """
        This function replaces the place holder "dt" within all matrices of all filters with a given float values
        :param time_delta: float - time since last calculation step
        """
        for filter in self.filters:
            filter.transition_function    = ParserLib.calculate_time_depended_matrix(filter.transition_function,
                                                                                     time_delta, "dt")
            filter.measurement_function   = ParserLib.calculate_time_depended_matrix(filter.measurement_function,
                                                                                     time_delta, "dt")
            filter.input_function         = ParserLib.calculate_time_depended_matrix(filter.input_function,
                                                                                     time_delta, "dt")
            filter.process_noise_function = ParserLib.calculate_time_depended_matrix(filter.process_noise_function,
                                                                                     time_delta, "dt")
            filter.covariance             = ParserLib.calculate_time_depended_matrix(filter.covariance,
                                                                                     time_delta, "dt")
            filter.state_uncertainty      = ParserLib.calculate_time_depended_matrix(filter.state_uncertainty,
                                                                                     time_delta, "dt")

    # EOC
# EOF