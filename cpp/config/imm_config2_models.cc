#avconfig_version 2

// -------------------------------------------------------------
namespace imm_config2_models
{
    //! List which defines which type of subfilters are to be used
    //!
    //! \suggested [KalmanFilter", ExtendedKalmanFilter]
    QStringList filter_types = [KalmanFilter; KalmanFilter];
    
    //! List which defines which subfilters are to be used
    //!
    //! \suggested [kf, "ekf]
    QStringList sub_filter_config_definition = [kf; kf1];
    
    //! List which defines the order of the state vector
    //!
    //! \suggested [pos_x, vel_x, pos_y, vel_y]
    QStringList state_vector = [pos_x; vel_x; acc_x; pos_y; vel_y; acc_y];
    
    
    //! Matrix which defines how likely it is to switch between filters
    //!
    //! \suggested []
    AVMatrix<float> markov_transition_matrix = [[         Filter1; Filter2;];
                                                [Filter1;    0.95;     0.05];
                                                [Filter2;    0.05;     0.95]];
    
    //! List which defines the intial probabilities of the subfilters
    //!
    //! \suggested [0.5 0.5]
    QList<float> mode_probabilities = [0.5; 0.5];
    
    //! Matrix which defines how the state of the subfilter is expanded to full state
    //!
    //! \suggested []
    AVMatrix<float> expansion_matrix = [[    x; vx; y; vy; ax; ay;];
                                        [x;  1;  0; 0;  0;  0;  0];
                                        [vx; 0;  1; 0;  0;  0;  0];
                                        [ax; 0;  0; 0;  0;  0;  0];
                                        [y;  0;  0; 1;  0;  0;  0];
                                        [vy; 0;  0; 0;  1;  0;  0];
                                        [ay; 0;  0; 0;  0;  0;  0]];
    
    
    //! Matrix which defines how the covariance of the subfilter is expanded to full covariance
    //!
    //! \suggested []
    AVMatrix<float> expansion_matrix_covariance = [[    x; vx; y; vy; ax; ay;];
                                                   [x;  1;  0; 0;  0;  0;  0];
                                                   [vx; 0;  1; 0;  0;  0;  0];
                                                   [ax; 0;  0; 0;  0;  9;  0];
                                                   [y;  0;  0; 1;  0;  0;  0];
                                                   [vy; 0;  0; 0;  1;  0;  0];
                                                   [ay; 0;  0; 0;  0;  0;  9]];
    
    //! Matrix which defines how the innovation of the subfilter is expanded to full size
    //!
    //! \suggested []
    AVMatrix<float> expansion_matrix_innovation = [[    x; vx; y; vy;   ax;   ay;];
                                                   [x;  1;  0; 0;  0;    0;    0];
                                                   [vx; 0;  1; 0;  0;    0;    0];
                                                   [ax; 0;  0; 0;  0; 1.05;    0];
                                                   [y;  0;  0; 1;  0;    0;    0];
                                                   [vy; 0;  0; 0;  1;    0;    0];
                                                   [ay; 0;  0; 0;  0;    0; 1.05]];
    
    //! Matrix which defines how the state and covariance of the subfilter are shrunk to subfilter size
    //!
    //! \suggested []
    AVMatrix<float> shrinking_matrix = [[    x;  vx;  ax;  y;  vy;  ay;];
                                        [x;  1; 0; 0; 0; 0; 0];
                                        [vx; 0; 1; 0; 0; 0; 0];
                                        [y;  0; 0; 0; 1; 0; 0];
                                        [vy; 0; 0; 0; 0; 1; 0]];
    namespace subfilters
    {
        // -------------------------------------------------------------
        namespace kf
        {
            //! Matrix which defines the underlying dynamic model of the subfilter
            //!
            //! \suggested []
            AVMatrix <QString> transition_matrix =
            [[    x; vx;     ax; y; vy;    ay;];
            [x;   1; dt; dt^2/2; 0;  0;      0];
            [vx;  0;  1;     dt; 0;  0;      0];
            [ax;  0;  0;      1; 0;  0;      0];
            [y;   0;  0;      0; 1; dt; dt^2/2];
            [vy;  0;  0;      0; 0;  1;     dt];
            [ay;  0;  0;      0; 0;  0;      1]];
            
            //! Matrix which defines which elements of the state vector shall be used
            //!
            //! \suggested []
            AVMatrix<float> measurement_control_matrix =
            [[   x; vx; ax; y; vy; ay;];
            [x;  1;  0;  0; 0;  0;  0];
            [vx; 0;  0;  0; 0;  0;  0];
            [ax; 0;  0;  0; 0;  0;  0];
            [y;  0;  0;  0; 1;  0;  0];
            [vy; 0;  0;  0; 0;  0;  0];
            [ay; 0;  0;  0; 0;  0;  0]];
        
            //! Matrix which defines how much we trust the prediction
            //!
            //! \suggested []
            AVMatrix <QString> process_noise_matrix =
            [[               x;           vx;           ax;             y;           vy;          ay;];
            [x;  sigma*dt^5/20; sigma*dt^4/8; sigma*dt^3/6;             0;            0;            0];
            [vx;  sigma*dt^4/8; sigma*dt^3/2; sigma*dt^2/2;             0;            0;            0];
            [ax;  sigma*dt^3/6; sigma*dt^2/2;     sigma*dt;             0;            0;            0];
            [y;              0;            0;            0; sigma*dt^5/20; sigma*dt^4/8; sigma*dt^3/6];
            [vy;             0;            0;            0;  sigma*dt^4/8; sigma*dt^3/2; sigma*dt^2/2];
            [ay;             0;            0;            0;  sigma*dt^3/6; sigma*dt^2/2;     sigma*dt]];
        
            //! Matrix which defines how much we trust the measurement
            //!
            //! \suggested []
            AVMatrix<float> measurement_uncertainty_matrix =
            [[   x; vx; ax; y; vy; ay;];
            [x;  1;  0;  0; 0;  0;  0];
            [vx; 0;  1;  0; 0;  0;  0];
            [ax; 0;  0;  1; 0;  0;  0];
            [y;  0;  0;  0; 1;  0;  0];
            [vy; 0;  0;  0; 0;  1;  0];
            [ay; 0;  0;  0; 0;  0;  1]];
         
            //! Matrix which defines how an external input affects the state directly
            //!
            //! \suggested []
            AVMatrix <QString> input_control_matrix =
            [[  ax; ay;];
            [x;  0;  0];
            [vx; 0;  0];
            [ax; 0;  0];
            [y;  0;  0];
            [vy; 0;  0];
            [ay; 0;  0]];
        } // namespace kf
    
        // -------------------------------------------------------------
        namespace kf1
        {
            //! Matrix which defines the underlying dynamic model of the subfilter
            //!
            //! \suggested []
            AVMatrix <QString> transition_matrix =
            [[    x; vx;     ax; y; vy;    ay;];
            [x;   1; dt; dt^2/2; 0;  0;      0];
            [vx;  0;  1;     dt; 0;  0;      0];
            [ax;  0;  0;      1; 0;  0;      0];
            [y;   0;  0;      0; 1; dt; dt^2/2];
            [vy;  0;  0;      0; 0;  1;     dt];
            [ay;  0;  0;      0; 0;  0;      1]];
    
            //! Matrix which defines which elements of the state vector shall be used
            //!
            //! \suggested []
            AVMatrix<float> measurement_control_matrix =
            [[   x; vx; ax; y; vy; ay;];
            [x;  1;  0;  0; 0;  0;  0];
            [vx; 0;  0;  0; 0;  0;  0];
            [ax; 0;  0;  0; 0;  0;  0];
            [y;  0;  0;  0; 1;  0;  0];
            [vy; 0;  0;  0; 0;  0;  0];
            [ay; 0;  0;  0; 0;  0;  0]];
    
            //! Matrix which defines how much we trust the prediction
            //!
            //! \suggested []
            AVMatrix <QString> process_noise_matrix =
            [[               x;           vx;           ax;             y;           vy;          ay;];
            [x;  sigma*dt^5/20; sigma*dt^4/8; sigma*dt^3/6;             0;            0;            0];
            [vx;  sigma*dt^4/8; sigma*dt^3/2; sigma*dt^2/2;             0;            0;            0];
            [ax;  sigma*dt^3/6; sigma*dt^2/2;     sigma*dt;             0;            0;            0];
            [y;              0;            0;            0; sigma*dt^5/20; sigma*dt^4/8; sigma*dt^3/6];
            [vy;             0;            0;            0;  sigma*dt^4/8; sigma*dt^3/2; sigma*dt^2/2];
            [ay;             0;            0;            0;  sigma*dt^3/6; sigma*dt^2/2;     sigma*dt]];
    
            //! Matrix which defines how much we trust the measurement
            //!
            //! \suggested []
            AVMatrix<float> measurement_uncertainty_matrix =
            [[   x; vx; ax; y; vy; ay;];
            [x;  1000;     0;     0;    0;     0;     0];
            [vx;    0;  1000;     0;    0;     0;     0];
            [ax;    0;     0;  1000;    0;     0;     0];
            [y;     0;     0;     0; 1000;     0;     0];
            [vy;    0;     0;     0;    0;  1000;     0];
            [ay;    0;     0;     0;    0;     0;  1000]];
    
            //! Matrix which defines how an external input affects the state directly
            //!
            //! \suggested []
            AVMatrix <QString> input_control_matrix =
            [[  ax; ay;];
            [x;  0;  0];
            [vx; 0;  0];
            [ax; 0;  0];
            [y;  0;  0];
            [vy; 0;  0];
            [ay; 0;  0]];
        } // namespace kf1
    } // namespace subfilters
} // namespace imm_config2_models

// EOF
