package idm

import (
	"fmt"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/provider"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

type Services interface {
	Authenticator() Authenticator
	Configurator() config.Configurator
	StsConfig() config.StsConfig
	// ***
}

func NewServices(cfg config.ConfigStoreInfo, logger diag.Logger) (Services, diag.Error) {

	ldapProvider := utils.NewConnectionProvider()
	idmCfg, err := config.NewConfigurator(cfg, ldapProvider, logger)
	if err != nil {
		return nil, err
	}

	svcs := &svcsImpl{
		configImpl:   idmCfg,
		connProvider: ldapProvider,
		cfg:          cfg,
	}
	svcs.authImpl = &authenticatorImpl{idpProviders: svcs}
	return svcs, nil
}

type svcsImpl struct {
	authImpl   Authenticator
	configImpl config.Configurator

	connProvider utils.LdapConnectionProvider
	cfg          config.ConfigStoreInfo
}

func (s *svcsImpl) Authenticator() Authenticator      { return s.authImpl }
func (s *svcsImpl) Configurator() config.Configurator { return s.configImpl }
func (s *svcsImpl) StsConfig() config.StsConfig       { return s.configImpl }

func (s *svcsImpl) Providers(tenant diag.TenantID, ctxt diag.RequestContext) ([]provider.IdentityProvider, diag.Error) {
	// todo caching
	configs, err := s.configImpl.ListIDSs(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	provs := make([]provider.IdentityProvider, 0, len(configs))
	for _, c := range configs {
		p, err := provider.NewIdentityProvider(tenant, c, s.connProvider, s.cfg, s.cfg.TrustedCerts, ctxt)
		if err != nil {
			return []provider.IdentityProvider{}, nil
		}
		provs = append(provs, p)
	}

	return provs, nil
}

type idpProvidersLookup interface {
	Providers(tenant diag.TenantID, ctxt diag.RequestContext) ([]provider.IdentityProvider, diag.Error)
}

type authenticatorImpl struct {
	idpProviders idpProvidersLookup
}

func (ai *authenticatorImpl) Authenticate(tenant diag.TenantID, creds types.Credentials, ctxt diag.RequestContext) (types.UserID, diag.Error) {

	unamePwd, ok := creds.(types.UnamePwdCredentials)
	if !ok {
		return types.NoneUserID,
			diag.MakeError(types.IdmErrorInvalidArgument,
				fmt.Sprintf("Unsupported Auth type: '%v'", creds), nil)
	}
	id, err := types.UserIDFromString(unamePwd.UserName())
	if err != nil {
		return types.NoneUserID, err
	}

	provider, err := ai.pickIDP(tenant, id, ctxt)
	if err != nil {
		return types.NoneUserID, err
	}

	return provider.Authenticate(creds, ctxt)
}

func (ai *authenticatorImpl) pickIDP(tenant diag.TenantID, uid types.UserID, ctxt diag.RequestContext) (provider.IdentityProvider, diag.Error) {
	provs, err := ai.idpProviders.Providers(tenant, ctxt)
	if err != nil {
		return nil, err
	}

	var provider provider.IdentityProvider
	for _, p := range provs {
		if p.ContainsUserDomain(uid) {
			provider = p
			break
		}
	}

	if provider == nil {
		return nil, diag.MakeError(types.IdmErrorInvalidArgument, fmt.Sprintf("unknown domain: '%v'", uid), nil)
	}

	return provider, nil
}

func (ai *authenticatorImpl) GetUserAttributes(
	tenant diag.TenantID, ctxt diag.RequestContext,
	user types.UserID, attributes ...types.AttributeID) (types.AttributeValues, diag.Error) {

	provider, err := ai.pickIDP(tenant, user, ctxt)
	if err != nil {
		return nil, err
	}

	return provider.GetUserAttributes(ctxt, user, attributes...)
}
