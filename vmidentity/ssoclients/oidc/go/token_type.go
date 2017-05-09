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

// TokenType enum
type TokenType string
const TokenType_BEARER  TokenType = "TokenType_BEARER"
const TokenType_HOK     TokenType = "TokenType_HOK"

func cTokenTypeToGoTokenType(cTokenType C.OIDC_TOKEN_TYPE) TokenType {
    var result TokenType
    if cTokenType == C.OIDC_TOKEN_TYPE_BEARER {
        result = TokenType_BEARER
    } else {
        result = TokenType_HOK
    }
    return result
}
