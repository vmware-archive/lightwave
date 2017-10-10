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

type ErrorResponse struct {
    p C.POIDC_ERROR_RESPONSE
}

func errorResponseNew(p C.POIDC_ERROR_RESPONSE) *ErrorResponse {
    var result *ErrorResponse = &ErrorResponse { p }
    runtime.SetFinalizer(result, errorResponseFinalize)
    return result
}

func errorResponseFinalize(this *ErrorResponse) {
    this.Close()
}

func (this *ErrorResponse) Close() error {
    if (this.p != nil) {
        C.OidcErrorResponseDelete(this.p)
        this.p = nil
    }
    return nil
}

func (this *ErrorResponse) GetError() string {
    return cStringToGoString(C.OidcErrorResponseGetError(this.p))
}

func (this *ErrorResponse) GetErrorDescription() string {
    return cStringToGoString(C.OidcErrorResponseGetErrorDescription(this.p))
}
