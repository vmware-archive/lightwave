package oidc

import "fmt"

// ErrorCode is unified definition of numeric error codes
type ErrorCode int32

// pre-defined error codes
const (
	OIDCError                      ErrorCode = 100
	OIDCMetadataError              ErrorCode = 101
	OIDCJwksRetrievalError         ErrorCode = 102
	OIDCTokenInvalidSignatureError ErrorCode = 103
	OIDCTokenExpiredError          ErrorCode = 104
	OIDCTokenNotYetValidError      ErrorCode = 105
	OIDCTokenInvalidError          ErrorCode = 106
	OIDCGetTokenError              ErrorCode = 107

	OIDCInvalidRequestError     ErrorCode = 108
	OIDCInvalidScopeError       ErrorCode = 109
	OIDCInvalidGrantError       ErrorCode = 110
	OIDCInvalidClientError      ErrorCode = 111
	OIDCUnauthorizedClientError ErrorCode = 112
	OIDCUnsupportedResponseType ErrorCode = 113
	OIDCUnsupportedGrantType    ErrorCode = 114
	OIDCAccessDeniedError       ErrorCode = 115
	OIDCServerError             ErrorCode = 116
	OIDCRandomGenError          ErrorCode = 117
	OIDCJWSError                ErrorCode = 118
	OIDCJsonError               ErrorCode = 119
	OIDCInvalidArgError         ErrorCode = 120
)

var errorText = map[ErrorCode]string{
	OIDCError:                      "OIDCError",
	OIDCMetadataError:              "OIDCMetadataError",
	OIDCJwksRetrievalError:         "OIDCJwksRetrievalError",
	OIDCTokenInvalidSignatureError: "OIDCTokenInvalidSignatureError",
	OIDCTokenExpiredError:          "OIDCTokenExpiredError",
	OIDCTokenNotYetValidError:      "OIDCTokenNotYetValidError",
	OIDCTokenInvalidError:          "OIDCTokenInvalidError",
	OIDCGetTokenError:              "OIDCGetTokenError",
	OIDCInvalidRequestError:        "OIDCInvalidRequestError",
	OIDCInvalidScopeError:          "OIDCInvalidScopeError",
	OIDCInvalidGrantError:          "OIDCInvalidGrantError",
	OIDCInvalidClientError:         "OIDCInvalidClientError",
	OIDCUnauthorizedClientError:    "OIDCUnauthorizedClientError",
	OIDCUnsupportedResponseType:    "OIDCUnsupportedResponseType",
	OIDCUnsupportedGrantType:       "OIDCUnsupportedGrantType",
	OIDCAccessDeniedError:          "OIDCAccessDeniedError",
	OIDCServerError:                "OIDCServerError",
}

var serverError = map[string]ErrorCode{
	"invalid_request":           OIDCInvalidRequestError,
	"invalid_scope":             OIDCInvalidScopeError,
	"invalid_grant":             OIDCInvalidGrantError,
	"invalid_client":            OIDCInvalidClientError,
	"unauthorized_client":       OIDCUnauthorizedClientError,
	"unsupported_response_type": OIDCUnsupportedResponseType,
	"unsupported_grant_type":    OIDCUnsupportedGrantType,
	"access_denied":             OIDCAccessDeniedError,
	"server_error":              OIDCServerError,
}

// Error is unified error
type Error struct {
	ErrorCode
	Msg string
	Err error
}

// Error implements error
func (e *Error) Error() string {
	if e.Err != nil {
		return fmt.Sprintf("Error [%d:%s]: %s %v", int32(e.ErrorCode), e.Name(), e.Msg, e.Err)
	}

	return fmt.Sprintf("Error [%d:%s]: %s", int32(e.ErrorCode), e.Name(), e.Msg)
}

// Name returns the string name of the error code
func (c ErrorCode) Name() string {
	return errorText[c]
}

// MakeError makes an error from errorcode, message, and optional error
func (c ErrorCode) MakeError(msg string, err error) *Error {
	return &Error{
		ErrorCode: c,
		Msg:       msg,
		Err:       err,
	}
}

// IsErrorWithCode checks whether the specified error is OIDC error with specifier ErrorCode
func IsErrorWithCode(err error, c ErrorCode) bool {
	e, ok := err.(*Error)
	if !ok {
		return false
	}

	return e.ErrorCode == c
}

type jsonErrorResponse struct {
	Error            string `json:"error"`
	ErrorDescription string `json:"error_description"`
}

func (c jsonErrorResponse) makeError() *Error {
	if errorCode, ok := serverError[c.Error]; ok {
		return &Error{
			ErrorCode: errorCode,
			Msg:       c.ErrorDescription,
		}
	}
	return &Error{
		ErrorCode: OIDCServerError,
		Msg:       fmt.Sprintf("error='%v',description='%v'", c.Error, c.ErrorDescription),
	}
}
