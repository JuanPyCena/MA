import numpy as np
import types
import sys
from math import log, exp
from scipy.stats import multivariate_normal

from src.utils.Decorators import *
from src.utils.logmod import Logger

# Global Variables
EmptyArray = np.array([])
logger = Logger()

class KalmanFilter(object):

    @typecheck(np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self, initial_state, inital_input, initial_measurement,
                 state_transition_matrix, control_input_matrix, process_noise_matrix,
                 initial_covariance_covariance_matrix, inital_measurement_function, inital_state_uncertainty):
        """
        Initializes a KalmanFilter object. For description of formulae, see: https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw
        :param initial_state: np.ndarray - intial state of the filter, x
        :param inital_input: np.ndarray - initial input for the filter, u
        :param initial_measurement: np.ndarray - initial measurement for the filter, z
        :param state_transition_matrix: np.ndarray - matrix which defines the system beahviour, F
        :param control_input_matrix: np.ndarray - matrix which control the influence of the input onto the state, G/B
        :param process_noise_matrix: np.ndarray - matrix which controls the process noise, Q
        :param initial_covariance_covariance_matrix: np.ndarray - initial covariance matrix, P
        :param inital_measurement_function: np.ndarray - matrix which transposes the measurement into the state space, H
        :param inital_state_uncertainty: np.ndarray - matrix which control the state uncertainty, R
        """
        self.state                  = initial_state                         # x
        self.input                  = inital_input                          # u
        self.measurement            = initial_measurement                   # z
        self.transition_function    = state_transition_matrix               # F
        self.input_function         = control_input_matrix                  # G
        self.process_noise_function = process_noise_matrix                  # Q
        self.covariance             = initial_covariance_covariance_matrix  # P
        self.measurement_function   = inital_measurement_function           # H
        self.state_uncertainty      = inital_state_uncertainty              # R
        self.jacobi_matrix          = state_transition_matrix               # J, Jacobi matrix, not used for KF

        # Variables to save the system uncertainty projected into the measurment space. To be used in likelyhood calculations
        self.system_uncertainty         = None  # S
        self.inverse_system_uncertainty = None  # inv(S)

        # Set values prior to prediction
        # these will always be a copy of state,covariance after predict() is called
        self.state_prior         = self.state.copy()
        self.covariance_prior    = self.covariance.copy()

        # Set values post to prediction
        # these will always be a copy of state,covariance after update() is called
        self.state_post         = self.state.copy()
        self.covariance_post    = self.covariance.copy()

        self.state_extrapolated      = self.state.copy()
        self.covariance_extrapolated = self.covariance.copy()

        # Private variables for saving the likelihood of this filter
        self._log_likelihood = log(sys.float_info.min)
        self._likelihood     = sys.float_info.min

    ##############################################################################

    def __repr__(self):
        return "Kalman Filter"

    ##############################################################################

    @typecheck((np.ndarray, ))
    def predict(self, input=None):
        """
        Predict next state (prior) using the Kalman filter state propagation
        equations.
        :param input: np.ndarray - input to directly influence the state, default(None)
        """
        if input is not None:
            self.input = input

        # x = Fx + Gu
        if self.input_function.shape != EmptyArray.shape and self.input.shape != EmptyArray.shape:
            self.state = np.dot(self.transition_function, self.state) + np.dot(self.input_function, self.input)
        else:
            self.state = np.dot(self.transition_function, self.state)

        # P = FPF' + Q
        self.covariance = np.dot(np.dot(self.transition_function, self.covariance), self.transition_function.T) + self.process_noise_function

        # save prior
        self.state_prior      = self.state.copy()
        self.covariance_prior = self.covariance.copy()

    ##############################################################################

    @typecheck((np.ndarray, ))
    def extrapolate(self, input=None):
        """
        Predict next state (prior) using the Kalman filter state propagation
        equations.
        :param input: np.ndarray - input to directly influence the state, default(None)
        """

        # x = Fx + Gu
        if self.input_function.shape != EmptyArray.shape and input is not None:
            self.state_extrapolated = np.dot(self.transition_function, self.state) + np.dot(self.input_function, input)
        else:
            self.state_extrapolated = np.dot(self.transition_function, self.state)

        # P = FPF' + Q
        self.covariance_extrapolated = np.dot(np.dot(self.transition_function, self.covariance), self.transition_function.T) + self.process_noise_function

    ##############################################################################

    @typecheck(np.ndarray)
    def update(self, z, expand_matrix, expand_vector):
        """
        Update the filter state and covariance matrix using the (prior) made predictions
        :param z: np.ndarray - measurement used to determine the quality of the prediction
        """
        # y = z - Hx
        # error (residual) between measurement and prediction
        y = z - np.dot(self.measurement_function, self.state)

        # common subexpression for speed
        PH_transpose = np.dot(self.covariance, self.measurement_function.T).astype(float)

        # S = HPH' + R
        # project system uncertainty into measurement space
        S = np.dot(self.measurement_function, PH_transpose) + self.state_uncertainty
        S = S.astype(float)
        S_inv = np.linalg.inv(S)

        # Save system uncertainty into member variables
        # self.system_uncertainty = expand_matrix(S, len(self.state))
        # self.inverse_system_uncertainty = expand_matrix(S_inv, len(self.state))
        self.system_uncertainty = S
        self.inverse_system_uncertainty = S_inv

        # K = PH'inv(S)
        # map system uncertainty into kalman gain
        K = np.dot(PH_transpose, S_inv)

        # x = x + Ky
        # predict new state with residual scaled by the kalman gain
        self.state = self.state + np.dot(K, y)
        self.error = np.round(z, 5) - np.round(np.dot(self.measurement_function, self.state).astype(float), 5)

        # P = (I-KH)P(I-KH)' + KRK'
        # This is more numerically stable
        # and works for non-optimal K vs the equation
        # P = (I-KH)P usually seen in the literature.
        I = np.eye(np.size(self.state))
        I_KH = I - np.dot(K, self.measurement_function)
        self.covariance = np.dot(np.dot(I_KH, self.covariance), I_KH.T) + np.dot(np.dot(K, self.state_uncertainty), K.T)

        # save measurement and posterior state
        self.measurement     = z.copy()
        self.state_post      = self.state.copy()
        self.covariance_post = self.covariance.copy()

        # set to None to force recompute
        self._log_likelihood = None
        self._likelihood     = None

    ##############################################################################

    @property
    def log_likelihood(self):
        """
        Computes the log_likelyhood of the last measurement based on the system uncertainty, S.
        :return: float - log likelihood of last measurement of this filter
        """

        if self._log_likelihood is None:
            flat_y = np.asarray(self.error).flatten()
            # Set mean to None, this is treated as having the zero vector being the mean.
            flat_mean = None
            det = np.linalg.det(self.system_uncertainty)
            self._log_likelihood = multivariate_normal.logpdf(flat_y, flat_mean, cov=self.system_uncertainty, allow_singular=True)
        return self._log_likelihood

    ##############################################################################

    @property
    def likelihood(self):
        """
        Computed from the log-likelihood. The log-likelihood can be very
        small,  meaning a large negative value such as -28000. Taking the
        exp() of that results in 0.0, which can break typical algorithms
        which multiply by this value, so by default we always return a
        number >= sys.float_info.min.
        :return: float - likelihood of last measurement of this filter
        """
        if self._likelihood is None:
            self._likelihood = exp(self.log_likelihood)
            if self._likelihood == 0:
                self._likelihood = sys.float_info.min
        return self._likelihood

    #EOC

##############################################################################

class ExtendedKalmanFilter(object):

    @typecheck(np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self, initial_state, inital_input, initial_measurement,
                 state_transition_matrix, control_input_matrix, process_noise_matrix,
                 initial_covariance_covariance_matrix, inital_measurement_function, inital_state_uncertainty, jacobi_matrix):
        """
        Initializes a KalmanFilter object. For further description, see: https://github.com/rlabbe/filterpy/blob/master/filterpy/kalman/EKF.py
        :param initial_state: np.ndarray - intial state of the filter, x
        :param inital_input: np.ndarray - initial input for the filter, u
        :param initial_measurement: np.ndarray - initial measurement for the filter, z
        :param state_transition_matrix: np.ndarray - matrix which defines the system beahviour, F
        :param control_input_matrix: np.ndarray - matrix which control the influence of the input onto the state, G/B
        :param process_noise_matrix: np.ndarray - matrix which controls the process noise, Q
        :param initial_covariance_covariance_matrix: np.ndarray - initial covariance matrix, P
        :param inital_measurement_function: np.ndarray - matrix which transposes the measurement into the state space, H
        :param inital_state_uncertainty: np.ndarray - matrix which control the state uncertainty, R
        """
        self.state                  = initial_state                         # x
        self.input                  = inital_input                          # u
        self.measurement            = initial_measurement                   # z
        self.transition_function    = state_transition_matrix               # F
        self.input_function         = control_input_matrix                  # G
        self.process_noise_function = process_noise_matrix                  # Q
        self.covariance             = initial_covariance_covariance_matrix  # P
        self.measurement_function   = inital_measurement_function           # H
        self.state_uncertainty      = inital_state_uncertainty              # R
        self.jacobi_matrix          = jacobi_matrix                         # J

        # Variables to save the system uncertainty projected into the measurment space. To be used in likelyhood calculations
        self.system_uncertainty         = None  # S
        self.inverse_system_uncertainty = None  # inv(S)

        self.previous_state = initial_state

        # Set values prior to prediction
        # these will always be a copy of state,covariance after predict() is called
        self.state_prior = self.state.copy()
        self.covariance_prior = self.covariance.copy()

        # Set values post to prediction
        # these will always be a copy of state,covariance after update() is called
        self.state_post = self.state.copy()
        self.covariance_post = self.covariance.copy()

        self.state_extrapolated = self.state.copy()
        self.covariance_extrapolated = self.covariance.copy()

        # Private variables for saving the likelihood of this filter
        self._log_likelihood = log(sys.float_info.min)
        self._likelihood     = sys.float_info.min

    ##############################################################################

    def __repr__(self):
        return "Extended Kalman Filter"

    ##############################################################################

    @typecheck(np.ndarray, types.FunctionType, (tuple, ), (tuple, ), (np.ndarray, ))
    def predict_update(self, z, Hx, args=(), hx_args=(), u=None):
        """ Performs the predict/update innovation of the extended Kalman
        filter.
        Parameters
        ----------
        z : np.array
            measurement for this step.
            If `None`, only predict step is perfomed.
        Hx : function
            function which takes as input the state variable (self.x) along
            with the optional arguments in hx_args, and returns the measurement
            that would correspond to that state.
        args : tuple, optional, default (,)
            arguments to be passed into HJacobian after the required state
            variable.
        hx_args : tuple, optional, default (,)
            arguments to be passed into Hx after the required state
            variable.
        u : np.array or scalar
            optional control vector input to the filter.
        """
        #pylint: disable=too-many-locals

        if not isinstance(args, tuple):
            args = (args,)

        if not isinstance(hx_args, tuple):
            hx_args = (hx_args,)

        if u is not None:
            self.input = u

        # predict step
        self.predict(self.input)

        # update step
        self.update(z, Hx, args, hx_args, self.input)

    ##############################################################################

    @typecheck((np.ndarray, ))
    def predict(self, input=None):
        """
        Predict next state (prior) using the Extended Kalman filter state propagation
        equations.
        :param input: np.ndarray - input to directly influence the state, default(None)
        """

        if input is not None:
            self.input = input

        # x = Fx + Gu
        if self.input_function.shape != EmptyArray.shape and self.input.shape != EmptyArray.shape:
            self.state = np.dot(self.transition_function, self.state) + np.dot(self.input_function, self.input)
        else:
            self.state = np.dot(self.transition_function, self.state)

        # P = FPF' + Q
        self.covariance = np.dot(np.dot(self.transition_function, self.covariance),
                                 self.transition_function.T) + self.process_noise_function

        # save prior
        self.state_prior      = np.copy(self.state)
        self.covariance_prior = np.copy(self.covariance)

    ##############################################################################

    @typecheck((np.ndarray, ))
    def extrapolate(self, input=None):
        """
        Predict next state (prior) using the Kalman filter state propagation
        equations.
        :param input: np.ndarray - input to directly influence the state, default(None)
        """
        # x = Fx + Gu
        if self.input_function.shape != EmptyArray.shape and input is not None:
            self.state_extrapolated = np.dot(self.transition_function, self.state) + np.dot(self.input_function, input)
        else:
            self.state_extrapolated = np.dot(self.transition_function, self.state)

        # P = FPF' + Q
        self.covariance_extrapolated = np.dot(np.dot(self.transition_function, self.covariance), self.transition_function.T) + self.process_noise_function

    ##############################################################################

    @typecheck(np.ndarray, types.FunctionType, (tuple, ), (tuple, ), (np.ndarray, ))
    def update(self, z, Hx, args=(), hx_args=(), u=None):
        """ Performs the predict/update innovation of the extended Kalman
        filter.
        Parameters
        ----------
        z : np.array
            measurement for this step.
            If `None`, only predict step is perfomed.
        Hx : function
            function which takes as input the state variable (self.x) along
            with the optional arguments in hx_args, and returns the measurement
            that would correspond to that state.
        args : tuple, optional, default (,)
            arguments to be passed into HJacobian after the required state
            variable.
        hx_args : tuple, optional, default (,)
            arguments to be passed into Hx after the required state
            variable.
        u : np.array or scalar
            optional control vector input to the filter.
        """
        # pylint: disable=too-many-locals

        if not isinstance(hx_args, tuple):
            hx_args = (hx_args,)

        if u is not None:
            self.input = u

        H = self.jacobi_matrix

        # update step
        PH_transpose = np.dot(self.covariance, H.T)
        S            = np.dot(H, PH_transpose) + self.state_uncertainty
        S_inv        = np.linalg.inv(S)
        K            = np.dot(PH_transpose, S_inv)

        # Save system uncertainty into member variables
        self.system_uncertainty = S
        self.inverse_system_uncertainty = S_inv

        y          = z - Hx(self.state, *hx_args)
        self.state = self.state + np.dot(K, y)
        self.error = np.round(z, 5) - np.round(Hx(self.state, *hx_args).astype(float), 5)

        I = np.eye(np.size(self.state))

        I_KH            = I - np.dot(K, H)
        self.covariance = np.dot(I_KH, self.covariance).dot(I_KH.T) + np.dot(K, self.state_uncertainty).dot(K.T)

        # save measurement and posterior state
        self.measurement     = np.copy(z)
        self.state_post      = np.copy(self.state)
        self.covariance_post = np.copy(self.covariance)
        self.previous_state  = np.copy(self.state)

        # set to None to force recompute
        self._log_likelihood = None
        self._likelihood = None

    ##############################################################################

    @property
    def log_likelihood(self):
        """
        Computes the log_likelyhood of the last measurement based on the system uncertainty, S.
        :return: float - log likelihood of last measurement of this filter
        """

        if self._log_likelihood is None:
            flat_y = np.asarray(self.error).flatten()
            # Set mean to None, this is treated as having the zero vector being the mean.
            flat_mean = None
            self._log_likelihood = multivariate_normal.logpdf(flat_y, flat_mean, cov=self.system_uncertainty,
                                                              allow_singular=True)
        return self._log_likelihood

    ##############################################################################

    @property
    def likelihood(self):
        """
        Computed from the log-likelihood. The log-likelihood can be very
        small,  meaning a large negative value such as -28000. Taking the
        exp() of that results in 0.0, which can break typical algorithms
        which multiply by this value, so by default we always return a
        number >= sys.float_info.min.
        :return: float - likelihood of last measurement of this filter
        """
        if self._likelihood is None:
            self._likelihood = exp(self.log_likelihood)
            if self._likelihood == 0:
                self._likelihood = sys.float_info.min
        return self._likelihood

    # EOC

##############################################################################

class UnscentedKalmanFilter():

    @typecheck(np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self, initial_state, inital_input, initial_measurement,
                 state_transition_matrix, control_input_matrix, process_noise_matrix,
                 initial_covariance_covariance_matrix, inital_measurement_function, inital_state_uncertainty):
        pass

    ##############################################################################

    def predict(self):
        pass

    ##############################################################################

    def update(self):
        pass

    # EOC

#EOF