from src.Filters import KalmanFilter
import numpy as np

def main():
    state                   = np.array([1,2]) #x
    input                   = np.array([1,0]) #u
    measurement             = np.array([1,0]) #z
    transition              = np.array([[1,0], [0,1]]) #F
    input_control_matrix    = np.array([[1,0], [0,1]]) #G
    noise                   = np.array([[1,0], [0,1]]) #Q
    covariance_matrix       = np.array([[1,0], [0,1]]) #P
    measurement_matrix      = np.array([[1,0], [0,1]]) #H
    measurement_uncertainty = np.array([[1,0], [0,1]]) #R

    kf = KalmanFilter(state, input, measurement, transition, input_control_matrix, noise, covariance_matrix, measurement_matrix, measurement_uncertainty)
    kf.predict()
    kf.update(np.array([1,2]))

if __name__ == "__main__":
    main()