package ldap

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"net"
	"net/url"
	"strconv"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"gopkg.in/ldap.v2"
)

type connectionFactoryImpl struct {
}

func (cf *connectionFactoryImpl) Connection(
	uri *url.URL, userName string, password string,
	bind BindType, certs TrustedCertsFunc, ctxt diag.RequestContext) (Connection, diag.Error) {

	logger := ctxt.Logger()

	if uri == nil {
		return nil, ldapErrorToDiagError(fmt.Errorf("Invalid uri (nil)"))
	}

	if !SupportedScheme(uri.Scheme) {
		return nil, ldapErrorToDiagError(fmt.Errorf("Unsupported uri scheme '%s'", uri.Scheme))
	}

	var tc *tls.Config

	if isLdaps(uri) {
		trustedCerts, err := certs(false, ctxt)
		if err != nil {
			return nil, err
		}
		tc = &tls.Config{
			InsecureSkipVerify: false,
		}
		if trustedCerts == nil {
			tc.InsecureSkipVerify = true
		} else {
			tc.RootCAs = trustedCerts
		}
	}

	var err diag.Error
	c := &connectionImpl{}

	if isLwDomainLdap(uri) {
		c.conn, err = dialLwDomain(tc, uri, certs, ctxt)
	} else {
		if tc != nil {
			tc.ServerName = uri.Hostname()
		}
		c.conn, err = dial(tc, uri.Host, certs, ctxt)
	}
	if err != nil {
		return nil, err
	}

	// TODO: implement srp
	e := c.conn.Bind(userName, password)
	if e != nil {
		logger.Errorf(diag.LDAP, "ldap bind (%s) connection failed: %v", userName, e)
		c.conn.Close()
		return nil, ldapErrorToDiagError(e)
	}

	return c, nil
}
func dial(tc *tls.Config, host string, trustedCerts TrustedCertsFunc, ctxt diag.RequestContext) (*ldap.Conn, diag.Error) {

	var conn *ldap.Conn
	var err error

	if tc == nil {
		conn, err = ldap.Dial("tcp", host)
	} else {
		conn, err = ldap.DialTLS("tcp", host, tc)
		if err != nil {
			_, ok := err.(x509.UnknownAuthorityError)
			if ok && trustedCerts != nil {
				roots, err1 := trustedCerts(true, ctxt)
				if err1 == nil {
					tc.RootCAs = roots
					conn, err = ldap.DialTLS("tcp", host, tc)
				}
			}
		}
	}
	if err != nil {
		ctxt.Logger().Errorf(diag.LDAP, "Unable to dial ldap connection to '%s': %v", host, err)
		return nil, ldapErrorToDiagError(err)
	}
	return conn, nil
}

func isLdaps(uri *url.URL) bool {
	return strings.EqualFold(uri.Scheme, SchemeLdaps) || strings.EqualFold(uri.Scheme, SchemeLwDomainLdaps)
}
func isLwDomainLdap(uri *url.URL) bool {
	return strings.EqualFold(uri.Scheme, SchemeLwDomainLdaps) || strings.EqualFold(uri.Scheme, SchemeLwDomainLdap)
}

func dialLwDomain(tc *tls.Config, uri *url.URL, certs TrustedCertsFunc, ctxt diag.RequestContext) (*ldap.Conn, diag.Error) {
	// consider caching
	domain := uri.Hostname()
	longname := domain + "."
	name := domain + "."
	if len(uri.RawPath) > 0 {
		site, e := url.PathUnescape(uri.RawPath)
		if e != nil {
			return nil, diag.MakeError(
				types.IdmErrorGeneric, fmt.Sprintf("Invalid lw domain url '%s': %v", uri.Scheme, e), e)
		}
		longname = fmt.Sprintf("%s._sites.%s.", site, domain)
	}
	// lookup site specific ldap SRV record first
	_, r, e := net.LookupSRV("ldap", "tcp", longname)
	if e != nil {
	        // lookup non-site specific ldap SRV record if needed
	        _, rlong, e := net.LookupSRV("ldap", "tcp", name)
	        if e != nil {
		        return nil, diag.MakeError(types.IdmErrorGeneric,
			        fmt.Sprintf("Unable to lookup ldap records for domain '%s' name '%s': %v", domain, name, e), e)
	        }
	        r = rlong;
	}
	if len(r) <= 0 {
		return nil, diag.MakeError(types.IdmErrorGeneric,
			fmt.Sprintf("Unable to find ldap records for domain '%s' name '%s'", domain, name), e)
	}

	// todo: this should ideally be read from srv record, but now
	// it is not accurate
	port := ":636"
	if !isLdaps(uri) {
		port = ":389"
	}

	var err diag.Error
	var conn *ldap.Conn
	var origSrvName string

	if tc != nil {
		origSrvName = tc.ServerName
	}

	for _, s := range r {
		if tc != nil {
			tc.ServerName = s.Target
		}
		conn, err = dial(tc, s.Target+port, certs, ctxt)
		if tc != nil {
			tc.ServerName = origSrvName
		}
		if err == nil {
			return conn, nil
		}
	}
	return nil, err
}

func ldapErrorToDiagError(err error) diag.Error {
	le, ok := err.(*ldap.Error)
	if ok {
		return diag.MakeError(
			ldapError(int32(le.ResultCode)|int32(diag.LdapFacility)), le.Error(), le.Err)
	}
	return diag.MakeError(ldapError(ldap.LDAPResultOther|diag.LdapFacility), err.Error(), err)
}

type connectionImpl struct {
	conn *ldap.Conn
}

func (c *connectionImpl) Close() {
	if c.conn != nil {
		c.conn.Close()
		c.conn = nil
	}
}

func (c *connectionImpl) Add(dn string, attributes []Attribute, ctxt diag.RequestContext) diag.Error {
	req := ldap.NewAddRequest(dn)
	for _, a := range attributes {
		vals := make([]string, 0, a.Value().Len())
		a.Value().IterateString(func(s string) diag.Error {
			vals = append(vals, s)
			return nil
		})
		req.Attribute(a.Name(), vals)
	}

	err := c.conn.Add(req)
	if err != nil {
		return ldapErrorToDiagError(err)
	}
	return nil
}

func (c *connectionImpl) Delete(dn string, ctxt diag.RequestContext) diag.Error {
	req := ldap.NewDelRequest(dn, []ldap.Control{})
	err := c.conn.Del(req)
	if err != nil {
		return ldapErrorToDiagError(err)
	}
	return nil
}

func (c *connectionImpl) Modify(dn string, mods []AttributeMod, ctxt diag.RequestContext) diag.Error {
	req := ldap.NewModifyRequest(dn)

	for _, m := range mods {
		a := m.Attribute()
		vals := make([]string, 0, a.Value().Len())
		a.Value().IterateString(func(s string) diag.Error {
			vals = append(vals, s)
			return nil
		})
		switch m.Mod() {
		case ModTypeAdd:
			req.Add(a.Name(), vals)
		case ModTypeDelete:
			req.Delete(a.Name(), vals)
		case ModTypeReplace:
			req.Replace(a.Name(), vals)
		}
	}

	err := c.conn.Modify(req)
	if err != nil {
		return ldapErrorToDiagError(err)
	}
	return nil
}

func (c *connectionImpl) Search(base string, scope Scope, filter string,
	attributes []string, attributesOnly bool, sizeLimit int, timeLimit int,
	ctxt diag.RequestContext) (Message, diag.Error) {
	req := ldap.NewSearchRequest(
		base, int(scope), 0, sizeLimit, timeLimit,
		attributesOnly, filter, attributes, []ldap.Control{})

	sr, err := c.conn.Search(req)
	if err != nil {
		return nil, ldapErrorToDiagError(err)
	}

	return (*searchResultWrapper)(sr), nil
}

func (c *connectionImpl) SearchPage(base string, scope Scope, filter string,
	attributes []string, attributesOnly bool, pagingSize uint32,
	sizeLimit int, timeLimit int, pagingCtxt PagingCtxt, ctxt diag.RequestContext) (Page, diag.Error) {

	if pagingCtxt != nil && !pagingCtxt.More() {
		return nil, nil // todo: error?
	}

	pageCtrl := ldap.NewControlPaging(pagingSize)

	if pagingCtxt != nil {
		pageCtrl.SetCookie(pagingCtxt.Cookie())
	}

	req := ldap.NewSearchRequest(
		base, int(scope), 0, sizeLimit, timeLimit,
		attributesOnly, filter, attributes, []ldap.Control{ldap.NewControlPaging(pagingSize)})

	sr, err := c.conn.Search(req)
	if err != nil {
		return nil, ldapErrorToDiagError(err)
	}

	return (*searchResultWrapper)(sr), nil
}

type entryWrapper ldap.Entry

func (e *entryWrapper) Dn(ctxt diag.RequestContext) (string, diag.Error) { return e.DN, nil }

func (e *entryWrapper) AttributeNames(ctxt diag.RequestContext) ([]string, diag.Error) {
	vals := make([]string, 0, len(e.Attributes))
	for _, an := range e.Attributes {
		if an != nil {
			vals = append(vals, an.Name)
		}
	}
	return vals, nil
}

func (e *entryWrapper) AttributeValues(name string, ctxt diag.RequestContext) (Value, bool, diag.Error) {
	for _, attr := range e.Attributes {
		if strings.EqualFold(attr.Name, name) {
			return stringsValue(attr.Values), true, nil
		}
	}

	return stringValue(""), false, nil
}

type searchResultWrapper ldap.SearchResult

func (s *searchResultWrapper) Close() {} // noop
func (s *searchResultWrapper) Len(ctxt diag.RequestContext) (int, diag.Error) {
	if s == nil {
		return 0, nil
	}

	return len(s.Entries), nil
}

func (s *searchResultWrapper) IterateEntries(f EntryIteratorFunc, ctxt diag.RequestContext) diag.Error {
	if s == nil {
		return nil
	}

	for _, e := range s.Entries {
		if e != nil {
			err := f((*entryWrapper)(e), ctxt)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

func (s *searchResultWrapper) Message() Message { return s }
func (s *searchResultWrapper) More() bool {
	if s == nil {
		return false
	}
	return len(s.Cookie()) > 0
}

func (s *searchResultWrapper) Cookie() []byte {
	if s == nil {
		return []byte{}
	}
	pagingResult := ldap.FindControl(s.Controls, ldap.ControlTypePaging)
	if pagingResult == nil {
		return []byte{}
	}

	return pagingResult.(*ldap.ControlPaging).Cookie
}

type attributeImpl struct {
	name string
	val  Value
}

func (a *attributeImpl) Name() string { return a.name }

func (a *attributeImpl) Value() Value { return a.val }

type attributeModImpl struct {
	mod  ModType
	name string
	val  Value
}

func (a *attributeModImpl) Mod() ModType { return a.mod }

func (a *attributeModImpl) Attribute() Attribute { return a }

func (a *attributeModImpl) Name() string { return a.name }

func (a *attributeModImpl) Value() Value { return a.val }

type ldapError int32

func (e ldapError) Code() uint32 { return uint32(e) }
func (e ldapError) Name() string { return ldap.LDAPResultCodeMap[uint8(e)] }

type stringValue string
type stringsValue []string
type boolValue bool
type boolsValue []bool
type intValue int
type int64Value int64
type uintValue uint
type uint32Value uint32
type intsValue []int
type binaryValue []byte
type binariesValue [][]byte
type certValue x509.Certificate
type certsValue []x509.Certificate
type certsPtrValue []*x509.Certificate
type urlValue url.URL
type urlsValue []url.URL
type urlsPtrValue []*url.URL

func (v stringValue) Len() int {
	if len(v) > 0 {
		return 1
	}
	return 0
}

func (v stringValue) IterateString(f StringValueFunc) diag.Error {
	if len(v) > 0 {
		return f(string(v))
	}
	return nil
}

func (v stringValue) IterateBinary(f BinaryValueFunc) diag.Error {
	if len(v) > 0 {
		return f([]byte(v))
	}
	return nil
}

func (v stringsValue) Len() int {
	leng := 0
	if len(v) > 0 {
		for _, vi := range v {
			if len(vi) > 0 {
				leng = leng + 1
			}
		}
	}
	return leng
}

func (v stringsValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		if len(vi) > 0 {
			err = f(vi)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (v stringsValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		if len(vi) > 0 {
			err = f([]byte(vi))
			if err != nil {
				return err
			}
		}
	}

	return nil
}

func (v boolValue) Len() int {
	return 1
}

func (v boolValue) IterateString(f StringValueFunc) diag.Error {
	if v {
		return f("TRUE")
	}
	return f("FALSE")
}

func (v boolValue) IterateBinary(f BinaryValueFunc) diag.Error {
	if v {
		return f([]byte("TRUE"))
	}
	return f([]byte("FALSE"))
}

func (v boolsValue) Len() int {
	return len(v)
}

func (v boolsValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	var s string
	for _, vi := range v {
		if vi {
			s = "TRUE"
		} else {
			s = "FALSE"
		}
		err = f(s)
		if err != nil {
			return err
		}
	}
	return nil
}

func (v boolsValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	var s string
	for _, vi := range v {
		if vi {
			s = "TRUE"
		} else {
			s = "FALSE"
		}
		err = f([]byte(s))
		if err != nil {
			return err
		}
	}

	return nil
}

func (v intValue) Len() int {
	return 1
}

func (v intValue) IterateString(f StringValueFunc) diag.Error {
	return f(strconv.FormatInt(int64(v), 10))
}

func (v intValue) IterateBinary(f BinaryValueFunc) diag.Error {
	return f([]byte(strconv.FormatInt(int64(v), 10)))
}

func (v int64Value) Len() int {
	return 1
}

func (v int64Value) IterateString(f StringValueFunc) diag.Error {
	return f(strconv.FormatInt(int64(v), 10))
}

func (v int64Value) IterateBinary(f BinaryValueFunc) diag.Error {
	return f([]byte(strconv.FormatInt(int64(v), 10)))
}

func (v uintValue) Len() int {
	return 1
}

func (v uintValue) IterateString(f StringValueFunc) diag.Error {
	return f(strconv.FormatUint(uint64(v), 10))
}

func (v uintValue) IterateBinary(f BinaryValueFunc) diag.Error {
	return f([]byte(strconv.FormatUint(uint64(v), 10)))
}

func (v uint32Value) Len() int {
	return 1
}

func (v uint32Value) IterateString(f StringValueFunc) diag.Error {
	return f(strconv.FormatUint(uint64(v), 10))
}

func (v uint32Value) IterateBinary(f BinaryValueFunc) diag.Error {
	return f([]byte(strconv.FormatUint(uint64(v), 10)))
}

func (v intsValue) Len() int {
	return len(v)
}

func (v intsValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		err = f(strconv.FormatInt(int64(vi), 10))
		if err != nil {
			return err
		}
	}
	return nil
}

func (v intsValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		err = f([]byte(strconv.FormatInt(int64(vi), 10)))
		if err != nil {
			return err
		}
	}

	return nil
}

func (v binaryValue) Len() int {
	if len(v) > 0 {
		return 1
	}
	return 0
}

func (v binaryValue) IterateString(f StringValueFunc) diag.Error {
	if len(v) > 0 {
		return f(string(v))
	}
	return nil
}

func (v binaryValue) IterateBinary(f BinaryValueFunc) diag.Error {
	if len(v) > 0 {
		return f(v)
	}
	return nil
}

func (v binariesValue) Len() int {
	leng := 0
	if len(v) > 0 {
		for _, vi := range v {
			if len(vi) > 0 {
				leng = leng + 1
			}
		}
	}
	return leng
}

func (v binariesValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		if len(vi) > 0 {
			err = f(string(vi))
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (v binariesValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		if len(vi) > 0 {
			err = f(vi)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

func (v *certValue) Len() int {
	if v == nil {
		return 0
	}

	return 1
}

func (v *certValue) IterateString(f StringValueFunc) diag.Error {
	if v == nil {
		return nil
	}

	return f(string((*x509.Certificate)(v).Raw))
}

func (v *certValue) IterateBinary(f BinaryValueFunc) diag.Error {
	if v == nil {
		return nil
	}

	return f((*x509.Certificate)(v).Raw)
}

func (v certsValue) Len() int {
	return len(v)
}

func (v certsValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	return v.IterateBinary(func(vi []byte) diag.Error {
		err = f(string(vi))
		if err != nil {
			return err
		}

		return nil
	})
}

func (v certsValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		err = f(x509.Certificate(vi).Raw)
		if err != nil {
			return err
		}
	}

	return nil
}

func (v certsPtrValue) Len() int {
	leng := 0
	if len(v) > 0 {
		for _, vi := range v {
			if vi != nil {
				leng = leng + 1
			}
		}
	}
	return leng
}

func (v certsPtrValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	return v.IterateBinary(func(vi []byte) diag.Error {
		err = f(string(vi))
		if err != nil {
			return err
		}
		return nil
	})
}

func (v certsPtrValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, vi := range v {
		if vi != nil {
			err = f((*x509.Certificate)(vi).Raw)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

func (v *urlValue) Len() int {
	if v == nil {
		return 0
	}

	return 1
}

func (v *urlValue) IterateString(f StringValueFunc) diag.Error {
	if v == nil {
		return nil
	}

	return f((*url.URL)(v).String())
}

func (v *urlValue) IterateBinary(f BinaryValueFunc) diag.Error {
	if v == nil {
		return nil
	}

	return f([]byte((*url.URL)(v).String()))
}

func (v urlsValue) Len() int {
	return len(v)
}

func (v urlsValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	for _, u := range v {
		err = f(u.String())
		if err != nil {
			return err
		}
	}
	return nil
}

func (v urlsValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, u := range v {
		err = f([]byte(u.String()))
		if err != nil {
			return err
		}
	}
	return nil
}

func (v urlsPtrValue) Len() int {
	leng := 0
	if len(v) > 0 {
		for _, u := range v {
			if u != nil {
				leng = leng + 1
			}
		}
	}
	return leng
}

func (v urlsPtrValue) IterateString(f StringValueFunc) diag.Error {
	var err diag.Error
	for _, u := range v {
		if u != nil {
			err = f(u.String())
			if err != nil {
				return err
			}
		}
	}
	return nil
}

func (v urlsPtrValue) IterateBinary(f BinaryValueFunc) diag.Error {
	var err diag.Error
	for _, u := range v {
		if u != nil {
			err = f([]byte(u.String()))
			if err != nil {
				return err
			}
		}
	}
	return nil
}
