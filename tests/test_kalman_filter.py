import unittest
import numpy as np
from src.Filters import KalmanFilter

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

    def test_simple_system(self):
        "TODO: implement a simple system and calculate 1 timestep. Verify all matrices afterwards"
        pass

if __name__ == '__main__':
    unittest.main()

#EOF
