#avconfig_version 2
 
#if SYS==AIRMAX
#if HOSTTYPE==DP
#search "airmax/dsw-dp"
#endif
#endif // SYS==AIRMAX && HOSTTYPE==DP
 
#if SYS==AIRMAX
#if HOSTTYPE==RE
#search "airmax/dsw-re"
#endif
#endif // SYS==AIRMAX && HOSTTYPE==RE
 
#if SYS==DIFLIS
#search "diflis/dsw"
#endif // SYS==DIFLIS
 
#if SYS==REPLAY
#search "repmax/dsw"
#endif // SYS==REPLAY
 
#if SYS==SIM
#search "simulation/dsw"
#endif // SYS==SIM
  
// To be done once transitioning to global_config worked alright:
// #include "egnx_ports"
// #include "egnx_hosts"
 
// EOF