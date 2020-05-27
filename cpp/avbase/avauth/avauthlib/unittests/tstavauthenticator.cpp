#include "avauthenticator.h"
#include <QObject>
///////////////////////////////////////////////////////////////////////////////

#include <avunittestmain.h>
#include <avauthenticator.h>
#include <mockqnetworkaccessmanager.h>
#include <QSignalSpy>

using namespace av::auth;




class AuthenticatorTest : public QObject
{
    Q_OBJECT

    std::unique_ptr<MockQNetworkAccessManager> createMockNAM();
private slots:

    void initTestCase()
    {
        AVLogger->setMinLevel(AVLog::LOG__DEBUG2);
    }


    void testSuccessfullLogin();

    void testLoginFailure();

    void testLogout();

};

static const QByteArray SERVICES = R"(
                                   {
                                   "authorization_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/auth",
                                   "end_session_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/logout",
                                   "grant_types_supported": [
                                   "authorization_code",
                                   "implicit",
                                   "refresh_token",
                                   "password",
                                   "client_credentials"
                                   ],
                                   "introspection_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/token/introspect",
                                   "issuer": "https://keycloak.adbsafegate.de/auth/realms/GATE",
                                   "jwks_uri": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/certs",
                                   "permission_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/authz/protection/permission",
                                   "policy_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/authz/protection/uma-policy",
                                   "registration_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/clients-registrations/openid-connect",
                                   "resource_registration_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/authz/protection/resource_set",
                                   "response_modes_supported": [
                                   "query",
                                   "fragment",
                                   "form_post"
                                   ],
                                   "response_types_supported": [
                                   "code",
                                   "none",
                                   "id_token",
                                   "token",
                                   "id_token token",
                                   "code id_token",
                                   "code token",
                                   "code id_token token"
                                   ],
                                   "scopes_supported": [
                                   "openid",
                                   "address",
                                   "email",
                                   "offline_access",
                                   "phone",
                                   "profile",
                                   "roles",
                                   "web-origins"
                                   ],
                                   "token_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/token",
                                   "token_endpoint_auth_methods_supported": [
                                   "private_key_jwt",
                                   "client_secret_basic",
                                   "client_secret_post",
                                   "client_secret_jwt"
                                   ],
                                   "token_endpoint_auth_signing_alg_values_supported": [
                                   "RS256"
                                   ],
                                   "token_introspection_endpoint": "https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/token/introspect"
                                   })";


static const QByteArray TOKEN = R"({
                                "access_token": "eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICI1cVN4Ui11czJKcmkwUmQ3dnN3WWN6dFlKbUlYZDVTeHJMWVdCcFNuRzdVIn0.eyJqdGkiOiI3OWViMjc0Zi01MDFkLTQwMDItYmNiOS0xZmI0ZjlmYzBkMDgiLCJleHAiOjE1Nzg5MTk5MDcsIm5iZiI6MCwiaWF0IjoxNTc4OTE5MDA3LCJpc3MiOiJodHRwczovL2tleWNsb2FrLmFkYnNhZmVnYXRlLmRlL2F1dGgvcmVhbG1zL0dBVEUiLCJzdWIiOiI5NzZkMzUzMS03ZmJkLTQ1MjgtOTEzMi1mMWNmNDU1YmM4NzkiLCJ0eXAiOiJCZWFyZXIiLCJhenAiOiJzYW0iLCJhdXRoX3RpbWUiOjE1Nzg5MTkwMDcsInNlc3Npb25fc3RhdGUiOiJiMmU1YTkyMi1kZWNmLTRiZDAtODNjNi0yZmJmYzk3NjU5ZDciLCJhY3IiOiIxIiwicmVzb3VyY2VfYWNjZXNzIjp7InNhbSI6eyJyb2xlcyI6WyJhY2tub3dsZWRnZS1hbGFybXMiLCJhY3RpdmF0ZS1lbGVjdHJpYy1zdG9ybS13YXJuaW5nIiwibG9naW4iLCJzaHV0ZG93biJdfX0sInNjb3BlIjoib3BlbmlkIGVtYWlsIHByb2ZpbGUiLCJlbWFpbF92ZXJpZmllZCI6dHJ1ZSwibmFtZSI6IlRlc3QgVXNlciIsInByZWZlcnJlZF91c2VybmFtZSI6InRlc3R1c2VyIiwiZ2l2ZW5fbmFtZSI6IlRlc3QiLCJmYW1pbHlfbmFtZSI6IlVzZXIiLCJlbWFpbCI6InRlc3R1c2VyQGFkYnNhZmVnYXRlLmNvbSJ9.NAO6belZMyMlPxB3D0PriLBvlPsnFBi-XQ8xVY_pRgG_DDpfU73zjvtKVq0JDydAk2AGxWmKqBKl9yM-IFoyLSVKh8ZxMywCv2lgSgHhZwHWMq7HJfqY5eCMSJBNl6z9Laxas86l9ioWNrANNxQY8bDfKIe0cWTKdmc-TyQYZgkgjxX56AJR9t7r0zf1yVblxdnAbB14L1m_3Fd3-tjB8s8Qq1gFyS8wDRd4KcCWKPTTk78J3D5xQpnQLuwXa4aiIp4EMlwpJvPlgWw8Wi4olxzK6SljSjKEMNP46I4b09UxSgh525z-CvS88u3nDVw8rhVtnRo8Uik_Obr-c3dCsA",
                                "expires_in": 900,
                                "id_token": "eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICI1cVN4Ui11czJKcmkwUmQ3dnN3WWN6dFlKbUlYZDVTeHJMWVdCcFNuRzdVIn0.eyJqdGkiOiJkMWI4ZjUwOC03MWExLTRmZjItOTUzNi1lNDM5YjJmNmExYmQiLCJleHAiOjE1Nzg5MTk5MDcsIm5iZiI6MCwiaWF0IjoxNTc4OTE5MDA3LCJpc3MiOiJodHRwczovL2tleWNsb2FrLmFkYnNhZmVnYXRlLmRlL2F1dGgvcmVhbG1zL0dBVEUiLCJhdWQiOiJzYW0iLCJzdWIiOiI5NzZkMzUzMS03ZmJkLTQ1MjgtOTEzMi1mMWNmNDU1YmM4NzkiLCJ0eXAiOiJJRCIsImF6cCI6InNhbSIsImF1dGhfdGltZSI6MTU3ODkxOTAwNywic2Vzc2lvbl9zdGF0ZSI6ImIyZTVhOTIyLWRlY2YtNGJkMC04M2M2LTJmYmZjOTc2NTlkNyIsImFjciI6IjEiLCJyZXNvdXJjZV9hY2Nlc3MiOnsic2FtIjp7InJvbGVzIjpbImFja25vd2xlZGdlLWFsYXJtcyIsImFjdGl2YXRlLWVsZWN0cmljLXN0b3JtLXdhcm5pbmciLCJsb2dpbiIsInNodXRkb3duIl19fSwiZW1haWxfdmVyaWZpZWQiOnRydWUsIm5hbWUiOiJUZXN0IFVzZXIiLCJwcmVmZXJyZWRfdXNlcm5hbWUiOiJ0ZXN0dXNlciIsImdpdmVuX25hbWUiOiJUZXN0IiwiZmFtaWx5X25hbWUiOiJVc2VyIiwiZW1haWwiOiJ0ZXN0dXNlckBhZGJzYWZlZ2F0ZS5jb20ifQ.PY_9QorZ9UoJbZtyStWwt_WFOYoMfnQybjomJwC2rw0HFrb4YfCW7U9t2jaP5SHJ2EVzAyF2fw_cPyZi0NjzBeH1_NhXcNo_IhD_Wm4FmE3F9TeBgaFCA7R9BwdGfI0vYJm6wp86Mzwst-FDZMAUP11EGMw_0zqU9JVtmFkt4njv7n5Y5GKdqi0kTDxPEodX9S6decGBfCKKJjMMUaerkxDDSBo5cvioVsgZ5yGY8OuKijqG2VYjtWaFolcixtkSncPyLsld0cV4zCNaJIf6WZjM8oGGQpLbhCZpmdnxnyLn8nQUB1323HUYYQ409NagSlA3u3kE8R8ZztUfnaAGbw",
                                "not-before-policy": 0,
                                "refresh_expires_in": 3600,
                                "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICJlNTljZTZhNi0yNjkwLTQwYTItYThhYS03OWY4MThlNGJiNDEifQ.eyJqdGkiOiJiNjY1MGUzYS05MjM5LTQ1NDMtOGQ1Zi03MzBiZGJiNDk3YTgiLCJleHAiOjE1Nzg5MjI2MDcsIm5iZiI6MCwiaWF0IjoxNTc4OTE5MDA3LCJpc3MiOiJodHRwczovL2tleWNsb2FrLmFkYnNhZmVnYXRlLmRlL2F1dGgvcmVhbG1zL0dBVEUiLCJhdWQiOiJodHRwczovL2tleWNsb2FrLmFkYnNhZmVnYXRlLmRlL2F1dGgvcmVhbG1zL0dBVEUiLCJzdWIiOiI5NzZkMzUzMS03ZmJkLTQ1MjgtOTEzMi1mMWNmNDU1YmM4NzkiLCJ0eXAiOiJSZWZyZXNoIiwiYXpwIjoic2FtIiwiYXV0aF90aW1lIjowLCJzZXNzaW9uX3N0YXRlIjoiYjJlNWE5MjItZGVjZi00YmQwLTgzYzYtMmZiZmM5NzY1OWQ3IiwicmVzb3VyY2VfYWNjZXNzIjp7InNhbSI6eyJyb2xlcyI6WyJhY2tub3dsZWRnZS1hbGFybXMiLCJhY3RpdmF0ZS1lbGVjdHJpYy1zdG9ybS13YXJuaW5nIiwibG9naW4iLCJzaHV0ZG93biJdfX0sInNjb3BlIjoib3BlbmlkIGVtYWlsIHByb2ZpbGUifQ.jbO4bI-OCjsMypeSyYnI95k_-dKyvlC1uu9fTTcXYSY",
                                "scope": "openid email profile",
                                "session_state": "b2e5a922-decf-4bd0-83c6-2fbfc97659d7",
                                "token_type": "bearer"
                                }
                                )";

std::unique_ptr<MockQNetworkAccessManager> AuthenticatorTest::createMockNAM()
{
    return std::make_unique<MockQNetworkAccessManager>(
        QVector<MockQNetworkAccessManager::MockResult> {
            MockQNetworkAccessManager::MockResult { 200, SERVICES},
            MockQNetworkAccessManager::MockResult { 200, TOKEN},
            MockQNetworkAccessManager::MockResult { 200, TOKEN},
            MockQNetworkAccessManager::MockResult { 200, TOKEN},
            MockQNetworkAccessManager::MockResult { 200, TOKEN},
            MockQNetworkAccessManager::MockResult { 200, TOKEN},
        }
    );
}

void AuthenticatorTest::testSuccessfullLogin()
{

    std::unique_ptr<MockQNetworkAccessManager> mock_nam = createMockNAM();

    av::auth::Authenticator auth { mock_nam.get(), [&](QUrl url){
            AVLogInfo << "Should open browser with url " << url.toString();

            QNetworkAccessManager nam;
            auto* result = nam.get(QNetworkRequest(QUrl("http://127.0.0.1:" % QString::number(auth.m_server_port) %
                                "/?state=ATTMiGw-9_lthTSWYqxZBId-BzMmlB5-37LBlu6ENsw&session_state=06a8a5f5-3a51-4553-94d9-eecf70e12a19&code=95aa8e3f-e63e-421a-ad36-c1c13ae8a6cc.06a8a5f5-3a51-4553-94d9-eecf70e12a19.704400cd-f656-4869-945d-d06147cd072c")));

            AVLogIndent intend("Wait for finished");
            QSignalSpy spy(result, &QNetworkReply::finished);


            AVASSERT(spy.wait());

            AVLogInfo << AV_FUNC_PREAMBLE << "Result: " << result->error();

            return true;
        } } ;
    auth.m_config->m_realm = "GATE";
    auth.m_config->m_client_id = "sam";
    auth.m_config->m_refresh_headstart_s = 898;

    connect(&auth, &Authenticator::stateChanged, [](auto new_state) { AVLogInfo << "AuthenticatorTest::testSuccessfullLogin: New state: " << new_state; });
    connect(&auth, &Authenticator::error, [](auto error) { AVLogError << "AuthenticatorTest::testSuccessfullLogin: Error: " << error; });

    QSignalSpy spy(&auth, &Authenticator::authenticated);

    auth.authenticate();

    QVERIFY(spy.wait());


    //wait for refresh

    QVERIFY(spy.wait());

    QVERIFY(spy.wait());

    QVERIFY(spy.wait());


}

void AuthenticatorTest::testLoginFailure()
{
    std::unique_ptr<MockQNetworkAccessManager> mock_nam = createMockNAM();

    auto  config = std::make_unique<AuthenticatorConfig>(AuthenticatorConfig::UNIT_TEST_CONSTRUCTOR);
    config->m_login_timeout_s = 4;

    av::auth::Authenticator auth { mock_nam.get(), [](QUrl url){
            AVLogInfo << "Should open browser with url " << url.toString();
            return true;
        }, std::move(config)};

    auth.authenticate();
    QSignalSpy spy(&auth, &Authenticator::error);

    QVERIFY(spy.wait());

    QCOMPARE(spy.first().first().value<Authenticator::ErrorCode>(), Authenticator::ErrorCode::TimedOut);


}

void AuthenticatorTest::testLogout()
{
    std::unique_ptr<MockQNetworkAccessManager> mock_nam = createMockNAM();

    av::auth::Authenticator auth { mock_nam.get(), [&](QUrl url){
            AVLogInfo << "Should open browser with url " << url.toString();

            QNetworkAccessManager nam;
            {
                auto* ignored_result = nam.get(QNetworkRequest(QUrl("http://127.0.0.1:" % QString::number(auth.m_server_port) %
                                            "/favicon.ico")));
                QSignalSpy spy(ignored_result, &QNetworkReply::finished);
                spy.wait();
                AVASSERT(ignored_result->attribute(QNetworkRequest::HttpStatusCodeAttribute) != 200);
            }

            auto* result = nam.get(QNetworkRequest(QUrl("http://127.0.0.1:" % QString::number(auth.m_server_port) %
                                "/?state=ATTMiGw-9_lthTSWYqxZBId-BzMmlB5-37LBlu6ENsw&session_state=06a8a5f5-3a51-4553-94d9-eecf70e12a19&code=95aa8e3f-e63e-421a-ad36-c1c13ae8a6cc.06a8a5f5-3a51-4553-94d9-eecf70e12a19.704400cd-f656-4869-945d-d06147cd072c")));

            AVLogIndent intend("Wait for finished");
            QSignalSpy spy(result, &QNetworkReply::finished);


            AVASSERT(spy.wait());

            AVLogInfo << AV_FUNC_PREAMBLE << "Result: " << result->error();

            return true;
        } } ;
    auth.m_config->m_realm = "GATE";
    auth.m_config->m_client_id = "sam";
    auth.m_config->m_refresh_headstart_s = 898;

    connect(&auth, &Authenticator::stateChanged, [](auto new_state) { AVLogInfo << "AuthenticatorTest::testSuccessfullLogin: New state: " << new_state; });
    connect(&auth, &Authenticator::error, [](auto error) { AVLogError << "AuthenticatorTest::testSuccessfullLogin: Error: " << error; });

    QSignalSpy spy(&auth, &Authenticator::authenticated);

    auth.authenticate();

    QVERIFY(spy.wait());

    mock_nam->m_handler = [&] (auto , const QNetworkRequest & request, QIODevice *) -> MockQNetworkAccessManager::MockResult
    {
        AVASSERT(request.url() == QUrl("https://keycloak.adbsafegate.de/auth/realms/GATE/protocol/openid-connect/logout"));
        return {200, ""};
    };

    QSignalSpy state_spy(&auth, &Authenticator::stateChanged);
    auth.logout();


    if (state_spy.size() == 0)
    {
        QVERIFY(state_spy.wait());
    }

    QCOMPARE(state_spy.first().first().value<Authenticator::State>(), Authenticator::State::Uninitialized);



}

AV_QTEST_MAIN(AuthenticatorTest);
#include "tstavauthenticator.moc"

