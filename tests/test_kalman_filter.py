import unittest
import numpy as np
from src.filterlib.Filters import KalmanFilter

from filterpy.kalman import KalmanFilter as KalmanFiler_filterpy

class TestKalmanFilter(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestKalmanFilter, self).__init__(*args, **kwargs)

        self.state = np.array([1, 2])  # x
        self.input = np.array([1, 0])  # u
        self.measurement = np.array([1, 0])  # z
        self.transition = np.array([[1, 0], [0, 1]])  # F
        self.input_control_matrix = np.array([[1, 0], [0, 1]])  # G
        self.noise = np.array([[1, 0], [0, 1]])  # Q
        self.covariance_matrix = np.array([[1, 0], [0, 1]])  # P
        self.measurement_matrix = np.array([[1, 0], [0, 1]])  # H
        self.measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

    ##############################################################################

    def test_constructor_error_no_arguments(self):
        with self.assertRaises(TypeError) as context:
            KalmanFilter()

        self.assertTrue("missing 9 required positional arguments: 'initial_state', "
                        "'inital_input', 'initial_measurement', 'state_transition_matrix', "
                        "'control_input_matrix', 'process_noise_matrix', 'initial_covariance_covariance_matrix', "
                        "'inital_measurement_function', and 'inital_state_uncertainty'" in str(context.exception))

    ##############################################################################

    def test_constructor_no_error(self):
        try:
            kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty)
        except TypeError:
            self.assertTrue(False)

    ##############################################################################

    def test_constructor_wrong_type(self):
        with self.assertRaises(AssertionError) as context:
            KalmanFilter(int(5), self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty)

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))


    ##############################################################################

    def test_predict_no_arguments(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        state_before = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        kf.predict()

        state_after = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_input_arguments(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        state_before      = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        kf.predict(input=np.array([2,2]))

        state_after      = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_argument_errors(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        with self.assertRaises(AssertionError) as context:
            kf.predict(input=int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            kf.predict(np.array([2,2]), int(5))

        self.assertTrue("predict() takes from 1 to 2 positional arguments but 3 were given" in str(context.exception))

    ##############################################################################

    def test_update_measurement_arguments(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        state_before      = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        kf.update(z=np.array([2,2]))

        state_after      = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_update_argument_errors(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        with self.assertRaises(TypeError) as context:
            kf.update()

        self.assertTrue("update() missing 1 required positional argument: 'z'" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            kf.update(z=int(5))

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            kf.update(np.array([2,2]), int(5))

        self.assertTrue("update() takes 2 positional arguments but 3 were given" in str(context.exception))

    ##############################################################################

    def test_compare_predict_with_filterpy(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix,
                          self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        kf_filterpy = KalmanFiler_filterpy(2, 2, 2)
        kf_filterpy.x = self.state
        kf_filterpy.P = self.covariance_matrix
        kf_filterpy.F = self.transition
        kf_filterpy.R = self.measurement_uncertainty
        kf_filterpy.B = self.input_control_matrix
        kf_filterpy.H = self.measurement_matrix
        kf_filterpy.Q = self.noise

        # Predict using implemented KF
        kf.predict(input=np.array([2, 2]))
        # Predict using KF from filterpy
        kf_filterpy.predict(u=np.array([2, 2]), B=self.input_control_matrix, F=self.transition, Q=self.noise)

        state_kf               = kf.state.copy()
        state_kf_filterpy      = kf_filterpy.x.copy()
        covariance_kf          = kf.covariance.copy()
        covariance_kf_filterpy = kf_filterpy.P.copy()

        # Prediction of both filters must be the same
        self.assertTrue(np.array_equal(state_kf, state_kf_filterpy))
        self.assertTrue(np.array_equal(covariance_kf, covariance_kf_filterpy))

    ##############################################################################

    def test_compare_update_with_filterpy(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix,
                          self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        kf_filterpy = KalmanFiler_filterpy(2, 2, 2)
        kf_filterpy.x = self.state
        kf_filterpy.P = self.covariance_matrix
        kf_filterpy.F = self.transition
        kf_filterpy.R = self.measurement_uncertainty
        kf_filterpy.B = self.input_control_matrix
        kf_filterpy.H = self.measurement_matrix
        kf_filterpy.Q = self.noise

        # Update using implemented KF
        kf.update(z=np.array([2, 2]))
        # Update using KF from filterpy
        kf_filterpy.update(z=np.array([2, 2]), R=self.measurement_uncertainty, H=self.measurement_matrix)

        state_kf               = kf.state.copy()
        state_kf_filterpy      = kf_filterpy.x.copy()
        covariance_kf          = kf.covariance.copy()
        covariance_kf_filterpy = kf_filterpy.P.copy()

        # Update of both filters must be the same
        self.assertTrue(np.array_equal(state_kf, state_kf_filterpy))
        self.assertTrue(np.array_equal(covariance_kf, covariance_kf_filterpy))

    ##############################################################################

    def test_compare_predict_update_with_filterpy(self):
        kf = KalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix,
                          self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        kf_filterpy = KalmanFiler_filterpy(2, 2, 2)
        kf_filterpy.x = self.state
        kf_filterpy.P = self.covariance_matrix
        kf_filterpy.F = self.transition
        kf_filterpy.R = self.measurement_uncertainty
        kf_filterpy.B = self.input_control_matrix
        kf_filterpy.H = self.measurement_matrix
        kf_filterpy.Q = self.noise

        # Predict using implemented KF
        kf.predict(input=np.array([2, 1]))
        # Update using implemented KF
        kf.update(z=np.array([1, 2]))

        # Predict using KF from filterpy
        kf_filterpy.predict(u=np.array([2, 1]), B=self.input_control_matrix, F=self.transition, Q=self.noise)
        # Update using KF from filterpy
        kf_filterpy.update(z=np.array([1, 2]), R=self.measurement_uncertainty, H=self.measurement_matrix)

        state_kf               = kf.state.copy()
        state_kf_filterpy      = kf_filterpy.x.copy()
        covariance_kf          = kf.covariance.copy()
        covariance_kf_filterpy = kf_filterpy.P.copy()

        # Prediction update of both filters must be the same
        self.assertTrue(np.array_equal(state_kf, state_kf_filterpy))
        self.assertTrue(np.array_equal(covariance_kf, covariance_kf_filterpy))

if __name__ == '__main__':
    unittest.main()

#EOF
