#avconfig_version 3

subconfig_template alertchecker.filters
{
    bool enabled = false;

    namespace client
    {
        QString name = "client";
    
        QString host = localhost;
        
        uint port = 8888;
    }
}

// EOF
