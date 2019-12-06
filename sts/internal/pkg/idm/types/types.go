package types

import (
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type ClientID string

func (cid ClientID) String() string          { return string(cid) }
func ClientIDFromString(cid string) ClientID { return ClientID(cid) }

// idm:"enum:vals=none password,set"
type LoginMethod uint8

type UserID string

const (
	NoneUserID UserID = UserID("")
)

func (uid UserID) String() string { return string(uid) }
func (uid *UserID) From(s string) diag.Error {
	if uid == nil {
		return diag.MakeError(IdmErrorGeneric, "Unable to unmarshal UserID to nil", nil)
	}
	*uid = UserID(s)
	return nil
}
func UserIDFromString(uid string) (UserID, diag.Error) { return UserID(uid), nil }

func DomainFromUserID(uid UserID) string {
	str := uid.String()
	at := strings.Split(str, "@")
	return at[len(at)-1]
}

func DnFromDomain(domain string) string {
	b := &strings.Builder{}
	comps := strings.Split(domain, ".")
	for i, c := range comps {
		if i > 0 {
			b.WriteString(",")
		}
		b.WriteString("dc=")
		b.WriteString(c)
	}
	return b.String()
}

func DomainFromDn(domain string) string {
	b := &strings.Builder{}
	comps := strings.Split(domain, ",")
	for i, c := range comps {
		if strings.HasPrefix(c, "dc=") || strings.HasPrefix(c, "DC=") {
			if i > 0 {
				b.WriteString(".")
			}
			b.WriteString(string([]byte(c)[3:]))
		}
	}
	return b.String()
}
