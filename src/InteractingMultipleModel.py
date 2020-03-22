import numpy as np
import types

from src.Decorators import *
from src.Filters import KalmanFilter as KF
from src.Filters import ExtendedKalmanFilter as EKF

# This filter is designed after the formulas described in https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw

class InteractingMultipleModel(object):

    @typecheck(list, np.ndarray, np.ndarray)
    def __init__(self, filters, initial_mode_probabilities, markov_transition_matrix):
        self.filters                  = filters  # a list off all filters
        self.mode_probabilities       = initial_mode_probabilities  # u(t) a vector holding the probabilities of all modes, gets updated each timestep
        self.markov_transition_matrix = markov_transition_matrix  # P_ij a matrix holding the transitioning probailities of all modes

        self.mixed_state      = list()  # a list of numpy.array which holds the mixed state of all filters within the IMM
        self.mixed_covariance = list()  # a list numpy.array which holds the mixed covariance matrix of all filters within the IMM

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
        self.mixed_state      = list()
        self.mixed_covariance = list()
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
            HP    = np.dot(filter.measurement_function, self.mixed_covariance[j])
            S     = np.dot(HP, filter.measurement_function.T) + filter.state_uncertainty
            S_inv = np.linalg.inv(S)

            # y = Z - Hx
            y     = measurement - np.dot(filter.measurement_function, self.mixed_state[j])

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

            new_mode_probabilities.append(mode_probability * lambdas[j] / c)

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

    @typecheck(np.ndarray, (np.ndarray,))
    def predict_update(self, measurement, input=np.array([]), **update_kwds):
        # Set mixed state to initial filter states
        if self.mixed_state == []:
            for idx, filter in enumerate(self.filters):
                self.mixed_state.append(filter.state)

        # Set mixed covariance to initial filter covariance
        if self.mixed_covariance == []:
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

    # EOC
# EOF