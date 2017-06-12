package oidc

import "testing"
import "os"
import "fmt"
import "errors"

func TestOidc(t *testing.T) {
    var server string = os.Getenv("LW_SERVER")
    var tenant string = os.Getenv("LW_TENANT")
    var username string = os.Getenv("LW_USERNAME")
    var password string = os.Getenv("LW_PASSWORD")
    var clientID string = os.Getenv("LW_CLIENT_ID")

    test(t, server, tenant, username, password, clientID)
}

func test(
        t *testing.T,
        server string,
        tenant string,
        username string,
        password string,
        clientID string) {
    OidcClientGlobalInit()
    defer OidcClientGlobalCleanup()

    serverMetadata, err := ServerMetadataAcquire(server, 443, tenant, "" /* skip tls validation */)
    exitOnError(t, err)
    defer serverMetadata.Close()

    client, err := OidcClientBuild(server, 443, tenant, clientID, "" /* skip tls validation */)
    exitOnError(t, err)
    defer client.Close()

    var successResponse *TokenSuccessResponse = nil
    var errorResponse *ErrorResponse = nil

    successResponse, errorResponse, err = client.AcquireTokensByPassword(
        username,
        password,
        "openid offline_access id_groups at_groups rs_admin_server")
    assert(t, successResponse != nil, "successResponse != nil")
    assert(t, errorResponse == nil, "errorResponse == nil")
    assert(t, err == nil, "err == nil")
    validateIDToken(t, successResponse.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(t, successResponse.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    var refreshToken string = successResponse.GetRefreshToken()
    successResponse.Close()

    successResponse, errorResponse, err = client.AcquireTokensByRefreshToken(refreshToken)
    assert(t, successResponse != nil, "successResponse != nil")
    assert(t, errorResponse == nil, "errorResponse == nil")
    assert(t, err == nil, "err == nil")
    validateIDToken(t, successResponse.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(t, successResponse.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    successResponse.Close()

    // test wrong password
    successResponse, errorResponse, err = client.AcquireTokensByPassword(username, password + "_nonmatching", "openid")
    assert(t, successResponse == nil, "successResponse == nil")
    assert(t, errorResponse != nil, "errorResponse != nil")
    assert(t, err != nil, "err != nil")
    assertEqual(t, "invalid_grant", errorResponse.GetError())
    assertEqual(t, "incorrect username or password", errorResponse.GetErrorDescription())
    assertEqual(t, "SSOERROR_OIDC_SERVER_INVALID_GRANT", err.Error())
    errorResponse.Close()
}

func validateIDToken(t *testing.T, jwt string, pem string) {
    id, err := IDTokenBuild(
        jwt,
        pem,
        "", /* issuer */
        10  /* clockToleranceInSeconds */)
    exitOnError(t, err)
    fmt.Printf(
        "id_token: [issuer: %v] [subject: %v] [audience: %v] [groups: %v]\n",
        id.GetIssuer(),
        id.GetSubject(),
        id.GetAudience(),
        id.GetGroups())
    id.Close()
}

func validateAccessToken(t *testing.T, jwt string, pem string) {
    at, err := AccessTokenBuild(
        jwt,
        pem,
        "", /* issuer */
        "", /* resourceServerName */
        10  /* clockToleranceInSeconds */)
    exitOnError(t, err)
    fmt.Printf(
        "access_token: [issuer: %v] [subject: %v] [audience: %v] [groups: %v]\n",
        at.GetIssuer(),
        at.GetSubject(),
        at.GetAudience(),
        at.GetGroups())
    at.Close()
}

func exitOnError(t *testing.T, err error) {
    if err != nil {
        t.Fatal(err.Error())
    }
}

func assert(t *testing.T, expression bool, message string) {
    if !expression {
        t.Fatal(errors.New(message))
    }
}

func assertEqual(t *testing.T, expected string, actual string) {
    if expected != actual {
        t.Fatal(errors.New(fmt.Sprintf("expected: [%s], actual: [%s]", expected, actual)))
    }
}
