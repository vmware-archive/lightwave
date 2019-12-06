package provider

import (
	"fmt"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

type IdentityProvider interface {
	SecurityDomains() types.SecurityDomainSet

	ContainsUserDomain(user types.UserID) bool

	Authenticate(creds types.Credentials, ctxt diag.RequestContext) (types.UserID, diag.Error)

	GetUserAttributes(ctxt diag.RequestContext,
		user types.UserID, attributes ...types.AttributeID) (types.AttributeValues, diag.Error)

	//LookupActiveUserByIdentity(user string, ctxt diag.RequestContext) (UserID, diag.Error)

	// additional user & groups lookup methods
}

func NewIdentityProvider(
	tenant diag.TenantID, cfg types.IDSConfig,
	connectionProvider utils.LdapConnectionProvider,
	stsAccount types.Account, lwdomainCerts ldap.TrustedCertsFunc,
	ctxt diag.RequestContext) (IdentityProvider, diag.Error) {
	switch cfg.Provider() {
	case types.ProviderTypeVmdir:
		return newVmDirProvider(tenant, cfg, connectionProvider, stsAccount, lwdomainCerts, ctxt), nil
	default:
		{
			return nil, diag.MakeError(
				types.IdmErrorInvalidArgument,
				fmt.Sprintf("Unsupported provider type: '%s'", cfg.Provider().String()), nil)
		}
	}
}
