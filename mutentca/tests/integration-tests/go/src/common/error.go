package common

import (
	"fmt"
	"log"
)

// ErrorCode is the error code type
type ErrorCode int32

// pre-defined error codes
const (
	LwCAInvalidCert   ErrorCode = 1501
	LwCAKeyGeneration ErrorCode = 1502
	LwCAInvalidCSR    ErrorCode = 1503
)

var errorMap = map[ErrorCode]LwCATestError{
	LwCAInvalidCert:   {ErrorCode: LwCAInvalidCert, Msg: "Invalid certificate"},
	LwCAKeyGeneration: {ErrorCode: LwCAKeyGeneration, Msg: "Unable to generate key"},
	LwCAInvalidCSR:    {ErrorCode: LwCAInvalidCSR, Msg: "Invalid CSR"},
}

// MakeError makes an error from message, and optional error
func (c ErrorCode) MakeError(detail string, err error) *LwCATestError {
	e := errorMap[c]
	return e.WithDetail(detail).WithCause(err)
}

// MakeErr makes an error with default message
func (c ErrorCode) MakeErr() *LwCATestError {
	e := errorMap[c]
	return &e
}

// LwCATestError is unified error
type LwCATestError struct {
	// ErrorCode is the numerical error code.
	ErrorCode
	// Msg provides a terse description of the error.
	Msg string
	// Cause is the internal error.
	Cause error
	// Detail provides a detailed description of the error. Its value is set using WithDetail.
	Detail string
	// Data provides an optional string-string map that provides additional detail and data.
	Data map[string]string
}

// Error implements error
func (e LwCATestError) Error() string {
	msg := fmt.Sprintf("Error [%d]: %s", int32(e.ErrorCode), e.Msg)
	if e.Detail != "" {
		msg = fmt.Sprintf("%s, detail: %s", msg, e.Detail)
	}
	return msg
}

// WithDetail adds a detailed message to error
func (e *LwCATestError) WithDetail(format string, v ...interface{}) *LwCATestError {
	e.Detail = fmt.Sprintf(format, v...)
	return e
}

// AppendDetail appends a new detailed message to existing detailed message
func (e *LwCATestError) AppendDetail(detail string) *LwCATestError {
	if len(e.Detail) > 0 {
		e.Detail = e.Detail + "." + detail
	} else {
		e.Detail = detail
	}
	return e
}

// GetDetail gets the detailed message of error
func (e *LwCATestError) GetDetail() string {
	if e.Detail != "" {
		return e.Detail
	}
	if e.Cause != nil {
		return e.Cause.Error()
	}
	return ""
}

// WithCause adds CauseBy to error
func (e *LwCATestError) WithCause(err error) *LwCATestError {
	e.Cause = err
	return e
}

// WithResource adds Resource to error
func (e *LwCATestError) WithResource(kind, ID string) *LwCATestError {
	e.WithData("kind", kind, "id", ID)
	return e
}

// WithData adds Data to error
func (e *LwCATestError) WithData(data ...string) *LwCATestError {
	// make sure number of data is even (key/value pairs)
	if (len(data) & 1) != 0 {
		log.Printf("LwCATestError.WithData: data is not pairs of key/value: %v", data)
		e.Msg = fmt.Sprintf("%s, data: %v", e.Msg, data)
	} else {
		if e.Data == nil {
			e.Data = make(map[string]string)
		}
		for i := 0; i < len(data); i += 2 {
			e.Data[data[i]] = data[i+1]
		}
	}
	return e
}
