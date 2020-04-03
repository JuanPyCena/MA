import unittest
from src.ConfigParser import ConfigurationParser as CP

CFGPATH="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_test_cfg.cfg"

class TestConfigurationParser(unittest.TestCase):
    def test_config_read(self):
        config = CP(cfg_path=CFGPATH)
        print(config.filters)
        print(config.state_variables)
        print(config.markov_transition_matrix)
        print(config.mode_probabilities)
        print(config.filter_configs["KF"].transition_matrix)
        print(config.filter_configs["KF1"].transition_matrix)
        print(config.filter_configs["EKF"].transition_matrix)
        print(config.filter_configs["EKF1"].transition_matrix)


if __name__ == '__main__':
    unittest.main()
