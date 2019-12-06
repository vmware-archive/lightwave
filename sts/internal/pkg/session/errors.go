package session

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type SessionError uint32

const (
	GenericSessionError SessionError = SessionError(diag.SessionFacility) + 0x001

	genericError = "session_error"
)

var errorText = map[SessionError]string{
	GenericSessionError: genericError,
}

func (c SessionError) Code() uint32 {
	return uint32(c)
}

func (c SessionError) Name() string {
	return errorText[c]
}
