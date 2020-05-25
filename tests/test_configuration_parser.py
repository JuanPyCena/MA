import unittest
from src.utils.ConfigParser import ConfigurationParser as CP
from src.utils.ConfigParser import ParserLib

CFGPATH="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_3models.cfg"
CFGPATH1="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_3models_constant_turning.cfg"
CFGPATH2="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_2models.cfg"

class TestConfigurationParser(unittest.TestCase):
    def test_config_read(self):
        config = CP(cfg_path=CFGPATH)
        print(config.filters)
        print(config.state_variables)
        print(config.markov_transition_matrix)
        print(config.mode_probabilities)
        print(config.filter_configs["KF"].transition_matrix)
        print(config.filter_configs["KF"].measurement_control_matrix)
        print(config.filter_configs["KF"].input_control_matrix)
        print(config.filter_configs["KF"].process_noise_matrix)
        print(config.filter_configs["EKF1"].transition_matrix)
        print(config.filter_configs["EKF1"].measurement_control_matrix)
        print(config.filter_configs["EKF1"].input_control_matrix)
        print(config.filter_configs["EKF1"].process_noise_matrix)
        print(config.filter_configs["EKF2"].transition_matrix)
        print(config.filter_configs["EKF2"].measurement_control_matrix)
        print(config.filter_configs["EKF2"].input_control_matrix)
        print(config.filter_configs["EKF2"].process_noise_matrix)

        print(ParserLib.calculate_time_depended_matrix(config.filter_configs["KF"].process_noise_matrix, 2, "dt"))
        print(ParserLib.calculate_time_depended_matrix(config.filter_configs["EKF1"].process_noise_matrix, 2, "dt"))

    def test_config2_read(self):
        config = CP(cfg_path=CFGPATH2)
        print(config.filters)
        print(config.state_variables)
        print(config.markov_transition_matrix)
        print(config.mode_probabilities)
        print(config.filter_configs["KF"].transition_matrix)
        print(config.filter_configs["KF"].measurement_control_matrix)
        print(config.filter_configs["KF"].input_control_matrix)
        print(config.filter_configs["KF"].process_noise_matrix)
        print(config.filter_configs["EKF"].transition_matrix)
        print(config.filter_configs["EKF"].measurement_control_matrix)
        print(config.filter_configs["EKF"].input_control_matrix)
        print(config.filter_configs["EKF"].process_noise_matrix)

        print(ParserLib.calculate_time_depended_matrix(config.filter_configs["KF"].process_noise_matrix, 2, "dt"))
        print(ParserLib.calculate_time_depended_matrix(config.filter_configs["EKF"].process_noise_matrix, 2, "dt"))

    def test_constant_turing(self):
        config = CP(cfg_path=CFGPATH1)
        variables = ["vx", "vy", "ax", "ay", "x", "y"]
        variable_replacement = [1, 1, 0, 0, 0, 0]
        functions = ["cos", "sin", "arctan"]
        function_replacement = ["np.cos", "np.sin", "np.arctan"]
        print(ParserLib.evaluate_functional_matrix(config.filter_configs["EKF1"].transition_matrix, 0.1, "dt", variables, variable_replacement, functions, function_replacement))
        print(ParserLib.evaluate_functional_matrix(config.filter_configs["EKF1"].jacobi_matrix, 0.1, "dt", variables, variable_replacement, functions, function_replacement))

if __name__ == '__main__':
    unittest.main()
