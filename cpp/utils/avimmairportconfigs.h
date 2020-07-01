//
// Created by felix on 6/26/20.
//

#ifndef AVIMMAIRPORTCONFIGMAP_H
#define AVIMMAIRPORTCONFIGMAP_H

#include "avimmconfig.h"
#include "avimmconfigparser.h"

// AviBit common includes
#include "avconfig2.h"
#include "avexplicitsingleton.h"

// Struct used to define config data
struct AVIMMConfigData
{
    // Static config data, does not change due to the targets position
    QMap<QString, AVMatrix<QString>> F_map;
    QMap<QString, AVMatrix<QString>> P_map;
    QMap<QString, AVMatrix<QString>> H_map;
    QMap<QString, AVMatrix<QString>> B_map;
    QMap<QString, AVMatrix<QString>> R_map;
    QMap<QString, AVMatrix<QString>> J_map;
    Matrix expansion_matrix;
    Matrix expansion_matrix_covariance;
    Matrix expansion_matrix_innovation;
    Matrix shrinking_matrix;
    Vector initial_mode_probabilities;
    QStringList sub_filter_config_keys;
    
    // dynamic config data, these may change due to the targets position and define the subfilter behaviour
    QMap<QString, AVMatrix<QString>> Q_map;
    Matrix markov_transition_matrix;
    float sigma;
};
// used to define areas
class AVIMMAreaConfig : public AVConfig2
{
    QPolygon m_area;
    QString m_area_name;
    AVIMMConfigData m_config;
    
public:
    explicit AVIMMAreaConfig(const QString& prefix, AVConfig2Container& config);
    virtual ~AVIMMAreaConfig() = default;
    
    DEFINE_ACCESSORS_REF(ConfigData, AVIMMConfigData, m_config);
    DEFINE_ACCESSORS_REF(AreaName, QString, m_area_name);
    
    void createArea(QList<QList<float>> corners);
    // Todo: proper implement this, find suitable algorithm for this.
    bool isInsideArea(const Vector& current_state) const;
};

// This Class is used to hold a map of the airport areas and their corresponding IMM matrices
class AVIMMAirportConfigs: public AVExplicitSingleton<AVIMMAirportConfigs>
{
public:
    explicit AVIMMAirportConfigs();
    ~AVIMMAirportConfigs();
    
    //! Initialise the global configuration data instance
    static AVIMMAirportConfigs& initializeSingleton()
    { return setSingleton(new AVIMMAirportConfigs()); }
    
    //! Answer the class name
    virtual const QString className() const { return QString("AVIMMAirportConfigs"); }
    
    // This function returns a AVIMMConfigData depending on the current position of the target.
    // This is selected from the defined airport map
    AVIMMConfigData getIMMConfigData(const Vector& current_state) const;
    
    DEFINE_ACCESSORS_REF(AirportAreas, QStringList, m_airport_config_areas);
    DEFINE_ACCESSORS_REF(AirportAreaConfigs, QList<AVIMMAreaConfig>, m_airport_configs);
    
private:
    QStringList m_airport_config_areas;
    QList<AVIMMAreaConfig> m_airport_configs;
};

// Class used to configure the areas of the airport. This is needed to establish a map of areas with the according IMM matrices.
class AVIMMAirportAreaConfigContainer : public AVConfig2, public AVExplicitSingleton<AVIMMAirportAreaConfigContainer>
{
public:
    explicit AVIMMAirportAreaConfigContainer();
    virtual ~AVIMMAirportAreaConfigContainer() = default;
    
    //! Initialise the global configuration data instance
    static AVIMMAirportAreaConfigContainer& initializeSingleton()
    { return setSingleton(new AVIMMAirportAreaConfigContainer()); }
    
    //! Answer the class name
    virtual const QString className() const { return QString("AVIMMAirportAreaConfigContainer"); }
    
    DEFINE_ACCESSORS_REF(AiportAreaConfigs, AVConfig2Map<AVIMMAreaConfig>, m_airport_area_configs);
    
private:
    AVConfig2Map<AVIMMAreaConfig> m_airport_area_configs;
};



#endif //AVIMMAIRPORTCONFIGMAP_H
