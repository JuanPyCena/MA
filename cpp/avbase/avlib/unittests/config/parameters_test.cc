#avconfig_version 2

// config file created for fixing SWE-5739
// -------------------------------------------------------------
namespace root_config
{
    //! the cause to SWE-5739 is not the null terminator value
    //! but due to call registerSubconfig("root_config")
    //!
    //! \suggested ""
    QString string_value_of_null_terminator = \0;

    // ---------------------------------------------------------
    namespace child_config
    {
    } // namespace child_config

} // namespace root_config

// EOF
