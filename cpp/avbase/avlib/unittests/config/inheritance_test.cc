#avconfig_version 3

overriden_instance.some_port = 66;

// -------------------------------------------------------------
namespace outer
{
    // -------------------------------------------------------------
    namespace template_config
    {
        uint some_port = 1;

        // -------------------------------------------------------------
        namespace elements
        {
            // -------------------------------------------------------------
            namespace element1
            {
                uint element_content = 1;

            } // namespace element1

        } // namespace elements

    } // namespace template_config

    // -------------------------------------------------------------
    namespace nested_template_config
    {
        QString nested_param = nested;

    } // namespace nested_template_config

} // namespace outer

// -------------------------------------------------------------
namespace instance1 : outer.template_config
{
    uint some_other_port = 3;

    //! \cmdline instance1_port
    uint some_port = 2;

    // -------------------------------------------------------------
    namespace nested_instance1 : outer.nested_template_config
    {
        QString nested_param = overridden_nested;

    } // namespace nested_instance1

} // namespace instance1

// -------------------------------------------------------------
namespace instance3 : instance1
{
    // -------------------------------------------------------------
    namespace nested_instance3 : outer.nested_template_config
    {
    } // namespace nested_instance3

} // namespace instance3

// -------------------------------------------------------------
namespace overridden_instance : outer.template_config
{
} // namespace overridden_instance

#if A==B

// -------------------------------------------------------------
namespace instance2 : outer.template_config
{
    uint some_other_port = 3;

} // namespace instance2

// -------------------------------------------------------------
namespace instance4 : outer.template_config
{
} // namespace instance4

#endif // A==B

// EOF
