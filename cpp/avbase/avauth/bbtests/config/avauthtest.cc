#avconfig_version 2

// -------------------------------------------------------------
namespace avauthtest
{
    // -------------------------------------------------------------
    namespace authentication
    {
        //! Keycloak URL (including protocol & port).
        QString keycloak_url = "https://keycloak.adbsafegate.de";

        //! The keycloak realm to use.
        QString realm = "GATE";

        //! The keycloak client ID.
        QString client_id = "sam";

        int refresh_headstart_s = 885;


    } // namespace authentication

} // namespace avauthtest

// EOF
