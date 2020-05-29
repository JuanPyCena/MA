{
    "a" : 0.95,
    "b" : 0.05,
    "sigma_a_sq" : 1,
    "filter_types" : ["KalmanFilter", "KalmanFilter"],
    "filters" : ["KF", "KF1"],
    "state_vector" : ["pos_x", "vel_x", "acc_x", "pos_y", "vel_y", "acc_y"],
    "markov_transition_matrix" : [[a, b], [b, a]],
    "mode_probabilities" : [0.5, 0.5],
    expansion_matrix : [[1, 0, 0, 0, 0, 0], [0, 1, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 1, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 0, 0]],
    expansion_matrix_covariance : [[1, 0, 0, 0, 0, 0], [0, 1, 0, 0, 0, 0], [0, 0, 0, 0, 9, 0], [0, 0, 1, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 0, 9]],
    expansion_matrix_s : [[1, 0, 0, 0, 0, 0], [0, 1, 0, 0, 0, 0], [0, 0, 0, 0, 1.05, 0], [0, 0, 1, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 0, 1.05]],
    shrinking_matrix : [[1, 0, 0, 0, 0, 0], [0, 1, 0, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 1, 0]],
    KF : {
        transition_matrix : [[1, dt, dt**2/2, 0, 0, 0], [0, 1, dt, 0, 0, 0], [0, 0, 1, 0, 0, 0] [0, 0, 0, 1, dt, dt**2/2], [0, 0, 0, 0, 1, dt], [0, 0, 0, 0, 0, 1]],
        measurement_control_matrix : [[1, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0]],
        process_noise_matrix : [[sigma_a_sq*dt**5/20, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/6, 0, 0, 0], [sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/2,  sigma_a_sq*dt**2/2, 0, 0, 0], [sigma_a_sq*dt**3/6, sigma_a_sq*dt**2/2,  sigma_a_sq*dt, 0, 0, 0], [ 0, 0, 0, sigma_a_sq*dt**5/20, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/6], [ 0, 0, 0, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/2,  sigma_a_sq*dt**2/2], [ 0, 0, 0, sigma_a_sq*dt**3/6, sigma_a_sq*dt**2/2,  sigma_a_sq*dt]],
        measurement_uncertainty_matrix : [[1, 0, 0, 0, 0, 0], [0, 1, 0, 0, 0, 0], [0, 0, 1, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 1, 0], [0, 0, 0, 0, 0, 1]],
        input_control_matrix : [[0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]],
    },

    KF1 : {
        transition_matrix : [[1, dt, dt**2/2, 0, 0, 0], [0, 1, dt, 0, 0, 0], [0, 0, 1, 0, 0, 0] [0, 0, 0, 1, dt, dt**2/2], [0, 0, 0, 0, 1, dt], [0, 0, 0, 0, 0, 1]],
        measurement_control_matrix : [[1, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 1, 0, 0], [0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0]],
        process_noise_matrix : [[sigma_a_sq*dt**5/20, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/6, 0, 0, 0], [sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/2,  sigma_a_sq*dt**2/2, 0, 0, 0], [sigma_a_sq*dt**3/6, sigma_a_sq*dt**2/2,  sigma_a_sq*dt, 0, 0, 0], [ 0, 0, 0, sigma_a_sq*dt**5/20, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/6], [ 0, 0, 0, sigma_a_sq*dt**4/8, sigma_a_sq*dt**3/2,  sigma_a_sq*dt**2/2], [ 0, 0, 0, sigma_a_sq*dt**3/6, sigma_a_sq*dt**2/2,  sigma_a_sq*dt]],
        measurement_uncertainty_matrix : [[1000, 0, 0, 0, 0, 0], [0, 1000, 0, 0, 0, 0], [0, 0, 1000, 0, 0, 0], [0, 0, 0, 1000, 0, 0], [0, 0, 0, 0, 1000, 0], [0, 0, 0, 0, 0, 1000]],
        input_control_matrix : [[0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]],
    },
}