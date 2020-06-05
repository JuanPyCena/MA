//
// Created by Felix on 26.05.2020.
//

#ifndef CPP_IMM_CONFIG_H
#define CPP_IMM_CONFIG_H
#include "utils/avimmmakros.h"
#include "utils/avimmtypedefs.h"
#include <map>
#include <QString>

// AviBit common includes
#include "avconfig2.h"
#include "avexplicitsingleton.h"

class AVIMMConfig : public AVConfig2
{
public:
    explicit AVIMMConfig(const QString& prefix, AVConfig2Container& config);
    virtual ~AVIMMConfig() = default;
    
    AVMatrix<QString> transition_matrix;
    AVMatrix<QString> covariance_matrix;
    AVMatrix<QString> measurement_control_matrix;
    AVMatrix<QString> input_control_matrix;
    AVMatrix<QString> process_noise_matrix;
    AVMatrix<QString> measurement_uncertainty_matrix;
    AVMatrix<QString> jacobi_matrix;
};

typedef QMap<QString, FilterType> FilterTypeMap;

class AVIMMConfigContainer : public AVConfig2, public AVExplicitSingleton<AVIMMConfigContainer>
{
public:
    explicit AVIMMConfigContainer();
    virtual ~AVIMMConfigContainer() = default;
    
    //! Initialise the global configuration data instance
    static AVIMMConfigContainer& initializeSingleton()
    { return setSingleton(new AVIMMConfigContainer()); }
    
    //! Answer the class name
    virtual const char *className() const { return "AVIMMConfigContainer"; }
    
    static Matrix createUnityMatrix(int size);
    
    QList<FilterType> filter_types;
    QStringList filter_definitions;
    QStringList sub_filter_config_definitions;
    QStringList state_definition;
    Vector mode_probabilities;
    Matrix markov_transition_matrix;
    Matrix expansion_matrix;
    Matrix expansion_matrix_covariance;
    Matrix expansion_matrix_innovation;
    Matrix shrinking_matrix;
    
    FilterTypeMap filter_type_map;
    AVConfig2Map<AVIMMConfig> filters;

private:
    QMap<QString, FilterType> m_enum_map;

    void getFilterTypesFromConfig();
    static Matrix convertAVMatrixFloatToEigenMatrix(const AVMatrix<float> &M);
    static Vector convertQListFloatToEigenVector(const QList<float> &L);
};
#endif //CPP_IMM_CONFIG_H
