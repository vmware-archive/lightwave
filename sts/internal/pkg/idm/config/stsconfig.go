package config

import (
	"crypto"
	"crypto/x509"
	"fmt"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type ServerRelativePathFunc func(tenant diag.TenantID) string

type StsConfig interface {
	// https://<lb|fqdn>[:port]
	PublicEndpoint(tenant diag.TenantID, rc diag.RequestContext) (string, diag.Error)

	Issuer(tenant diag.TenantID, sr ServerRelativePathFunc, rc diag.RequestContext) (string, diag.Error)

	// signer certs
	SignerCerts(tenant diag.TenantID, rc diag.RequestContext) ([]*x509.Certificate, diag.Error)

	SignerCert(tenant diag.TenantID, rc diag.RequestContext) (*x509.Certificate, diag.Error)
	SignerKey(tenant diag.TenantID, rc diag.RequestContext) (crypto.PrivateKey, diag.Error)

	// token lifetimes
	TokenPolicy(tenant diag.TenantID, rc diag.RequestContext) (types.TokenPolicy, diag.Error)

	LookupTenant(name diag.TenantID, ctxt diag.RequestContext) (types.TenantInfo, diag.Error)

	LookupOidcClient(name diag.TenantID, id string, ctxt diag.RequestContext) (types.OidcClient, diag.Error)
}

func (c *idmConfigImpl) PublicEndpoint(tenant diag.TenantID, rc diag.RequestContext) (string, diag.Error) {
	di, err := c.GetDeploymentInfo(rc)
	if err != nil {
		return "", err
	}
	return "https://" + di.PublicEndpoint(), nil
}

func (c *idmConfigImpl) Issuer(tenant diag.TenantID, sr ServerRelativePathFunc, rc diag.RequestContext) (string, diag.Error) {
	serverRelPath := sr(tenant)
	ep, err := c.PublicEndpoint(tenant, rc)
	if err != nil {
		return "", err
	}
	return fmt.Sprintf("%s%s", ep, serverRelPath), nil
}

// signer certs
func (c *idmConfigImpl) SignerCerts(tenant diag.TenantID, rc diag.RequestContext) ([]*x509.Certificate, diag.Error) {
	t, err := c.GetTenant(tenant, rc)
	if err != nil {
		return []*x509.Certificate{}, err
	}
	return t.SignerCerts(), nil
}

func (c *idmConfigImpl) SignerCert(tenant diag.TenantID, rc diag.RequestContext) (*x509.Certificate, diag.Error) {
	t, err := c.GetTenant(tenant, rc)
	if err != nil {
		return nil, err
	}
	return t.SignerCert(), nil
}
func (c *idmConfigImpl) SignerKey(tenant diag.TenantID, rc diag.RequestContext) (crypto.PrivateKey, diag.Error) {
	t, err := c.GetTenant(tenant, rc)
	if err != nil {
		return nil, err
	}
	return t.SignerKey(), nil
}

// token lifetimes
func (c *idmConfigImpl) TokenPolicy(tenant diag.TenantID, rc diag.RequestContext) (types.TokenPolicy, diag.Error) {
	t, err := c.GetTenant(tenant, rc)
	if err != nil {
		return nil, err
	}
	return t, nil
}

func (c *idmConfigImpl) LookupOidcClient(name diag.TenantID, id string, ctxt diag.RequestContext) (types.OidcClient, diag.Error) {
	cli, err := c.GetOidcClient(name, id, ctxt)
	// todo: only when error is not-found
	if err != nil {
		var err1 diag.Error
		cli, err1 = c.GetOidcClient(c.cfg.SystemTenant(), id, ctxt)
		if err1 != nil {
			return nil, err
		}
		if !cli.CrossTenant() {
			return nil, err
		}
	}

	return cli, nil
}
