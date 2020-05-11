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
        self.likelihood               = np.zeros(self.mode_probabilities.shape)  # a vector to hold the likelihood of each filter
        self.c                        = np.zeros(self.mode_probabilities.shape) # vector used for the calculation of the mode probabilities

        self.mixed_state      = []  # a list of numpy.array which holds the mixed state of all filters within the IMM
        self.mixed_covariance = []  # a list numpy.array which holds the mixed covariance matrix of all filters within the IMM

        self.state      = np.zeros(self.filters[0].state.shape)  # the state of the IMM filter, x
        self.covariance = np.zeros(self.filters[0].covariance.shape)  # the covariance of the IMM filter, P

        self.state_prior      = np.zeros(self.filters[0].state.shape)  # the state of the IMM filter after the prediction, x
        self.covariance_prior = np.zeros(self.filters[0].covariance.shape)  # the covariance of the IMM filter after the prediction, P
        self.state_post       = np.zeros(self.filters[0].state.shape)  # the state of the IMM filter after the update, x
        self.covariance_post  = np.zeros(self.filters[0].covariance.shape)  # the covariance of the IMM filter after the update, P

        # Mode probability matrix, U
        self.mode_probability_matrix = np.divide(np.ones(self.markov_transition_matrix.shape),
                                                 np.size(self.markov_transition_matrix))

        # calculated initial values
        self._calc_mode_probability_matrix()
        self._calc_imm_state()

    ##############################################################################

    def _calc_mode_probability_matrix(self):
        """
        Compute the mixing probability for each filter.
        """
        self.c = np.dot(self.mode_probabilities, self.markov_transition_matrix)
        # get num of cols and rows of transition matrix
        M, N = np.size(self.markov_transition_matrix, 0), np.size(self.markov_transition_matrix, 1)

        # Calculate mode probability matrix according to filterpy.IMM
        for i in range(M):
            for j in range(N):
                self.mode_probability_matrix[i, j] = \
                    (self.markov_transition_matrix[i, j] * self.mode_probabilities[i]) / self.c[j]

    ##############################################################################

    def _calc_imm_state(self):
        """
        Computes the IMM's mixed state estimate from each filter using
        the mode probability to weight the estimates.
        """

        # The IMM mixed state is the sum of all filter state weighted by its probability
        self.state.fill(0)
        for filter, probability in zip(self.filters, self.mode_probabilities):
            self.state += filter.state * probability

        # The covariance is the sum of each covariance + the square of the state difference of each
        # filter to the mixed state weighted by its probability
        self.covariance.fill(0)
        for filter, probability in zip(self.filters, self.mode_probabilities):
            # the difference in state between the IMM and each filter is used to calculate the covariance matrix
            state_diff = filter.state - self.state
            # P_imm = sum(mu[i] *(state_diff * state_diff' + P_filter))
            self.covariance += probability * (np.outer(state_diff, state_diff) + filter.covariance)

    ##############################################################################

    def _calc_mixed_state(self):
        """
        Calculate the mixed states and covariances of the filters
        """
        xs, Ps = [], []
        for i, (f_i, w_i) in enumerate(zip(self.filters, self.mode_probability_matrix.T)):
            x = np.zeros(self.state.shape)
            for filter, probability_j in zip(self.filters, w_i):
                x += filter.state * probability_j
            xs.append(x)

            P = np.zeros(self.covariance.shape)
            for filter, probability_j in zip(self.filters, w_i):
                state_diff = filter.state - x
                P += probability_j * (np.outer(state_diff, state_diff) + filter.covariance)
            Ps.append(P)

        self.mixed_state      = xs
        self.mixed_covariance = Ps

    ##############################################################################

    def _calc_mode_probability(self):
        self.mode_probabilities = self.c * self.likelihood
        # normalise mode probabilities to not be greater than 1
        self.mode_probabilities /= np.sum(self.mode_probabilities)
        if float("%.3f"%np.sum(self.mode_probabilities)) > 1:
            raise ValueError("The sum of all mode probabilities is greater than 1: mode probabilities {}, sum is {}"
                             .format(self.mode_probabilities, np.sum(self.mode_probabilities)))

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
            input = np.array([])

        self._calc_mixed_state()

        for idx, filter in enumerate(self.filters):
            # update filter state and covariance to current mixed values
            filter.state      = self.mixed_state[idx]
            filter.covariance = self.mixed_covariance[idx]
            filter.predict(input)

        # Calculate the IMM state after prediction of each filter has finished
        self._calc_imm_state()
        self.state_prior      = self.state.copy()
        self.covariance_prior = self.covariance.copy()

        for idx, filter in enumerate(self.filters):
            kwds = dict()
            if "update_kwds" in update_kwds.keys():
                kwds = update_kwds["update_kwds"][idx]
            filter.update(measurement, **kwds)
            self.likelihood[idx] = filter.likelihood

        # Calculate the mode probabilities and recalculate the probability matrix
        self._calc_mode_probability()
        self._calc_mode_probability_matrix()

        self._calc_imm_state()
        self.state_post      = self.state.copy()
        self.covariance_post = self.covariance.copy()
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