///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
//////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief   Consolidated SHM variable names
*/

#ifndef __AVSHM_VARNAMES_H__
#define __AVSHM_VARNAMES_H__

// Qt includes
#include <QString>

//! Common value used by all DAQs
const QString AV_SHM_DAQ_INPUT_BLOCK("input_block");

//----- TIME REFERENCE

const QString AV_SHM_TIMEREF_POSTFIX(".Time.Reference");

//----- DAQ_NAVAID_ILS

const QString AV_SHM_DAQ_NAVAID_ILS_STATUS_COUNTER("status_counter");
const QString AV_SHM_DAQ_NAVAID_ILS_STATUS_AT("status_at");
const QString AV_SHM_DAQ_NAVAID_ILS_COMMAND_CONN_STATUS("commandConn");
const QString AV_SHM_DAQ_NAVAID_ILS_COMMAND_ERRORS("commandErrors");
const QString AV_SHM_DAQ_NAVAID_ILS_BLOCK("input_block");

//----- DAQ_EUROCAT

const QString AV_SHM_DAQ_EUROCAT_EUROCAT_COUNTER("eurocat_counter");
const QString AV_SHM_DAQ_EUROCAT_EUROCAT_AT("eurocat_at");
const QString AV_SHM_DAQ_EUROCAT_FDP_OUT_CONN_STATUS("fdpOutConn");
const QString AV_SHM_DAQ_EUROCAT_FDP_IN_CONN_STATUS("fdpInConn");
const QString AV_SHM_DAQ_EUROCAT_EUROCAT_CONN_STATUS("eurocatConn");
const QString AV_SHM_DAQ_EUROCAT_BLOCK("input_block");
const QString AV_SHM_DAQ_EUROCAT_SERIALIO_TIME_LAST_CONNECTION("timeLastConnected");
const QString AV_SHM_DAQ_EUROCAT_SERIALIO_RTS_STATUS("serialRTSStatus");
const QString AV_SHM_DAQ_EUROCAT_SERIALIO_DTR_STATUS("serialDTRStatus");

//----- DAQ_AFTN

const QString AV_SHM_DAQ_AFTN_AFTN_COUNTER("aftn_counter");
const QString AV_SHM_DAQ_AFTN_AFTN_AT("aftn_at");
const QString AV_SHM_DAQ_AFTN_FDP_CONN_STATUS("aftnFDPConn");
const QString AV_SHM_DAQ_AFTN_COM_CONN_STATUS("aftnCOMConn");
const QString AV_SHM_DAQ_AFTN_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;
const QString AV_SHM_DAQ_AFTN_LAST_CHANNEL_CHECK_DT("aftnLastChannelCheckDt");

//----- DAQ_DF

const QString AV_SHM_DAQ_DF_BEARING_COUNTER("bearing_counter");
const QString AV_SHM_DAQ_DF_BEARING_AT("bearing_at");
const QString AV_SHM_DAQ_DF_PARSE_ERROR("parse_error");
const QString AV_SHM_DAQ_DF_CONNECTION_ERROR("connection_error");
const QString AV_SHM_DAQ_DF_DF_ERROR("df_error");
const QString AV_SHM_DAQ_DF_LAST_MESSAGE_RECEIVED_AT("last_message_received_at");
const QString AV_SHM_DAQ_DF_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_LIGHTING

const QString AV_SHM_DAQ_LIGHTING_SB_CHANGE_COUNTER("stopbarchange_counter");
const QString AV_SHM_DAQ_LIGHTING_SB_CHANGE_AT("stopbarchange_at");
const QString AV_SHM_DAQ_LIGHTING_CL_CHANGE_COUNTER("centerlinechange_counter");
const QString AV_SHM_DAQ_LIGHTING_CL_CHANGE_AT("centerlinechange_at");
const QString AV_SHM_DAQ_LIGHTING_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_WUT

const QString AV_SHM_DAQ_WUT_PREFIX                   ("wut");
const QString AV_SHM_DAQ_WUT_CONNECTION_STATUS_POSTFIX("connection_status");
const QString AV_SHM_DAQ_WUT_TIME_LAST_MSG_POSTFIX    ("time_last_msg");
const QString AV_SHM_DAQ_WUT_INPUTPORT_STATUS_POSTFIX ("inputport_status");
const QString AV_SHM_DAQ_WUT_OUTPUTPORT_STATUS_POSTFIX("outputport_status");
const QString AV_SHM_DAQ_WUT_BLOCK                     = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_AFL_LATI

const QString AV_SHM_DAQ_AFL_LATI_STATUS("afl_status");
const QString AV_SHM_DAQ_AFL_LATI_BLOCK                = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_DME_LATI

const QString AV_SHM_DAQ_DME_LATI_TRX1_ON        ("trx1_on");
const QString AV_SHM_DAQ_DME_LATI_TRX1_WARNING   ("trx1_warning");
const QString AV_SHM_DAQ_DME_LATI_TRX1_ALARM     ("trx1_alarm");
const QString AV_SHM_DAQ_DME_LATI_TRX1_ON_ANTENNA("trx1_on_antenna");
const QString AV_SHM_DAQ_DME_LATI_TRX2_ON        ("trx2_on");
const QString AV_SHM_DAQ_DME_LATI_TRX2_WARNING   ("trx2_warning");
const QString AV_SHM_DAQ_DME_LATI_TRX2_ALARM     ("trx2_alarm");
const QString AV_SHM_DAQ_DME_LATI_TRX2_ON_ANTENNA("trx2_on_antenna");
const QString AV_SHM_DAQ_DME_LATI_BLOCK           = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_AWOS_LATI

const QString AV_SHM_DAQ_AWOS_LATI_STATUS        ("awos_status");
const QString AV_SHM_DAQ_AWOS_LATI_BLOCK          = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_ILS_LATI

const QString AV_SHM_DAQ_ILS_LATI_OP_STATUS_CAT1_LED       ("op_status_cat1_led");
const QString AV_SHM_DAQ_ILS_LATI_OP_STATUS_CAT2_LED       ("op_status_cat2_led");
const QString AV_SHM_DAQ_ILS_LATI_OP_STATUS_CAT3_LED       ("op_status_cat3_led");
const QString AV_SHM_DAQ_ILS_LATI_OP_STATUS_LOC_LED        ("op_status_loc_led");
const QString AV_SHM_DAQ_ILS_LATI_LOC_LED_PREFIX           ("loc_led");
const QString AV_SHM_DAQ_ILS_LATI_GLSL_LED_PREFIX          ("glsl_led");
const QString AV_SHM_DAQ_ILS_LATI_COLOCDME_LED_PREFIX      ("colocdme_led");
const QString AV_SHM_DAQ_ILS_LATI_OUTER_MARKER_LED_PREFIX  ("outer_marker_led");
const QString AV_SHM_DAQ_ILS_LATI_MIDDLE_MARKER_LED_PREFIX ("middle_marker_led");
const QString AV_SHM_DAQ_ILS_LATI_INNER_MARKER_LED_PREFIX  ("inner_marker_led");
const QString AV_SHM_DAQ_ILS_LATI_VOR_LED_PREFIX           ("vor_led");
const QString AV_SHM_DAQ_ILS_LATI_VORDME_LED_PREFIX        ("vordme_led");
const QString AV_SHM_DAQ_ILS_LATI_FFM_LED_PREFIX           ("ffm_led");
const QString AV_SHM_DAQ_ILS_LATI_AUX_LED_PREFIX           ("aux_led");
const QString AV_SHM_DAQ_ILS_LATI_INTERLOCK                ("interlock");
const QString AV_SHM_DAQ_ILS_LATI_RWY_ALPHANUMERIC_DISPLAY ("rwy_alphanumeric_display");
const QString AV_SHM_DAQ_ILS_LATI_AUDIO                    ("audio");
const QString AV_SHM_DAQ_ILS_LATI_BLOCK                     = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_POS

const QString AV_SHM_DAQ_POS_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;
const QString AV_SHM_DAQ_POS_INPUT_COUNT("input_count");
const QString AV_SHM_DAQ_POS_INPUT_NAME_PREFIX("input_name_");
const QString AV_SHM_DAQ_POS_INPUT_STATUS_PREFIX("input_status_");
const QString AV_SHM_DAQ_POS_INPUTS_OK("inputs_ok");
const QString AV_SHM_DAQ_POS_OVERALL_MESSAGE_COUNT("overall_message_count");
const QString AV_SHM_DAQ_POS_FILTERED_TARGET_COUNT("filtered_target_count");
const QString AV_SHM_DAQ_POS_UPDATE_CYCLE_MSG_COUNT("update_cycle_msg_count");
const QString AV_SHM_DAQ_POS_SYSTEM_STATUS_MSG_COUNT("system_status_msg_count");
const QString AV_SHM_DAQ_POS_PROCESSED_MLAT_TARGET_COUNT("processed_mlat_target_count");
const QString AV_SHM_DAQ_POS_PROCESSED_ADSB_TARGET_COUNT("processed_adsb_target_count");
const QString AV_SHM_DAQ_POS_PROCESSED_PRIMARY_TARGET_COUNT("processed_primary_target_count");
const QString AV_SHM_DAQ_POS_PROCESSED_SSR_TARGET_COUNT("processed_ssr_target_count");
const QString AV_SHM_DAQ_POS_PROCESSED_TRACKED_TARGET_COUNT("processed_tracked_target_count");
const QString AV_SHM_DAQ_POS_PROCESSED_OTHER_TARGET_COUNT("processed_other_target_count");
const QString AV_SHM_DAQ_POS_TEST_TARGET_MISSING("test_target_missing");
const QString AV_SHM_DAQ_POS_LAST_RECEIVED_MSG_DG("last_received_msg_dt");

//----- SMR PROXY

const QString AV_SHM_SMR_PROXY_SMR_CH1_HW_OK_DT("smr_ch1_hw_ok_dt");
const QString AV_SHM_SMR_PROXY_SMR_CH2_HW_OK_DT("smr_ch2_hw_ok_dt");
const QString AV_SHM_SMR_PROXY_STATE("state");
const QString AV_SHM_SMR_PROXY_PREVIOUS_STATE("previous_state");
const QString AV_SHM_SMR_PROXY_LAST_FULL_STATUS_UPDATE("last_full_status_update");
const QString AV_SHM_SMR_PROXY_RADAR_CHANNEL_INFO_LAST_UPDATED("radar_channel_info_last_updated");

//----- DAQ_SMRU

const QString AV_SHM_DAQ_SMRU_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;
const QString AV_SHM_DAQ_SMRU_OVERALL_IMAGE_COUNT("overall_processed_image_count");

const QString AV_SHM_DAQ_SMRU_INPUT_OVERLOAD("input_overload");
const QString AV_SHM_DAQ_SMRU_FILTER_OVERLOAD("filter_chain_overload");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_OVERLOAD("output_handler_overload");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_OVERLOAD("extractor_overload");

// input and scan converter
const QString AV_SHM_DAQ_SMRU_INPUT_CONNECTED("input_connected");
const QString AV_SHM_DAQ_SMRU_INPUT_TIMEOUT("input_timeout");
const QString AV_SHM_DAQ_SMRU_SCAN_CONVERTER_TIME_LAST_OVERLOAD("scan_converter_last_overload");
const QString AV_SHM_DAQ_SMRU_SCAN_CONVERTER_OVERLOAD_COUNTER("scan_converter_overload_count");
const QString AV_SHM_DAQ_SMRU_SCAN_CONVERTER_TIME_LAST_RESET("scan_converter_last_reset");

// scan converter 3 (PCI card);
const QString AV_SHM_DAQ_SMRU_PCI_CARD_ACP_ERROR    ("pci_card_acp_error");
const QString AV_SHM_DAQ_SMRU_PCI_CARD_ARP_ERROR    ("pci_card_arp_error");
const QString AV_SHM_DAQ_SMRU_PCI_CARD_TRIGGER_ERROR("pci_card_trigger_error");

// filters
const QString AV_SHM_DAQ_SMRU_FILTER_COUNT("filter_count");
const QString AV_SHM_DAQ_SMRU_FILTER_NAME_PREFIX("filter_name_");
const QString AV_SHM_DAQ_SMRU_FILTER_TIME_LAST_OVERLOAD_PREFIX("filter_time_last_overload_");
const QString AV_SHM_DAQ_SMRU_FILTER_OVERLOAD_COUNTER_PREFIX("filter_overload_count_");
const QString AV_SHM_DAQ_SMRU_FILTER_TIME_LAST_RESET_PREFIX("filter_time_last_reset_");

const QString AV_SHM_DAQ_SMRU_APM_FILTER_CHAIN_INIT_ERROR("apm_filter_initialization_error");

// plot extractor
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_NUM_EXTRACTED_TARGETS("extractor_extracted_targets");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_TIME_LAST_REPORT("extractor_time_last_report");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_TIME_LAST_RESET("extractor_time_last_reset");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_TARGET_OVERLOAD("extractor_target_overload");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_OVERLOAD_COUNTER("extractor_overload_count");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_TIME_LAST_OVERLOAD("extractor_time_last_overload");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_CURR_NUM_PLOTS_PER_SEC("extractor_curr_targets_per_sec");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_AVG_NUM_PLOTS_PER_SEC("extractor_avg_targets_per_sec");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_MAX_NUM_PLOTS_PREV_HOUR("extractor_max_num_targets_prev_hour");
const QString AV_SHM_DAQ_SMRU_PLOT_EXTRACTOR_MAX_NUM_PLOTS_CURR_HOUR("extractor_max_num_targets_curr_hour");

// output handler
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_COUNT("output_handler_count");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_NAME_PREFIX("output_handler_name_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_TIME_LAST_OVERLOAD_PREFIX \
       ("output_handler_time_last_overload_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_OVERLOAD_COUNTER_PREFIX \
       ("output_handler_overload_count_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_TIME_LAST_RESET_PREFIX \
       ("output_handler_time_last_reset_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_NUM_PROCESSED_IMAGES_PREFIX \
       ("output_handler_image_count_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_LAST_IMAGE_PROCESSED_PREFIX \
       ("output_handler_time_last_image_processed_");
const QString AV_SHM_DAQ_SMRU_OUTPUT_HANDLER_QUALITY_INDICATOR_PREFIX \
       ("output_handler_quality_indicator_");

// active radar channel
const QString AV_SHM_DAQ_SMRU_ACTIVE_CHANNEL("active_radar_channel");

//----- DAQ_VAS

const QString AV_SHM_DAQ_VAS_CONN_STATUS("vasConn");
const QString AV_SHM_DAQ_VAS_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_AUSTRAS

const QString AV_SHM_DAQ_AUSTRAS_CONN_STATUS("vasConn");

const QString AV_SHM_DAQ_AUSTRAS_FP("fp");
const QString AV_SHM_DAQ_AUSTRAS_STOPBAR("stopbar");
const QString AV_SHM_DAQ_AUSTRAS_SSR("ssr");
const QString AV_SHM_DAQ_AUSTRAS_WEATHER("weather");
const QString AV_SHM_DAQ_AUSTRAS_ML("ml");
const QString AV_SHM_DAQ_AUSTRAS_SMR("smr");

const QString AV_SHM_DAQ_AUSTRAS_FP_BLOCK("fp_block");
const QString AV_SHM_DAQ_AUSTRAS_STOPBAR_BLOCK("stopbar_block");
const QString AV_SHM_DAQ_AUSTRAS_WEATHER_BLOCK("weather_block");
const QString AV_SHM_DAQ_AUSTRAS_SSR_BLOCK("ssr_block");
const QString AV_SHM_DAQ_AUSTRAS_ML_BLOCK("ml_block");
const QString AV_SHM_DAQ_AUSTRAS_SMR_BLOCK("smr_block");

//----- DAQ_WEATHER

const QString AV_SHM_DAQ_WEATHER_COUNTER("wm_counter");
const QString AV_SHM_DAQ_WEATHER_AT("wm_at");
const QString AV_SHM_DAQ_WEATHER_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_ALS

const QString AV_SHM_DAQ_ALS_ALS_CONNECTION_STATUS ("als_connection_status");
const QString AV_SHM_DAQ_ALS_ALS_ONLINE_STATUS ("als_online_status");

//----- DAQ_ATIS

const QString AV_SHM_DAQ_ATIS_EXT_INP_STATUS("atis_external_input_status");
const QString AV_SHM_DAQ_ATIS_EXT_INP_KEEP_ALIVE_STATUS("atis_external_input_keep_alive_status");
const QString AV_SHM_DAQ_ATIS_INT_INP_STATUS("atis_internal_input_status");
const QString AV_SHM_DAQ_ATIS_EXT_OUTP_STATUS("atis_external_output_status");
const QString AV_SHM_DAQ_ATIS_INT_OUTP_STATUS("atis_internal_output_status");
const QString AV_SHM_DAQ_ATIS_SLAVE_OUTP_STATUS("atis_slave_output_status");
const QString AV_SHM_DAQ_ATIS_LAST_KEEP_RECEIVED_AT("atis_keep_alive_received_at");
const QString AV_SHM_DAQ_ATIS_LAST_ATIS_RECEIVED_AT("atis_report_received_at");

//----- DAQ_AWOS

const QString AV_SHM_DAQ_AWOS_DSW_CONN_STATUS("awos_dsw_conn_status");
const QString AV_SHM_DAQ_AWOS_DSW_TIME_LAST_APCFG_MSG("awos_dsw_last_apcfg_msg");
const QString AV_SHM_DAQ_AWOS_EXT_AWOS_TIME_LAST_MSG("awos_external_connection_status");
const QString AV_SHM_DAQ_AWOS_RELAY_CONN_STATUS("awos_relay_connection_status");
const QString AV_SHM_DAQ_AWOS_RELAY_TIME_LAST_MSG("awos_relay_time_last_msg");
const QString AV_SHM_DAQ_AWOS_RELAY_SENT_OK("awos_relay_msg_send_ok");
const QString AV_SHM_DAQ_AWOS_RELAY_ERROR_HISTORY("awos_relay_error_history");
const QString AV_SHM_DAQ_AWOS_RELAY_HAS_ERROR_CURRENTLY("awos_relay_has_internal_error");

//----- DAQ_SOS SHM
const QString AV_SHM_DAQ_SOS_IM_CONNECTED("im_connected");
const QString AV_SHM_DAQ_SOS_SDK_CONNECTED("sdk_connected_");

//----- DAQ_ALS SHM
const QString AV_SHM_DAQ_ALS_IM_CONNECTED("im_connected");

//----- DAQ_TMS SHM
const QString AV_SHM_DAQ_TMS_IM_CONNECTED("im_connected");

//----- DAQ_ACPOS SHM
const QString AV_SHM_DAQ_ACPOS_SOS_CONNECTOR_INFO("sos_connector_info");

//----- DAQ_MACH SHM

const QString AV_SHM_MACH_CONN_STATUS("machConn");
const QString AV_SHM_MACH_LAST_MSG_TIME("machLastMsgTime");
const QString AV_SHM_MACH_MSG_COUNT("machMsgCount");
const QString AV_SHM_MACH_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ_ISP SHM

const QString AV_SHM_ISP_CONN_STATUS("ispConn");

//----- DFUSE2

const QString AV_SHM_DFUSE2_OVERLOAD_STATUS("overload");
const QString AV_SHM_DFUSE2_DEGRADED_STATUS("degraded");
const QString AV_SHM_DFUSE2_RECEIVED_TARGET_COUNT("received_target_count");
const QString AV_SHM_DFUSE2_DISTRIBUTED_TARGET_COUNT("distributed_target_count");
const QString AV_SHM_DFUSE2_ASTERIX_DIST_ERROR("asterix_dist_error");
const QString AV_SHM_DFUSE2_INPUT_COUNT("input_count");
const QString AV_SHM_DFUSE2_SENSORGROUP_NAME_PREFIX("sensorgroup_name_");
const QString AV_SHM_DFUSE2_SENSORGROUP_STATUS_PREFIX("sensorgroup_status_");
const QString AV_SHM_DFUSE2_LAST_TRANSMITTED_TARGET_REPORT("LastTransmittedTargetReportDt");

//----- FDP

const QString AV_SHM_FDP_CMD_PORT_NR_CLIENTS("NCmdConn");
const QString AV_SHM_FDP_DIST_PORT_NR_CLIENTS("NDistConn");
const QString AV_SHM_FDP_LAST_TRANSMITTED_FP_REPORT("LastTransmittedFPReportDt");
const QString AV_SHM_FDP_AIRPORT_CONFIG_ARRIVAL_ATIS("ApCfgArrivalAtis");
const QString AV_SHM_FDP_AIRPORT_CONFIG_DEPARTURE_ATIS("ApCfgDepartureAtis");
const QString AV_SHM_FDP_AIRPORT_CONFIG_MAIN_ARRIVAL_RWY("ApCfgMainArrivalRunway");
const QString AV_SHM_FDP_AIRPORT_CONFIG_MAIN_DEPARTURE_RWY("ApCfgMainDepartureRunway");

//----- SIM_SMR

const QString AV_SHM_SIM_SMR_IMG_COUNTER("smr_img_counter");
const QString AV_SHM_SIM_SMR_IMG_AT("smr_img_at");

//----- STATC2

const QString AV_SHM_TERMA_5000_LAST_UPDATE_DT("terma5000_last_update_dt");

//----- ALARMC

const QString AV_SHM_ALARMC_RWY_INCURSION_COUNTER("rwyinc_counter");
const QString AV_SHM_ALARMC_TAXIWAY_COUNTER("taxiway_counter");
const QString AV_SHM_ALARMC_APM_COUNTER("apm_counter");
const QString AV_SHM_ALARMC_STOPBAR_CROSSING_COUNTER("stopbar_crossing_counter");
const QString AV_SHM_ALARMC_MSAW_COUNTER("msaw_counter");
const QString AV_SHM_ALARMC_COLLISION_COUNTER("coll_counter");
const QString AV_SHM_ALARMC_ROUTE_COUNTER("route_counter");
const QString AV_SHM_ALARMC_MARCAS_COUNTER("marcas_counter");
const QString AV_SHM_ALARMC_ALARM_AT("alarm_at");
const QString AV_SHM_ALARMC_PROCESSING_STATE("alarmc_processing_state");

//----- DSWITCH

const QString AV_SHM_DSWITCH_SERVER_STATE("serverState");
const QString AV_SHM_DSWITCH_SIC_STATE("sicState");
const QString AV_SHM_DSWITCH_FORCE_STATE("forceState");
const QString AV_SHM_DSWITCH_CLIENT_COUNT_POSTFIX("ClientCount");

//----- FILESRV

const QString AV_SHM_FILESRV_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- SMRFUSION

const QString AV_SHM_SMRFUSION_NUMBER_SENSORS("number_sensors");
const QString AV_SHM_SMRFUSION_IMAGE_COUNTER("image_counter");
const QString AV_SHM_SMRFUSION_SENSOR_NAME_PREFIX("sensor_name_");
const QString AV_SHM_SMRFUSION_SENSOR_STATUS_PREFIX("sensor_status_");
const QString AV_SHM_SMRFUSION_SENSOR_IMAGE_COUNTER_PREFIX("sensor_image_counter_");
const QString AV_SHM_SMRFUSION_SENSOR_UPDATE_AT_PREFIX("sensor_update_at_");
const QString AV_SHM_SMRFUSION_SENSOR_BLOCK_PREFIX("sensor_input_block_");

//----- DAQ_CMD

const QString AV_SHM_DAQ_CMD_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;
const QString AV_SHM_DAQ_CMD_OVERALL_REQUEST_COUNT("overall_request_count");
const QString AV_SHM_DAQ_CMD_PROCESSED_REQUEST_COUNT("processed_request_count");

//----- DIFLIS

const QString AV_SHM_DCORE_DCL_READY("dclReady");

//----- DECLOS X25IO

const QString AV_SHM_X25IO_LINK_STATE("linkState");
const QString AV_SHM_X25IO_LINK_ENABLED("linkEnabled");

//----- DECLOS DCLCORECCC

const QString AV_SHM_DCLCORE_SERVER_UP("serverUp");
const QString AV_SHM_DCLCORE_SERVER_MODE("serverMode");
const QString AV_SHM_DCLCORE_CDA_SUCCESSES("cdaSuccesses");
const QString AV_SHM_DCLCORE_CDA_FAILURES("cdaFailures");
const QString AV_SHM_DCLCORE_CDA_CRITICAL("cdaCritical");
const QString AV_SHM_DCLCORE_FDP_LINK_UP("fdpLinkUp");
const QString AV_SHM_DCLCORE_FDP_LINK_ENABLED("fdpLinkEnabled");
const QString AV_SHM_DCLCORE_ATIS_LINK_UP("atisLinkUp");
const QString AV_SHM_DCLCORE_ATIS_LINK_ENABLED("atisLinkEnabled");
const QString AV_SHM_DCLCORE_AVINET_LINK_STATE("avinetLinkState");

//----- ASTOS HMI
const QString AV_SHM_ASTOS_HMI_LAST_RECV_MSG_DT("lastReceivedMsgDt");
const QString AV_SHM_ASTOS_HMI_TARGET_LATENCY_LAST_UPDATE_DT("TargetLatencyLastUpdateDt");
const QString AV_SHM_ASTOS_HMI_TARGET_LATENCY_MIN_MS("TargetLatencyMinMs");
const QString AV_SHM_ASTOS_HMI_TARGET_LATENCY_MAX_MS("TargetLatencyMaxMs");
const QString AV_SHM_ASTOS_HMI_TARGET_LATENCY_MEAN_MS("TargetLatencyMeanMs");
const QString AV_SHM_ASTOS_HMI_TARGET_LATENCY_COUNT("TargetLatencyCount");
const QString AV_SHM_ASTOS_HMI_IMG_PATCH_LATENCY_LAST_UPDATE_DT("ImagePatchLatencyLastUpdateDt");
const QString AV_SHM_ASTOS_HMI_IMG_PATCH_LATENCY_MIN_MS("ImagePatchLatencyMinMs");
const QString AV_SHM_ASTOS_HMI_IMG_PATCH_LATENCY_MAX_MS("ImagePatchLatencyMaxMs");
const QString AV_SHM_ASTOS_HMI_IMG_PATCH_LATENCY_MEAN_MS("ImagePatchLatencyMeanMs");
const QString AV_SHM_ASTOS_HMI_IMG_PATCH_LATENCY_COUNT("ImagePatchLatencyCount");

//----- Monitoring and Control
const QString AV_SHM_MAC_LAST_SENT_CTRL_REP_DT("lastSentControlReportDt");
const QString AV_SHM_MAC_LAST_RCVD_REP_DT("lastReceivedReportDt");
const QString AV_SHM_MAC_LAST_RCVD_STAT_REP_DT("lastReceivedStatusReportDt");
const QString AV_SHM_MAC_LAST_RCVD_ALARM_REP_DT("lastReceivedStatusReportDt");
const QString AV_SHM_MAC_LAST_RCVD_DIAG_REP_DT("lastReceivedAlarmReportDt");
const QString AV_SHM_MAC_NR_SENT_CTRL_REP("nrSentControlReports");
const QString AV_SHM_MAC_NR_RCVD_STAT_REP("nrReceivedStatusReports");
const QString AV_SHM_MAC_NR_RCVD_ALARM_REP("nrReceivedAlarmReports");
const QString AV_SHM_MAC_NR_RCVD_DIAG_REP("nrReceivedDiagnosticReports");

//----- ARCHRAW

const QString AV_SHM_ARCHRAW_LAST_RECEIVED_PACKET("LastReceivedPacketDt");

//----- HDPS

const QString AV_SHM_HDPS_EXECUTOR_STATE("processState");
const QString AV_SHM_HDPS_EXECUTOR_ACTIVE_PLOT_PROCESSORS("activePlotProcessors");
const QString AV_SHM_HDPS_EXECUTOR_ACTIVE_TVE_PROCESSORS("activeTveProcessors");
const QString AV_SHM_HDPS_EXECUTOR_KALMAN_RATE_FAILURE_COUNT("kalmanRateFailureCount");

const QString AV_SHM_HDPS_PLOT_INTF_STATE("processState");
const QString AV_SHM_PLOT_INTF_GO_STATUS("goStatus");

const QString AV_SHM_HDPS_MET_INTF_STATE("processState");
const QString AV_SHM_HDPS_MET_INTF_FAILURE_COUNT("failureCount");

const QString AV_SHM_HDPS_MET_IMPORT_STATE("processState");
const QString AV_SHM_HDPS_MET_IMPORT_FAILURE_COUNT("failureCount");

//----- DAQ INDRA

const QString AV_SHM_DAQ_FDP_INDRA_STATUS("indraConn");
const QString AV_SHM_DAQ_FDP_INDRA_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- SDDC2

const QString AV_SHM_SDDC2_INPUT_OK("input_ok");
const QString AV_SHM_SDDC2_INPUT_DATA_FORMAT_OK("input_data_format_ok");
const QString AV_SHM_SDDC2_OVERALL_MESSAGE_COUNT("overall_message_count");
const QString AV_SHM_SDDC2_LAST_RECEIVED_MSG_DT("last_received_msg_dt");
const QString AV_SHM_SDDC2_BLOCK = AV_SHM_DAQ_INPUT_BLOCK;

//----- DAQ ISC BULLETIN

const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_RECEIVED_SNOWTAM("last_snowtam");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_RECEIVED_SNOWTAM_AT("last_snowtam_at");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_SNOWTAM_ERROR("last_snowtam_error");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_SNOWTAM_ERROR_AT("last_snowtam_error_at");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_REQUEST_SENT("last_request_sent");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_REQUEST_SENT_AT("last_request_sent_at");
const QString AV_SHM_DAQ_ISC_BULLETIN_LAST_NON_PARSE("last_non_parseable");
const QString AV_SHM_DAQ_ISC_BULLETIN_TIMEOUT("bulletin_timeout");
const QString AV_SHM_DAQ_ISC_BULLETIN_TIMEOUT_AT("bulletin_timeout_at");
const QString AV_SHM_DAQ_ISC_BULLETIN_IM_CONNECTED("im_connected");
const QString AV_SHM_DAQ_ISC_BULLETIN_IM_AT("im_at");

//----- DAQ ISC VASADD

const QString AV_SHM_DAQ_ISC_VASADD_IM_CONNECTED("im_connected");
const QString AV_SHM_DAQ_ISC_VASADD_IM_AT("im_at");

//----- DAQ EGS IODE
const QString AV_SHM_DAQ_EGS_IODE_FDP_CONNECTED("fdp_connected");
const QString AV_SHM_DAQ_EGS_IODE_IM_CONNECTED("im_connected");
const QString AV_SHM_DAQ_EGS_IODE_IM_AT("im_at");
const QString AV_SHM_DAQ_EGS_IODE_FDP_AT("fdp_at");

//----- DAQ OLDI
const QString AV_SHM_DAQ_OLDI_FDP_CONNECTED("fdp_connected");
const QString AV_SHM_DAQ_OLDI_FDP_AT("fdp_at");

//----- DAQ AMHS
const QString AV_SHM_DAQ_AMHS_FDP_CONNECTED("fdp_connected");
const QString AV_SHM_DAQ_AMHS_FDP_AT("fdp_at");

//----- AFTN GWT
const QString AV_SHM_AFTN_GWT_LAST_LINK_ERROR("aftn_gwt_last_link_error");
const QString AV_SHM_AFTN_GWT_FORCE_LINK("aftn_gwt_force_link");

//----- IO_APM

const QString AV_SHM_IO_APM_AFTN_CONNECTED("apm_aftn_connected");
const QString AV_SHM_IO_APM_LAST_LINK_ERROR("apm_last_link_error");
const QString AV_SHM_IO_APM_FDP_CONNECTED("apm_fdp_connected");
const QString AV_SHM_IO_APM_FDP_AT("apm_fdp_at");
const QString AV_SHM_IO_APM_COUNTER("apm_counter");
const QString AV_SHM_IO_APM_AT("apm_at");
const QString AV_SHM_IO_APM_LAST_MSG("apm_last_msg");
const QString AV_SHM_IO_APM_LAST_CHANNEL_CHECK_DT("aftn_channel_check_at");
const QString AV_SHM_IO_APM_BLOCK("input_block");

//----- IO_ADD

const QString AV_SHM_IO_ADD_AFTN_CONNECTED("add_aftn_connected");
const QString AV_SHM_IO_ADD_LAST_LINK_ERROR("add_last_link_error");
const QString AV_SHM_IO_ADD_FDP_CONNECTED("add_fdp_connected");
const QString AV_SHM_IO_ADD_FDP_AT("add_fdp_at");
const QString AV_SHM_IO_ADD_COUNTER("add_counter");
const QString AV_SHM_IO_ADD_AT("add_at");
const QString AV_SHM_IO_ADD_LAST_MSG("add_last_msg");
const QString AV_SHM_IO_ADD_LAST_CHANNEL_CHECK_DT("aftn_channel_check_at");
const QString AV_SHM_IO_ADD_BLOCK("input_block");

//----- DAQ_ALS_LUCEBIT

const QString AV_SHM_DAQ_ALS_LUCEBIT_CONNECTION_STATUS ("als_lucebit_connection_status");
const QString AV_SHM_DAQ_ALS_LUCEBIT_PLC_STATUS ("als_lucebit_plc_status");

//----- IMS

const QString AV_SHM_IMS_WEATHER_METAR("lastMetar");
const QString AV_SHM_IMS_WEATHER_SPECI("lastSpeci");
const QString AV_SHM_IMS_WEATHER_TAF  ("lastTaf");
const QString AV_SHM_IMS_WEATHER_ATIS ("lastAtis");
const QString AV_SHM_IMS_WIND         ("lastWind");

//----- IM2SERVER
const QString AV_SHM_IM2SERVER_NUM_CONNECTED_CLIENTS("numConnectedClients");

//----- INFOMAX DAQ BASE LOGIC
const QString AV_SHM_INFOMAX_SERVER_CONNECTION_STATUS("infomax_server_connection_status");

//----- IMPROXY_FDP
const QString AV_SHM_IMPROXY_FDP_FDP_CMD_CONNECTION_STATUS("fdp_command_connection_status");
const QString AV_SHM_IMPROXY_FDP_FDP_DIST_CONNECTION_STATUS("fdp_dist_connection_status");

//----- IMPROXY_AFTN
const QString AV_SHM_IMPROXY_AFTN_DAQ_REPORT_DIST_CONNECTION_STATUS("daq_report_connection_status");
const QString AV_SHM_IMPROXY_AFTN_DAQ_RAW_AFTN_MESSAGE_DIST_CONNECTION_STATUS("daq_raw_aftn_message_connection_status");

//----- IMPROXY_AWOS
const QString AV_SHM_IMPROXY_AWOS_DAQ_DIST_CONNECTION_STATUS("daq_connection_status");

//----- IMPROXY_NAVAID
const QString AV_SHM_IMPROXY_NAVAID_DAQ_DIST_CONNECTION_STATUS("daq_connection_status");
const QString AV_SHM_IMPROXY_NAVAID_DAQ_ILS_DIST_CONNECTION_STATUS("daq_connection_status");
const QString AV_SHM_IMPROXY_NAVAID_DAQ_ILS_COMMAND_CONNECTION_STATUS("daq_connection_status");

//----- IMPROXY_WUT
const QString AV_SHM_IMPROXY_WUT_DAQ_WUT_DIST_CONNECTION_STATUS("daq_wut_dist_connection_status");
const QString AV_SHM_IMPROXY_WUT_DAQ_WUT_COMMAND_CONNECTION_STATUS("daq_wut_command_connection_status");

//----- STATCOLLECTOR

const QString AV_SHM_STATCOLLECTOR_MSG_COUNTER("statcollector_msg_counter");
const QString AV_SHM_STATCOLLECTOR_MSG_AT("statcollector_msg_at");
const QString AV_SHM_STATCOLLECTOR_DB_ACCESS_STATE("statcollector_db_access_state");
const QString AV_SHM_STATCOLLECTOR_DB_ERR_TYPE("statcollector_db_err_type");
const QString AV_SHM_STATCOLLECTOR_NUM_CLIENTS_TARGET("statcollector_num_clients_target");
const QString AV_SHM_STATCOLLECTOR_NUM_CLIENTS_ACTUAL("statcollector_num_clients_actual");

//----- DAQ_GWY_ROMATSA

//! Indicates if the DAQ is connected to the command input port of the AviBit FDP2
const QString AV_SHM_DAQ_GWY_ROMATSA_FDP_OUT_CONNECTION_STATUS("fdpOutConnectionStatus");
//! Indicates if the AviBit FDP2 is connected to the command input port of the DAQ
const QString AV_SHM_DAQ_GWY_ROMATSA_FDP_IN_CONNECTION_STATUS("fdpInConnectionStatus");
//! Indicates if the TCP/IP connection between the DAQ and the gateway is up
const QString AV_SHM_DAQ_GWY_ROMATSA_GWY_COM_STATUS("gwyCommunicationStatus");
//! Indicates if the DAQ is identified at the gateway
const QString AV_SHM_DAQ_GWY_ROMATSA_GWY_IDENTIFICATION_STATUS("gwyIdentificationStatus");
//! Indicates if the connection to the database is up
const QString AV_SHM_DAQ_GWY_ROMATSA_DB_CONNECTION_STATUS("DBConnectionStatus");
const QString AV_SHM_DAQ_GWY_ROMATSA_BLOCK("input_block");

//----- AVARCHINDEXER
//! Indicates the name of the last file that was indexed
const QString AV_SHM_AVARCHINDEXER_LAST_INDEXED_FILENAME("archindexer_last_indexed_filename");
//! Indicates the percent of the current file analysis done
const QString AV_SHM_AVARCHINDEXER_PERCENT_DONE("archindexer_percent_done");
//! Indicates the last time the index file was updated/written
const QString AV_SHM_AVARCHINDEXER_INDEX_FILE_UPDATED_DT("archindexer_index_file_updated_dt");
//! Indicates whether the indexer is currently indexing a file or not
const QString AV_SHM_AVARCHINDEXER_IS_CURRENTLY_INDEXING("archindexer_is_indexing");


//----- DAQ_STAND_CGK
const QString AV_SHM_DAQ_STAND_CGK_DATA_READ_STATUS("dataReadStatus");

//----- DAQ_BASE_LOGIC

const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_LAST_RECEIVED_MESSAGE("external_connector_last_msg");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_LAST_RECEIVED_MESSAGE_AT("external_connector_last_msg_at");

const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_LAST_SENT_MESSAGE("external_connector_last_sent_msg");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_LAST_SENT_MESSAGE_AT("external_connector_last_sent_msg_at");

const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_INFO ("external_connector_info");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_ERROR("external_connector_error");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_HOST ("external_connector_host");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_PORT ("external_connector_port");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_LINK_CONTROL   ("external_link_control");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_LAST_HEARTBEAT_AT("external_connector_last_heartbeat_at");
const QString AV_SHM_DAQ_BASE_LOGIC_EXTERNAL_CONNECTOR_CONNECTED_PEERS("external_connector_connected_peers");
const QString AV_SHM_DAQ_BASE_LOGIC_DATALINK                ("datalink");

//----- DAQ_DGS_LOGIC
const QString AV_SHM_DAQ_DGS_RECEIVED_TARGET_MSG_COUNTER ("received_target_message_counter");
const QString AV_SHM_DAQ_DGS_RECEIVED_AVERAGE_TARGET_MSG_COUNTER ("received_average_target_message_counter");
const QString AV_SHM_DAQ_DGS_DROPPED_TARGET_MSG_COUNTER ("dropped_target_message_counter");
const QString AV_SHM_DAQ_DGS_HEARTBEAT_UPDATES ("hearbeat_updates");

//----- DAQ_RADAR_CHANNEL
const QString AV_SHM_DAQ_RADAR_CHANNEL_STATUS_CHANNEL("status_channel");
const QString AV_SHM_DAQ_RADAR_CHANNEL_VALUE_CHANNEL("value_channel");
const QString AV_SHM_DAQ_RADAR_CHANNEL_WUT_HAS_ERRORS("wut_has_errors");

//----- AMTP

// general process parameters
const QString AV_SHM_AMTP_EXEC           ("exec");
const QString AV_SHM_AMTP_NOGO           ("nogo");
const QString AV_SHM_AMTP_OVERLOAD       ("overload");

const QString AV_SHM_AMTP_RU_NOGO_PREFIX              ("ru");
const QString AV_SHM_AMTP_RU_RX_NOGO_SUFFIX           ("_rx_nogo");
const QString AV_SHM_AMTP_RU_TX_NOGO_SUFFIX           ("_tx_nogo");
const QString AV_SHM_AMTP_RU_TX_READY_FOC_CHECK_SUFFIX("_tx_ready_for_check");

const QString AV_SHM_AMTP_REF_TRANSP_NOGO_PREFIX      ("rt_");
const QString AV_SHM_AMTP_REF_TRANSP_NOGO_SUFFIX      ("_nogo");

const QString AV_SHM_AMTP_TEST_TARGET_FAILURE         ("test_target_failure");
const QString AV_SHM_AMTP_NOGO_EXPLAINATION           ("nogo_explaination");
const QString AV_SHM_AMTP_OVERLOAD_EXPLAINATION       ("overload_explaination");

// input handlers
const QString AV_SHM_AMTP_INPUT_PREFIX                       ("input_ru");
const QString AV_SHM_AMTP_INPUT_HEARTBEAT_SUFFIX             ("_heartbeat");
const QString AV_SHM_AMTP_INPUT_TIME_LAST_MSG_RCVD_SUFFIX    ("_time_last_msg_rcvd");
const QString AV_SHM_AMTP_INPUT_NUM_MSGS_PER_S_SUFFIX        ("_num_msg_per_second");

// ru controllers
const QString AV_SHM_AMTP_RU_CNT_PREFIX                      ("ru_control_ru");
const QString AV_SHM_AMTP_RU_CNT_HEARTBEAT_SUFFIX            ("_heartbeat");
const QString AV_SHM_AMTP_RU_CNT_NUM_INT_PER_S_SUFFIX        ("_num_interrogations_per_second");
const QString AV_SHM_AMTP_RU_CNT_CONNECTION_OK_SUFFIX        ("_connection_ok");
const QString AV_SHM_AMTP_RU_CNT_TX_MSG_SENDING_SUCC_SUFFIX  ("_tx_msg_sending_success");

// synchronization
const QString AV_SHM_AMTP_SYNC_ALL_RUS_SYNCHED                ("sync_all_rus_synchronized");
const QString AV_SHM_AMTP_SYNC_NUM_MSG_SYNCHED_PER_S          ("sync_num_msgs_synchronized_per_second");
const QString AV_SHM_AMTP_SYNC_NUM_SYNC_MSGS_PROCESSED_PER_S  ("sync_num_sync_msgs_processed_per_second");
const QString AV_SHM_AMTP_SYNC_OVERLOAD                       ("sync_overload");
const QString AV_SHM_AMTP_SYNC_PER_RU_PREFIX                  ("sync_ru");
const QString AV_SHM_AMTP_SYNC_PER_RU_SYNCHRONIZED_SUFFIX     ("_synchronized");
const QString AV_SHM_AMTP_SYNC_HEARTBEAT                      ("sync_heartbeat");

// mapper
const QString AV_SHM_AMTP_MAPPER_HEARTBEAT                   ("mapper_heartbeat");
const QString AV_SHM_AMTP_MAPPER_NUM_MEAS_PER_S              ("mapper_num_meas_processed_per_second");
const QString AV_SHM_AMTP_MAPPER_NUM_TUPLES_PER_S            ("mapper_num_tuples_extracted_per_second");
const QString AV_SHM_AMTP_MAPPER_NUM_BUFFERED_MEAS           ("mapper_num_buffered_meas");
const QString AV_SHM_AMTP_MAPPER_OVERLOAD                    ("mapper_overload");

// calculator
const QString AV_SHM_AMTP_CALC_PREFIX                ("calc_");
const QString AV_SHM_AMTP_CALC_HEARTBEAT_SUFFIX      ("_heartbeat");
const QString AV_SHM_AMTP_CALC_NUM_CALC_PER_S_SUFFIX ("_num_calculations_per_second");
const QString AV_SHM_AMTP_CALC_OVERLOAD_SUFFIX       ("_overload");

const QString AV_SHM_AMTP_CALC_COMMON_HEARTBEAT      ("calc_common_heartbeat");
const QString AV_SHM_AMTP_CALC_COMMON_NUM_CALC_PER_S ("calc_common_num_calculations_per_second");
const QString AV_SHM_AMTP_CALC_COMMON_OVERLOAD       ("calc_common_overload");

// mode-s decoding
const QString AV_SHM_AMTP_MODE_S_HEARTBEAT                   ("modes_heartbeat");
const QString AV_SHM_AMTP_MODE_S_NUM_MODES_TARGETS_IN_REACH  ("modes_targets_in_reach");
const QString AV_SHM_AMTP_MODE_S_NUM_ADSB_TARGETS_IN_REACH   ("modes_adsb_targets_in_reach");
const QString AV_SHM_AMTP_MODE_S_OVERLOAD                    ("modes_overload");

// tracker
const QString AV_SHM_AMTP_TRACKER_NUM_TRACKS_MODE_S  ("tracker_num_tracks_mode_s");
const QString AV_SHM_AMTP_TRACKER_NUM_TRACKS_MODE_AC ("tracker_num_tracks_mode_ac");
const QString AV_SHM_AMTP_TRACKER_OVERLOAD           ("tracker_overload");

// interrogation logic
const QString AV_SHM_AMTP_INT_LOGIC_HEARTBEAT                        ("int_logic_heartbeat");
const QString AV_SHM_AMTP_INT_LOGIC_NUM_S_INTER_PER_S                ("int_logic_num_interrogations_per_second_mode_s");
const QString AV_SHM_AMTP_INT_LOGIC_NUM_AC_INTER_PER_S               ("int_logic_num_interrogations_per_second_mode_ac");
const QString AV_SHM_AMTP_INT_LOGIC_NUM_INTER_PER_S                  ("int_logic_num_interrogations_per_second");
const QString AV_SHM_AMTP_INT_LOGIC_SOFT_INT_RATE_LIMIT_EXCEEDED     ("int_logic_soft_interrogation_rate_limit_exceeded");
const QString AV_SHM_AMTP_INT_LOGIC_HARD_INT_RATE_LIMIT_REACHED      ("int_logic_hard_interrogation_rate_limit_reached");
const QString AV_SHM_AMTP_INT_LOGIC_OVERLOAD                         ("int_logic_overload");

const QString AV_SHM_AMTP_INT_LOGIC_PER_RU_PREFIX                    ("int_logic_ru");
const QString AV_SHM_AMTP_INT_LOGIC_LAST_INT_CHECK_TRIED_SUFFIX      ("_last_int_check_tried");
const QString AV_SHM_AMTP_INT_LOGIC_LAST_INT_CHECK_TRY_TRANSP_SUFFIX ("_last_int_check_try_transponder");
const QString AV_SHM_AMTP_INT_LOGIC_LAST_INT_CHECK_RCVD_SUFFIX       ("_last_int_check_rcvd");
const QString AV_SHM_AMTP_INT_LOGIC_LAST_INT_CHECK_ANSW_RCVD_SUFFIX  ("_last_int_check_answer_rcvd");
const QString AV_SHM_AMTP_INT_LOGIC_LAST_INT_SENT_SUFFIX             ("_last_time_interrogation_sent");
const QString AV_SHM_AMTP_INT_LOGIC_INT_STAT_CHECK_OK_SUFFIX         ("_statistics_check_ok");
const QString AV_SHM_AMTP_INT_LOGIC_INT_CHECK_OK_SUFFIX              ("_interrogator_check_ok");

// height aiding logic
const QString AV_SHM_AMTP_HA_LOGIC_BARO_ALT_NUM_AV       ("height_aiding_logic_gnd_level_baro_alt_num_averages");
const QString AV_SHM_AMTP_HA_LOGIC_BARO_ALT_FT           ("height_aiding_logic_gnd_level_baro_alt_ft");
const QString AV_SHM_AMTP_HA_LOGIC_BARO_ALT_FT_STD       ("height_aiding_logic_gnd_level_baro_alt_ft_std");
const QString AV_SHM_AMTP_HA_LOGIC_BARO_ALT_VALID        ("height_aiding_logic_gnd_level_baro_alt_valid");
const QString AV_SHM_AMTP_HA_LOGIC_BARO_ALT_TIME         ("height_aiding_logic_gnd_level_baro_alt_time");
const QString AV_SHM_AMTP_HA_LOGIC_QNH                   ("height_aiding_logic_qnh");

// core logic
const QString AV_SHM_AMTP_C_LOGIC_HEARTBEAT                           ("core_logic_heartbeat");
const QString AV_SHM_AMTP_C_LOGIC_AVERAGE_DATE_OUTPUT_AGE_RCV_TIME_MS ("core_logic_average_output_data_age_rcv_time_ms");
const QString AV_SHM_AMTP_C_LOGIC_AVERAGE_DATE_OUTPUT_AGE_TOE_MS      ("core_logic_average_output_data_age_toe_ms");

// QNH interface
const QString AV_SHM_AMTP_QNH_INTERFACE_CONNECTED                     ("qnh_interface_connected");
const QString AV_SHM_AMTP_QNH_INTERFACE_LAST_QNH_RCVD                 ("qnh_interface_last_qnh_rcvd");

// reference transponder monitor
const QString AV_SHM_AMTP_RT_MONITOR_PREFIX                          ("rt_monitor_");
const QString AV_SHM_AMTP_RT_MONITOR_LAST_TIME_REPLY_RCVD_SUFFIX     ("_last_time_reply_received");
const QString AV_SHM_AMTP_RT_MONITOR_DIST_FROM_RAW_POS_SUFFIX        ("_dist_from_raw_pos_m");
const QString AV_SHM_AMTP_RT_MONITOR_DIST_FROM_TRACKED_POS_SUFFIX    ("_dist_from_tracked_pos_m");
const QString AV_SHM_AMTP_RT_MONITOR_SIGMA0_SUFFIX                   ("_sigma0_m");
const QString AV_SHM_AMTP_RT_MONITOR_TIME_LAST_POS_RCVD_SUFFIX       ("_time_last_pos_rcvd");
const QString AV_SHM_AMTP_RT_MONITOR_TRANSPONDER_OK_SUFFIX           ("_transponder_ok");
const QString AV_SHM_AMTP_RT_MONITOR_VERIF_OK_SUFFIX                 ("_verification_ok");
const QString AV_SHM_AMTP_RT_MONITOR_LAST_TIME_INTERROGATED_SUFFIX   ("_last_time_interrogated");
const QString AV_SHM_AMTP_RT_MONITOR_LAST_TIME_INTER_ANSWERED_SUFFIX ("_last_time_interrogation_answered");
const QString AV_SHM_AMTP_RT_MONITOR_LAST_RU_ID_INTERROGATED_SUFFIX  ("_last_ru_id_interrogated");

// output handler
const QString AV_SHM_AMTP_O_HANDLER_PREFIX                           ("output_");
const QString AV_SHM_AMTP_O_HANDLER_NAME_SUFFIX                      ("_name");
const QString AV_SHM_AMTP_O_HANDLER_HEARTBEAT_SUFFIX                 ("_heartbeat");
const QString AV_SHM_AMTP_O_HANDLER_NUM_MLAT_MSGS_PER_S_SUFFIX       ("_num_mlat_msgs_per_second");
const QString AV_SHM_AMTP_O_HANDLER_NUM_ADSB_MSGS_PER_S_SUFFIX       ("_num_adsb_msgs_per_second");

//----- ADCORE
const QString AV_SHM_ADCORE_LAST_CALCULATION("last_calculation");
const QString AV_SHM_ADCORE_NUM_PROCESSED_TARGETS("num_processed_targets");
const QString AV_SHM_ADCORE_TURN_ADCORE_ON_OFF("turn_adcore_on_off");
const QString AV_SHM_ADCORE_TURN_ADCORE_ON_OFF_TARGET_STATE("turn_adcore_on_off_target_state");
const QString AV_SHM_ADCORE_CONNECTION_LIST("connection_list");

// RVR
const QString AV_SHM_RVR_INTERFACE_STATUS        ("rvr_interface_status");

#endif /* __AVSHM_VARNAMES_H__ */

// End of file
