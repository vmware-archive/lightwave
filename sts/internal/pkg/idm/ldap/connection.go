package ldap

import (
	"crypto/x509"
	"net/url"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type StringValueFunc func(s string) diag.Error
type BinaryValueFunc func(s []byte) diag.Error

type Value interface {
	Len() int
	IterateString(f StringValueFunc) diag.Error
	IterateBinary(f BinaryValueFunc) diag.Error
}

type Attribute interface {
	Name() string
	Value() Value
}

type ModType uint8

const (
	ModTypeAdd ModType = iota
	ModTypeDelete
	ModTypeReplace
)

type AttributeMod interface {
	Mod() ModType
	Attribute() Attribute
}

type Scope uint8

const (
	ScopeBase Scope = iota
	ScopeOneLevel
	ScopeSubTree
)

type Entry interface {
	Dn(ctxt diag.RequestContext) (string, diag.Error)
	AttributeNames(ctxt diag.RequestContext) ([]string, diag.Error)
	AttributeValues(name string, ctxt diag.RequestContext) (Value, bool, diag.Error)
}

type EntryIteratorFunc func(e Entry, ctxt diag.RequestContext) diag.Error

type Message interface {
	Close()
	Len(ctxt diag.RequestContext) (int, diag.Error) // number of entries
	IterateEntries(f EntryIteratorFunc, ctxt diag.RequestContext) diag.Error
}

type PagingCtxt interface {
	Cookie() []byte
	More() bool
}
type Page interface {
	Message() Message
	PagingCtxt
}

type Connection interface {
	Close()

	Add(dn string, attributes []Attribute, ctxt diag.RequestContext) diag.Error
	Delete(dn string, ctxt diag.RequestContext) diag.Error
	Modify(dn string, mods []AttributeMod, ctxt diag.RequestContext) diag.Error

	Search(base string, scope Scope, filter string,
		attributes []string, attributesOnly bool,
		sizeLimit int, timeLimit int, ctxt diag.RequestContext) (Message, diag.Error)

	SearchPage(base string, scope Scope, filter string,
		attributes []string, attributesOnly bool, pagingSize uint32,
		sizeLimit int, timeLimit int, pagingCtxt PagingCtxt, ctxt diag.RequestContext) (Page, diag.Error)
}

type BindType uint8

const (
	BindTypeSimple BindType = iota
	BindTypeSRP
)

const (
	SchemeLdap          = "ldap"      // format ldap://host:port
	SchemeLdaps         = "ldaps"     // format ldaps://host:port
	SchemeLwDomainLdap  = "lwdnldap"  // format lwdnldap://<domain_name>[/<site_name>]
	SchemeLwDomainLdaps = "lwdnldaps" // format lwdnldaps://<domain_name>[/<site_name>]
)

func SupportedScheme(scheme string) bool {
	return strings.EqualFold(scheme, SchemeLdaps) ||
		strings.EqualFold(scheme, SchemeLwDomainLdaps) ||
		strings.EqualFold(scheme, SchemeLdap) ||
		strings.EqualFold(scheme, SchemeLwDomainLdap)
}

func LwDomainScheme(scheme string) bool {
	return strings.EqualFold(scheme, SchemeLwDomainLdaps) ||
		strings.EqualFold(scheme, SchemeLwDomainLdap)
}
func LwDomainLdapsScheme(scheme string) bool {
	return strings.EqualFold(scheme, SchemeLwDomainLdaps)
}

// LwDomainURI constructs a lw domain uri (site is optional)
func LwDomainURI(domain string, site string, ldaps bool) *url.URL {
	uri := &url.URL{
		Scheme:  SchemeLwDomainLdaps,
		Host:    domain,
		Path:    url.PathEscape(site),
		RawPath: site,
	}
	if !ldaps {
		uri.Scheme = SchemeLwDomainLdap
	}
	return uri
}

type TrustedCertsFunc func(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error)
type ConnectionFactory interface {
	Connection(
		uri *url.URL, userName string, password string,
		bind BindType, certs TrustedCertsFunc, ctxt diag.RequestContext) (Connection, diag.Error)
}

var DefaultConnectionFactory ConnectionFactory = &connectionFactoryImpl{}
