import unittest
import numpy as np
from math import sqrt

from src.InteractingMultipleModel import InteractingMultipleModel
from src.Filters import ExtendedKalmanFilter
from src.Filters import KalmanFilter

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
        self.markov_transition_matrix = np.array([[1, 1,], [1, 0]])

        kf = KalmanFilter(state_kf, input_kf, measurement_kf, transition_kf, input_control_matrix_kf, noise_kf,
                          covariance_matrix_kf, measurement_matrix_kf, measurement_uncertainty_kf)

        ekf = ExtendedKalmanFilter(state_ekf, input_ekf, measurement_ekf, transition_ekf,
                                   input_control_matrix_ekf, noise_ekf, covariance_matrix_ekf,
                                   measurement_matrix_ekf, measurement_uncertainty_ekf)

        self.filter = [kf, ekf]

        self.measuremnet = np.array([1,2,0,1])

    ##############################################################################

    def H_of(self, x):
        """ compute Jacobian of H matrix for state x """

        denom = sqrt(x[0] ** 2 + x[1] ** 2 + x[2] ** 2 + x[3] ** 2)
        return np.array([[x[0] / denom, 0, 0, 0], [0, x[1] / denom, 0, 0], [0, 0, x[2] / denom, 0], [0, 0, 0, x[3] / denom]])

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

        kwds = [{}, {"HJacobian": self.H_of, "Hx": self.hx}]

        imm.predict_update(self.measuremnet, input=np.array([2, 2, 0, 0]), update_kwds=kwds)

        state_after      = imm.state.copy()
        covariance_after = imm.covariance.copy()

        self.assertFalse(np.array_equal(state_before, state_after))
        self.assertFalse(np.array_equal(covariance_before, covariance_after))

    ##############################################################################

    def test_simple_system(self):
        "TODO: implement a simple system and calculate 1 timestep. Verify all matrices afterwards"
        pass

if __name__ == '__main__':
    unittest.main()

#EOF
