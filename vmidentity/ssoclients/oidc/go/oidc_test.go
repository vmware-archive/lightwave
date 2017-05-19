package oidc

import "testing"
import "os"
import "fmt"

func TestOidc(t *testing.T) {
    var server string = os.Getenv("LW_SERVER")
    var tenant string = os.Getenv("LW_TENANT")
    var username string = os.Getenv("LW_USERNAME")
    var password string = os.Getenv("LW_PASSWORD")

    test(t, server, tenant, username, password)
}

func test(
        t *testing.T,
        server string,
        tenant string,
        username string,
        password string) {
    OidcClientGlobalInit()
    defer OidcClientGlobalCleanup()

    serverMetadata, err := ServerMetadataAcquire(server, 443, tenant)
    exitOnError(t, err)
    defer serverMetadata.Close()

    client, err := OidcClientBuild(server, 443, tenant)
    exitOnError(t, err)
    defer client.Close()

    successResponse, errorResponse, err := client.AcquireTokensByPassword(
        username,
        password,
        "openid offline_access id_groups at_groups rs_admin_server")
    if errorResponse != nil {
        t.Error(errorResponse.GetErrorDescription())
        errorResponse.Close()
    }
    exitOnError(t, err)
    validateIDToken(t, successResponse.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(t, successResponse.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    var refreshToken string = successResponse.GetRefreshToken()
    successResponse.Close()

    successResponse2, errorResponse2, err := client.AcquireTokensByRefreshToken(refreshToken)
    if errorResponse2 != nil {
        t.Error(errorResponse2.GetErrorDescription())
        errorResponse2.Close()
    }
    exitOnError(t, err)
    validateIDToken(t, successResponse2.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(t, successResponse2.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    successResponse2.Close()
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
