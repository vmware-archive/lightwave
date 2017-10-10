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

type IDToken struct {
    p C.POIDC_ID_TOKEN
}

// (TODO) Deprecated
func IDTokenBuild(
        jwt string,
        signingCertificatePEM string,
        issuer string,
        clockToleranceInSeconds int64) (result *IDToken, err error) {
    jwtCStr                     := goStringToCString(jwt)
    signingCertificatePEMCStr   := goStringToCString(signingCertificatePEM)
    issuerCStr                  := goStringToCString(issuer)

    defer freeCString(jwtCStr)
    defer freeCString(signingCertificatePEMCStr)
    defer freeCString(issuerCStr)

    var p C.POIDC_ID_TOKEN = nil
    var e C.SSOERROR = C.OidcIDTokenBuild(
        &p,
        jwtCStr,
        signingCertificatePEMCStr,
        issuerCStr,
        C.SSO_LONG(clockToleranceInSeconds))
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &IDToken { p }
    runtime.SetFinalizer(result, idTokenFinalize)
    return
}

// on success, result will be non-null, Close it when done
func IDTokenParse(jwt string) (result *IDToken, err error) {
    jwtCStr := goStringToCString(jwt)
    defer freeCString(jwtCStr)

    var p C.POIDC_ID_TOKEN = nil
    var e C.SSOERROR = C.OidcIDTokenParse(&p, jwtCStr)
    if e != 0 {
        err = cErrorToGoError(e)
        return
    }

    result = &IDToken { p }
    runtime.SetFinalizer(result, idTokenFinalize)
    return
}

func (this *IDToken) IDTokenValidate(
        signingCertificatePEM string,
        issuer string,
        clockToleranceInSeconds int64) error {
    signingCertificatePEMCStr   := goStringToCString(signingCertificatePEM)
    issuerCStr                  := goStringToCString(issuer)

    defer freeCString(signingCertificatePEMCStr)
    defer freeCString(issuerCStr)

    var e C.SSOERROR = C.OidcIDTokenValidate(
        this.p,
        signingCertificatePEMCStr,
        issuerCStr,
        C.SSO_LONG(clockToleranceInSeconds))

    var err error = nil
    if e != 0 {
        err = cErrorToGoError(e)
    }
    return err
}

func idTokenFinalize(this *IDToken) {
    this.Close()
}

func (this *IDToken) Close() error {
    if (this.p != nil) {
        C.OidcIDTokenDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *IDToken) GetTokenType() TokenType {
    return cTokenTypeToGoTokenType(C.OidcIDTokenGetTokenType(this.p))
}

func (this *IDToken) GetIssuer() string {
    return cStringToGoString(C.OidcIDTokenGetIssuer(this.p))
}

func (this *IDToken) GetSubject() string {
    return cStringToGoString(C.OidcIDTokenGetSubject(this.p))
}

func (this *IDToken) GetAudience() []string {
    var size int = int(C.OidcIDTokenGetAudienceSize(this.p))
    var result []string = make([]string, size)
    for i := 0; i < size; i++ {
        result[i] = cStringToGoString(C.OidcIDTokenGetAudienceEntry(this.p, C.int(i)))
    }
    return result
}

func (this *IDToken) GetIssueTime() time.Time {
    return time.Unix(int64(C.OidcIDTokenGetIssueTime(this.p)), 0)
}

func (this *IDToken) GetExpirationTime() time.Time {
    return time.Unix(int64(C.OidcIDTokenGetExpirationTime(this.p)), 0)
}

func (this *IDToken) GetHolderOfKeyPEM() string {
    return cStringToGoString(C.OidcIDTokenGetHolderOfKeyPEM(this.p))
}

func (this *IDToken) GetGroups() []string {
    var size int = int(C.OidcIDTokenGetGroupsSize(this.p))
    var result []string = make([]string, size)
    for i := 0; i < size; i++ {
        result[i] = cStringToGoString(C.OidcIDTokenGetGroupsEntry(this.p, C.int(i)))
    }
    return result
}

func (this *IDToken) GetTenant() string {
    return cStringToGoString(C.OidcIDTokenGetTenant(this.p))
}

func (this *IDToken) GetStringClaim(key string) string {
    keyCStr := goStringToCString(key)
    defer freeCString(keyCStr)

    var value C.PSTRING = nil
    C.OidcIDTokenGetStringClaim(
        this.p,
        keyCStr,
        &value)
    return ssoAllocatedStringToGoString(value)
}
