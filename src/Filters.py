import numpy as np
import types

from src.Decorators import *

class KalmanFilter(object):

    @typecheck(np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self, initial_state, inital_input, initial_measurement,
                 state_transition_matrix, control_input_matrix, process_noise_matrix,
                 initial_covariance_covariance_matrix, inital_measurement_function, inital_state_uncertainty):
        self.state                  = initial_state #x
        self.input                  = inital_input #u
        self.measurement            = initial_measurement #z
        self.transition_function    = state_transition_matrix #F
        self.input_function         = control_input_matrix #G
        self.process_noise_function = process_noise_matrix #Q
        self.covariance             = initial_covariance_covariance_matrix #P
        self.measurement_function   = inital_measurement_function #H
        self.state_uncertainty      = inital_state_uncertainty #R

        # Set values prior to prediction
        # these will always be a copy of state,covariance after predict() is called
        self.state_prior         = self.state.copy()
        self.covariance_prior    = self.covariance.copy()

        # Set values post to prediction
        # these will always be a copy of state,covariance after update() is called
        self.state_post         = self.state.copy()
        self.covariance_post    = self.covariance.copy()

    ##############################################################################

    @typecheck((np.ndarray,None, ))
    def predict(self, input=None):
        """
        Predict next state (prior) using the Kalman filter state propagation
        equations.
        """
        if input != None:
            self.input = input

        # x = Fx + Gu
        if self.input_function.shape != np.array([]).shape and self.input.shape != np.array([]).shape:
            self.state = np.dot(self.transition_function, self.state) + np.dot(self.input_function, self.input)
        else:
            self.state = np.dot(self.transition_function, self.state)

        # P = FPF' + Q
        self.covariance = np.dot(np.dot(self.transition_function, self.covariance), self.transition_function.T) + self.process_noise_function

        # save prior
        self.state_prior      = self.state.copy()
        self.covariance_prior = self.covariance.copy()

    ##############################################################################

    @typecheck(np.ndarray)
    def update(self, z):
        # y = z - Hx
        # error (residual) between measurement and prediction
        y = z - np.dot(self.measurement_function, self.state)

        # common subexpression for speed
        PH_transpose = np.dot(self.covariance, self.measurement_function.T)

        # S = HPH' + R
        # project system uncertainty into measurement space
        S = np.dot(self.measurement_function, PH_transpose) + self.state_uncertainty
        S_inv = np.linalg.inv(S)

        # K = PH'inv(S)
        # map system uncertainty into kalman gain
        K = np.dot(PH_transpose, S_inv)

        # x = x + Ky
        # predict new state with residual scaled by the kalman gain
        self.state = self.state + np.dot(K, y)

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

    #EOC

##############################################################################

class ExtendedKalmanFilter(object):

    @typecheck(np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self, initial_state, inital_input, initial_measurement,
                 state_transition_matrix, control_input_matrix, process_noise_matrix,
                 initial_covariance_covariance_matrix, inital_measurement_function, inital_state_uncertainty):
        self.state                  = initial_state  # x
        self.input                  = inital_input  # u
        self.measurement            = initial_measurement  # z
        self.transition_function    = state_transition_matrix  # F
        self.input_function         = control_input_matrix  # G
        self.process_noise_function = process_noise_matrix  # Q
        self.covariance             = initial_covariance_covariance_matrix  # P
        self.measurement_function   = inital_measurement_function  # H
        self.state_uncertainty      = inital_state_uncertainty  # R

        # Set values prior to prediction
        # these will always be a copy of state,covariance after predict() is called
        self.state_prior = self.state.copy()
        self.covariance_prior = self.covariance.copy()

        # Set values post to prediction
        # these will always be a copy of state,covariance after update() is called
        self.state_post = self.state.copy()
        self.covariance_post = self.covariance.copy()

    ##############################################################################

    @typecheck(np.ndarray, types.FunctionType, types.FunctionType, (tuple, ), (tuple, ), (int, np.ndarray, ))
    def predict_update(self, z, HJacobian, Hx, args=(), hx_args=(), u=0):
        """ Performs the predict/update innovation of the extended Kalman
        filter.
        Parameters
        ----------
        z : np.array
            measurement for this step.
            If `None`, only predict step is perfomed.
        HJacobian : function
           function which computes the Jacobian of the H matrix (measurement
           function). Takes state variable (self.x) as input, along with the
           optional arguments in args, and returns H.
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

        self.input = u

        H = HJacobian(self.state, *args)

        # predict step
        self.state      = np.dot(self.transition_function, self.state) + np.dot(self.input_function, self.input)
        self.covariance = np.dot(self.transition_function, self.covariance).dot(self.transition_function.T) + self.process_noise_function

        # save prior
        self.state_prior = np.copy(self.state)
        self.covariance  = np.copy(self.covariance)

        # save prior
        self.state_prior      = self.state.copy()
        self.covariance_prior = self.covariance.copy()

        # update step
        PH_transpose = np.dot(self.covariance, H.T)
        S            = np.dot(H, PH_transpose) + self.state_uncertainty
        S_inv        = np.linalg.inv(S)
        K            = np.dot(PH_transpose, S_inv)

        y          = z - Hx(self.state, *hx_args)
        self.state = self.state + np.dot(K, y)

        I = np.eye(np.size(self.state))

        I_KH            = I - np.dot(K, H)
        self.covariance = np.dot(I_KH, self.covariance).dot(I_KH.T) + np.dot(K, self.state_uncertainty).dot(K.T)

        # save measurement and posterior state
        self.measurement = z.copy()
        self.x_post      = self.x.copy()
        self.P_post      = self.P.copy()

    ##############################################################################

    @typecheck((np.ndarray,None, ))
    def predict(self, input=None):
        """
                Predict next state (prior) using the Kalman filter state propagation
                equations.
                """
        # x = Fx + Gu
        self.state = np.dot(self.transition_function, self.state) + np.dot(self.input_function, self.input)

        # P = FPF' + Q
        self.covariance = np.dot(np.dot(self.transition_function, self.covariance),
                                 self.transition_function.T) + self.process_noise_function

        # save prior
        self.state_prior      = np.copy(self.state)
        self.covariance_prior = np.copy(self.covariance.copy)

    ##############################################################################

    @typecheck(np.ndarray, types.FunctionType, types.FunctionType, (tuple, ), (tuple, ), (int, np.ndarray,))
    def update(self, z, HJacobian, Hx, args=(), hx_args=(), u=0):
        """ Performs the predict/update innovation of the extended Kalman
        filter.
        Parameters
        ----------
        z : np.array
            measurement for this step.
            If `None`, only predict step is perfomed.
        HJacobian : function
           function which computes the Jacobian of the H matrix (measurement
           function). Takes state variable (self.x) as input, along with the
           optional arguments in args, and returns H.
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

        if not isinstance(args, tuple):
            args = (args,)

        if not isinstance(hx_args, tuple):
            hx_args = (hx_args,)

        self.input = u

        H = HJacobian(self.state, *args)

        # update step
        PH_transpose = np.dot(self.covariance, H.T)
        S            = np.dot(H, PH_transpose) + self.state_uncertainty
        S_inv        = np.linalg.inv(S)
        K            = np.dot(PH_transpose, S_inv)

        y          = z - Hx(self.state, *hx_args)
        self.state = self.state + np.dot(K, y)

        I = np.eye(np.size(self.state))

        I_KH            = I - np.dot(K, H)
        self.covariance = np.dot(I_KH, self.covariance).dot(I_KH.T) + np.dot(K, self.state_uncertainty).dot(K.T)

        # save measurement and posterior state
        self.measurement     = np.copy(z)
        self.state_post      = np.copy(self.state)
        self.covariance_post = np.copy(self.covariance)

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