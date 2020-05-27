#avconfig_version 3

// arbitrary uint param to test references...
#include "deprecated_test_config1"

// -------------------------------------------------------------
subconfig_template parent.submap
{
    //! no help
    uint param = ;

} // subconfig_template parent.submap

// -------------------------------------------------------------
namespace parent
{
    // -------------------------------------------------------------
    namespace submap
    {
        // -------------------------------------------------------------
        namespace z
        {
            //! no help
            uint param = 1;

        } // namespace z

        // -------------------------------------------------------------
        namespace f
        {
            //! no help
            uint & param = deprecated_test_config.some_port;

        } // namespace f

        // -------------------------------------------------------------
        namespace a
        {
            //! no help
            uint param = 3;

        } // namespace a

    } // namespace submap

} // namespace parent

// EOF
