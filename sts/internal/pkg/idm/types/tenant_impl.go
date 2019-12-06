package types

import (
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"fmt"
	"math/big"
	mathrand "math/rand"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

// idm:"ldap:oc=lightwaveSTSTenant;t=Tenant;subt=TokenPolicy"
// idm:"marshal:enc=j;dec=j;t=Tenant;subt=TokenPolicy"
type tenantImpl struct {

	// idm:"marshal:name=name;m=Name"
	name diag.TenantID // idm:"ldap:name=cn;u=false;m=Name"

	// idm:"marshal:name=domain;m=Domain"
	domain string // idm:"ldap:name=lightwaveSTSDomainName;u=false;m=Domain"

	// todo: marshal
	privateKey crypto.PrivateKey // idm:"ldap:name=privateKey;m=SignerKey"
	// todo: marshal
	signingCert *x509.Certificate // idm:"ldap:name=userCertificate;m=SignerCert"

	// todo: marshal
	signingCertsSet []*x509.Certificate // idm:"ldap:name=lightwaveSTSSigners;m=SignerCerts"

	// idm:"marshal:name=clock_tolerance;m=ClockTolerance;subt=TokenPolicy"
	clockTolerance time.Duration // idm:"ldap:name=lightwaveSTSClockTolerance;m=ClockTolerance;subt=TokenPolicy"

	// idm:"marshal:name=max_bearer_lifetime;m=MaxBearerLifetime;subt=TokenPolicy"
	maxBearerTokenLifetime time.Duration // idm:"ldap:name=lightwaveSTSMaxBearerTokenLifetime;m=MaxBearerLifetime;subt=TokenPolicy"

	// idm:"marshal:name=max_hok_lifetime;m=MaxHOKLifetime;subt=TokenPolicy"
	maxHOKTokenLifetime time.Duration // idm:"ldap:name=lightwaveSTSMaxHOKTokenLifetime;m=MaxHOKLifetime;subt=TokenPolicy"

	// idm:"marshal:name=max_bearer_refresh_lifetime;m=MaxBearerRefreshLifetime;subt=TokenPolicy"
	maxBearerRefreshTokenLifetime time.Duration // idm:"ldap:name=lightwaveSTSMaxBearerRefreshTokenLifetime;m=MaxBearerRefreshLifetime;subt=TokenPolicy"

	// idm:"marshal:name=max_hok_refresh_lifetime;m=MaxHOKRefreshLifetime;subt=TokenPolicy"
	maxHOKRefreshTokenLifetime time.Duration // idm:"ldap:name=lightwaveSTSMaxHOKRefreshTokenLifetime;m=MaxHOKRefreshLifetime;subt=TokenPolicy"

	// idm:"marshal:name=session_lifetime;m=SessionLifetime;subt=TokenPolicy"
	sessionLifetime time.Duration // idm:"ldap:name=lightwaveSTSSessionLifetime;m=SessionLifetime;subt=TokenPolicy"

	// idm:"marshal:name=delegation_count;m=DelegationCount;subt=TokenPolicy"
	delegationCount uint32 // idm:"ldap:name=lightwaveSTSDelegationCount;m=DelegationCount;subt=TokenPolicy"

	// idm:"marshal:name=renew_count;m=RenewCount;subt=TokenPolicy"
	renewCount uint32 // idm:"ldap:name=lightwaveSTSRenewCount;m=RenewCount;subt=TokenPolicy"

	// todo: marshal
	loginMethods LoginMethodSet // idm:"ldap:name=lightwaveSTSAuthnTypes;omitempty;m=LoginMethods"
}

func (t *tenantImpl) Name() diag.TenantID { return t.name }
func (t *tenantImpl) Domain() string      { return t.domain }

func (t *tenantImpl) SignerCert() *x509.Certificate    { return t.signingCert }
func (t *tenantImpl) SignerKey() crypto.PrivateKey     { return t.privateKey }
func (t *tenantImpl) SignerCerts() []*x509.Certificate { return t.signingCertsSet }

func (t *tenantImpl) ClockTolerance() time.Duration { return t.clockTolerance }

func (t *tenantImpl) MaxBearerLifetime() time.Duration        { return t.maxBearerTokenLifetime }
func (t *tenantImpl) MaxHOKLifetime() time.Duration           { return t.maxHOKTokenLifetime }
func (t *tenantImpl) MaxBearerRefreshLifetime() time.Duration { return t.maxBearerRefreshTokenLifetime }
func (t *tenantImpl) MaxHOKRefreshLifetime() time.Duration    { return t.maxHOKRefreshTokenLifetime }

func (t *tenantImpl) SessionLifetime() time.Duration { return t.sessionLifetime }

func (t *tenantImpl) DelegationCount() uint32 { return t.delegationCount }
func (t *tenantImpl) RenewCount() uint32      { return t.renewCount }

func (t *tenantImpl) LoginMethods() LoginMethodSet { return t.loginMethods }

type tenantBuilderImpl tenantImpl

func (b *tenantBuilderImpl) Name(n diag.TenantID) { b.name = n }
func (b *tenantBuilderImpl) Domain(d string)      { b.domain = d }

func (b *tenantBuilderImpl) SignerCert(c *x509.Certificate)     { b.signingCert = c }
func (b *tenantBuilderImpl) SignerKey(k crypto.PrivateKey)      { b.privateKey = k }
func (b *tenantBuilderImpl) SignerCerts(cs []*x509.Certificate) { b.signingCertsSet = cs }

func (b *tenantBuilderImpl) ClockTolerance(d time.Duration) { b.clockTolerance = d }

func (b *tenantBuilderImpl) MaxBearerLifetime(d time.Duration) { b.maxBearerTokenLifetime = d }
func (b *tenantBuilderImpl) MaxHOKLifetime(d time.Duration)    { b.maxHOKTokenLifetime = d }
func (b *tenantBuilderImpl) MaxBearerRefreshLifetime(d time.Duration) {
	b.maxBearerRefreshTokenLifetime = d
}
func (b *tenantBuilderImpl) MaxHOKRefreshLifetime(d time.Duration) { b.maxHOKRefreshTokenLifetime = d }

func (b *tenantBuilderImpl) SessionLifetime(d time.Duration) { b.sessionLifetime = d }

func (b *tenantBuilderImpl) DelegationCount(c uint32) { b.delegationCount = c }
func (b *tenantBuilderImpl) RenewCount(c uint32)      { b.renewCount = c }

func (b *tenantBuilderImpl) LoginMethods(ms LoginMethodSet) { b.loginMethods = ms }

func (b *tenantBuilderImpl) Build() (Tenant, diag.Error) {
	// todo: validations/defaults
	if b.privateKey == nil || b.signingCert == nil {
		pk, c, err := genSigner()
		if err != nil {
			return nil, err
		}
		b.privateKey = pk
		b.signingCert = c

		b.signingCertsSet = append(b.signingCertsSet, b.signingCert)
	}
	return (*tenantImpl)(b), nil
}

func (b *tenantBuilderImpl) BuildTokenPolicy() (TokenPolicy, diag.Error) {
	// todo: validations/defaults
	return (*tenantImpl)(b), nil
}

func genSigner() (crypto.PrivateKey, *x509.Certificate, diag.Error) {
	r := mathrand.New(mathrand.NewSource(time.Now().UnixNano()))
	bi := big.NewInt(int64(r.Int()))

	template := &x509.Certificate{
		IsCA: true,
		BasicConstraintsValid: true,
		SubjectKeyId:          []byte{1, 2, 3},
		SerialNumber:          bi,
		Subject: pkix.Name{
			CommonName:   "STSSignerCert",
			Country:      []string{"US"},
			Province:     []string{"WA"},
			Organization: []string{"WMware"},
		},
		NotBefore: time.Now(),
		NotAfter:  time.Now().AddDate(10, 0, 0),
		// see http://golang.org/pkg/crypto/x509/#KeyUsage
		ExtKeyUsage: []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth, x509.ExtKeyUsageServerAuth},
		KeyUsage:    x509.KeyUsageDigitalSignature | x509.KeyUsageCertSign,
	}

	// generate private key
	privatekey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return nil, nil,
			diag.MakeError(IdmErrorGeneric, fmt.Sprintf("Failed to gen private key: %v", err), err)
	}

	// create a self-signed certificate. template = parent
	var parent = template
	cert, err := x509.CreateCertificate(rand.Reader, template, parent, &privatekey.PublicKey, privatekey)
	if err != nil {
		return nil, nil,
			diag.MakeError(IdmErrorGeneric, fmt.Sprintf("Failed to gen cert: %v", err), err)
	}

	x509Cert, err := x509.ParseCertificate(cert)
	if err != nil {
		return nil, nil,
			diag.MakeError(IdmErrorGeneric, fmt.Sprintf("Failed to parse cert: %v", err), err)
	}

	return privatekey, x509Cert, nil
}
