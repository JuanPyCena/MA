#avconfig_version 2

namespace namespace1
{
    bool fact_level_1   = true;
    bool fact_level_1_first = true;
    #if IF_LEVEL_1==""
        int nesting_level_2 = 2;
    #endif
    
    #if IF_LEVEL_1==VALUE
        int nesting_level_2   = 2;
        int nesting_level_2_2 = 2;
    #endif
    
    bool factLevel1_second = true;
        
    namespace namespace2
    {
        bool factLevel2_first = true;        
    }
    
    namespace namespace2_2
    {
        bool factLevel2_first = true;        
        namespace namespace3_1
        {
            #if NESTED_IF_LEVEL_3==""            
            bool factLevel3_first = true;
            #endif
        }
        #if NESTED_IF_LEVEL_2==""
            int level_3_first = 3;
        #endif
        #if NESTED_IF_LEVEL_2==VALUE
            int level_3_first = 300;
        #endif
    }
    
    #if CONDITION_LEVEL_1==""
        namespace under4Nodes
        {
                int level_4 = 4;
                int level_4_4 = 4;
        }
    #endif

}

// EOF
