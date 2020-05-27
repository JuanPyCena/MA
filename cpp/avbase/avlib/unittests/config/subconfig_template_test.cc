#avconfig_version 3

subconfig_template alertchecker.filters
{
    uint enabled = true;

    namespace client
    {
        QString host = localhost;
        
        uint port = 2233;
    }
}

subconfig_template alertchecker.filters.*.regions
{
    QRect area = [1;2;3;4];
}

// EOF
