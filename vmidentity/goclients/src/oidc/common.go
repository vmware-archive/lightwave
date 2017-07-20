package oidc

/*
#include <stdlib.h>
#include "ssotypes.h"
#include "ssoerrors.h"
#include "ssocommon.h"
#include "oidc_types.h"
#include "oidc.h"
*/
import "C"

import "errors"
import "unsafe"

// returns a c string allocated on the c heap, needs to be freed via freeCString()
func goStringToCString(gostr string) (cstr C.PSTRING) {
    if gostr == "" {
        cstr = nil // go's empty string becomes null as that is how we do OPTIONAL parameters in c
    } else {
        cstr = C.CString(gostr)
    }
    return cstr
}

func cStringToGoString(cstr C.PCSTRING) (gostr string) {
    if cstr == nil {
        gostr = ""
    } else {
        gostr = C.GoString(cstr)
    }
    return gostr
}

// free c string after copying it to go string
func ssoAllocatedStringToGoString(cstr C.PSTRING) (gostr string) {
    if cstr == nil {
        gostr = ""
    } else {
        gostr = C.GoString(cstr)
        C.SSOStringFree(cstr)
    }
    return gostr
}

func cErrorToGoError(e C.SSOERROR) (err error) {
    return errors.New(C.GoString(C.SSOErrorToString(e)))
}

func freeCString(cstr C.PSTRING) {
    if (cstr != nil) {
        C.free(unsafe.Pointer(cstr))
    }
}