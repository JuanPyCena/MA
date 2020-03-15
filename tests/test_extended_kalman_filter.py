import unittest
import numpy as np
from math import sqrt

from src.Filters import ExtendedKalmanFilter

class TestExtendedKalmanFilter(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestExtendedKalmanFilter, self).__init__(*args, **kwargs)

        self.state                   = np.array([1, 2])  # x
        self.input                   = np.array([1, 0])  # u
        self.measurement             = np.array([1, 0])  # z
        self.transition              = np.array([[2, 0], [0, 1]])  # F
        self.input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        self.noise                   = np.array([[1, 0], [0, 1]])  # Q
        self.covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        self.measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        self.measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

    def H_of(self, x):
        """ compute Jacobian of H matrix for state x """

        horiz_dist = x[0]
        altitude = x[1]

        denom = sqrt(horiz_dist ** 2 + altitude ** 2)
        return np.array([[horiz_dist / denom, 0], [0, altitude / denom]])

    def hx(self, x):
        """ takes a state variable and returns the measurement that would
        correspond to that state.
        """
        return sqrt(x[0] ** 2 + x[1] ** 2)

    def test_constructor_error_no_arguments(self):
        with self.assertRaises(TypeError) as context:
            ExtendedKalmanFilter()

        self.assertTrue("missing 9 required positional arguments: 'initial_state', "
                        "'inital_input', 'initial_measurement', 'state_transition_matrix', "
                        "'control_input_matrix', 'process_noise_matrix', 'initial_covariance_covariance_matrix', "
                        "'inital_measurement_function', and 'inital_state_uncertainty'" in str(context.exception))

    ##############################################################################

    def test_constructor_no_error(self):
        try:
            ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty)
        except TypeError:
            self.assertTrue(False)

    ##############################################################################

    def test_constructor_wrong_type(self):
        with self.assertRaises(AssertionError) as context:
            ExtendedKalmanFilter(int(5), self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty)

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))


    ##############################################################################

    def test_predict_no_arguments(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        state_before      = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.predict()

        state_after      = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_input_arguments(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        state_before      = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.predict(input=np.array([2,2]))

        state_after      = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_argument_errors(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        with self.assertRaises(AssertionError) as context:
            ekf.predict(input=int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            ekf.predict(np.array([2,2]), int(5))

        self.assertTrue("predict() takes from 1 to 2 positional arguments but 3 were given" in str(context.exception))

    ##############################################################################

    def test_update_measurement_arguments(self):
        ekf = ExtendedKalmanFilter(np.copy(self.state), self.input, self.measurement, self.transition,
                                   self.input_control_matrix, self.noise, np.copy(self.covariance_matrix),
                                   self.measurement_matrix, self.measurement_uncertainty)

        state_before      = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.update(z=np.array([2,2]), HJacobian=self.H_of, Hx=self.hx)

        state_after      = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_update_argument_errors(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        with self.assertRaises(TypeError) as context:
            ekf.update()

        self.assertTrue("update() missing 3 required positional arguments: 'z', 'HJacobian', and 'Hx'" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(z=int(5))

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), int(5))

        self.assertTrue("arg 5 does not match <class 'function'>" in str(context.exception))

        def fun():
            pass

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun, int(5))

        self.assertTrue("arg 5 does not match <class 'function'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun, fun, int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun, fun, tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun, fun, tuple(), tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            ekf.update(np.array([2,2]), fun, fun, tuple(), tuple(), np.array([2,2]), int(5))

        self.assertTrue("update() takes from 4 to 7 positional arguments but 8 were given" in str(context.exception))

    ##############################################################################

    def test_predict_update_measurement_arguments(self):
        ekf = ExtendedKalmanFilter(np.copy(self.state), self.input, self.measurement, self.transition,
                                   self.input_control_matrix, self.noise, np.copy(self.covariance_matrix),
                                   self.measurement_matrix, self.measurement_uncertainty)

        state_before = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.predict_update(z=np.array([2, 2]), HJacobian=self.H_of, Hx=self.hx)

        state_after = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_update_argument_errors(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty)

        with self.assertRaises(TypeError) as context:
            ekf.predict_update()

        self.assertTrue("predict_update() missing 3 required positional arguments: 'z', 'HJacobian', and 'Hx'" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(z=int(5))

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), int(5))

        self.assertTrue("arg 5 does not match <class 'function'>" in str(context.exception))

        def fun():
            pass

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, int(5))

        self.assertTrue("arg 5 does not match <class 'function'>" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, fun, int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, fun, tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, fun, tuple(), tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            ekf.predict_update(np.array([2,2]), fun, fun, tuple(), tuple(), np.array([2,2]), int(5))

        self.assertTrue("predict_update() takes from 4 to 7 positional arguments but 8 were given" in str(context.exception))

    ##############################################################################

    def test_simple_system(self):
        "TODO: implement a simple system and calculate 1 timestep. Verify all matrices afterwards"
        pass

if __name__ == '__main__':
    unittest.main()

#EOF
