from src.filterlib.InteractingMultipleModel import InteractingMultipleModel as IMM
from src.utils.ConfigParser import ConfigurationParser as CP
from src.filterlib.Filters import KalmanFilter as KF
from src.filterlib.Filters import ExtendedKalmanFilter as EKF

import re
import numpy as np
from math import sqrt
from math import exp as e

from datetime import datetime
from datetime import timedelta

from src.utils.logmod import Logger

# Global variables
CFGPATH = "D:\\programming\\masterarbeit\\config\\imm_CV_ICSM.cfg"
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering

# Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
ALPHA_2 = 0.016
ALPHA_1 = 0.01
SIGMA_M_SQ_1 = 3
SIGMA_M_SQ_2 = 4

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
        """
        TODO: extrapolation does not work quite yet. Leads to massively overextrapolated tracks which are far off!!! How to fix this
        """
        targets_for_deletion = []
        for target, track in zip(self.__track_table.keys(), self.__track_table.values()):
            time_diff = float(extrapolation_time.timestamp() - track.time_of_last_update.timestamp())

            # Track is finished
            if time_diff >= 1:
                targets_for_deletion.append(target)
                continue

            track.calculate_time_depended_matrices_of_filters(time_diff, track.measurement)
            # self.__singer_process_model(track.filters[0], time_diff, ALPHA_1, SIGMA_M_SQ_1)
            self.__singer_process_model(track.filters[1], time_diff, ALPHA_2, SIGMA_M_SQ_2)
            # state, covariance, mode_probabilities = track.extrapolate(input=self.__a_mean(self.__track_table[target].state))
            state, covariance, mode_probabilities = track.extrapolate()
            # log.write_to_log("Filter 1 : {}".format(track.filters[0].transition_function * track.state))
            # log.write_to_log("Filter 2 : {}".format(track.filters[1].transition_function * track.state))
            # log.write_to_log(f"State: {state}")
            # state, covariance, mode_probabilities = track.state.copy(), track.covariance.copy(), track.mode_probabilities.copy()
            f0 = track.filters[0]
            f1 = track.filters[1]
            metadata_filter0 = (f0.state_prior.copy(), f0.state_post.copy(), f0.K.copy(), f0.system_uncertainty.copy(),
                                f0.process_noise_function.copy(), f0.state_uncertainty.copy(), f0.measurement.copy())
            metadata_filter1 = (f1.state_prior.copy(), f1.state_post.copy(), f1.K.copy(), f1.system_uncertainty.copy(),
                                f1.process_noise_function.copy(), f1.state_uncertainty.copy(), f1.measurement.copy())
            self.__extrapolated_targets[target].append((state, covariance, extrapolation_time, mode_probabilities,
                                                        ",".join(plot_chain_id[target]),
                                                        metadata_filter0, metadata_filter1))

        for target in targets_for_deletion:
            del self.__track_table[target]

    ##############################################################################

    def add_plot_data(self, target: str, plot_position: np.ndarray, plot_covariance: np.ndarray, plot_time: datetime, sensor_id: int) -> None:
        if target not in self.__track_table.keys():
            self.__track_table[target] = self.__initialize(plot_position, self.__prepareR(plot_covariance, sensor_id))
            self.__track_table[target].time_of_last_update = plot_time
            if target not in self.__extrapolated_targets.keys():
                self.__extrapolated_targets[target] = []
            return

        # Predict and update the state
        previous_state = self.__track_table[target].state
        time_diff = float(plot_time.timestamp() - self.__track_table[target].time_of_last_update.timestamp())
        self.__track_table[target].calculate_time_depended_matrices_of_filters(time_diff, plot_position)
        # For ICSM model
        #self.__singer_process_model(self.__track_table[target].filters[0], time_diff, ALPHA_1, SIGMA_M_SQ_1)
        self.__singer_process_model(self.__track_table[target].filters[1], time_diff, ALPHA_2, SIGMA_M_SQ_2)
        self.__track_table[target].predict_update(measurement=plot_position,
                                                  R=self.__prepareR(plot_covariance, sensor_id),
                                                  input=self.__a_mean(previous_state))
        # self.__track_table[target].predict_update(measurement=plot_position,
        #                                           R=self.__prepareR(plot_covariance, sensor_id))
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

    def __prepareR(self, mat: np.ndarray, sensor_id: int) -> np.ndarray:
        if sensor_id == 25188:  # mlat
            return mat*10
        elif sensor_id == 20:  # adsb
            return mat/10
        return mat

    ##############################################################################

    def __singer_process_model(self, filter, time_diff, alpha, sigma):
        filter.transition_function = self.__F(time_diff, alpha)
        filter.input_function = self.__G(time_diff, alpha)
        filter.process_noise_function = self.__Q(time_diff, alpha, sigma)

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __F(self, dt: float, alpha: float) -> np.ndarray:
        F0 = (1/alpha**2)*(-1 + alpha*dt + e(-alpha*dt))
        F1 = (1/alpha)*(1 - e(-alpha*dt))
        F2 = e(-alpha*dt)
        F = np.array([[1, dt, F0, 0,  0,  0],
                      [0,  1, F1, 0,  0,  0],
                      [0,  0, F2, 0,  0,  0],
                      [0,  0,  0, 1, dt, F0],
                      [0,  0,  0, 0,  1, F1],
                      [0,  0,  0, 0,  0, F2]])
        return F

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __Q(self, dt: float, alpha: float, sigma) -> np.ndarray:
        q11 = (1/2*alpha**5) * (1 - e(-2*alpha*dt) + 2*alpha*dt + (2*(alpha**3)*(dt**3))/3 - 2*(alpha**2)*(dt**2) - 4*alpha*dt*e(-alpha*dt))
        q12 = (1/2*alpha**4) * (1 + e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt) + 2*alpha*dt*e(-alpha*dt) - 2*alpha*dt + (alpha**2)*(dt**2))
        q13 = (1/2*alpha**3) * (1 - e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt))
        q22 = (1/2*alpha**3) * (4*e(-alpha*dt) - 3 - e(-2*alpha*dt) + 2*alpha*dt)
        q23 = (1/2*alpha**2) * (1 - 2*e(-alpha*dt) + e(-2*alpha*dt))
        q33 = (1/2*alpha) * (1 - e(-2*alpha*dt))

        Q = np.array([[q11, q12, q13,   0,   0,   0],
                      [q12, q22, q23,   0,   0,   0],
                      [q13, q23, q33,   0,   0,   0],
                      [0,     0,   0, q11, q12, q13],
                      [0,     0,   0, q12, q22, q23],
                      [0,     0,   0, q13, q23, q33]])

        return 2*alpha*sigma*Q

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __G(self, dt: float, alpha: float) -> np.ndarray:
        g1 = (dt**2/2) - ((alpha*dt - 1 + e(-alpha*dt))/alpha**2)
        g2 = dt - ((1 - e(-alpha*dt))/alpha)
        g3 = 1 - e(-alpha*dt)

        G = np.array([[g1, 0],
                      [g2, 0],
                      [g3, 0],
                      [0, g1],
                      [0, g2],
                      [0, g3]])

        return G

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __a_mean(self, x_k: np.ndarray) -> np.ndarray:
        return np.array([x_k[2], x_k[5]])

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
