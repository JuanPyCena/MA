import unittest
import numpy as np
from src.Filters import KalmanFilter

class TestKalmanFilter(unittest.TestCase):

    def test_constructor_error_no_arguments(self):
        with self.assertRaises(TypeError) as context:
            KalmanFilter()

        self.assertTrue("missing 9 required positional arguments: 'initial_state', "
                        "'inital_input', 'initial_measurement', 'state_transition_matrix', "
                        "'control_input_matrix', 'process_noise_matrix', 'initial_covariance_covariance_matrix', "
                        "'inital_measurement_function', and 'inital_state_uncertainty'" in str(context.exception))

    ##############################################################################

    def test_constructor_no_error(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        try:
            kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                              measurement_matrix, measurement_uncertainty)
        except TypeError:
            self.assertTrue(False)

    ##############################################################################

    def test_constructor_wrong_type(self):
        state                   = int(5)  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        with self.assertRaises(AssertionError) as context:
            KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                              measurement_matrix, measurement_uncertainty)

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))


    ##############################################################################

    def test_predict_no_arguments(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                          measurement_matrix, measurement_uncertainty)

        state_before = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, state))
        self.assertTrue(np.array_equal(covariance_before, covariance_matrix))

        kf.predict()

        state_after = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_input_arguments(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                          measurement_matrix, measurement_uncertainty)

        state_before      = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, state))
        self.assertTrue(np.array_equal(covariance_before, covariance_matrix))

        kf.predict(input=np.array([2,2]))

        state_after      = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_argument_errors(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                          measurement_matrix, measurement_uncertainty)

        with self.assertRaises(AssertionError) as context:
            kf.predict(input=int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            kf.predict(np.array([2,2]), int(5))

        self.assertTrue("predict() takes from 1 to 2 positional arguments but 3 were given" in str(context.exception))

    ##############################################################################

    def test_update_measurement_arguments(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                          measurement_matrix, measurement_uncertainty)

        state_before      = kf.state.copy()
        covariance_before = kf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, state))
        self.assertTrue(np.array_equal(covariance_before, covariance_matrix))

        kf.update(z=np.array([2,2]))

        state_after      = kf.state.copy()
        covariance_after = kf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_update_argument_errors(self):
        state                   = np.array([1, 2])  # x
        input                   = np.array([1, 0])  # u
        measurement             = np.array([1, 0])  # z
        transition              = np.array([[1, 0], [0, 1]])  # F
        input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        noise                   = np.array([[1, 0], [0, 1]])  # Q
        covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix,
                          measurement_matrix, measurement_uncertainty)

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