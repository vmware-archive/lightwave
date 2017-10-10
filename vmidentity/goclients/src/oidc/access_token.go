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
import "time"

type AccessToken struct {
    p C.POIDC_ACCESS_TOKEN
}

// (TODO) Deprecated
func AccessTokenBuild(
        jwt string,
        signingCertificatePEM string,
        issuer string,
        resourceServerName string,
        clockToleranceInSeconds int64) (result *AccessToken, err error) {
    jwtCStr                     := goStringToCString(jwt)
    signingCertificatePEMCStr   := goStringToCString(signingCertificatePEM)
    issuerCStr                  := goStringToCString(issuer)
    resourceServerNameCStr      := goStringToCString(resourceServerName)

    defer freeCString(jwtCStr)
    defer freeCString(signingCertificatePEMCStr)
    defer freeCString(issuerCStr)
    defer freeCString(resourceServerNameCStr)

    var p C.POIDC_ACCESS_TOKEN = nil
    var e C.SSOERROR = C.OidcAccessTokenBuild(
        &p,
        jwtCStr,
        signingCertificatePEMCStr,
        issuerCStr,
        resourceServerNameCStr,
        C.SSO_LONG(clockToleranceInSeconds))
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &AccessToken { p }
    runtime.SetFinalizer(result, accessTokenFinalize)
    return
}

// on success, result will be non-null, Close it when done
func AccessTokenParse(jwt string) (result *AccessToken, err error) {
    jwtCStr := goStringToCString(jwt)
    defer freeCString(jwtCStr)

    var p C.POIDC_ACCESS_TOKEN = nil
    var e C.SSOERROR = C.OidcAccessTokenParse(&p, jwtCStr)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &AccessToken { p }
    runtime.SetFinalizer(result, accessTokenFinalize)
    return
}

func (this *AccessToken) AccessTokenValidate(
        signingCertificatePEM string,
        issuer string,
        resourceServerName string,
        clockToleranceInSeconds int64) error {
    signingCertificatePEMCStr   := goStringToCString(signingCertificatePEM)
    issuerCStr                  := goStringToCString(issuer)
    resourceServerNameCStr      := goStringToCString(resourceServerName)

    defer freeCString(signingCertificatePEMCStr)
    defer freeCString(issuerCStr)
    defer freeCString(resourceServerNameCStr)

    var e C.SSOERROR = C.OidcAccessTokenValidate(
        this.p,
        signingCertificatePEMCStr,
        issuerCStr,
        resourceServerNameCStr,
        C.SSO_LONG(clockToleranceInSeconds))

    var err error = nil
    if e != 0 {
        err = cErrorToGoError(e)
    }
    return err
}

func accessTokenFinalize(this *AccessToken) {
    this.Close()
}

func (this *AccessToken) Close() error {
    if (this.p != nil) {
        C.OidcAccessTokenDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *AccessToken) GetTokenType() TokenType {
    return cTokenTypeToGoTokenType(C.OidcAccessTokenGetTokenType(this.p))
}

func (this *AccessToken) GetIssuer() string {
    return cStringToGoString(C.OidcAccessTokenGetIssuer(this.p))
}

func (this *AccessToken) GetSubject() string {
    return cStringToGoString(C.OidcAccessTokenGetSubject(this.p))
}

func (this *AccessToken) GetAudience() []string {
    var size int = int(C.OidcAccessTokenGetAudienceSize(this.p))
    var result []string = make([]string, size)
    for i := 0; i < size; i++ {
        result[i] = cStringToGoString(C.OidcAccessTokenGetAudienceEntry(this.p, C.int(i)))
    }
    return result
}

func (this *AccessToken) GetIssueTime() time.Time {
    return time.Unix(int64(C.OidcAccessTokenGetIssueTime(this.p)), 0)
}

func (this *AccessToken) GetExpirationTime() time.Time {
    return time.Unix(int64(C.OidcAccessTokenGetExpirationTime(this.p)), 0)
}

func (this *AccessToken) GetHolderOfKeyPEM() string {
    return cStringToGoString(C.OidcAccessTokenGetHolderOfKeyPEM(this.p))
}

func (this *AccessToken) GetGroups() []string {
    var size int = int(C.OidcAccessTokenGetGroupsSize(this.p))
    var result []string = make([]string, size)
    for i := 0; i < size; i++ {
        result[i] = cStringToGoString(C.OidcAccessTokenGetGroupsEntry(this.p, C.int(i)))
    }
    return result
}

func (this *AccessToken) GetTenant() string {
    return cStringToGoString(C.OidcAccessTokenGetTenant(this.p))
}

func (this *AccessToken) GetStringClaim(key string) string {
    keyCStr := goStringToCString(key)
    defer freeCString(keyCStr)

    var value C.PSTRING = nil
    C.OidcAccessTokenGetStringClaim(
        this.p,
        keyCStr,
        &value)
    return ssoAllocatedStringToGoString(value)
}
