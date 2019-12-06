package types

import (
	"crypto"
	"crypto/x509"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type TenantInfo interface {
	Name() diag.TenantID
	Domain() string
}

type Tenant interface {
	TenantInfo

	SignerCert() *x509.Certificate
	SignerKey() crypto.PrivateKey
	SignerCerts() []*x509.Certificate

	TokenPolicy

	LoginMethods() LoginMethodSet
}

type TenantBuilder interface {
	Name(n diag.TenantID)
	Domain(d string)

	SignerCert(c *x509.Certificate)
	SignerKey(k crypto.PrivateKey)
	SignerCerts(cs []*x509.Certificate)

	ClockTolerance(d time.Duration)

	MaxBearerLifetime(d time.Duration)
	MaxHOKLifetime(d time.Duration)
	MaxBearerRefreshLifetime(d time.Duration)
	MaxHOKRefreshLifetime(d time.Duration)

	SessionLifetime(d time.Duration)

	DelegationCount(c uint32)
	RenewCount(c uint32)

	LoginMethods(ms LoginMethodSet)

	Build() (Tenant, diag.Error)
	BuildTokenPolicy() (TokenPolicy, diag.Error)
}

func NewTenantBuilder() TenantBuilder {
	return &tenantBuilderImpl{}
}

type TokenPolicy interface {
	MaxBearerLifetime() time.Duration
	MaxHOKLifetime() time.Duration
	ClockTolerance() time.Duration
	MaxBearerRefreshLifetime() time.Duration
	MaxHOKRefreshLifetime() time.Duration
	SessionLifetime() time.Duration
	DelegationCount() uint32
	RenewCount() uint32
}
