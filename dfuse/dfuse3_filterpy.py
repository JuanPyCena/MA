from src.utils.ConfigParser import ConfigurationParser as CP

from filterpy.kalman.IMM import IMMEstimator as IMM
from filterpy.kalman import KalmanFilter as KF

import re
import numpy as np
from math import sqrt
from math import exp as e

from datetime import datetime
from datetime import timedelta
from copy import deepcopy

from src.utils.logmod import Logger

# Global variables
CFGPATH = "D:\\programming\\masterarbeit\\config\\imm_CV_ICSM.cfg"
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering

# Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
# CV
ALPHA_1 = 0
SIGMA_M_SQ_1 = 1
# Singer
ALPHA_2 = 0.1
SIGMA_M_SQ_2 = 5
#
# ALPHA_3 = 0.2
# SIGMA_M_SQ_3 = 2

ROLLING_MEAN_WINDOW = 20
COASTING_TIME = 2

FADING_MEMORY1 = 1.0
FADING_MEMORY2 = 1.5

OUTLIER_SIGMA = 2

log = Logger()


class DFuse3(object):
    def __init__(self):
        log.write_to_log("---------------------Start DFUSE3---------------------")

        self.__track_table          = dict()  # Saves all track states calculated by IMM as an instance of the IMM
        self.__track_times          = dict()  # Saves last timestamp of track
        self.__output_filter_table  = dict()
        self.__extrapolated_targets = dict()  # Extrapolation of targets, this is the output of the IMM!!!
        self.__target_plots         = dict()  # Plots of targets, this is the input of the IMM!!!
        self.__target_outliers      = dict()  # Outlieres of targets, this is the input of the IMM!!!
        self.__a_rolling_mean       = dict()
        self.__sensor_correction_value = 1
        self.__vehicle = dict()

        self.__config = CP(cfg_path=CFGPATH)

    ##############################################################################

    def __del__(self):
        log.write_to_log("---------------------Stop DFUSE3---------------------")

    ##############################################################################

    def __models(self, time_diff, filters, R=None):
        self.__constant_velocity_model(filters[0], time_diff, ALPHA_1, SIGMA_M_SQ_1, R)
        self.__singer_process_model(filters[1], time_diff, ALPHA_2, SIGMA_M_SQ_2, R)
        # self.__singer_process_model(filters[2], time_diff, ALPHA_3, SIGMA_M_SQ_3, R)

    ##############################################################################

    def extrapolate(self, plot_chain_id: dict, extrapolation_time: datetime) -> None:
        targets_for_deletion = []
        for target, track in zip(self.__track_table.keys(), self.__track_table.values()):
            time_diff = float(extrapolation_time.timestamp() - self.__track_times[target].timestamp())

            # Track is finished
            if time_diff >= COASTING_TIME:
                targets_for_deletion.append(target)
                continue

            # Save previous states to overwrite with correct parameters after extrapolation
            pre_state, pre_covariance, pre_mode_probabilities = track.x.copy(), track.P.copy(), track.mu.copy()

            # state, covariance, mode_probabilities = track.x.copy(), track.P.copy(), track.mu.copy()

            f0 = track.filters[0]
            f1 = track.filters[1]
            # f2 = track.filters[2]
            metadata_filter0 = (f0.x_prior.copy(), f0.x_post.copy(), f0.K.copy(), f0.S.copy(),
                                f0.Q.copy(), f0.R.copy(), f0.z.copy())
            metadata_filter1 = (f1.x_prior.copy(), f1.x_post.copy(), f1.K.copy(), f1.S.copy(),
                                f1.Q.copy(), f1.R.copy(), f1.z.copy())
            # metadata_filter2 = (f2.x_prior.copy(), f2.x_post.copy(), f2.K.copy(), f2.S.copy(),
            #                     f2.Q.copy(), f2.R.copy(), f2.z.copy())

            self.__models(time_diff, track.filters)

            # Extrapolate
            self.__track_table[target].predict()
            state, covariance, mode_probabilities = track.x.copy(), track.P.copy(), track.mu.copy()
            # Overwrite with old parameters
            track.x, track.P, track.mu = pre_state.copy(), pre_covariance.copy(), pre_mode_probabilities.copy()

            self.__extrapolated_targets[target].append((state, covariance, extrapolation_time,
                                                        mode_probabilities, ",".join(plot_chain_id[target]),
                                                        metadata_filter0, metadata_filter1))

        for target in targets_for_deletion:
            del self.__track_table[target]
            del self.__track_times[target]
            # del self.__output_filter_table[target]

    ##############################################################################

    def add_plot_data(self, target: str, plot_position: np.ndarray, plot_covariance: np.ndarray, plot_time: datetime, sensor_id: int) -> None:
        if target not in self.__track_table.keys():
            self.__track_table[target] = self.__initialize(plot_position, 10*self.__prepareR(target, plot_covariance, sensor_id))
            self.__track_times[target] = plot_time
            # self.__output_filter_table[target] = self.__set_up_output_filter(plot_position, plot_covariance)
            if target not in self.__extrapolated_targets.keys():
                self.__extrapolated_targets[target] = []
            if target not in self.__target_plots.keys():
                self.__target_plots[target] = []
            if target not in self.__target_outliers.keys():
                self.__target_outliers[target] = []
            return

        previous_state = self.__track_table[target].x
        time_diff = float(plot_time.timestamp() - self.__track_times[target].timestamp())

        # correct R for sensor error model and outliers
        corrected_R = self.__outlier_correction(target, plot_position, plot_covariance, plot_time, sensor_id)

        # Set model matrices
        self.__models(time_diff, self.__track_table[target].filters, R=corrected_R)

        # predict and update
        # self.__track_table[target].predict(u=self.__a_mean(previous_state, target))
        self.__track_table[target].predict()
        self.__track_table[target].update(z=plot_position)

        # Remove sensor correction
        self.__track_table[target].P /= self.__sensor_correction_value
        self.__track_table[target].P_post /= self.__sensor_correction_value

        self.__target_plots[target].append((plot_time, plot_position, plot_covariance))
        self.__track_times[target] = plot_time

    ##############################################################################

    @property
    def extrapolated_targets(self):
        return self.__extrapolated_targets

    ##############################################################################
    @property
    def vehicle(self):
        return self.__vehicle

    ##############################################################################

    @vehicle.setter
    def vehicle(self, dict):
        self.__vehicle = dict

    ##############################################################################

    @property
    def target_plots(self):
        return self.__target_plots

    ##############################################################################

    @property
    def target_outliers(self):
        return self.__target_outliers

    ##############################################################################

    @property
    def track_table(self):
        return self.__track_table

    ##############################################################################

    def __prepareR(self, target: str, mat: np.ndarray, sensor_id: int) -> np.ndarray:
        x = mat[0][0]
        y = mat[0][0]
        xy = mat[1][0]

        var_xy = np.sign(xy) * np.sqrt(abs(xy)) * np.sqrt(x) * np.sqrt(y)
        var_xy = 0

        self.__sensor_correction_value = 1
        vehicle = self.vehicle[target] == 1

        if sensor_id == 25188:  # mlat
            mat = np.array([[x, var_xy], [var_xy, y]])
            self.__sensor_correction_value = 529
        elif sensor_id == 20 and not vehicle:  # adsb
            self.__sensor_correction_value = 1/20
        elif sensor_id == 20 and vehicle:  # adsb
            self.__sensor_correction_value = 1/1000

        mat *= self.__sensor_correction_value

        # if self.__sensor_correction_value <= 1:
        #     self.__sensor_correction_value = 1

        return mat

    ##############################################################################

    def __singer_process_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.F = self.__F_singer(time_diff, alpha)
        filter.B = self.__G_singer(time_diff, alpha)
        filter.Q = self.__Q_singer(time_diff, alpha, sigma)
        if R is not None:
            filter.R = R

    ##############################################################################

    def __constant_velocity_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.alpha = FADING_MEMORY1
        filter.F = self.__F_CV(time_diff, alpha)
        filter.B = self.__G_CV(time_diff, alpha)
        filter.Q = self.__Q_CV(time_diff, alpha, sigma)
        if R is not None:
            filter.R = R

    ##############################################################################

    def __constant_acceleration_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.alpha = FADING_MEMORY2
        filter.F = self.__F_CA(time_diff, alpha)
        filter.B = self.__G_CA(time_diff, alpha)
        filter.Q = self.__Q_CA(time_diff, alpha, sigma)
        if R is not None:
            filter.R = R

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __F_singer(self, dt: float, alpha: float) -> np.ndarray:
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
    def __Q_singer(self, dt: float, alpha: float, sigma) -> np.ndarray:
        q11 = (1/2*alpha**5) * (1 - e(-2*alpha*dt) + 2*alpha*dt + (2*(alpha**3)*(dt**3))/3 - 2*(alpha**2)*(dt**2) - 4*alpha*dt*e(-alpha*dt))
        q12 = (1/2*alpha**4) * (1 + e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt) + 2*alpha*dt*e(-alpha*dt) - 2*alpha*dt + (alpha**2)*(dt**2))
        q13 = (1/2*alpha**3) * (1 - e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt))
        q22 = (1/2*alpha**3) * (4*e(-alpha*dt) - 3 - e(-2*alpha*dt) + 2*alpha*dt)
        q23 = (1/2*alpha**2) * (1 - 2*e(-alpha*dt) + e(-2*alpha*dt))
        q33 = (1/2*alpha) * (1 - e(-2*alpha*dt))

        # q33 = (1 / 2 * alpha ** 5) * (1 - e(-2 * alpha * dt) + 2 * alpha * dt + (2 * (alpha ** 3) * (dt ** 3)) / 3 - 2 * (alpha ** 2) * (
        #                 dt ** 2) - 4 * alpha * dt * e(-alpha * dt))
        # q23 = (1 / 2 * alpha ** 4) * (1 + e(-2 * alpha * dt) - 2 * alpha * dt * e(-alpha * dt) + 2 * alpha * dt * e(
        #     -alpha * dt) - 2 * alpha * dt + (alpha ** 2) * (dt ** 2))
        # q13 = (1 / 2 * alpha ** 3) * (1 - e(-2 * alpha * dt) - 2 * alpha * dt * e(-alpha * dt))
        # q22 = (1 / 2 * alpha ** 3) * (4 * e(-alpha * dt) - 3 - e(-2 * alpha * dt) + 2 * alpha * dt)
        # q12 = (1 / 2 * alpha ** 2) * (1 - 2 * e(-alpha * dt) + e(-2 * alpha * dt))
        # q11 = (1 / 2 * alpha) * (1 - e(-2 * alpha * dt))

        Q = np.array([[q11, q12, q13,   0,   0,   0],
                      [q12, q22, q23,   0,   0,   0],
                      [q13, q23, q33,   0,   0,   0],
                      [0,     0,   0, q11, q12, q13],
                      [0,     0,   0, q12, q22, q23],
                      [0,     0,   0, q13, q23, q33]])

        return 2*alpha*sigma*Q

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __G_singer(self, dt: float, alpha: float) -> np.ndarray:
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
    def __F_CV(self, dt: float, alpha: float) -> np.ndarray:
        F = np.array([[1, dt, 0, 0,  0,  0],
                      [0,  1, 0, 0,  0,  0],
                      [0,  0, 0, 0,  0,  0],
                      [0,  0,  0, 1, dt, 0],
                      [0,  0,  0, 0,  1, 0],
                      [0,  0,  0, 0,  0, 0]])
        return F

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __Q_CV(self, dt: float, alpha: float, sigma: float) -> np.ndarray:
        # q33 = sigma*dt**4/4
        # q23 = sigma*dt**3/2
        # q13 = sigma*dt**2/2
        # q22 = sigma*dt**2
        # q12 = sigma*dt
        # q11 = sigma

        q11 = sigma*dt**4/4
        q12 = sigma*dt**3/2
        q13 = sigma*dt**2/2
        q22 = sigma*dt**2
        q23 = sigma*dt
        q33 = sigma

        Q = np.array([[q11, q12, q13,   0,   0,   0],
                      [q12, q22, q23,   0,   0,   0],
                      [q13, q23, q33,   0,   0,   0],
                      [0,     0,   0, q11, q12, q13],
                      [0,     0,   0, q12, q22, q23],
                      [0,     0,   0, q13, q23, q33]])

        return Q

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __G_CV(self, dt: float, alpha: float) -> np.ndarray:
        g1 = dt**2/2
        g2 = dt
        g3 = 1

        G = np.array([[g1, 0],
                      [g2, 0],
                      [g3, 0],
                      [0, g1],
                      [0, g2],
                      [0, g3]])

        return G

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __F_CA(self, dt: float, alpha: float) -> np.ndarray:
        F = np.array([[1, dt, 0, 0,  0,  0],
                      [0,  1, 0, 0,  0,  0],
                      [0,  0, 0, 0,  0,  0],
                      [0,  0,  0, 1, dt, 0],
                      [0,  0,  0, 0,  1, 0],
                      [0,  0,  0, 0,  0, 0]])
        return F

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __Q_CA(self, dt: float, alpha: float, sigma: float) -> np.ndarray:
        q11 = (dt**5)/20
        q12 = (dt**4)/8
        q13 = (dt**3)/6
        q22 = (dt**3)/2
        q23 = (dt**2)/2
        q33 = dt

        # q33 = (dt ** 5) / 20
        # q23 = (dt ** 4) / 8
        # q13 = (dt ** 3) / 6
        # q22 = (dt ** 3) / 2
        # q12 = (dt ** 2) / 2
        # q11 = dt

        Q = sigma * np.array([[q11, q12, q13,   0,   0,   0],
                              [q12, q22, q23,   0,   0,   0],
                              [q13, q23, q33,   0,   0,   0],
                              [0,  0,      0, q11, q12, q13],
                              [0,  0,      0, q12, q22, q23],
                              [0,  0,      0, q13, q23, q33]])

        return Q

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __G_CA(self, dt: float, alpha: float) -> np.ndarray:
        g1 = dt**2/2
        g2 = dt
        g3 = 1

        G = np.array([[g1, 0],
                      [g2, 0],
                      [g3, 0],
                      [0, g1],
                      [0, g2],
                      [0, g3]])

        return G

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __a_mean(self, x_k: np.ndarray, target: str) -> np.ndarray:
        new_a = np.array([x_k[2], x_k[5]])

        if target not in self.__a_rolling_mean.keys():
            self.__a_rolling_mean[target] = []

        if len(self.__a_rolling_mean[target]) >= ROLLING_MEAN_WINDOW:
            _ = self.__a_rolling_mean[target].pop()

        self.__a_rolling_mean[target].append(new_a)

        mean = np.array([0.0, 0.0])
        for a in self.__a_rolling_mean[target]:
            mean += a

        mean = mean/len(self.__a_rolling_mean[target])

        return mean

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

            sub_filters.append(sub)

        return IMM(sub_filters, self.__config.mode_probabilities, self.__config.markov_transition_matrix)

    ##############################################################################

    def __set_up_kalman_filter(self, filter_name: str, position: np.ndarray, covariance: np.ndarray):
        """
        This functions initiliazes a kalman filter with the read configuration and saves it into a list and dictionary
        :param filter_name: str - name of the kalman filter to be initialized
        """
        transition_matrix = np.array([[1, 0, 0, 0, 0, 0],
                                      [0, 1, 0, 0, 0, 0],
                                      [0, 0, 1, 0, 0, 0],
                                      [0, 0, 0, 1, 0, 0],
                                      [0, 0, 0, 0, 1, 0],
                                      [0, 0, 0, 0, 0, 1]])  # F
        measurement_control_matrix = self.__config.filter_configs[filter_name].measurement_control_matrix  # H
        input_control_matrix = np.array([[0, 0],
                                         [0, 0],
                                         [0, 0],
                                         [0, 0],
                                         [0, 0],
                                         [0, 0]])  # B/G
        process_noise_matrix = np.array([[0, 0, 0, 0, 0, 0],
                                         [0, 0, 0, 0, 0, 0],
                                         [0, 0, 0, 0, 0, 0],
                                         [0, 0, 0, 0, 0, 0],
                                         [0, 0, 0, 0, 0, 0],
                                         [0, 0, 0, 0, 0, 0]])  # Q
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

        kf = KF(6, 2, 6)
        kf.F = transition_matrix
        kf.H = measurement_control_matrix
        kf.B = input_control_matrix
        kf.Q = process_noise_matrix
        kf.R = measurement_uncertainty
        kf.P = covariance_matrix
        kf.x = initial_state
        kf.z = initial_measurement

        return (kf, {})

    ##############################################################################

    def __outlier_correction(self, target: str, plot_position: np.ndarray, plot_covariance: np.ndarray, plot_time: datetime, sensor_id: int) -> np.ndarray:
        imm = deepcopy(self.__track_table[target])

        # Predict and update the state
        previous_imm  = deepcopy(self.__track_table[target])
        previous_time = deepcopy(self.__track_times[target])

        time_diff = float(plot_time.timestamp() - previous_time.timestamp())
        R         = self.__prepareR(target, plot_covariance, sensor_id)

        # Set model matrices
        self.__models(time_diff, imm.filters, R)

        # predict
        imm.predict()
        x_imm, P_imm = deepcopy(imm.x_prior), deepcopy(imm.P_prior)
        H = deepcopy(imm.filters[0].H)

        # reset IMM just in case
        self.__track_table[target] = previous_imm
        self.__track_times[target] = previous_time

        # calculate mahalanobis distance for IMM and adjust R accordingly

        y_imm = plot_position - np.dot(H, x_imm).astype(float)
        S_imm = np.dot(H, np.dot(P_imm, H.T).astype(float)) + R

        MD = np.sqrt(float(np.dot(np.dot(y_imm.T, np.linalg.inv(S_imm)), y_imm)))

        if MD >= OUTLIER_SIGMA:
            self.__sensor_correction_value /= 2 * MD
            return 2 * MD * R
        return R

    # EOC
