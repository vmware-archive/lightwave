package oidc

/*
#cgo CFLAGS: -I/root/git/lightwave/vmidentity/ssoclients/common/include/public/
#cgo CFLAGS: -I/root/git/lightwave/vmidentity/ssoclients/oidc/include/public/
#cgo LDFLAGS: -L/root/git/lightwave/vmidentity/build/ssoclients/common/src/.libs/ -l ssocommon
#cgo LDFLAGS: -L/root/git/lightwave/vmidentity/build/ssoclients/oidc/src/.libs/ -l ssooidc
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

// TODO: GetAudience
/*
void
OidcAccessTokenGetAudience(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppzAudience,
    size_t* pAudienceSize);
*/

func (this *AccessToken) GetIssueTime() time.Time {
    return time.Unix(int64(C.OidcAccessTokenGetIssueTime(this.p)), 0)
}

func (this *AccessToken) GetExpirationTime() time.Time {
    return time.Unix(int64(C.OidcAccessTokenGetExpirationTime(this.p)), 0)
}

func (this *AccessToken) GetHolderOfKeyPEM() string {
    return cStringToGoString(C.OidcAccessTokenGetHolderOfKeyPEM(this.p))
}

// TODO: GetGroups
/*
void
OidcAccessTokenGetGroups(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize);
*/

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
