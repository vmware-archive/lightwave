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

type IDToken struct {
    p C.POIDC_ID_TOKEN
}

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

// TODO: GetAudience
/*
void
OidcIDTokenGetAudience(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppzAudience,
    size_t* pAudienceSize);
*/

func (this *IDToken) GetIssueTime() time.Time {
    return time.Unix(int64(C.OidcIDTokenGetIssueTime(this.p)), 0)
}

func (this *IDToken) GetExpirationTime() time.Time {
    return time.Unix(int64(C.OidcIDTokenGetExpirationTime(this.p)), 0)
}

func (this *IDToken) GetHolderOfKeyPEM() string {
    return cStringToGoString(C.OidcIDTokenGetHolderOfKeyPEM(this.p))
}

// TODO: GetGroups
/*
void
OidcIDTokenGetGroups(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize);
*/

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
