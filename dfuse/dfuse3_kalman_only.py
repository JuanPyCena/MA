from src.utils.ConfigParser import ConfigurationParser as CP

from filterpy.kalman.IMM import IMMEstimator as IMM
from filterpy.kalman import KalmanFilter as KF

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
ALPHA_1 = 0.7
SIGMA_M_SQ_1 = 10
# ALPHA_2 = 0.01
# SIGMA_M_SQ_2 = 9
# ALPHA_3 = 0.2
# SIGMA_M_SQ_3 = 5

OUTPUT_SIGMA = 1

ROLLING_MEAN_WINDOW = 20

log = Logger()


class DFuse3(object):
    def __init__(self):
        log.write_to_log("---------------------Start DFUSE3---------------------")

        self.__track_table          = dict()  # Saves all track states calculated by IMM as an instance of the IMM
        self.__track_times          = dict()  # Saves last timestamp of track
        self.__output_filter_table  = dict()
        self.__extrapolated_targets = dict()  # Extrapolation of targets, this is the output of the IMM!!!
        self.__a_rolling_mean       = dict()

        self.__config = CP(cfg_path=CFGPATH)

    ##############################################################################

    def __del__(self):
        log.write_to_log("---------------------Stop DFUSE3---------------------")

    ##############################################################################

    def __models(self, time_diff, filter, R=None):
        self.__constant_velocity_model(filter, time_diff, ALPHA_1, SIGMA_M_SQ_1, R)

    ##############################################################################

    def extrapolate(self, plot_chain_id: dict, extrapolation_time: datetime) -> None:
        targets_for_deletion = []
        for target, track in zip(self.__track_table.keys(), self.__track_table.values()):
            time_diff = float(extrapolation_time.timestamp() - self.__track_times[target].timestamp())

            # Track is finished
            if time_diff >= 10:
                targets_for_deletion.append(target)
                continue

            # Save previous states to overwrite with correct parameters after extrapolation
            # pre_state, pre_covariance, pre_mode_probabilities = track.x.copy(), track.P.copy(), track.mu.copy()

            state, covariance, mode_probabilities = track.x.copy(), track.P.copy(), np.array([1.0, 0])

            # Use seperate Kalman Filter to perform output filtering
            # state, covariance = self._perform_output_filtering(target, track.x.copy(), track.P.copy(), time_diff)
            # mode_probabilities = track.mu.copy()

            f0 = track
            f1 = track
            # f2 = track.filters[2]
            metadata_filter0 = (f0.x_prior.copy(), f0.x_post.copy(), f0.K.copy(), f0.S.copy(),
                                f0.Q.copy(), f0.R.copy(), f0.z.copy())
            metadata_filter1 = (f1.x_prior.copy(), f1.x_post.copy(), f1.K.copy(), f1.S.copy(),
                                f1.Q.copy(), f1.R.copy(), f1.z.copy())
            # metadata_filter2 = (f1.x_prior.copy(), f1.x_post.copy(), f1.K.copy(), f1.S.copy(),
            #                     f1.Q.copy(), f1.R.copy(), f1.z.copy())

            # self.__models(time_diff, track.filters)
            #
            # # Extrapolate
            # self.__track_table[target].predict()
            # state, covariance, mode_probabilities = track.x.copy(), track.P.copy(), track.mu.copy()
            # # Overwrite with old parameters
            # track.x, track.P, track.mu = pre_state.copy(), pre_covariance.copy(), pre_mode_probabilities.copy()

            self.__extrapolated_targets[target].append((state, covariance, extrapolation_time,
                                                        mode_probabilities, ",".join(plot_chain_id[target]),
                                                        metadata_filter0, metadata_filter1))

        for target in targets_for_deletion:
            del self.__track_table[target]
            del self.__track_times[target]

    ##############################################################################

    def add_plot_data(self, target: str, plot_position: np.ndarray, plot_covariance: np.ndarray, plot_time: datetime, sensor_id: int) -> None:
        if target not in self.__track_table.keys():
            self.__track_table[target] = self.__initialize(plot_position, self.__prepareR(100*plot_covariance, sensor_id))
            self.__track_times[target] = plot_time
            if target not in self.__extrapolated_targets.keys():
                self.__extrapolated_targets[target] = []
            return

        # Predict and update the state
        previous_state = self.__track_table[target].x
        time_diff = float(plot_time.timestamp() - self.__track_times[target].timestamp())
        # Set model matrices
        self.__models(time_diff, self.__track_table[target], R=self.__prepareR(plot_covariance, sensor_id))

        # predict and update
        self.__track_table[target].predict(u=self.__a_mean(previous_state, target))
        # self.__track_table[target].predict()
        self.__track_table[target].update(z=plot_position)

        self.__track_times[target] = plot_time

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
        x = mat[0][0]
        y = mat[0][0]
        xy = mat[1][0]

        var_xy = np.sign(xy) * np.sqrt(abs(xy)) * np.sqrt(x) * np.sqrt(y)
        var_xy = 0

        if sensor_id == 25188:  # mlat
            mat = np.array([[x, var_xy], [var_xy, y]])
            return mat*25
        elif sensor_id == 20:  # adsb
            return mat/60
        return mat
    ##############################################################################

    def __singer_process_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.F = self.__F_singer(time_diff, alpha)
        filter.B = self.__G_singer(time_diff, alpha)
        filter.Q = self.__Q_singer(time_diff, alpha, sigma)
        # filter.Q = self.__Q(time_diff, alpha, sigma)
        if R is not None:
            filter.R = R

    ##############################################################################

    def __constant_velocity_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.F = self.__F_CV(time_diff, alpha)
        filter.B = self.__G_CV(time_diff, alpha)
        filter.Q = self.__Q_CV(time_diff, alpha, sigma)
        # filter.Q = self.__Q(time_diff, alpha, sigma)
        if R is not None:
            filter.R = R

    ##############################################################################

    def __constant_acceleration_model(self, filter, time_diff, alpha: float, sigma, R=None):
        filter.F = self.__F_CA(time_diff, alpha)
        filter.B = self.__G_CA(time_diff, alpha)
        filter.Q = self.__Q_CA(time_diff, alpha, sigma)
        # filter.Q = self.__Q(time_diff, alpha, sigma)
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

    def __Q(self, dt: float, alpha: float, sigma) -> np.ndarray:
        # q11 = 10
        # q12 = 0
        # q13 = 0
        # q22 = 10
        # q23 = 0
        # q33 = 10
        q11 = sigma * dt ** 4 / 4
        q12 = sigma * dt ** 3 / 2
        q13 = sigma * dt ** 2 / 2
        q22 = sigma * dt ** 2
        q23 = sigma * dt
        q33 = sigma

        Q = np.array([[q11, q12, q13, 0, 0, 0],
                      [q12, q22, q23, 0, 0, 0],
                      [q13, q23, q33, 0, 0, 0],
                      [0, 0, 0, q11, q12, q13],
                      [0, 0, 0, q12, q22, q23],
                      [0, 0, 0, q13, q23, q33]])

        return sigma * Q

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __Q_singer(self, dt: float, alpha: float, sigma) -> np.ndarray:
        # q33 = (1/2*alpha**5) * (1 - e(-2*alpha*dt) + 2*alpha*dt + (2*(alpha**3)*(dt**3))/3 - 2*(alpha**2)*(dt**2) - 4*alpha*dt*e(-alpha*dt))
        # q23 = (1/2*alpha**4) * (1 + e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt) + 2*alpha*dt*e(-alpha*dt) - 2*alpha*dt + (alpha**2)*(dt**2))
        # q13 = (1/2*alpha**3) * (1 - e(-2*alpha*dt) - 2*alpha*dt*e(-alpha*dt))
        # q22 = (1/2*alpha**3) * (4*e(-alpha*dt) - 3 - e(-2*alpha*dt) + 2*alpha*dt)
        # q12 = (1/2*alpha**2) * (1 - 2*e(-alpha*dt) + e(-2*alpha*dt))
        # q11 = (1/2*alpha) * (1 - e(-2*alpha*dt))

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
        F = np.array([[1, dt, 0.5*dt**2, 0,  0,         0],
                      [0,  1,        dt, 0,  0,         0],
                      [0,  0,         1, 0,  0,         0],
                      [0,  0,         0, 1, dt, 0.5*dt**2],
                      [0,  0,         0, 0,  1,        dt],
                      [0,  0,         0, 0,  0,         1]])
        return F

    ##############################################################################

    # Refer to https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5972417
    def __Q_CA(self, dt: float, alpha: float, sigma: float) -> np.ndarray:
        # q33 = (dt**5)/20
        # q23 = (dt**4)/8
        # q13 = (dt**3)/6
        # q22 = (dt**3)/2
        # q12 = (dt**2)/2
        # q11 = dt

        q11 = (dt ** 5) / 20
        q12 = (dt ** 4) / 8
        q13 = (dt ** 3) / 6
        q22 = (dt ** 3) / 2
        q23 = (dt ** 2) / 2
        q33 = dt

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
        kf = KF(6, 2, 2)
        kf.x = np.array([position[0], 0, 0, position[1], 0, 0])
        kf.z = np.array([position[0], 0, 0, position[1], 0, 0])
        kf.P = np.array([[covariance[0][0], 0, 0, covariance[1][0], 0, 0],
                          [0, 0, 0, 0, 0, 0],
                          [0, 0, 0, 0, 0, 0],
                          [covariance[0][1], 0, 0, covariance[1][1], 0, 0],
                          [0, 0, 0, 0, 0, 0],
                          [0, 0, 0, 0, 0, 0]])
        kf.H = np.array([[1, 0, 0, 0, 0, 0],
                         [0, 0, 0, 1, 0, 0]])

        return kf

    # EOC
