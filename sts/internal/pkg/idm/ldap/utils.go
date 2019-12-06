package ldap

import (
	"crypto"
	"fmt"
	"strconv"
	"strings"
	"time"

	"crypto/x509"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	ldap "gopkg.in/ldap.v2"
)

// helpers for Value interface over common types
// implements Value interface

func ValueForString(v string) (Value, diag.Error)             { return stringValue(v), nil }
func ValueForStrings(v []string) (Value, diag.Error)          { return stringsValue(v), nil }
func ValueForBool(v bool) (Value, diag.Error)                 { return boolValue(v), nil }
func ValueForBools(v []bool) (Value, diag.Error)              { return boolsValue(v), nil }
func ValueForInt(v int) (Value, diag.Error)                   { return intValue(v), nil }
func ValueForUint(v uint) (Value, diag.Error)                 { return uintValue(v), nil }
func ValueForUint32(v uint32) (Value, diag.Error)             { return uint32Value(v), nil }
func ValueForDuration(v time.Duration) (Value, diag.Error)    { return int64Value(int64(v)), nil }
func ValueForInts(v []int) (Value, diag.Error)                { return intsValue(v), nil }
func ValueForBinary(v []byte) (Value, diag.Error)             { return binaryValue(v), nil }
func ValueForBinaries(v [][]byte) (Value, diag.Error)         { return binariesValue(v), nil }
func ValueForCert(v x509.Certificate) (Value, diag.Error)     { return (*certValue)(&v), nil }
func ValueForCertPtr(v *x509.Certificate) (Value, diag.Error) { return (*certValue)(v), nil }
func ValueForPrivateKey(v crypto.PrivateKey) (Value, diag.Error) {

	val, err := x509.MarshalPKCS8PrivateKey(v)
	if err != nil {
		return nil, ldapErrorToDiagError(err)
	}

	return binaryValue(val), nil
}
func ValueForCerts(v []x509.Certificate) (Value, diag.Error)     { return certsValue(v), nil }
func ValueForCertsPtr(v []*x509.Certificate) (Value, diag.Error) { return certsPtrValue(v), nil }
func ValueForUrl(v url.URL) (Value, diag.Error)                  { return (*urlValue)(&v), nil }
func ValueForUrlPtr(v *url.URL) (Value, diag.Error)              { return (*urlValue)(v), nil }
func ValueForUrls(v []url.URL) (Value, diag.Error)               { return urlsValue(v), nil }
func ValueForUrlsPtr(v []*url.URL) (Value, diag.Error)           { return urlsPtrValue(v), nil }

func NewAttribute(name string, val Value) Attribute {
	return &attributeImpl{name: name, val: val}
}

func NewAttributeMod(mod ModType, name string, val Value) AttributeMod {
	return &attributeModImpl{mod: mod, name: name, val: val}
}

func IsNoSuchObjectError(err diag.Error) bool {
	if !diag.IsErrorWithFacility(err, diag.LdapFacility) {
		return false
	}

	return (err.Code().Code() & 0x0000FFFF) == ldap.LDAPResultNoSuchObject
}

func IsAlredyExistsError(err diag.Error) bool {
	if !diag.IsErrorWithFacility(err, diag.LdapFacility) {
		return false
	}

	return (err.Code().Code() & 0x0000FFFF) == ldap.LDAPResultEntryAlreadyExists
}

func IsAttributeOrValueExistsError(err diag.Error) bool {
	if !diag.IsErrorWithFacility(err, diag.LdapFacility) {
		return false
	}

	return (err.Code().Code() & 0x0000FFFF) == ldap.LDAPResultAttributeOrValueExists
}

func GetAttributeValue(e Entry, name string, required bool, ctxt diag.RequestContext) (Value, diag.Error) {

	v, ok, err := e.AttributeValues(name, ctxt)
	if err != nil {
		return nil, err
	}

	if required && (!ok || v == nil || v.Len() <= 0) {
		return nil,
			diag.MakeError(
				ldapError(ldap.LDAPResultOther+diag.LdapFacility),
				fmt.Sprintf("'%s' is required", name), nil)
	}

	if !ok || (v == nil || v.Len() <= 0) {
		return nil, nil
	}

	return v, nil
}

func StringForValue(v Value) (string, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return "", nil
	}

	str := ""

	err := v.IterateString(func(v string) diag.Error {
		str = v
		return nil
	})

	if err != nil {
		return "", err
	}
	return str, nil
}

func StringsForValue(v Value) ([]string, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return []string{}, nil
	}

	res := make([]string, 0, v.Len())

	err := v.IterateString(func(v string) diag.Error {
		res = append(res, v)
		return nil
	})

	if err != nil {
		return []string{}, err
	}
	return res, nil
}

func CertPtrForValue(v Value) (*x509.Certificate, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return nil, nil
	}

	var res *x509.Certificate

	err := v.IterateBinary(func(v []byte) diag.Error {
		c, e := x509.ParseCertificate(v)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		res = c
		return nil
	})

	if err != nil {
		return nil, err
	}
	return res, nil
}

func CertsPtrForValue(v Value) ([]*x509.Certificate, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return []*x509.Certificate{}, nil
	}

	res := make([]*x509.Certificate, 0, v.Len())

	err := v.IterateBinary(func(v []byte) diag.Error {
		c, e := x509.ParseCertificate(v)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		res = append(res, c)
		return nil
	})

	if err != nil {
		return nil, err
	}
	return res, nil
}

func UrlsPtrForValue(v Value) ([]*url.URL, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return []*url.URL{}, nil
	}

	res := make([]*url.URL, 0, v.Len())

	err := v.IterateString(func(v string) diag.Error {
		u, e := url.Parse(v)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		res = append(res, u)
		return nil
	})

	if err != nil {
		return []*url.URL{}, err
	}
	return res, nil
}

func UrlPtrForValue(v Value) (*url.URL, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return nil, nil
	}

	var res *url.URL

	err := v.IterateString(func(v string) diag.Error {
		u, e := url.Parse(v)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		res = u
		return nil
	})

	if err != nil {
		return nil, err
	}
	return res, nil
}

func Uint32ForValue(v Value) (uint32, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return uint32(0), nil
	}

	var val uint32

	err := v.IterateString(func(v string) diag.Error {
		v64, e := strconv.ParseUint(v, 10, 32)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		val = uint32(v64)
		return nil
	})

	if err != nil {
		return 0, err
	}
	return val, nil
}

func Int64ForValue(v Value) (int64, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return 0, nil
	}

	var val int64

	err := v.IterateString(func(v string) diag.Error {
		v64, e := strconv.ParseInt(v, 10, 64)
		if e != nil {
			return ldapErrorToDiagError(e)
		}
		val = v64
		return nil
	})

	if err != nil {
		return 0, err
	}
	return val, nil
}

func DurationForValue(v Value) (time.Duration, diag.Error) {

	val, err := Int64ForValue(v)
	if err != nil {
		return time.Duration(0), err
	}
	return time.Duration(val), nil
}

func PrivateKeyForValue(v Value) (crypto.PrivateKey, diag.Error) {

	val, err := BinaryForValue(v)
	if err != nil {
		return nil, err
	}

	key, e := x509.ParsePKCS8PrivateKey(val)
	if e != nil {
		return nil, ldapErrorToDiagError(e)
	}

	return key, nil
}

func BoolForValue(v Value) (bool, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return false, nil
	}

	var val bool

	err := v.IterateString(func(v string) diag.Error {
		if strings.EqualFold("TRUE", v) {
			val = true
		} else if strings.EqualFold("FALSE", v) {
			val = false
		} else {
			return ldapErrorToDiagError(fmt.Errorf("Unexpected bool value: '%s'", v))
		}
		return nil
	})

	if err != nil {
		return false, err
	}
	return val, nil
}

//func BoolForValue(v Value) bool         {}
//func BoolsForValue(v Value) []bool      {}
//func IntForValue(v Value) int           {}
//func IntsForValue(v Value) []int        {}
func BinaryForValue(v Value) ([]byte, diag.Error) {
	if v == nil || v.Len() <= 0 {
		return []byte{}, nil
	}
	var val []byte
	err := v.IterateBinary(func(v []byte) diag.Error {
		val = v
		return nil
	})
	if err != nil {
		return []byte{}, err
	}

	return val, nil
}

//func BinariesForValue(v Value) [][]byte {}

func DeleteSubtree(conn Connection, dn string, deleteSelf bool, ctxt diag.RequestContext) diag.Error {

	var err diag.Error
	dns := make([]string, 0, 20)
	dns = append(dns, dn)
	processed := make(map[string]struct{})

	for len(dns) > 0 {
		currentDn := dns[len(dns)-1]
		_, ok := processed[currentDn]
		if ok { // delete
			if !deleteSelf && currentDn == dn {
				continue
			}
			dns = dns[:len(dns)-1]
			err = conn.Delete(currentDn, ctxt)
			if err != nil {
				if !IsNoSuchObjectError(err) {
					return err
				}
				err = nil
			}
			continue
		}

		processed[currentDn] = exists

		msg, err := conn.Search(currentDn, ScopeOneLevel, "(objectClass=*)", []string{}, true, 0, 0, ctxt)
		if err != nil {
			if !IsNoSuchObjectError(err) {
				return err
			}
			dns = dns[:len(dns)-1]
			continue
		}
		defer msg.Close()

		err = msg.IterateEntries(func(e Entry, ctxt diag.RequestContext) diag.Error {
			cdn, err := e.Dn(ctxt)
			if err != nil {
				return err
			}
			dns = append(dns, cdn)
			return nil
		}, ctxt)
		if err != nil {
			return err
		}
	}

	return nil
}

var (
	exists = struct{}{}
)
