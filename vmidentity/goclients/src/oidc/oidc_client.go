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

/*
 * IMPORTANT: you must call this function at process startup while there is only a single thread running
 * This is a wrapper for curl_global_init, from its documentation:
 * This function is not thread safe.
 * You must not call it when any other thread in the program (i.e. a thread sharing the same memory) is running.
 * This doesn't just mean no other thread that is using libcurl.
 * Because curl_global_init calls functions of other libraries that are similarly thread unsafe,
 * it could conflict with any other thread that uses these other libraries.
 */
func OidcClientGlobalInit() (err error) {
    var e C.SSOERROR = C.OidcClientGlobalInit()
    if e != 0 {
        err = cErrorToGoError(e)
    }
    return err
}

// this function is not thread safe. Call it right before process exit
func OidcClientGlobalCleanup() {
    C.OidcClientGlobalCleanup()
}

// make sure you call OidcClientGlobalInit once per process before calling this
// on success, result will be non-null, Close it when done
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

// on success, successResponse will be non-null
// on error, errorResponse might be non-null (it will carry error info returned by the server if any)
// Close both when done, whether invocation is successful or not
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
    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    if e != 0 {
        err = cErrorToGoError(e)
    }

    return
}

// on success, successResponse will be non-null
// on error, errorResponse might be non-null (it will carry error info returned by the server if any)
// Close both when done, whether invocation is successful or not
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
    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    if e != 0 {
        err = cErrorToGoError(e)
    }

    return
}

// on success, successResponse will be non-null
// on error, errorResponse might be non-null (it will carry error info returned by the server if any)
// Close both when done, whether invocation is successful or not
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
    if oidcTokenSuccessResponse != nil {
        successResponse = tokenSuccessResponseNew(oidcTokenSuccessResponse)
    }
    if oidcErrorResponse != nil {
        errorResponse = errorResponseNew(oidcErrorResponse)
    }
    if e != 0 {
        err = cErrorToGoError(e)
    }

    return
}

func (this *OidcClient) GetSigningCertificatePEM() string {
    return cStringToGoString(C.OidcClientGetSigningCertificatePEM(this.p))
}
