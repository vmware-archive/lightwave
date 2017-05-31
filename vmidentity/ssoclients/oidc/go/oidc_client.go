package oidc

/*
#include <stdlib.h>
#include "ssotypes.h"
#include "ssoerrors.h"
#include "oidc_types.h"
#include "oidc.h"
*/
import "C"

import "runtime"

// keep this in sync with LIGHTWAVE_TLS_CA_PATH in http_client.c
const LIGHTWAVE_TLS_CA_PATH = "/etc/ssl/certs/"

type OidcClient struct {
    p C.POIDC_CLIENT
}

func OidcClientGlobalInit() (err error) {
    var e C.SSOERROR = C.OidcClientGlobalInit()
    if e != 0 {
        err = cErrorToGoError(e)
    }
    return err
}

func OidcClientGlobalCleanup() {
    C.OidcClientGlobalCleanup()
}

// tlsCAPath: empty means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
func OidcClientBuild(
        server string,
        portNumber int,
        tenant string,
        clientID string, /* optional */
        tlsCAPath string /* optional, see comment above */) (result *OidcClient, err error) {
    serverCStr      := goStringToCString(server)
    tenantCStr      := goStringToCString(tenant)
    clientIDCStr    := goStringToCString(clientID)
    tlsCAPathCStr   := goStringToCString(tlsCAPath)

    defer freeCString(serverCStr)
    defer freeCString(tenantCStr)
    defer freeCString(clientIDCStr)
    defer freeCString(tlsCAPathCStr)

    var p C.POIDC_CLIENT = nil
    var e C.SSOERROR = C.OidcClientBuild(
        &p,
        serverCStr,
        C.int(portNumber),
        tenantCStr,
        clientIDCStr,
        tlsCAPathCStr)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &OidcClient { p }
    runtime.SetFinalizer(result, oidcClientFinalize)
    return
}

func oidcClientFinalize(this *OidcClient) {
    this.Close()
}

func (this *OidcClient) Close() error {
    if (this.p != nil) {
        C.OidcClientDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *OidcClient) AcquireTokensByPassword(
        username string,
        password string,
        scope string) (successResponse *TokenSuccessResponse, errorResponse *ErrorResponse, err error) {
    usernameCStr    := goStringToCString(username)
    passwordCStr    := goStringToCString(password)
    scopeCStr       := goStringToCString(scope)

    defer freeCString(usernameCStr)
    defer freeCString(passwordCStr)
    defer freeCString(scopeCStr)

    var oidcTokenSuccessResponse C.POIDC_TOKEN_SUCCESS_RESPONSE = nil
    var oidcErrorResponse C.POIDC_ERROR_RESPONSE = nil
    var e C.SSOERROR = C.OidcClientAcquireTokensByPassword(
        this.p,
        usernameCStr,
        passwordCStr,
        scopeCStr,
        &oidcTokenSuccessResponse,
        &oidcErrorResponse)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    return
}

func (this *OidcClient) AcquireTokensByRefreshToken(
        refreshToken string) (successResponse *TokenSuccessResponse, errorResponse *ErrorResponse, err error) {
    refreshTokenCStr := goStringToCString(refreshToken)
    defer freeCString(refreshTokenCStr)

    var oidcTokenSuccessResponse C.POIDC_TOKEN_SUCCESS_RESPONSE = nil
    var oidcErrorResponse C.POIDC_ERROR_RESPONSE = nil
    var e C.SSOERROR = C.OidcClientAcquireTokensByRefreshToken(
        this.p,
        refreshTokenCStr,
        &oidcTokenSuccessResponse,
        &oidcErrorResponse)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    return
}

func (this *OidcClient) AcquireTokensBySolutionUserCredentials(
        certificateSubjectDN string,
        privateKeyPEM string,
        scope string) (successResponse *TokenSuccessResponse, errorResponse *ErrorResponse, err error) {
    certificateSubjectDNCStr    := goStringToCString(certificateSubjectDN)
    privateKeyPEMCStr           := goStringToCString(privateKeyPEM)
    scopeCStr                   := goStringToCString(scope)

    defer freeCString(certificateSubjectDNCStr)
    defer freeCString(privateKeyPEMCStr)
    defer freeCString(scopeCStr)

    var oidcTokenSuccessResponse C.POIDC_TOKEN_SUCCESS_RESPONSE = nil
    var oidcErrorResponse C.POIDC_ERROR_RESPONSE = nil
    var e C.SSOERROR = C.OidcClientAcquireTokensBySolutionUserCredentials(
        this.p,
        certificateSubjectDNCStr,
        privateKeyPEMCStr,
        scopeCStr,
        &oidcTokenSuccessResponse,
        &oidcErrorResponse)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    return
}

func (this *OidcClient) GetSigningCertificatePEM() string {
    return cStringToGoString(C.OidcClientGetSigningCertificatePEM(this.p))
}
