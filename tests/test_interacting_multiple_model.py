import unittest
import numpy as np
from math import sqrt

from src.filterlib.InteractingMultipleModel import InteractingMultipleModel
from src.filterlib.Filters import ExtendedKalmanFilter
from src.filterlib.Filters import KalmanFilter

from filterpy.kalman import IMMEstimator as InteractingMultipleModel_filterpy
from filterpy.kalman import KalmanFilter as KalmanFilter_filterpy

class TestInteractingMultipleModel(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestInteractingMultipleModel, self).__init__(*args, **kwargs)

        state_kf                   = np.array([1, 0, 0, 0])  # x
        input_kf                   = np.array([1, 1, 0, 0])  # u
        measurement_kf             = np.array([1, 1, 0, 0])  # z
        transition_kf              = np.array([[2, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # F
        input_control_matrix_kf    = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]])  # G
        noise_kf                   = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]])  # Q
        covariance_matrix_kf       = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # P
        measurement_matrix_kf      = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # H
        measurement_uncertainty_kf = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]])  # R

        state_ekf                   = np.array([1, 2, 1, 0])  # x
        input_ekf                   = np.array([1, 0, 0, 1])  # u
        measurement_ekf             = np.array([1, 1, 0, 1])  # z
        transition_ekf              = np.array([[2, 0, 1, 0], [0, 1, 0, 1], [1, 0, 1, 0], [0, 1, 0, 1]])  # F
        input_control_matrix_ekf    = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # G
        noise_ekf                   = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # Q
        covariance_matrix_ekf       = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # P
        measurement_matrix_ekf      = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # H
        measurement_uncertainty_ekf = np.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])  # R

        self.initial_mode_probability = np.array([0.5, 0.5])
        self.markov_transition_matrix = np.array([[0.97, 0.03], [0.03, 0.97]])

        kf = KalmanFilter(state_kf, input_kf, measurement_kf, transition_kf, input_control_matrix_kf, noise_kf,
                          covariance_matrix_kf, measurement_matrix_kf, measurement_uncertainty_kf)

        kf1 = KalmanFilter(state_ekf, input_ekf, measurement_ekf, transition_ekf, input_control_matrix_ekf, noise_ekf,
                                  covariance_matrix_ekf, measurement_matrix_ekf, measurement_uncertainty_ekf)

        ekf = ExtendedKalmanFilter(state_ekf, input_ekf, measurement_ekf, transition_ekf,
                                   input_control_matrix_ekf, noise_ekf, covariance_matrix_ekf,
                                   measurement_matrix_ekf, measurement_uncertainty_ekf, transition_ekf)

        self.filter = [kf, ekf]

        self.measuremnet = np.array([1,2,0,1])

        # Used for comparing with IMM from filterpy
        kf_filterpy1 = KalmanFilter_filterpy(4, 4, 4)
        kf_filterpy1.x = state_kf
        kf_filterpy1.P = covariance_matrix_kf
        kf_filterpy1.F = transition_kf
        kf_filterpy1.R = measurement_uncertainty_kf
        kf_filterpy1.B = input_control_matrix_kf
        kf_filterpy1.H = measurement_matrix_kf
        kf_filterpy1.Q = noise_kf

        kf_filterpy2 = KalmanFilter_filterpy(4, 4, 4)
        kf_filterpy2.x = state_ekf
        kf_filterpy2.P = covariance_matrix_ekf
        kf_filterpy2.F = transition_ekf
        kf_filterpy2.R = measurement_uncertainty_ekf
        kf_filterpy2.B = input_control_matrix_ekf
        kf_filterpy2.H = measurement_matrix_ekf
        kf_filterpy2.Q = noise_ekf

        self.filterpy_filters = [kf_filterpy1, kf_filterpy2]
        self.kf_filters       = [kf, kf1]

    ##############################################################################

    def hx(self, x):
        """ takes a state variable and returns the measurement that would
        correspond to that state.
        """
        return sqrt(x[0] ** 2 + x[1] ** 2 + x[2] ** 2 + x[3] ** 2)

    ##############################################################################

    def test_constructor_error_no_arguments(self):
        with self.assertRaises(TypeError) as context:
            InteractingMultipleModel()

        self.assertTrue("missing 3 required positional arguments: 'filters', 'initial_mode_probabilities', "
                        "and 'markov_transition_matrix'" in str(context.exception))

    ##############################################################################

    def test_constructor_no_error(self):
        try:
            imm = InteractingMultipleModel(self.filter, self.initial_mode_probability, self.markov_transition_matrix)
        except TypeError:
            self.assertTrue(False)

    ##############################################################################

    def test_constructor_wrong_type(self):
        with self.assertRaises(AssertionError) as context:
            InteractingMultipleModel(int(5), self.initial_mode_probability, self.markov_transition_matrix)
        self.assertTrue("arg 5 does not match <class 'list'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            InteractingMultipleModel(self.filter, int(5), self.markov_transition_matrix)
        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            InteractingMultipleModel(self.filter, self.initial_mode_probability, int(5))
        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))


    ##############################################################################

    def test_predict_update_wrong_arguments(self):
        imm = InteractingMultipleModel(self.filter, self.initial_mode_probability, self.markov_transition_matrix)

        with self.assertRaises(TypeError) as context:
            imm.predict_update()
        self.assertTrue("predict_update() missing 1 required positional argument: 'measurement'" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            imm.predict_update(int(5))
        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            imm.predict_update(self.measuremnet, input=int(5))
        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

    ##############################################################################

    def test_predict_input_arguments(self):
        imm = InteractingMultipleModel(self.filter, self.initial_mode_probability, self.markov_transition_matrix)

        state_before      = imm.state.copy()
        covariance_before = imm.covariance.copy()

        kwds = [{}, {"Hx": self.hx}]

        imm.predict_update(self.measuremnet, input=np.array([2, 2, 0, 0]), update_kwds=kwds)

        state_after      = imm.state.copy()
        covariance_after = imm.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_compare_to_filterpy(self):
        # We can only test the KF since the filterpy IMM only works with KF filters
        imm = InteractingMultipleModel(self.kf_filters, self.initial_mode_probability, self.markov_transition_matrix)
        imm_filterpy = InteractingMultipleModel_filterpy(self.filterpy_filters, self.initial_mode_probability, self.markov_transition_matrix)

        # Predict update using implemented IMM
        imm.predict_update(self.measuremnet, input=np.array([2, 2, 0, 0]))

        # Predict update using filterpy update
        imm_filterpy.predict(u=np.array([2, 2, 0, 0]))
        imm_filterpy.update(z=self.measuremnet)

        state_imm               = imm.state.copy()
        state_imm_filterpy      = imm_filterpy.x.copy()
        covariance_imm          = imm.covariance.copy()
        covariance_imm_filterpy = imm_filterpy.P.copy()

        # The states and covariances must be within a certain range of each other.
        # Since the likelihood can be calculated differently in the filterpy IMM,
        # it is highly unlikely that both values are equal.
        # As long as we are within a reasonable range we are fine
        state_difference             = np.abs(np.subtract(state_imm, state_imm_filterpy))
        covariance_difference         = np.abs(np.subtract(covariance_imm, covariance_imm_filterpy))
        maximum_state_difference      = np.amax(state_difference)
        maximum_covariance_difference = np.amax(covariance_difference)

        self.assertTrue(maximum_state_difference == 0)
        self.assertTrue(maximum_covariance_difference == 0)

if __name__ == '__main__':
    unittest.main()

#EOF
