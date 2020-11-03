from src.filterlib.InteractingMultipleModel import InteractingMultipleModel as IMM
from src.utils.ConfigParser import ConfigurationParser as CP
from src.filterlib.Filters import KalmanFilter as KF
from src.filterlib.Filters import ExtendedKalmanFilter as EKF

import re
import numpy as np
from math import sqrt

from datetime import datetime
from datetime import timedelta

from src.utils.logmod import Logger

# Global variables
CFGPATH = "D:\\programming\\masterarbeit\\config\\imm.cfg"
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering
SIGMA_A_SQ = 0

log = Logger()


class DFuse3(object):
    def __init__(self):
        log.write_to_log("---------------------Start DFUSE3---------------------")

        self.__track_table          = dict()  # Saves all track states calculated by IMM as an instance of the IMM
        self.__extrapolated_targets = dict()  # Extrapolation of targets, this is the output of the IMM!!!

        self.__config = CP(cfg_path=CFGPATH)

    ##############################################################################

    def __del__(self):
        log.write_to_log("---------------------Stop DFUSE3---------------------")

    ##############################################################################

    def extrapolate(self, plot_chain_id: dict, extrapolation_time: datetime) -> None:
        for target, track in zip(self.__track_table.keys(), self.__track_table.values()):
            time_diff = float(extrapolation_time.timestamp() - track.time_of_last_update.timestamp())
            track.calculate_time_depended_matrices_of_filters(time_diff, track.measurement)
            state, covariance, mode_probabilities = track.extrapolate()
            self.__extrapolated_targets[target].append((state, covariance, mode_probabilities, ",".join(plot_chain_id[target])))

    ##############################################################################

    def add_plot_data(self, target: str, plot_position: np.ndarray, plot_covariance: np.ndarray, plot_time: datetime) -> None:
        if target not in self.__track_table.keys():
            self.__track_table[target] = self.__initialize(plot_position, self.__prepareR(plot_covariance))
            self.__track_table[target].time_of_last_update = plot_time
            self.__extrapolated_targets[target] = []
            return

        # Predict and update the state
        time_diff = float(plot_time.timestamp() - self.__track_table[target].time_of_last_update.timestamp())
        self.__track_table[target].calculate_time_depended_matrices_of_filters(time_diff, plot_position)
        self.__track_table[target].predict_update(plot_position, self.__prepareR(plot_covariance))
        self.__track_table[target].time_of_last_update = plot_time

    ##############################################################################

    @property
    def extrapolated_targets(self):
        return self.__extrapolated_targets

    ##############################################################################

    @property
    def track_table(self):
        return self.__track_table

    ##############################################################################

    def __prepareR(self, mat: np.ndarray) -> np.ndarray:
        return 1000*mat

    ##############################################################################
    def __initialize(self, position: np.ndarray, covariance: np.ndarray) -> IMM:
        """
        This functions initializes all subfilters for the IMM and then initializes the IMM itself
        :return: imm - InteractingMultipleModel: initialized IMM filter
        """
        sub_filters = []
        # Initialize all kalman filters and save them into member variable
        for filter in self.__config.filters:
            if re.compile(KALMANFILTERKEY).match(filter):
                sub, _ = self.__set_up_kalman_filter(filter, position, covariance)

            if re.compile(EXTENDEDKALMANFILTERKEY).match(filter):
                sub, _ = self.__set_up_extended_kalman_filter(filter)

            sub_filters.append(sub)

        return IMM(sub_filters, self.__config.mode_probabilities, self.__config.markov_transition_matrix,
                   self.__config.expansion_matrix, self.__config.expansion_matrix_covariance,
                   self.__config.expansion_matrix_S,
                   self.__config.shrinking_matrix)

    ##############################################################################

    def __set_up_kalman_filter(self, filter_name: str, position: np.ndarray, covariance: np.ndarray):
        """
        This functions initiliazes a kalman filter with the read configuration and saves it into a list and dictionary
        :param filter_name: str - name of the kalman filter to be initialized
        """
        transition_matrix = self.__config.filter_configs[filter_name].transition_matrix  # F
        measurement_control_matrix = self.__config.filter_configs[filter_name].measurement_control_matrix  # H
        input_control_matrix = self.__config.filter_configs[filter_name].input_control_matrix  # B/G
        process_noise_matrix = self.__config.filter_configs[filter_name].process_noise_matrix  # Q
        measurement_uncertainty = covariance  # R

        covariance_matrix = np.array([[covariance[0][0], 0, 0, covariance[0][1], 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [covariance[1][0], 0, 0, covariance[1][1], 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [0, 0, 0, 0, 0, 0]])  # P
        initial_state = np.array([position[0], 0, 0, position[1], 0, 0])
        initial_input = np.array([0, 0, 0, 0, 0, 0])
        initial_measurement = initial_state.copy()

        kf = KF(initial_state, initial_input, initial_measurement, transition_matrix,
                input_control_matrix, process_noise_matrix, covariance_matrix,
                measurement_control_matrix, measurement_uncertainty)

        return (kf, {})

    ##############################################################################

    def __set_up_extended_kalman_filter(self, filter_name: str, position: np.ndarray, covariance: np.ndarray) -> tuple:
        """
        This functions initiliazes an extended kalman filter with the read configuration and saves it into a list and dictionary
        :param filter_name: str - name of the extended kalman filter to be initialized
        """
        jacobi_matrix = self.__config.filter_configs[filter_name].jacobi_matrix  # J
        transition_matrix = self.__config.filter_configs[filter_name].transition_matrix  # F
        measurement_control_matrix = self.__config.filter_configs[filter_name].measurement_control_matrix  # H
        input_control_matrix = self.__config.filter_configs[filter_name].input_control_matrix  # B/G
        process_noise_matrix = self.__config.filter_configs[filter_name].process_noise_matrix  # Q
        measurement_uncertainty = covariance  # R

        covariance_matrix = np.array([[covariance[0][0], 0, 0, covariance[0][1], 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [covariance[1][0], 0, 0, covariance[1][1], 0, 0],
                                      [0, 0, 0, 0, 0, 0],
                                      [0, 0, 0, 0, 0, 0]])  # P
        initial_state = np.array([position[0], 0, 0, position[1], 0, 0])
        initial_input = np.array([0, 0, 0, 0, 0, 0])
        initial_measurement = initial_state.copy()

        ekf = EKF(initial_state, initial_input, initial_measurement, transition_matrix,
                  input_control_matrix, process_noise_matrix, covariance_matrix,
                  measurement_control_matrix, measurement_uncertainty, jacobi_matrix)

        def hx(x):
            """ takes a state variable and returns the measurement that would
            correspond to that state.
            """
            return x

        # Append required functions for the EKF
        return ekf, {"Hx": hx}

    # EOC
