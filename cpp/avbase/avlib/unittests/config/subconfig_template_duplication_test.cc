#avconfig_version 3

subconfig_template alertchecker.filters
{
    uint enabled = true;

    namespace client
    {
        QString host = localhost;
        
        uint port = 1122;
    }
}

subconfig_template alertchecker.filters.*.regions
{
    QRect area = [4;3;2;1];
}

// EOF
