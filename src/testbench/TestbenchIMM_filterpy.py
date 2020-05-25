from src.utils.Decorators import *
from src.utils.ConfigParser import ConfigurationParser as CP
from src.filterlib.InteractingMultipleModel import InteractingMultipleModel as IMM
from src.filterlib.Filters import KalmanFilter as KF
from src.filterlib.Filters import ExtendedKalmanFilter as EKF
from src.utils.DataFileInterface import DataFilteInterface as DFI

from src.utils.ConfigParser import ParserLib
from filterpy.kalman import IMMEstimator, KalmanFilter

import re
import numpy as np
from math import sqrt

from src.utils.logmod import Logger

# Global variables
CFGPATH="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_2models.cfg"
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering
SIGMA_A_SQ = 1

log = Logger()

##############################################################################

class TestbenchIMM(object):
    @typecheck(dict, str)
    def __init__(self, test_data, imm_data_file):
        """
        :param test_data: dict - holds the test data to be used for the simulation
        :param imm_data_file: str - holds the path were the simulated data shall be saved to
        """
        # Save test data into member variables
        self.test_data_time         = test_data["time"]
        self.test_data_position     = test_data["position"]
        self.test_data_velocity     = test_data["velocity"]
        self.test_data_acceleration = test_data["acceleration"]

        self.imm_data_writer = DFI(imm_data_file)

        self.config = CP(cfg_path=CFGPATH)

        # define common initial values
        self.initial_input       = np.array([float(self.test_data_acceleration[0][0]), float(self.test_data_acceleration[0][1])])
        self.initial_state       = np.array([float(self.test_data_position[0][0]), float(self.test_data_velocity[0][0]), float(self.test_data_acceleration[0][0]),
                                             float(self.test_data_position[0][1]), float(self.test_data_velocity[0][1]), float(self.test_data_acceleration[0][1])])
        self.initial_measurement = self.initial_state

        # Keyword arguments needed for the EKF to work
        self.__ekf_kwds = []

        # dictionary and list to save filters for IMM
        self.sub_filters_dict = dict()
        self.sub_filters = []

        # Initialize IMM using config parameters
        self.imm = self.__initialize()

    ##############################################################################

    def run(self):
        """
        This functions runs the IMM simulation and saves the calculated data into a given csv file
        """
        print("Start running Testbench IMM")
        last_update_time_stamp = float(self.test_data_time[0])
        for idx, t in enumerate(self.test_data_time[1:]):
            idx += 1
            measurement = np.array([float(self.test_data_position[idx][0]), float(self.test_data_velocity[idx][0]), float(self.test_data_acceleration[idx][0]),
                                    float(self.test_data_position[idx][1]), float(self.test_data_velocity[idx][1]), float(self.test_data_acceleration[idx][1])])

            # Replace all placeholders of sub filters
            time_delta = float(t) - last_update_time_stamp
            variables = ["sigma_a_sq", "vx", "vy", "ax", "ay", "x_m", "y_m", "x", "y"]
            variable_replacement = [SIGMA_A_SQ, self.imm.x[1], self.imm.x[3], self.imm.x[4], self.imm.x[5],
                                    measurement[0],
                                    measurement[1], self.imm.x[0], self.imm.x[2]]
            functions = ["cos", "sin", "arctan"]
            function_replacement = ["np.cos", "np.sin", "np.arctan"]

            for filter in self.imm.filters:
                filter.P = ParserLib.evaluate_functional_matrix(filter.P, time_delta, "dt",  variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
                filter.H = ParserLib.evaluate_functional_matrix(filter.H, time_delta, "dt",  variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
                filter.F = ParserLib.evaluate_functional_matrix(filter.F, time_delta, "dt",  variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
                filter.Q = ParserLib.evaluate_functional_matrix(filter.Q, time_delta, "dt",  variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)
                filter.R = ParserLib.evaluate_functional_matrix(filter.R, time_delta, "dt",  variables,
                                                                                 variable_replacement, functions,
                                                                                 function_replacement)

            # Save update time stamp
            last_update_time_stamp = float(t)
            # Predict and update the state
            self.imm.predict()
            self.imm.update(measurement)
            state              = self.imm.x.copy()
            mode_probabilities = self.imm.mu.copy()
            test_data_state    = np.array([float(self.test_data_position[idx][0]), float(self.test_data_velocity[idx][0]), float(self.test_data_acceleration[idx][0]),
                                           float(self.test_data_position[idx][1]), float(self.test_data_velocity[idx][1]), float(self.test_data_acceleration[idx][1])])

            state_error        = np.abs(np.subtract(test_data_state, state))

            self.imm_data_writer.measurement_data   = measurement.copy()
            self.imm_data_writer.state_data         = state.copy()
            self.imm_data_writer.mode_probabilities = mode_probabilities.copy()
            self.imm_data_writer.state_errors       = state_error.copy()


        print("Finished running Testbench IMM, saving data...")
        self.imm_data_writer.write()
        print("Data saved in: {}".format(self.imm_data_writer.file_name))

    ##############################################################################

    def __initialize(self):
        """
        This functions initializes all subfilters for the IMM and then initializes the IMM itself
        :return: imm - InteractingMultipleModel: initialized IMM filter
        """
        # Initialize all kalman filters and save them into member variable
        for filter in self.config.filters:
            if re.compile(KALMANFILTERKEY).match(filter):
                self.__set_up_kalman_filter(filter)

            if re.compile(EXTENDEDKALMANFILTERKEY).match(filter):
                self.__set_up_extended_kalman_filter(filter)

        return IMMEstimator(self.sub_filters, self.config.mode_probabilities, self.config.markov_transition_matrix)

    ##############################################################################

    def __set_up_kalman_filter(self, filter_name):
        """
        This functions initiliazes a kalman filter with the read configuration and saves it into a list and dictionary
        :param filter_name: str - name of the kalman filter to be initialized
        """
        transition_matrix          = self.config.filter_configs[filter_name].transition_matrix               # F
        measurement_control_matrix = self.config.filter_configs[filter_name].measurement_control_matrix      # H
        input_control_matrix       = self.config.filter_configs[filter_name].input_control_matrix            # B/G
        process_noise_matrix       = self.config.filter_configs[filter_name].process_noise_matrix            # Q
        covariance_matrix          = np.eye(np.size(self.config.filter_configs[filter_name].transition_matrix, 0))  # P
        measurement_uncertainty    = self.config.filter_configs[filter_name].measurement_uncertainty_matrix  # R

        initial_state = self.initial_state
        initial_input = self.initial_input
        initial_measurement = self.initial_measurement

        kf = KalmanFilter(len(initial_state), len(initial_measurement))
        kf.x = initial_state
        kf.F = transition_matrix
        kf.H = measurement_control_matrix
        kf.P = covariance_matrix
        kf.R = measurement_uncertainty
        kf.Q = process_noise_matrix

        # Append empty dictionary since we don't need any additional functions for the KF
        self.__ekf_kwds.append({})

        self.sub_filters_dict[filter_name] = kf
        self.sub_filters.append(kf)

    #EOC
#EOF