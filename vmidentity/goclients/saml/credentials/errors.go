package credentials

import "fmt"

// ErrorCode is unified definition of numeric error codes
type ErrorCode int32

// pre-defined error codes
const (
	SAMLError                      ErrorCode = 100
	SAMLMetadataError              ErrorCode = 101
	SAMLTokenInvalidSignatureError ErrorCode = 102
	SAMLTokenExpiredError          ErrorCode = 103
	SAMLTokenNotYetValidError      ErrorCode = 104
	SAMLTokenInvalidError          ErrorCode = 105
	SAMLGetTokenError              ErrorCode = 106

	SAMLInvalidRequestError     ErrorCode = 107
	SAMLInvalidClientError      ErrorCode = 108
	SAMLUnauthorizedClientError ErrorCode = 109
	SAMLUnsupportedOperation    ErrorCode = 110
	SAMLAccessDeniedError       ErrorCode = 111
	SAMLServerError             ErrorCode = 112
	SAMLInvalidArgError         ErrorCode = 113
	SAMLMetadataRetrievalError  ErrorCode = 114
	SAMLParseError              ErrorCode = 115
)

var errorText = map[ErrorCode]string{
	SAMLError:                      "SAMLError",
	SAMLMetadataError:              "SAMLMetadataError",
	SAMLTokenInvalidSignatureError: "SAMLTokenInvalidSignatureError",
	SAMLTokenExpiredError:          "SAMLTokenExpiredError",
	SAMLTokenNotYetValidError:      "SAMLTokenNotYetValidError",
	SAMLTokenInvalidError:          "SAMLTokenInvalidError",
	SAMLGetTokenError:              "SAMLGetTokenError",
	SAMLInvalidRequestError:        "SAMLInvalidRequestError",
	SAMLInvalidClientError:         "SAMLInvalidClientError",
	SAMLUnauthorizedClientError:    "SAMLUnauthorizedClientError",
	SAMLUnsupportedOperation:       "SAMLUnsupportedOperation",
	SAMLAccessDeniedError:          "SAMLAccessDeniedError",
	SAMLServerError:                "SAMLServerError",
	SAMLMetadataRetrievalError:     "SAMLMetadataRetrievalError",
	SAMLParseError:                 "SAMLParseError",
}

// Error is unified error
type Error struct {
	ErrorCode
	Msg    string
	Detail string
}

// Error implements error
func (e *Error) Error() string {
	return fmt.Sprintf("Error [%d:%s] [Msg: %s] [Detail: %s]", int32(e.ErrorCode), e.Name(), e.Msg, e.Detail)
}

// WithMsg overwrites the default error message
func (e *Error) WithMsg(format string, v ...interface{}) *Error {
	e.Msg = fmt.Sprintf(format, v...)
	return e
}

// WithDetail adds a detailed message to error
func (e *Error) WithDetail(format string, v ...interface{}) *Error {
	if len(e.Detail) == 0 {
		e.Detail = fmt.Sprintf(format, v...)
	} else {
		e.Detail += ", " + fmt.Sprintf(format, v...)
	}
	return e
}

// WithCause adds CauseBy to error
func (e *Error) WithCause(err error) *Error {
	if err != nil {
		// Store error message in Detail, so the info can be preserved
		// when CascadeError is marshaled to json.
		if len(e.Detail) == 0 {
			e.Detail = err.Error()
		} else {
			e.Detail += ", cause: " + err.Error()
		}
	}
	return e
}

// Name returns the string name of the error code
func (c ErrorCode) Name() string {
	return errorText[c]
}

// MakeError makes an error from errorcode, message, and optional error
func (c ErrorCode) MakeError(msg string, detail string, err error) *Error {
	e := &Error{
		ErrorCode: c,
		Msg:       msg,
		Detail:    detail,
	}
	return e.WithCause(err)
}

// IsErrorWithCode checks whether the specified error is SAML error with specifier ErrorCode
func IsErrorWithCode(err error, c ErrorCode) bool {
	e, ok := err.(*Error)
	if !ok {
		return false
	}

	return e.ErrorCode == c
}
