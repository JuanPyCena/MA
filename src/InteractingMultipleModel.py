import numpy as np
import types

from src.Decorators import *
from src.Filters import KalmanFilter as KF
from src.Filters import ExtendedKalmanFilter as EKF

class InteractingMultipleModel(object):

    @typecheck(list, np.ndarray, np.ndarray)
    def __init__(self, filters, initial_mode_probability, markov_transition_matrix):
        self.filters                  = filters  # a list off all filters
        self.mode_probability         = initial_mode_probability  # u(t) a vector holding the probabilities of all modes, gets updated each timestep
        self.markov_transition_matrix = markov_transition_matrix  # P_ij a matrix holding the transitioning probailities of all modes

        self.mixed_state  # a list of numpy.array which holds the mixed state of all filters within the IMM
        self.mixed_covariance  # a list numpy.array which holds the mixed covariance matrix of all filters within the IMM

        self.state  # the state of the IMM filter, x
        self.covariance  # the covariance of the IMM filter, P

    ##############################################################################

    def _calc_mixed_state(self):
        pass

    ##############################################################################

    def _calc_mode_probability(self):
        pass

    ##############################################################################

    def _calculate_IMM_state_covariance(self):
        state = np.zeros(self.filter[0].states.shape)
        covariance = np.zeros(self.filter[0].covariance.shape)
        for idx, filter in enumerate(self.filters):
            state += self.mode_probability[idx] * filter.state

        # this need to be done in a separate loop because the state has to be calculated before hand
        for idx, filter in enumerate(self.filters):
            covariance += self.mode_probability[idx] *( filter.covariance +
                                                        np.dot((filter.state - state), (filter.state - state).T))

        self.state      = state
        self.covariance = covariance

    ##############################################################################

    @typecheck((np.ndarray,))
    def predict_update(self, measurement, input=np.array([]), *args, **kwds):
        # Get mixed state before prediction
        self._calc_mixed_state()

        for idx, filter in enumerate(self.filters):
            # update filter state and covariance to current mixed values
            filter.state      = self.mixed_state[idx]
            filter.covariance = self.mixed_covariance[idx]

            filter.predict(input)
            filter.update(measurement, *args, **kwds)


        # Calculate the mode probability for the state and covariance update
        self._calc_mode_probability()

        self._calculate_IMM_state_covariance()

    # EOC
# EOF