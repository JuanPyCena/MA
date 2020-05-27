#avconfig_version 3

// -------------------------------------------------------------
subconfig_template parent.submap
{
    //! no help
    //!
    //! \suggested 1
    uint param1 = 1;

} // subconfig_template parent.submap

// -------------------------------------------------------------
subconfig_template parent.submap.*.submap2
{
    //! no help
    //!
    //! \suggested 2
    uint param2 = 2;

    // -------------------------------------------------------------
    namespace submap2_element_namespace
    {
        //! no help
        //!
        //! \suggested param1
        QString ns_param1 = param1;

        //! no help
        //!
        //! \suggested param2
        QString ns_param2 = param2;

    } // namespace submap2_element_namespace

} // subconfig_template parent.submap.*.submap2

// -------------------------------------------------------------
subconfig_template parent.submap.*.submap2.*.submap3
{
    //! no help
    //!
    //! \suggested 3
    uint param3 = 3;

    // -------------------------------------------------------------
    namespace submap3_element_namespace
    {
        //! no help
        //!
        //! \suggested 1122
        int ns_param1 = 1122;
        
        // -------------------------------------------------------------
        namespace submap3_element_subnamespace
        {
            //! no help
            //!
            //! \suggested 2233
            int ns_param = 2233;
            
        } // namespace submap3_element_subnamespace

    } // namespace submap3_element_namespace

} // subconfig_template parent.submap.*.submap2.*.submap3

// -------------------------------------------------------------
namespace parent
{
    // -------------------------------------------------------------
    namespace submap
    {
        // -------------------------------------------------------------
        namespace child1
        {
            //! no help
            uint param1 = 1;

            // -------------------------------------------------------------
            namespace submap2
            {
                // -------------------------------------------------------------
                namespace submap2_child1
                {
                    //! no help
                    uint param2 = 2;

                    // -------------------------------------------------------------
                    namespace submap2_element_namespace
                    {
                        //! no help
                        QString ns_param1 = param1;

                        //! no help
                        QString ns_param2 = param2;

                    } // namespace submap2_element_namespace

                    // -------------------------------------------------------------
                    namespace submap3
                    {
                        // -------------------------------------------------------------
                        namespace child3
                        {
                            //! no help
                            uint param3 = 3;

                            // -------------------------------------------------------------
                            namespace submap3_element_namespace
                            {
                                //! no help
                                int ns_param1 = 1122;

                            } // namespace submap3_element_namespace

                        } // namespace child3

                    } // namespace submap3

                } // namespace submap2_child1

                // -------------------------------------------------------------
                namespace submap2_child2
                {
                    //! no help
                    uint param2 = 4;

                    // -------------------------------------------------------------
                    namespace submap2_element_namespace
                    {
                        //! no help
                        QString ns_param1 = child2_param1;

                        //! no help
                        QString ns_param2 = child2_param2;

                    } // namespace submap2_element_namespace

                } // namespace submap2_child2

            } // namespace submap2

        } // namespace child1

    } // namespace submap

} // namespace parent

// EOF
