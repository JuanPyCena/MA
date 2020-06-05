//
// Created by Felix on 26.05.2020.
//

#include "avimmconfig.h"

#define CFGPATH "/home/users/felix/workspace/trunk/svn/avcommon/src5/avimmlib/config/imm_config2_models.cc"
#define CFGFILE "imm_config2_models"

AVIMMConfig::AVIMMConfig(const QString &prefix, AVConfig2Container &config)
    : AVConfig2(prefix, config)
{
    registerParameter("transition_matrix", &transition_matrix,
                      "Matrix which defines the underlying dynamic model of the subfilter").
            setSuggestedValue(AVMatrix<QString>());
    
    registerParameter("measurement_control_matrix", &measurement_control_matrix,
                      "Matrix which defines which elements of the state vector shall be used").
            setSuggestedValue(AVMatrix<QString>());
    
    registerParameter("process_noise_matrix", &process_noise_matrix,
                      "Matrix which defines how much we trust the prediction").
            setSuggestedValue(AVMatrix<QString>());
    
    registerParameter("measurement_uncertainty_matrix", &measurement_uncertainty_matrix,
                      "Matrix which defines how much we trust the measurement").
            setSuggestedValue(AVMatrix<QString>());
    
    registerParameter("input_control_matrix", &input_control_matrix,
                      "Matrix which defines how an external input affects the state directly").
            setSuggestedValue(AVMatrix<QString>());
    
    registerParameter("jacobi_matrix", &jacobi_matrix,
                      "Matrix which defines the linearized system around the state").
            setSuggestedValue(AVMatrix<QString>());
}

AVIMMConfigContainer::AVIMMConfigContainer()
    : AVConfig2(CFGFILE)
{
    m_enum_map["KalmanFilter"]         = KalmanFilter;
    m_enum_map["ExtendedKalmanFilter"] = ExtendedKalmanFilter;
    
    setHelpGroup("AVIMMConfigContainer");
    
    QStringList suggested_filter_definition = {"KalmanFilter", "ExtendedKalmanFilter"};
    registerParameter("filter_types", &filter_definitions,
                      "List which defines which type of subfilters are to be used").
            setSuggestedValue(suggested_filter_definition);
    
    QStringList suggested_sub_filter_config_definition = {"kf", "ekf"};
    registerParameter("sub_filter_config_definition", &sub_filter_config_definitions,
                      "List which defines which subfilters are to be used").
            setSuggestedValue(suggested_sub_filter_config_definition);
    
    QStringList suggested_state_definition = {"pos_x", "vel_x", "pos_y", "vel_y"};
    registerParameter("state_vector", &state_definition,
                      "List which defines the order of the state vector").
            setSuggestedValue(suggested_state_definition);
    
    AVMatrix<float> markov_transition_matrix_helper;
    registerParameter("markov_transition_matrix", &markov_transition_matrix_helper,
                      "List which defines the order of the state vector").
            setSuggestedValue(AVMatrix<float>());
    
    QList<float> mode_probabilities_helper;
    QList<float> suggested_mode_probabilities = {0.5, 0.5};
    registerParameter("mode_probabilities", &mode_probabilities_helper,
                      "List which defines the intial probabilities of the subfilters").
            setSuggestedValue(suggested_mode_probabilities);
    
    AVMatrix<float> expansion_matrix_helper;
    registerParameter("expansion_matrix", &expansion_matrix_helper,
                      "Matrix which defines how the state of the subfilter is expanded to full state").
            setSuggestedValue(AVMatrix<float>());
    
    AVMatrix<float> expansion_matrix_covariance_helper;
    registerParameter("expansion_matrix_covariance", &expansion_matrix_covariance_helper,
                      "Matrix which defines how the covariance of the subfilter is expanded to full covariance").
            setSuggestedValue(AVMatrix<float>());
    
    AVMatrix<float> expansion_matrix_innovation_helper;
    registerParameter("expansion_matrix_innovation", &expansion_matrix_innovation_helper,
                      "Matrix which defines how the innovation of the subfilter is expanded to full size").
            setSuggestedValue(AVMatrix<float>());
    
    AVMatrix<float> shrinking_matrix_helper;
    registerParameter("shrinking_matrix", &shrinking_matrix_helper,
                      "Matrix which defines how the state and covariance of the subfilter are shrunk to subfilter size").
            setSuggestedValue(AVMatrix<float>());
    
    // Read subconfigs
    registerSubconfig(m_prefix + ".subfilters", &filters);
    
    AVConfig2Global::singleton().loadConfigAbsolute(CFGPATH);
    AVConfig2Global::singleton().refreshAllParams();
    
    // Fill list of Filtertype with corresponding enum values to string definition of config
    getFilterTypesFromConfig();
    // Convert float matrices
    markov_transition_matrix    = convertAVMatrixFloatToEigenMatrix(markov_transition_matrix_helper);
    mode_probabilities          = convertQListFloatToEigenVector(mode_probabilities_helper);
    expansion_matrix            = convertAVMatrixFloatToEigenMatrix(expansion_matrix_helper);
    expansion_matrix_covariance = convertAVMatrixFloatToEigenMatrix(expansion_matrix_covariance_helper);
    expansion_matrix_innovation = convertAVMatrixFloatToEigenMatrix(expansion_matrix_innovation_helper);
    shrinking_matrix            = convertAVMatrixFloatToEigenMatrix(shrinking_matrix_helper);
    
    // Create a dict so we know with what type of filter we are dealing with when initializing the IMM
    for (auto &subfilter : filters)
        filter_type_map[subfilter->getConfigMapKey()] = KalmanFilter;
}

void AVIMMConfigContainer::getFilterTypesFromConfig()
{
    for (auto &filter_def : filter_definitions)
        filter_types.push_back(m_enum_map[filter_def]);
}

Matrix AVIMMConfigContainer::convertAVMatrixFloatToEigenMatrix(const AVMatrix<float> &M)
{
    int rows = M.getRows();
    int cols = M.getColumns();
    Matrix converted_matrix(rows, cols);
    
    // Transfer Data element wise
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            converted_matrix(i, j) = static_cast<double>(M.get(i, j));;
    
    return converted_matrix;
}

Vector AVIMMConfigContainer::convertQListFloatToEigenVector(const QList<float> &L)
{
    int num_elems = L.count();
    Vector converted_vector(num_elems, 1);
    int i = 0;
    for (auto& elem : L)
    {
        converted_vector(i) = static_cast<double>(elem);
        i++;
    }
    
    return converted_vector;
}

// This has to be done manually since we don't know the size of the matrices at compile time
Matrix AVIMMConfigContainer::createUnityMatrix(int size)
{
    Matrix m(size, size);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
        {
            if (i == j)
                m(i, j) = 1.0;
            else
                m(i, j) = 0.0;
        }
    return m;
}