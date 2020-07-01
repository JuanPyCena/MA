//
// Created by felix on 6/26/20.
//

#include "avimmairportconfigs.h"

#define CFGPATH "/home/users/felix/workspace/trunk/svn/avcommon/src5/avimmlib/config/imm_airport_areas.cc"
#define AREACFG "areas"

AVIMMAreaConfig::AVIMMAreaConfig(const QString &prefix, AVConfig2Container &config)
        : AVConfig2(prefix, config)
{
    QList<QList<float>> area_helper;
    registerParameter("area", &area_helper,
                      "List of Vector which define the endpoints of the polygon describing this area").
            setSuggestedValue(QList<QList<float>>());
    
    m_area_name = m_config_map_key;
    
    createArea(area_helper);
    
    auto& avimm_static_config = AVIMMStaticConfigContainer::singleton();
    auto& avimm_dynamic_config = AVIMMDynamicConfigContainer::singleton();
    
    AVIMMConfigData area_config_data;
    area_config_data.markov_transition_matrix    = avimm_dynamic_config.area_filter_configs[m_area_name]->markov_transition_matrix;
    area_config_data.sigma                       = avimm_dynamic_config.area_filter_configs[m_area_name]->sigma;
    area_config_data.expansion_matrix            = avimm_static_config.expansion_matrix;
    area_config_data.expansion_matrix_covariance = avimm_static_config.expansion_matrix_covariance;
    area_config_data.expansion_matrix_innovation = avimm_static_config.expansion_matrix_innovation;
    area_config_data.shrinking_matrix            = avimm_static_config.shrinking_matrix;
    area_config_data.initial_mode_probabilities  = avimm_static_config.mode_probabilities;
    area_config_data.sub_filter_config_keys      = avimm_static_config.sub_filter_config_definitions;
    
    for (const auto& filter_name : avimm_static_config.sub_filter_config_definitions)
    {
        area_config_data.Q_map[filter_name] = avimm_dynamic_config.area_filter_configs[m_area_name]->area_sub_configs[filter_name]->process_noise_matrix;
        area_config_data.F_map[filter_name] = avimm_static_config.filters[filter_name]->transition_matrix;
        area_config_data.P_map[filter_name] = avimm_static_config.filters[filter_name]->covariance_matrix;
        area_config_data.H_map[filter_name] = avimm_static_config.filters[filter_name]->measurement_control_matrix;
        area_config_data.B_map[filter_name] = avimm_static_config.filters[filter_name]->input_control_matrix;
        area_config_data.R_map[filter_name] = avimm_static_config.filters[filter_name]->measurement_uncertainty_matrix;
        area_config_data.J_map[filter_name] = avimm_static_config.filters[filter_name]->jacobi_matrix;
    }
    m_config = area_config_data;
}

//--------------------------------------------------------------------------

void AVIMMAreaConfig::createArea(QList<QList<float> > corners)
{
    QPolygon area;
    for (auto& corner : corners)
    {
        Vector vector_corner = AVIMMStaticConfigContainer::convertQListFloatToEigenVector(corner);
        QPoint corner_point(vector_corner(0), vector_corner(1));
        area << corner_point;
    }
    m_area = area;
}

//--------------------------------------------------------------------------

bool AVIMMAreaConfig::isInsideArea(const Vector &current_state) const
{
    // Get current position from current state
    QPoint current_position;
    current_position.setX(current_state(AVIMMStaticConfigContainer::singleton().state_definition.indexOf("pos_x")));
    current_position.setY(current_state(AVIMMStaticConfigContainer::singleton().state_definition.indexOf("pos_y")));
    
    // Return of the current position is in this area
    return m_area.containsPoint(current_position,Qt::OddEvenFill);
}

//--------------------------------------------------------------------------

AVIMMAirportAreaConfigContainer::AVIMMAirportAreaConfigContainer()
        : AVConfig2(AREACFG)
{
    // Read subconfigs
    registerSubconfig(m_prefix, &m_airport_area_configs);
    
    AVConfig2Global::singleton().loadConfigAbsolute(CFGPATH);
    AVConfig2Global::singleton().refreshAllParams();
}

//--------------------------------------------------------------------------

AVIMMAirportConfigs::AVIMMAirportConfigs()
{
    // Initialize singletons of config containers to read in imm configs
    AVIMMStaticConfigContainer::initializeSingleton();
    AVIMMDynamicConfigContainer::initializeSingleton();
    AVIMMAirportAreaConfigContainer::initializeSingleton();
    AVIMMConfigParser::initializeSingleton();
    for (auto& config : AVIMMAirportAreaConfigContainer::singleton().getAiportAreaConfigs())
    {
        m_airport_config_areas.append(config->getAreaName());
        m_airport_configs.append(*config.get());
    }
}

//--------------------------------------------------------------------------

AVIMMAirportConfigs::~AVIMMAirportConfigs()
{
    // Delete all singletons of the config containers
    AVIMMConfigParser::deleteSingleton();
    AVIMMAirportAreaConfigContainer::deleteSingleton();
    AVIMMDynamicConfigContainer::deleteSingleton();
    AVIMMStaticConfigContainer::deleteSingleton();
}

//--------------------------------------------------------------------------

AVIMMConfigData AVIMMAirportConfigs::getIMMConfigData(const Vector& current_state) const
{
    // Iterate through all areas and find out in which area we are currently in
    // First hit wins
    for (const auto& area : getAirportAreaConfigs())
        if (area.isInsideArea(current_state))
            return area.getConfigData();
        
    return AVIMMConfigData();
    AVLogFatal << "No suitable area found!";
}

//EOF