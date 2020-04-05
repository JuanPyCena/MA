import unittest
from src.ConfigParser import ConfigurationParser as CP
from src.ConfigParser import ParserLib

CFGPATH="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_3models.cfg"

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

if __name__ == '__main__':
    unittest.main()
