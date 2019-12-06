package config

import (
	"crypto/x509"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

type ConfigStoreInfo interface {
	types.Account
	SystemTenant() diag.TenantID
	Addresses() []*url.URL
	TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error)
}

type TenantConfig interface {
	StsConfig
	ListTenants(ctxt diag.RequestContext) ([]types.TenantInfo, diag.Error)

	GetTenant(name diag.TenantID, ctxt diag.RequestContext) (types.Tenant, diag.Error)
}

type TenantConfigurator interface {
	TenantConfig

	CreateTenant(t types.Tenant, ctxt diag.RequestContext) diag.Error
	DeleteTenant(name diag.TenantID, ctxt diag.RequestContext) diag.Error
	UpdateTenant(t types.Tenant, ctxt diag.RequestContext) diag.Error
}

type IDSConfig interface {
	GetIDS(tenant diag.TenantID, name string, ctxt diag.RequestContext) (types.IDSConfig, diag.Error)
	ListIDSs(tenant diag.TenantID, ctxt diag.RequestContext) ([]types.IDSConfig, diag.Error)
}

type IDSConfigurator interface {
	IDSConfig
	CreateIDS(tenant diag.TenantID, ids types.IDSConfig, ctxt diag.RequestContext) diag.Error
	DeleteIDS(tenant diag.TenantID, name string, ctxt diag.RequestContext) diag.Error
	UpdateIDS(tenant diag.TenantID, ids types.IDSConfig, ctxt diag.RequestContext) diag.Error
}

type OidcClientConfig interface {
	GetOidcClient(tenant diag.TenantID, id string, ctxt diag.RequestContext) (types.OidcClient, diag.Error)
	ListOidcClients(tenant diag.TenantID, ctxt diag.RequestContext) ([]types.OidcClient, diag.Error)
}

type OidcClientConfigurator interface {
	OidcClientConfig
	CreateOidcClient(tenant diag.TenantID, cli types.OidcClient, ctxt diag.RequestContext) diag.Error
	DeleteOidcClient(tenant diag.TenantID, id string, ctxt diag.RequestContext) diag.Error
	UpdateOidcClient(tenant diag.TenantID, cli types.OidcClient, ctxt diag.RequestContext) diag.Error
}

type DeploymentConfig interface {
	GetDeploymentInfo(ctxt diag.RequestContext) (types.DeploymentInfo, diag.Error)
}

type DeploymentConfigurator interface {
	DeploymentConfig
	CreateDeploymentInfo(inf types.DeploymentInfo, ctxt diag.RequestContext) diag.Error
	DeleteDeploymentInfo(ctxt diag.RequestContext) diag.Error
	UpdateDeploymentInfo(inf types.DeploymentInfo, ctxt diag.RequestContext) diag.Error
}

type Config interface {
	IDSConfig
	TenantConfig
	OidcClientConfig
	DeploymentConfig
}

type Configurator interface {
	TenantConfigurator
	IDSConfigurator
	OidcClientConfigurator
	DeploymentConfigurator
}

func NewConfigurator(cfg ConfigStoreInfo, connProvider utils.LdapConnectionProvider, logger diag.Logger) (Configurator, diag.Error) {
	return &idmConfigImpl{
		cfg:          cfg,
		connProvider: connProvider,
	}, nil
}

type idmConfigImpl struct {
	cfg          ConfigStoreInfo
	connProvider utils.LdapConnectionProvider
}

func (c *idmConfigImpl) ListTenants(ctxt diag.RequestContext) ([]types.TenantInfo, diag.Error) {
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()
	parentDn := c.getSystemTenantConfigDn()

	refs, err := ldapListtenantRef(conn, parentDn, ctxt)
	if err != nil {
		return nil, err
	}

	infos := make([]types.TenantInfo, 0, len(refs))
	for _, t := range refs {
		infos = append(infos, t)
	}

	return infos, nil
}

func (c *idmConfigImpl) LookupTenant(name diag.TenantID, ctxt diag.RequestContext) (types.TenantInfo, diag.Error) {
	return c.lookupTenant(name, ctxt)
}

func (c *idmConfigImpl) GetTenant(name diag.TenantID, ctxt diag.RequestContext) (types.Tenant, diag.Error) {

	tr, err := c.lookupTenant(name, ctxt)
	if err != nil {
		return nil, err
	}

	parentDn := tr.getConfigDn()
	conn, err := c.connProvider.PooledConnection(
		name, c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	return ldapGetTenant(conn, parentDn, name.String(), ctxt)
}

func (c *idmConfigImpl) CreateTenant(t types.Tenant, ctxt diag.RequestContext) diag.Error {

	b := newtenantRefBuilder()
	b.Name(t.Name())
	b.DomainDn(types.DnFromDomain(t.Domain()))

	tenantRef, err := b.Build()
	if err != nil {
		return err
	}

	stConfigDn := c.getSystemTenantConfigDn()
	stConn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer stConn.Close()
	err = ensureSTSConfigContainer(stConn, types.DnFromDomain(c.cfg.SystemTenant().String()), ctxt)
	if err != nil {
		return err
	}

	err = ldapCreatetenantRef(stConn, stConfigDn, tenantRef, ctxt)
	if err != nil {
		return err
	}

	configDn := tenantRef.getConfigDn()

	conn, err := c.connProvider.PooledConnection(
		t.Name(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()
	err = ensureSTSConfigContainer(conn, types.DnFromDomain(t.Domain()), ctxt)
	if err != nil {
		return err
	}

	return ldapCreateTenant(conn, configDn, t, ctxt)
}

func (c *idmConfigImpl) DeleteTenant(name diag.TenantID, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(name, ctxt)
	if err != nil {
		return err
	}

	configDn := tr.getConfigDn()

	conn, err := c.connProvider.PooledConnection(
		name, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	err = ldapDeleteTenant(conn, configDn, name.String(), ctxt)
	if err != nil {
		return err
	}

	stConfigDn := c.getSystemTenantConfigDn()
	stConn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer stConn.Close()

	return ldapDeletetenantRef(conn, stConfigDn, name.String(), ctxt)
}

func (c *idmConfigImpl) UpdateTenant(t types.Tenant, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(t.Name(), ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getConfigDn()
	conn, err := c.connProvider.PooledConnection(
		t.Name(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapUpdateTenant(conn, parentDn, t, ctxt)
}

func (c *idmConfigImpl) CreateIDS(tenant diag.TenantID, ids types.IDSConfig, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapCreateIDSConfig(conn, parentDn, ids, tr.encrypt, ctxt)
}

func (c *idmConfigImpl) DeleteIDS(tenant diag.TenantID, name string, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapDeleteIDSConfig(conn, parentDn, name, ctxt)
}
func (c *idmConfigImpl) UpdateIDS(tenant diag.TenantID, ids types.IDSConfig, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()

	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapUpdateIDSConfig(conn, parentDn, ids, tr.encrypt, ctxt)
}
func (c *idmConfigImpl) ListIDSs(tenant diag.TenantID, ctxt diag.RequestContext) ([]types.IDSConfig, diag.Error) {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	parentDn := tr.getTenantsDn()

	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	return ldapListIDSConfig(conn, parentDn, tr.decrypt, ctxt)
}

func (c *idmConfigImpl) GetIDS(tenant diag.TenantID, name string, ctxt diag.RequestContext) (types.IDSConfig, diag.Error) {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	return ldapGetIDSConfig(conn, parentDn, name, tr.decrypt, ctxt)
}

func (c *idmConfigImpl) GetOidcClient(tenant diag.TenantID, id string, ctxt diag.RequestContext) (types.OidcClient, diag.Error) {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	return ldapGetOidcClient(conn, parentDn, id, ctxt)
}

func (c *idmConfigImpl) ListOidcClients(tenant diag.TenantID, ctxt diag.RequestContext) ([]types.OidcClient, diag.Error) {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	parentDn := tr.getTenantsDn()

	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	return ldapListOidcClient(conn, parentDn, ctxt)
}

func (c *idmConfigImpl) CreateOidcClient(tenant diag.TenantID, cli types.OidcClient, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapCreateOidcClient(conn, parentDn, cli, ctxt)
}

func (c *idmConfigImpl) DeleteOidcClient(tenant diag.TenantID, id string, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()
	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapDeleteOidcClient(conn, parentDn, id, ctxt)
}

func (c *idmConfigImpl) UpdateOidcClient(tenant diag.TenantID, cli types.OidcClient, ctxt diag.RequestContext) diag.Error {
	tr, err := c.lookupTenant(tenant, ctxt)
	if err != nil {
		return err
	}

	parentDn := tr.getTenantsDn()

	conn, err := c.connProvider.PooledConnection(
		tenant, c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()

	return ldapUpdateOidcClient(conn, parentDn, cli, ctxt)
}

func (c *idmConfigImpl) GetDeploymentInfo(ctxt diag.RequestContext) (types.DeploymentInfo, diag.Error) {
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()
	parentDn := c.getSystemTenantConfigDn()

	return ldapGetDeploymentInfo(conn, parentDn, ctxt)
}

func (c *idmConfigImpl) CreateDeploymentInfo(inf types.DeploymentInfo, ctxt diag.RequestContext) diag.Error {
	logger := ctxt.Logger()
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		logger.Errorf(diag.IDM, "Failed obtaining ldap connection: %v", err)
		return err
	}
	defer conn.Close()
	err = ensureSTSConfigContainer(conn, types.DnFromDomain(c.cfg.SystemTenant().String()), ctxt)
	if err != nil {
		logger.Errorf(diag.IDM, "Failed ensuring sts config comtainer: %v", err)
		return err
	}
	parentDn := c.getSystemTenantConfigDn()

	return ldapCreateDeploymentInfo(conn, parentDn, inf, ctxt)
}

func (c *idmConfigImpl) DeleteDeploymentInfo(ctxt diag.RequestContext) diag.Error {
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()
	parentDn := c.getSystemTenantConfigDn()

	return ldapDeleteDeploymentInfo(conn, parentDn, ctxt)
}

func (c *idmConfigImpl) UpdateDeploymentInfo(inf types.DeploymentInfo, ctxt diag.RequestContext) diag.Error {
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return err
	}
	defer conn.Close()
	parentDn := c.getSystemTenantConfigDn()

	return ldapUpdateDeploymentInfo(conn, parentDn, inf, ctxt)
}

func (c *idmConfigImpl) lookupTenant(name diag.TenantID, ctxt diag.RequestContext) (tenantRef, diag.Error) {
	conn, err := c.connProvider.PooledConnection(
		c.cfg.SystemTenant(), c.cfg, c.cfg, ctxt)
	if err != nil {
		return nil, err
	}
	defer conn.Close()
	parentDn := c.getSystemTenantConfigDn()

	return ldapGettenantRef(conn, parentDn, name.String(), ctxt)
}

func (c *idmConfigImpl) getSystemTenantConfigDn() string {
	return getConfigDnForDomain(c.cfg.SystemTenant().String())
}

func EnsureContainer(conn ldap.Connection, parentDn string, name string, ctxt diag.RequestContext) diag.Error {
	lvName, err := ldap.ValueForString(name)
	if err != nil {
		return err
	}
	lvOC, err := ldap.ValueForString("container")
	if err != nil {
		return err
	}
	attributes := []ldap.Attribute{
		ldap.NewAttribute("objectClass", lvOC),
		ldap.NewAttribute("cn", lvName),
	}
	objectDn := "cn=" + name + "," + parentDn

	err = conn.Add(objectDn, attributes, ctxt)
	if err != nil {
		if ldap.IsAlredyExistsError(err) {
			err = nil
		}
	}
	return err
}

func ensureSTSConfigContainer(conn ldap.Connection, parentDn string, ctxt diag.RequestContext) diag.Error {
	return EnsureContainer(conn, "cn="+StsContainer+","+parentDn, StsConfigContainer, ctxt)
}

const (
	StsContainer       = "SecureTokenServer"
	StsAccountsGroup   = "SecureTokenServer"
	StsConfigContainer = "Configuration"
	StsGroupsContainer = "Groups"
	StsUsersContainer  = "Users"
)
