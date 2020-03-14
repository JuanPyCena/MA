import numpy as np

from src.Decorators import *

class KalmanFilter():

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

    @typecheck()
    def predict(self):
        """
        Predict next state (prior) using the Kalman filter state propagation
        equations.
        """
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

class ExtendedKalmanFilter():

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