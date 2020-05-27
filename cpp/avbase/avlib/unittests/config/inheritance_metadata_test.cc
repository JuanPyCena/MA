#avconfig_version 3

// -------------------------------------------------------------
namespace template_config
{
    uint port = 123;

    QString test_string = "abcd";

} // namespace template_config

// -------------------------------------------------------------
namespace empty_test_config : template_config
{
} // namespace empty_test_config

// -------------------------------------------------------------
namespace filled_test_config : template_config
{
    uint port = 234;

    QString test_string = "xyz";

} // namespace filled_test_config

// -------------------------------------------------------------
namespace filled_test_config_with_metadata : template_config
{
    uint port = 234;

    QString test_string = "xyz";

} // namespace filled_test_config_with_metadata

// EOF
