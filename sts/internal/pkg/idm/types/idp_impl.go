package types

import (
	"crypto/x509"
	"net/url"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type securityDomainImpl struct {
	name  string
	alias string
}

func (sd *securityDomainImpl) Name() string  { return sd.name }
func (sd *securityDomainImpl) Alias() string { return sd.alias }

type securityDomainBuilderImpl securityDomainImpl

func (sb *securityDomainBuilderImpl) Name(n string)  { sb.name = n }
func (sb *securityDomainBuilderImpl) Alias(a string) { sb.alias = a }

func (sb *securityDomainBuilderImpl) Build() (SecurityDomain, diag.Error) {
	// todo: validations
	return (*securityDomainImpl)(sb), nil
}

type securityDomainSetImpl map[string]SecurityDomain

func (s securityDomainSetImpl) Len() int {
	return len(s)
}
func (s securityDomainSetImpl) Iterate(f SecDomainFunc) diag.Error {
	for _, v := range s {
		err := f(v)
		if err != nil {
			return err
		}
	}
	return nil
}
func (s securityDomainSetImpl) Contains(v SecurityDomain) bool {
	ve, ok := s[strings.ToLower(v.Name())]
	if !ok {
		return false
	}
	return strings.EqualFold(ve.Alias(), v.Alias())
}

func (s securityDomainSetImpl) ContainsName(domainOrAlias string) bool {
	_, ok := s[strings.ToLower(domainOrAlias)]
	return ok
}

func (s securityDomainSetImpl) Add(v SecurityDomain) {
	s[strings.ToLower(v.Name())] = v
	if len(v.Alias()) > 0 {
		s[strings.ToLower(v.Alias())] = v
	}
}

func (s securityDomainSetImpl) Build() (SecurityDomainSet, diag.Error) {
	// todo: validations
	return s, nil
}

// idm:"ldap:map:oc=lightwaveSTSIDSAttribute;ccn=Attributes;t=IDSAttribute;"
// idm:"marshal:map;enc=j;dec=j;t=IDSAttribute"
type iDSAttributeImpl struct {
	// idm:"marshal:name=id;m=ID"
	iD AttributeID // idm:"ldap:name=cn;u=false;m=ID"
	// idm:"marshal:name=type;m=AttrType"
	attrType IDSAttributeType // idm:"ldap:name=lightwaveSTSAttributeType;m=AttrType"
	// idm:"marshal:name=value;omitempty;m=AttrValue"
	attrValue IDSAttributeValue // idm:"ldap:name=lightwaveSTSAttributeValue;omitempty;m=AttrValue"
}

func (av *iDSAttributeImpl) ID() AttributeID {
	return av.iD
}

func (av *iDSAttributeImpl) AttrType() IDSAttributeType {
	return av.attrType
}
func (av *iDSAttributeImpl) AttrValue() IDSAttributeValue {
	return av.attrValue
}

type iDSAttributeBuilderImpl iDSAttributeImpl

func (b *iDSAttributeBuilderImpl) ID(id AttributeID) {
	b.iD = id
}
func (b *iDSAttributeBuilderImpl) AttrType(t IDSAttributeType) {
	b.attrType = t
}
func (b *iDSAttributeBuilderImpl) AttrValue(v IDSAttributeValue) {
	b.attrValue = v
}

func (b *iDSAttributeBuilderImpl) Build() (IDSAttribute, diag.Error) {
	// todo: validate anything needed
	return (*iDSAttributeImpl)(b), nil
}

type iDSAttributeMapImpl map[AttributeID]IDSAttribute

func (m iDSAttributeMapImpl) Add(attr IDSAttribute) {
	m[attr.ID()] = attr
}
func (m iDSAttributeMapImpl) Build() (IDSAttributeMap, diag.Error) {
	// validations as needed
	return m, nil
}

func (m iDSAttributeMapImpl) Len() int {
	return len(m)
}
func (m iDSAttributeMapImpl) Contains(attr AttributeID) bool {
	if m == nil {
		return false
	}
	_, ok := m[attr]
	return ok
}
func (m iDSAttributeMapImpl) V(attr AttributeID) (IDSAttribute, bool) {
	if m == nil {
		return nil, false
	}
	v, ok := m[attr]
	return v, ok
}

func (m iDSAttributeMapImpl) Iterate(f IDSAttributeFunc) diag.Error {
	if m == nil {
		return nil
	}

	for _, v := range m {
		err := f(v)
		if err != nil {
			return err
		}
	}
	return nil
}

// idm:"ldap:oc=lightwaveSTSIdentityStore;ccn=IdentityProviders;t=IDSConfig"
// idm:"marshal:enc=j;dec=j;t=IDSConfig"
type idsConfigImpl struct {
	// idm:"marshal:name=name;m=Name"
	name string // idm:"ldap:name=cn;u=false;m=Name" // non updateable on ldap
	// idm:"marshal:name=domain;m=Domain"
	domain string // idm:"ldap:name=lightwaveSTSDomainName;m=Domain"
	// idm:"marshal:name=alias;m=Alias"
	alias string // idm:"ldap:name=lightwaveSTSAlias;omitempty;m=Alias"
	// idm:"marshal:name=type;m=Provider"
	provider ProviderType // idm:"ldap:name=lightwaveSTSProviderType;m=Provider"

	// connection info
	// idm:"marshal:name=authType;m=AuthType"
	authType AuthType // idm:"ldap:name=lightwaveSTSAuthenticationType;m=AuthType"
	// idm:"marshal:name=addresses;m=Addresses"
	addresses []*url.URL // idm:"ldap:name=lightwaveSTSConnectionStrings;m=Addresses"
	// todo: certs marshalling
	certificates []*x509.Certificate // idm:"ldap:name=userCertificate;omitempty;m=Certificates"

	// idm:"marshal:name=userName;omitempty;m=UserName"
	userName string   // idm:"ldap:name=lightwaveSTSUserName;omitempty;m=UserName"
	pwd      []string // idm:"ldap:name=lightwaveSTSPassword;omitempty;m=Pwd;encrypt"

	// idm:"marshal:name=user_base_dn;omitempty;m=UserBaseDN"
	userBaseDn string // idm:"ldap:name=lightwaveSTSUserBaseDN;omitempty;m=UserBaseDN"
	// idm:"marshal:name=group_base_dn;omitempty;m=GroupBaseDN"
	groupBaseDn string // idm:"ldap:name=lightwaveSTSGroupBaseDN;omitempty;m=GroupBaseDN"

	// idm:"marshal:child;name=attributes;omitempty;m=Attributes"
	attributes IDSAttributeMap // idm:"ldap:child;m=Attributes"

	// todo:marshal
	loginMethods LoginMethodSet // idm:"ldap:name=lightwaveSTSAuthnTypes;omitempty;m=LoginMethods"

	// calculated
	certs *x509.CertPool
}

func (ids *idsConfigImpl) AuthType() AuthType { return ids.authType }

func (ids *idsConfigImpl) Addresses() []*url.URL { return ids.addresses }

func (ids *idsConfigImpl) Certificates() []*x509.Certificate { return ids.certificates }

func (ids *idsConfigImpl) TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error) {
	return ids.certs, nil
}

func (ids *idsConfigImpl) UserName() string { return ids.userName }

func (ids *idsConfigImpl) Pwd() []string { return ids.pwd }

func (ids *idsConfigImpl) Name() string { return ids.name }

func (ids *idsConfigImpl) Domain() string { return ids.domain }

func (ids *idsConfigImpl) Provider() ProviderType { return ids.provider }

func (ids *idsConfigImpl) Alias() string { return ids.alias }

func (ids *idsConfigImpl) UserBaseDN() string { return ids.userBaseDn }

func (ids *idsConfigImpl) GroupBaseDN() string { return ids.groupBaseDn }

func (ids *idsConfigImpl) Attributes() IDSAttributeMap { return ids.attributes }

func (ids *idsConfigImpl) LoginMethods() LoginMethodSet { return ids.loginMethods }

type idsConfigBuilderImpl idsConfigImpl

func (ib *idsConfigBuilderImpl) AuthType(t AuthType) {
	ib.authType = t
}
func (ib *idsConfigBuilderImpl) Addresses(ads []*url.URL) {
	ib.addresses = ads
}
func (ib *idsConfigBuilderImpl) Certificates(certs []*x509.Certificate) {
	ib.certificates = certs
}
func (ib *idsConfigBuilderImpl) UserName(n string) {
	ib.userName = n
}
func (ib *idsConfigBuilderImpl) Pwd(pwd []string) {
	ib.pwd = pwd
}
func (ib *idsConfigBuilderImpl) Name(name string) {
	ib.name = name
}
func (ib *idsConfigBuilderImpl) Domain(domain string) {
	ib.domain = domain
}
func (ib *idsConfigBuilderImpl) Alias(alias string) {
	ib.alias = alias
}
func (ib *idsConfigBuilderImpl) Provider(provider ProviderType) {
	ib.provider = provider
}
func (ib *idsConfigBuilderImpl) UserBaseDN(dn string) {
	ib.userBaseDn = dn
}
func (ib *idsConfigBuilderImpl) GroupBaseDN(dn string) {
	ib.groupBaseDn = dn
}
func (ib *idsConfigBuilderImpl) Attributes(attrs IDSAttributeMap) {
	ib.attributes = attrs
}
func (ib *idsConfigBuilderImpl) LoginMethods(methods LoginMethodSet) {
	ib.loginMethods = methods
}

func (ib *idsConfigBuilderImpl) Build() (IDSConfig, diag.Error) {
	// todo: validations

	if len(ib.certificates) > 0 {
		ib.certs = x509.NewCertPool()
		for _, c := range ib.certificates {
			ib.certs.AddCert(c)
		}
	}
	return (*idsConfigImpl)(ib), nil
}
