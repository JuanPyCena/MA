import unittest
import numpy as np
from math import sqrt

from src.filterlib.Filters import ExtendedKalmanFilter

from filterpy.kalman import ExtendedKalmanFilter as ExtendedKalmanFilter_filterpy

class TestExtendedKalmanFilter(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestExtendedKalmanFilter, self).__init__(*args, **kwargs)

        self.state                   = np.array([1, 2])  # x
        self.input                   = np.array([1, 0])  # u
        self.measurement             = np.array([1, 0])  # z
        self.transition              = np.array([[2, 0], [0, 1]])  # F
        self.jacobi                  = np.array([[2, 0], [0, 1]])  # F
        self.input_control_matrix    = np.array([[1, 0], [0, 1]])  # G
        self.noise                   = np.array([[1, 0], [0, 1]])  # Q
        self.covariance_matrix       = np.array([[1, 0], [0, 1]])  # P
        self.measurement_matrix      = np.array([[1, 0], [0, 1]])  # H
        self.measurement_uncertainty = np.array([[1, 0], [0, 1]])  # R

        self.state = self.state.T # x
        self.input = self.input.T  # u
        self.measurement = self.measurement.T  # z

    def H_of(self, x):
        return self.transition

    def hx(self, x):
        return x

    def test_constructor_error_no_arguments(self):
        with self.assertRaises(TypeError) as context:
            ExtendedKalmanFilter()

        self.assertTrue("missing 10 required positional arguments: 'initial_state', "
                        "'inital_input', 'initial_measurement', 'state_transition_matrix', "
                        "'control_input_matrix', 'process_noise_matrix', 'initial_covariance_covariance_matrix', "
                        "'inital_measurement_function', 'inital_state_uncertainty', and 'jacobi_matrix'" in str(context.exception))

    ##############################################################################

    def test_constructor_no_error(self):
        try:
            ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty, self.jacobi)
        except TypeError:
            self.assertTrue(False)

    ##############################################################################

    def test_constructor_wrong_type(self):
        with self.assertRaises(AssertionError) as context:
            ExtendedKalmanFilter(int(5), self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                              self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        self.assertTrue("arg 5 does not match <class 'numpy.ndarray'>" in str(context.exception))


    ##############################################################################

    def test_predict_no_arguments(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

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
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

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
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

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
                                   self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        state_before      = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.update(z=np.array([2,2]), Hx=self.hx)

        state_after      = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_update_argument_errors(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        with self.assertRaises(TypeError) as context:
            ekf.update()

        self.assertTrue("update() missing 2 required positional arguments: 'z' and 'Hx'" in str(context.exception))

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

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun,  tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.update(np.array([2,2]), fun,  tuple(), tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            ekf.update(np.array([2,2]), fun, tuple(), tuple(), np.array([2,2]), int(5))

        self.assertTrue("update() takes from 3 to 6 positional arguments but 7 were given" in str(context.exception))

    ##############################################################################

    def test_predict_update_measurement_arguments(self):
        ekf = ExtendedKalmanFilter(np.copy(self.state), self.input, self.measurement, self.transition,
                                   self.input_control_matrix, self.noise, np.copy(self.covariance_matrix),
                                   self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        state_before = ekf.state.copy()
        covariance_before = ekf.covariance.copy()

        self.assertTrue(np.array_equal(state_before, self.state))
        self.assertTrue(np.array_equal(covariance_before, self.covariance_matrix))

        ekf.predict_update(z=np.array([2, 2]), Hx=self.hx)

        state_after = ekf.state.copy()
        covariance_after = ekf.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_predict_update_argument_errors(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        with self.assertRaises(TypeError) as context:
            ekf.predict_update()

        self.assertTrue("predict_update() missing 2 required positional arguments: 'z' and 'Hx'" in str(context.exception))

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

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'tuple'>,)" in str(context.exception))

        with self.assertRaises(AssertionError) as context:
            ekf.predict_update(np.array([2,2]), fun, tuple(), tuple(), int(5))

        self.assertTrue("arg 5 does not match (<class 'numpy.ndarray'>,)" in str(context.exception))

        with self.assertRaises(TypeError) as context:
            ekf.predict_update(np.array([2,2]), fun, tuple(), tuple(), np.array([2,2]), int(5))

        self.assertTrue("predict_update() takes from 3 to 6 positional arguments but 7 were given" in str(context.exception))

    ##############################################################################

    def test_compare_predict_with_filterpy(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        ekf_filterpy = ExtendedKalmanFilter_filterpy(2, 2, 2)
        ekf_filterpy.x = self.state
        ekf_filterpy.P = self.covariance_matrix
        ekf_filterpy.F = self.transition
        ekf_filterpy.R = self.measurement_uncertainty
        ekf_filterpy.B = self.input_control_matrix
        ekf_filterpy.H = self.measurement_matrix
        ekf_filterpy.Q = self.noise

        # Predict using implemented EKF
        ekf.predict(input=np.array([2,2]))
        # Predict using EKF from filterpy
        ekf_filterpy.predict(u=np.array([2,2]))

        state_ekf               = ekf.state.copy()
        covariance_ekf          = ekf.covariance.copy()
        state_ekf_filterpy      = ekf_filterpy.x.copy()
        covariance_ekf_filterpy = ekf_filterpy.P.copy()

        # Prediction of both filters must be the same
        self.assertTrue(np.array_equal(state_ekf, state_ekf_filterpy))
        self.assertTrue(np.array_equal(covariance_ekf, covariance_ekf_filterpy))

    ##############################################################################

    def test_compare_update_with_filterpy(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        ekf_filterpy = ExtendedKalmanFilter_filterpy(2, 2, 2)
        ekf_filterpy.x = self.state
        ekf_filterpy.P = self.covariance_matrix
        ekf_filterpy.F = self.transition
        ekf_filterpy.R = self.measurement_uncertainty
        ekf_filterpy.B = self.input_control_matrix
        ekf_filterpy.H = self.measurement_matrix
        ekf_filterpy.Q = self.noise

        # Update using implemented EKF
        ekf.update(z=np.array([1,2]), Hx=self.hx)
        # Predict using EKF from filterpy
        ekf_filterpy.update(z=np.array([1,2]), HJacobian=self.H_of, Hx=self.hx, R=self.covariance_matrix)

        state_ekf               = ekf.state.copy()
        covariance_ekf          = ekf.covariance.copy()
        state_ekf_filterpy      = ekf_filterpy.x.copy()
        covariance_ekf_filterpy = ekf_filterpy.P.copy()

        # Update of both filters must be the same
        self.assertTrue(np.array_equal(state_ekf, state_ekf_filterpy))
        self.assertTrue(np.array_equal(covariance_ekf, covariance_ekf_filterpy))

    ##############################################################################

    def test_compare_predict_update_with_filterpy(self):
        ekf = ExtendedKalmanFilter(self.state, self.input, self.measurement, self.transition, self.input_control_matrix, self.noise, self.covariance_matrix,
                          self.measurement_matrix, self.measurement_uncertainty, self.jacobi)

        ekf_filterpy = ExtendedKalmanFilter_filterpy(2, 2, 2)
        ekf_filterpy.x = self.state
        ekf_filterpy.P = self.covariance_matrix
        ekf_filterpy.F = self.transition
        ekf_filterpy.R = self.measurement_uncertainty
        ekf_filterpy.B = self.input_control_matrix
        ekf_filterpy.H = self.measurement_matrix
        ekf_filterpy.Q = self.noise

        # Predict Update using implemented EKF
        ekf.predict_update(z=np.array([2, 1]), Hx=self.hx, args=(), hx_args=(), u=np.array([1, 2]))

        # Predict and updates using EKF from filterpy. Do not use predict_update as this leads to a different
        # result then using predict and update in series
        ekf_filterpy.predict(u=np.array([1, 2]))
        ekf_filterpy.update(z=np.array([2, 1]), HJacobian=self.H_of, Hx=self.hx, R=self.covariance_matrix)

        state_ekf               = ekf.state.copy()
        covariance_ekf          = ekf.covariance.copy()
        state_ekf_filterpy      = ekf_filterpy.x.copy()
        covariance_ekf_filterpy = ekf_filterpy.P.copy()

        # Update of both filters must be the same
        self.assertTrue(np.array_equal(state_ekf, state_ekf_filterpy))
        self.assertTrue(np.array_equal(covariance_ekf, covariance_ekf_filterpy))

if __name__ == '__main__':
    unittest.main()

#EOF
