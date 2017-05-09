package main

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

type TokenSuccessResponse struct {
    p C.POIDC_TOKEN_SUCCESS_RESPONSE
}

func tokenSuccessResponseNew(p C.POIDC_TOKEN_SUCCESS_RESPONSE) *TokenSuccessResponse {
    var result *TokenSuccessResponse = &TokenSuccessResponse { p }
    runtime.SetFinalizer(result, tokenSuccessResponseFinalize)
    return result
}

func tokenSuccessResponseFinalize(this *TokenSuccessResponse) {
    this.Close()
}

func (this *TokenSuccessResponse) Close() error {
    if (this.p != nil) {
        C.OidcTokenSuccessResponseDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *TokenSuccessResponse) GetIDToken() string {
    return cStringToGoString(C.OidcTokenSuccessResponseGetIDToken(this.p))
}

func (this *TokenSuccessResponse) GetAccessToken() string {
    return cStringToGoString(C.OidcTokenSuccessResponseGetAccessToken(this.p))
}

func (this *TokenSuccessResponse) GetRefreshToken() string {
    return cStringToGoString(C.OidcTokenSuccessResponseGetRefreshToken(this.p))
}
