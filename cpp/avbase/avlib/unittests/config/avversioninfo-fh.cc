#avconfig_version 2

// -------------------------------------------------------------
namespace avversioninfo
{
    //! File polling interval in seconds
    //!
    //! \suggested 5
    uint polling_interval = 5;

    //! Full path to Server Software Version File
    //!
    //! \suggested $(AVIBIT_HOME)/VERSION
    AVPath server_software_version_file = "";

    //! Full path to Server Config Version File
    //!
    //! \suggested $(AVIBIT_HOME)/servercfgversion.txt
    AVPath server_config_version_file = "";

    //! Full path to Client File holding all client software and config
    //! versions
    //!
    //! \suggested $(AVIBIT_HOME)/clientversions.txt
    AVPath client_file = "";

    //! Column delimiter used in Client File
    //!
    //! \suggested ";"
    QString client_file_delimiter = ";";

    //! Column of software version in Client File
    //!
    //! \suggested 0
    uint client_id_col = 0;

    //! Column of software version in Client File
    //!
    //! \suggested 1
    uint client_software_version_col = 1;

    //! Column of base config version in Client File
    //!
    //! \suggested 2
    uint client_base_config_version_col = 2;

    //! Column of admin config version in Client File
    //!
    //! \suggested 3
    uint client_admin_config_version_col = 3;

} // namespace avversioninfo

// EOF
