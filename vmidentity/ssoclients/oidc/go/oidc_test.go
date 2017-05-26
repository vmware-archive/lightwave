package oidc

import "testing"
import "os"

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

    successResponse, errorResponse, err := client.AcquireTokensByPassword(username, password, "openid offline_access")
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
    at.Close()
}

func exitOnError(t *testing.T, err error) {
    if err != nil {
        t.Fatal(err.Error())
    }
}
