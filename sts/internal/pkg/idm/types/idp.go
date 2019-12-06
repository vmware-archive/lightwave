package types

import (
	"crypto/x509"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type SecurityDomain interface {
	Name() string  // required
	Alias() string // optional
}

type SecurityDomainBuilder interface {
	Name(n string)
	Alias(a string)

	Build() (SecurityDomain, diag.Error)
}

func NewSecurityDomainBuilder() SecurityDomainBuilder {
	return &securityDomainBuilderImpl{}
}

type SecDomainFunc func(d SecurityDomain) diag.Error

type SecurityDomainSet interface {
	Len() int
	Iterate(f SecDomainFunc) diag.Error
	Contains(v SecurityDomain) bool
	ContainsName(domainOrAlias string) bool
}

type SecurityDomainSetBuilder interface {
	Add(v SecurityDomain)

	Build() (SecurityDomainSet, diag.Error)
}

func NewSecurityDomainSetBuilder(capacity int) SecurityDomainSetBuilder {
	return make(securityDomainSetImpl, capacity)
}

// constant -> defined value
// ldap_attribute -> name of the attribute
// attribute -> attribue id defines handling semantics for this attribute (such as user identity, or group membership)
// idm:"enum:vals=constant ldap_attribute attribute"
type IDSAttributeType uint8

type IDSAttributeValue string

func (ia IDSAttributeValue) String() string { return string(ia) }
func (ia *IDSAttributeValue) From(str string) diag.Error {
	if ia == nil {
		return diag.MakeError(IdmErrorInvalidArgument, "Unable to unmarshal IDSAttributeValue into nil value", nil)
	}
	*ia = IDSAttributeValue(str)
	return nil
}

// idm:"enum:vals=user_identity first_name last_name group_identities"
type AttributeID string

const (
	NoneAttributeID AttributeID = AttributeID("")
)

type IDSAttribute interface {
	ID() AttributeID
	AttrType() IDSAttributeType
	AttrValue() IDSAttributeValue
}

type IDSAttributeBuilder interface {
	ID(AttributeID)
	AttrType(IDSAttributeType)
	AttrValue(IDSAttributeValue)

	Build() (IDSAttribute, diag.Error)
}

type IDSAttributeFunc func(IDSAttribute) diag.Error

type IDSAttributeMap interface {
	Len() int
	Contains(attr AttributeID) bool
	V(attr AttributeID) (IDSAttribute, bool)
	Iterate(f IDSAttributeFunc) diag.Error
}

type IDSAttributeMapBuilder interface {
	Add(IDSAttribute)
	Build() (IDSAttributeMap, diag.Error)
}

// idm:"enum:vals=none simple srp sts_account"
type AuthType uint8

type Account interface {
	AuthType() AuthType
	UserName() string
	Pwd() []string
}

type ConnectionInfo interface {
	Addresses() []*url.URL
	TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error)
	/*Certificates() []*x509.Certificate // ldaps*/
}

type IDSConfig interface {
	Name() string
	Domain() string
	Alias() string // optional
	Provider() ProviderType

	AuthType() AuthType
	UserName() string
	Pwd() []string

	Addresses() []*url.URL
	Certificates() []*x509.Certificate // ldaps
	TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error)

	UserBaseDN() string  // optional, defaults to domain
	GroupBaseDN() string // optional, defaults to domain

	Attributes() IDSAttributeMap

	LoginMethods() LoginMethodSet
}

type IDSConfigBuilder interface {
	AuthType(t AuthType)
	Addresses(ads []*url.URL)
	Certificates(certs []*x509.Certificate)

	UserName(n string)
	Pwd(pwd []string)

	Name(name string)
	Domain(string)
	Alias(alias string)
	Provider(provider ProviderType)

	UserBaseDN(dn string)
	GroupBaseDN(dn string)

	Attributes(attrs IDSAttributeMap)

	LoginMethods(methods LoginMethodSet)

	Build() (IDSConfig, diag.Error)
}

// idm:"enum:vals=vmdir native_ad open_ldap ad_over_ldap"
type ProviderType uint8

func NewIDSAttributeBuilder() IDSAttributeBuilder {
	return &iDSAttributeBuilderImpl{}
}

func NewIDSAttributeMapBuilder() IDSAttributeMapBuilder {
	return make(iDSAttributeMapImpl)
}

func NewIDSConfigBuilder() IDSConfigBuilder {
	return &idsConfigBuilderImpl{}
}
