//
// Created by Felix on 26.05.2020.
//

#ifndef AVIMM_CONFIG_H
#define AVIMM_CONFIG_H
#include "utils/avimmmakros.h"
#include "utils/avimmtypedefs.h"
#include <map>
#include <QString>

// AviBit common includes
#include "avconfig2.h"
#include "avexplicitsingleton.h"

class AVIMMStaticSubfilterConfig : public AVConfig2
{
public:
    explicit AVIMMStaticSubfilterConfig(const QString& prefix, AVConfig2Container& config);
    virtual ~AVIMMStaticSubfilterConfig() = default;
    
    AVMatrix<QString> transition_matrix;
    AVMatrix<QString> covariance_matrix;
    AVMatrix<QString> measurement_control_matrix;
    AVMatrix<QString> input_control_matrix;
    AVMatrix<QString> process_noise_matrix;
    AVMatrix<QString> measurement_uncertainty_matrix;
    AVMatrix<QString> jacobi_matrix;
};

// used to define areas subconfig
class AVIMMDynamicAreaSubConfig : public AVConfig2
{
public:
    AVMatrix<QString> process_noise_matrix;
    explicit AVIMMDynamicAreaSubConfig(const QString& prefix, AVConfig2Container& config);
    virtual ~AVIMMDynamicAreaSubConfig() = default;
};

class AVIMMDynamicAreaConfig : public AVConfig2
{
public:
    explicit AVIMMDynamicAreaConfig(const QString& prefix, AVConfig2Container& config);
    virtual ~AVIMMDynamicAreaConfig() = default;
    
    float sigma;
    Matrix markov_transition_matrix;
    
    AVConfig2Map<AVIMMDynamicAreaSubConfig> area_sub_configs;
};

typedef QMap<QString, FilterType> FilterTypeMap;

class AVIMMStaticConfigContainer : public AVConfig2, public AVExplicitSingleton<AVIMMStaticConfigContainer>
{
public:
    explicit AVIMMStaticConfigContainer();
    virtual ~AVIMMStaticConfigContainer() = default;
    
    //! Initialise the global configuration data instance
    static AVIMMStaticConfigContainer& initializeSingleton()
    { return setSingleton(new AVIMMStaticConfigContainer()); }
    
    //! Answer the class name
    virtual const QString className() const { return QString("AVIMMStaticConfigContainer"); }
    
    static Matrix createUnityMatrix(int size);
    
    QList<FilterType> filter_types;
    QStringList filter_definitions;
    QStringList sub_filter_config_definitions;
    QStringList state_definition;
    Vector mode_probabilities;
    Matrix expansion_matrix;
    Matrix expansion_matrix_covariance;
    Matrix expansion_matrix_innovation;
    Matrix shrinking_matrix;
    
    FilterTypeMap filter_type_map;
    AVConfig2Map<AVIMMStaticSubfilterConfig> filters;
    
    static Matrix convertAVMatrixFloatToEigenMatrix(const AVMatrix<float> &M);
    static Vector convertQListFloatToEigenVector(const QList<float> &L);
private:
    
    QMap<QString, FilterType> m_enum_map;
    void getFilterTypesFromConfig();
    
    friend class TstAVIMMConfigReader;
};


class AVIMMDynamicConfigContainer : public AVConfig2, public AVExplicitSingleton<AVIMMDynamicConfigContainer>
{
public:
    explicit AVIMMDynamicConfigContainer();
    virtual ~AVIMMDynamicConfigContainer() = default;
    
    //! Initialise the global configuration data instance
    static AVIMMDynamicConfigContainer& initializeSingleton()
    { return setSingleton(new AVIMMDynamicConfigContainer()); }
    
    //! Answer the class name
    virtual const QString className() const { return QString("AVIMMDynamicConfigContainer"); }

    AVConfig2Map<AVIMMDynamicAreaConfig> area_filter_configs;
    
    QStringList getAreas() { return area_filter_configs.keys(); }
};
#endif //AVIMM_CONFIG_H
