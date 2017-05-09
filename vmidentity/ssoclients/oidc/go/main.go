package main

import "fmt"
import "os"

func main() {
    var server string = os.Args[1]
    var tenant string = os.Args[2]
    var username string = os.Args[3]
    var password string = os.Args[4]

    start(server, tenant, username, password)
}

func start(
        server string,
        tenant string,
        username string,
        password string) {
    OidcClientGlobalInit()
    defer OidcClientGlobalCleanup()

    serverMetadata, err := ServerMetadataAcquire(server, 443, tenant)
    exitOnError(err)
    defer serverMetadata.Close()

    client, err := OidcClientBuild(server, 443, tenant)
    exitOnError(err)
    defer client.Close()

    successResponse, errorResponse, err := client.AcquireTokensByPassword(username, password, "openid offline_access")
    if errorResponse != nil {
        fmt.Println(errorResponse.GetErrorDescription())
        errorResponse.Close()
    }
    exitOnError(err)
    validateIDToken(successResponse.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(successResponse.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    var refreshToken string = successResponse.GetRefreshToken()
    successResponse.Close()

    successResponse2, errorResponse2, err := client.AcquireTokensByRefreshToken(refreshToken)
    if errorResponse2 != nil {
        fmt.Println(errorResponse2.GetErrorDescription())
        errorResponse2.Close()
    }
    exitOnError(err)
    validateIDToken(successResponse2.GetIDToken(), serverMetadata.GetSigningCertificatePEM())
    validateAccessToken(successResponse2.GetAccessToken(), serverMetadata.GetSigningCertificatePEM())
    successResponse2.Close()
}

func validateIDToken(jwt string, pem string) {
    id, err := IDTokenBuild(
        jwt,
        pem,
        "", /* issuer */
        10  /* clockToleranceInSeconds */)
    fmt.Println(id.GetSubject())
    exitOnError(err)
    id.Close()
}

func validateAccessToken(jwt string, pem string) {
    at, err := AccessTokenBuild(
        jwt,
        pem,
        "", /* issuer */
        "", /* resourceServerName */
        10  /* clockToleranceInSeconds */)
    fmt.Println(at.GetSubject())
    exitOnError(err)
    at.Close()
}

func exitOnError(err error) {
    if err != nil {
        fmt.Println(err)
        os.Exit(-1)
    }
}