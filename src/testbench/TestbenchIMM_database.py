from src.utils.Decorators import *
from src.utils.ConfigParser import ConfigurationParser as CP
from src.filterlib.InteractingMultipleModel import InteractingMultipleModel as IMM
from src.filterlib.Filters import KalmanFilter as KF
from src.filterlib.Filters import ExtendedKalmanFilter as EKF
from src.utils.DataFileInterface import DataFilteInterface as DFI
from src.database.ADBConnector import ADBConnector as DataBase

import re
import numpy as np
from math import sqrt

from datetime import datetime

from src.utils.logmod import Logger

# Global variables
CFGPATH="D:\\programming\\pycharm\\Masterarbeit\\MA\\config\\imm_2models.cfg"
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering
SIGMA_A_SQ = 0

log = Logger()

def hx(x):
    """ takes a state variable and returns the measurement that would
    correspond to that state.
    """
    return x

##############################################################################

class TestbenchIMMDataBase(object):
    @typecheck(str, str)
    def __init__(self, test_data, imm_data_file, target):
        """
        :param test_data: str - path to the database
        :param imm_data_file: str - holds the path were the simulated data shall be saved to
        """

        db_data = DataBase(test_data).data[target]

        # Save test data into member variables
        self.test_data_measurement = db_data[0]
        self.test_data_covariance  = db_data[1]
        self.test_data_time        = db_data[2]

        self.imm_data_writer = DFI(imm_data_file)

        self.config = CP(cfg_path=CFGPATH)

        # define common initial values
        self.initial_input       = self.test_data_measurement[1]
        self.initial_state       = self.test_data_measurement[0]
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
        last_update_time_stamp = float(datetime.timestamp(self.test_data_time[0]))
        for idx, time in enumerate(self.test_data_time[1:]):
            t = datetime.timestamp(time)
            idx += 1
            measurement = self.test_data_measurement[idx]

            # Replace all placeholders of sub filters
            self.imm.calculate_time_depended_matrices_of_filters(float(t) - last_update_time_stamp, measurement)
            # Save update time stamp
            last_update_time_stamp = float(t)
            self.__ekf_kwds.append({"R": self.test_data_covariance[idx]})
            # Predict and update the state
            self.imm.predict_update(measurement, update_kwds=self.__ekf_kwds)

            state              = self.imm.state.copy()
            mode_probabilities = self.imm.mode_probabilities.copy()
            # test_data_state    = np.array([float(self.test_data_position[idx][0]), float(self.test_data_velocity[idx][0]), float(self.test_data_acceleration[idx][0]),
            #                                float(self.test_data_position[idx][1]), float(self.test_data_velocity[idx][1]), float(self.test_data_acceleration[idx][1])])
            #
            # state_error        = np.abs(np.subtract(test_data_state, state))

            self.imm_data_writer.measurement_data   = measurement.copy()
            self.imm_data_writer.state_data         = state.copy()
            self.imm_data_writer.mode_probabilities = mode_probabilities.copy()
            # self.imm_data_writer.state_errors       = state_error.copy()


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

        return IMM(self.sub_filters, self.config.mode_probabilities, self.config.markov_transition_matrix,
                   self.config.expansion_matrix, self.config.expansion_matrix_covariance, self.config.expansion_matrix_S,
                   self.config.shrinking_matrix)

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

        if np.size(self.config.filter_configs[filter_name].transition_matrix, 0) == 4:
            initial_state = np.array([self.initial_state[0], self.initial_state[1], self.initial_state[3], self.initial_state[4]])
            initial_input = np.array([self.initial_input[0], self.initial_input[1]])
            initial_measurement = np.array([self.initial_measurement[0], self.initial_measurement[1], self.initial_measurement[3], self.initial_measurement[4]])
        elif np.size(self.config.filter_configs[filter_name].transition_matrix, 0) == 2:
            initial_state = np.array([self.initial_state[0], self.initial_state[3]])
            initial_input = np.array([self.initial_input[0], self.initial_input[1]])
            initial_measurement = np.array([self.initial_measurement[0], self.initial_measurement[3]])
        else:
            initial_state = self.initial_state
            initial_input = self.initial_input
            initial_measurement = self.initial_measurement

        kf = KF(initial_state, initial_input, initial_measurement, transition_matrix,
                input_control_matrix, process_noise_matrix, covariance_matrix,
                measurement_control_matrix, measurement_uncertainty)

        # Append empty dictionary since we don't need any additional functions for the KF
        self.__ekf_kwds.append({})

        self.sub_filters_dict[filter_name] = kf
        self.sub_filters.append(kf)

    ##############################################################################

    def __set_up_extended_kalman_filter(self, filter_name):
        """
        This functions initiliazes an extended kalman filter with the read configuration and saves it into a list and dictionary
        :param filter_name: str - name of the extended kalman filter to be initialized
        """
        transition_matrix          = self.config.filter_configs[filter_name].transition_matrix               # F
        jacobi_matrix              = self.config.filter_configs[filter_name].jacobi_matrix                   # J
        measurement_control_matrix = self.config.filter_configs[filter_name].measurement_control_matrix      # H
        input_control_matrix       = self.config.filter_configs[filter_name].input_control_matrix            # B/G
        process_noise_matrix       = self.config.filter_configs[filter_name].process_noise_matrix            # Q
        covariance_matrix          = np.eye(np.size(self.config.filter_configs[filter_name].transition_matrix, 0))   # P
        measurement_uncertainty    = self.config.filter_configs[filter_name].measurement_uncertainty_matrix  # R

        if np.size(self.config.filter_configs[filter_name].transition_matrix, 0) == 4:
            initial_state = np.array([self.initial_state[0], self.initial_state[1], self.initial_state[3], self.initial_state[4]])
            initial_input = np.array([self.initial_input[0], self.initial_input[1], self.initial_input[3], self.initial_input[4]])
            initial_measurement = np.array([self.initial_measurement[0], self.initial_measurement[1], self.initial_measurement[3], self.initial_measurement[4]])
        elif np.size(self.config.filter_configs[filter_name].transition_matrix, 0) == 2:
            initial_state = np.array([self.initial_state[0], self.initial_state[3]])
            initial_input = np.array([self.initial_input[0], self.initial_input[3]])
            initial_measurement = np.array([self.initial_measurement[0], self.initial_measurement[3]])
        else:
            initial_state = self.initial_state
            initial_input = self.initial_input
            initial_measurement = self.initial_measurement

        ekf = EKF(initial_state, initial_input, initial_measurement, transition_matrix,
                  input_control_matrix, process_noise_matrix, covariance_matrix,
                  measurement_control_matrix, measurement_uncertainty, jacobi_matrix)

        # Append required functions for the EKF
        self.__ekf_kwds.append({"Hx": hx})

        self.sub_filters_dict[filter_name] = ekf
        self.sub_filters.append(ekf)

    #EOC
#EOF