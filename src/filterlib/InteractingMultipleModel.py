import numpy as np

from src.utils.Decorators import *
from src.utils.ConfigParser import ParserLib
from src.utils.logmod import Logger

# Global Variables
EmptyArray = np.array([])
EmptyList = []
SIGMA_A_SQ = 1

# This filter is designed after the formulas described in https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw

class InteractingMultipleModel(object):

    @typecheck(list, np.ndarray, np.ndarray)
    def __init__(self, filters, initial_mode_probabilities, markov_transition_matrix, expansion_matrix, shrinking_matrix):
        """
        Initializes a InteractingMultipleModel object. For description of formulae, see: https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw
        :param filters: list - a list which holds the filter objects used by the IMM
        :param initial_mode_probabilities: np.ndarray - a vector which holds the initial probilities of the various filter/modes
        :param markov_transition_matrix: np.ndarray - a matrix which defines the probabilities of a filter/mode transition
        """
        # start logging
        self.log = Logger()
        self.log.write_to_log("---------------------Start IMM---------------------")

        # delete logfiles that are older then one day
        if self.log.delete_old_logfiles():
            self.log.write_to_log("INFO: Old logfile removed")

        self.filters                  = filters  # a list off all filters
        self.mode_probabilities       = initial_mode_probabilities  # u(t) a vector holding the probabilities of all modes, gets updated each timestep
        self.markov_transition_matrix = markov_transition_matrix  # P_ij a matrix holding the transitioning probailities of all modes
        self.likelihood               = np.zeros(self.mode_probabilities.shape)  # a vector to hold the likelihood of each filter
        self.c                        = np.zeros(self.mode_probabilities.shape) # vector used for the calculation of the mode probabilities

        self.mixed_state      = []  # a list of numpy.array which holds the mixed state of all filters within the IMM
        self.mixed_covariance = []  # a list numpy.array which holds the mixed covariance matrix of all filters within the IMM

        self.state       = np.zeros(6)  # the state of the IMM filter, x
        self.covariance  = np.zeros(6)  # the covariance of the IMM filter, P
        self.measurement = np.zeros(6)  # z

        self.state_prior      = np.zeros(self.state.shape)  # the state of the IMM filter after the prediction, x
        self.covariance_prior = np.zeros(self.covariance.shape)  # the covariance of the IMM filter after the prediction, P
        self.state_post       = np.zeros(self.state.shape)  # the state of the IMM filter after the update, x
        self.covariance_post  = np.zeros(self.covariance.shape)  # the covariance of the IMM filter after the update, P

        # Mode probability matrix, U
        self.mode_probability_matrix = np.divide(np.ones(self.markov_transition_matrix.shape),
                                                 np.size(self.markov_transition_matrix))

        # Matrix used to to expand the filter state to the IMM state
        expansion_matrix = ParserLib.calculate_time_depended_matrix(expansion_matrix, self.filters[0].input[0], "sigma_a_sq")
        shrinking_matrix = ParserLib.calculate_time_depended_matrix(shrinking_matrix, self.filters[0].input[0], "sigma_a_sq")

        self.expansion_matrix = expansion_matrix
        self.shrinking_matrix = shrinking_matrix

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
            filter_state_expanded = self._expand_to_imm_state(filter.state)
            self.state = self.state + filter_state_expanded * probability

        # The covariance is the sum of each covariance + the square of the state difference of each
        # filter to the mixed state weighted by its probability
        self.covariance.fill(0)
        for filter, probability in zip(self.filters, self.mode_probabilities):
            # the difference in state between the IMM and each filter is used to calculate the covariance matrix
            filter_state_expanded = self._expand_to_imm_state(filter.state)
            filter_covariance_expanded = self._expand_to_imm_covariance(filter.covariance, len(filter.state))
            state_diff = filter_state_expanded - self.state
            # P_imm = sum(mu[i] *(state_diff * state_diff' + P_filter))
            self.covariance = self.covariance + probability * (np.outer(state_diff, state_diff) + filter_covariance_expanded)

    ##############################################################################

    def _calc_mixed_state(self):
        """
        Calculate the mixed states and covariances of the filters
        """
        xs, Ps = [], []
        for i, (f_i, w_i) in enumerate(zip(self.filters, self.mode_probability_matrix.T)):
            x = np.zeros(self.state.shape)
            for filter, probability_j in zip(self.filters, w_i):
                filter_state_expanded = self._expand_to_imm_state(filter.state)
                x = x + filter_state_expanded * probability_j
            xs.append(x.astype(float))

            P = np.zeros(self.covariance.shape)
            for filter, probability_j in zip(self.filters, w_i):
                filter_state_expanded = self._expand_to_imm_state(filter.state)
                filter_covariance_expanded = self._expand_to_imm_covariance(filter.covariance, len(filter.state))
                state_diff = filter_state_expanded - x
                P = P + probability_j * (np.outer(state_diff, state_diff) + filter_covariance_expanded)
            Ps.append(P.astype(float))

        self.mixed_state      = xs
        self.mixed_covariance = Ps

    ##############################################################################

    def _calc_mode_probability(self):
        self.mode_probabilities = self.c * self.likelihood
        # normalise mode probabilities to not be greater than 1
        self.mode_probabilities /= np.sum(self.mode_probabilities)
        self.log.write_to_log("INFO: IMM mode_probabilities {}".format(self.mode_probabilities))
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

        self.expansion_matrix = ParserLib.evaluate_functional_matrix(self.expansion_matrix, 0,
                                                                "dt", ["sigma_a_sq"],
                                                                [input], [], [])
        self.shrinking_matrix = ParserLib.evaluate_functional_matrix(self.shrinking_matrix, 0,
                                                                "dt", ["sigma_a_sq"],
                                                                [input], [], [])

        self.measurement = measurement

        self._calc_mixed_state()

        for idx, filter in enumerate(self.filters):
            # update filter state and covariance to current mixed values
            filter.state      = self._shrink_to_filter_state(self.mixed_state[idx], len(filter.state))
            filter.covariance = self._shrink_to_filter_covariance(self.mixed_covariance[idx], len(filter.state))
            filter.predict(input)
            self.log.write_to_log("INFO: {} state after prediction: {}".format(filter, filter.state))

        # Calculate the IMM state after prediction of each filter has finished
        self._calc_imm_state()
        self.state_prior      = self.state.copy()
        self.covariance_prior = self.covariance.copy()

        self.log.write_to_log("INFO: IMM state after prediction: {}".format(self.state_prior))

        for idx, filter in enumerate(self.filters):
            kwds = dict()
            if "update_kwds" in update_kwds.keys():
                kwds = update_kwds["update_kwds"][idx]
            z = self._shrink_to_filter_state(measurement, len(filter.state))
            filter.update(z, **kwds)
            self.log.write_to_log("INFO: {} state after update: {}".format(filter, filter.state))
            self.likelihood[idx] = filter.likelihood

        # Calculate the mode probabilities and recalculate the probability matrix
        self._calc_mode_probability()
        self._calc_mode_probability_matrix()

        self._calc_imm_state()
        self.state_post      = self.state.copy()
        self.covariance_post = self.covariance.copy()

        self.log.write_to_log("INFO: IMM state after update: {}".format(self.state_prior))
    ##############################################################################

    @typecheck(float)
    def calculate_time_depended_matrices_of_filters(self, time_delta, measurement):
        """
        This function replaces the place holder "dt" within all matrices of all filters with a given float values
        :param time_delta: float - time since last calculation step
        """
        variables = ["sigma_a_sq", "vx", "vy", "ax", "ay", "x_m", "y_m", "x", "y"]
        variable_replacement = [self.filters[0].input[0], self.state[1], self.state[3], self.state[4], self.state[5], measurement[0],
                                measurement[1], self.state[0], self.state[2]]
        functions = ["cos", "sin", "arctan"]
        function_replacement = ["np.cos", "np.sin", "np.arctan"]

        for filter in self.filters:
            filter.transition_function    = ParserLib.evaluate_functional_matrix(filter.transition_function,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.jacobi_matrix          = ParserLib.evaluate_functional_matrix(filter.jacobi_matrix,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.measurement_function   = ParserLib.evaluate_functional_matrix(filter.measurement_function,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.input_function         = ParserLib.evaluate_functional_matrix(filter.input_function,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.process_noise_function = ParserLib.evaluate_functional_matrix(filter.process_noise_function,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.covariance             = ParserLib.evaluate_functional_matrix(filter.covariance,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
            filter.state_uncertainty      = ParserLib.evaluate_functional_matrix(filter.state_uncertainty,
                                                                                 time_delta, "dt", variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)

    ##############################################################################

    def _expand_to_imm_state(self, state):
        if len(state) == 4:
            state = np.append(state, [1, 1])
            new_state = np.dot(self.expansion_matrix, state)
            return new_state
        else:
            return state

    ##############################################################################

    def _expand_to_imm_covariance(self, covariance, filter_state_dim):
        if filter_state_dim == 4:
            new_covariance = np.eye(len(self.state))
            M, N = np.size(covariance, 0), np.size(covariance, 1)
            for i in range(M):
                for j in range(N):
                    new_covariance[i, j] = covariance[i, j]
            new_covariance = np.dot(np.dot(self.expansion_matrix, new_covariance), self.expansion_matrix.T)
            return new_covariance
        else:
            return covariance

    ##############################################################################

    def _shrink_to_filter_state(self, imm_state, filter_state_dim):
        if filter_state_dim == 4:
            new_state = np.dot(self.shrinking_matrix, imm_state)
            return new_state
        else:
            return imm_state

    ##############################################################################

    def _shrink_to_filter_covariance(self, imm_covariance, filter_state_dim):
        if filter_state_dim == 4:
            new_covariance = np.dot(np.dot(self.shrinking_matrix, imm_covariance), self.shrinking_matrix.T)
            return new_covariance
        else:
            return imm_covariance

    # EOC
# EOF