package types

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type idmError uint32

const (
	IdmErrorGeneric idmError = idmError(diag.IdmFacility) + iota + 1
	IdmErrorInvalidArgument
	IdmErrorLdapError
)

const (
	genericError    = "idm_error"
	invalidArgError = "idm_invalid_arg"
	ldapError       = "idm_ldap_error"
)

var errorText = map[idmError]string{
	IdmErrorGeneric:         genericError,
	IdmErrorInvalidArgument: invalidArgError,
	IdmErrorLdapError:       ldapError,
}

func (c idmError) Code() uint32 {
	return uint32(c)
}

func (c idmError) Name() string {
	return errorText[c]
}
