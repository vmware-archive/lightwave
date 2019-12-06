package utils

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type LdapConnectionProvider interface {
	Connection(
		tenant diag.TenantID, ci types.ConnectionInfo, acct types.Account, ctxt diag.RequestContext) (ldap.Connection, diag.Error)

	PooledConnection(
		tenant diag.TenantID, ci types.ConnectionInfo, acct types.Account, ctxt diag.RequestContext) (ldap.Connection, diag.Error)
}

func NewConnectionProvider() LdapConnectionProvider {
	return &ldapConnectionProviderImpl{}
}

type ldapConnectionProviderImpl struct {
}

func (lcp *ldapConnectionProviderImpl) Connection(
	tenant diag.TenantID, ci types.ConnectionInfo, acct types.Account, ctxt diag.RequestContext) (ldap.Connection, diag.Error) {

	// identity ldap bind
	bind := ldap.BindTypeSimple
	if acct.AuthType() == types.AuthTypeNone {
		return nil, diag.MakeError(types.IdmErrorInvalidArgument, "AuthTypeNone is not supported for ldap bind", nil)
	}
	if acct.AuthType() == types.AuthTypeSrp {
		return nil, diag.MakeError(types.IdmErrorInvalidArgument, "AuthTypeSRP is not supported for ldap bind", nil)
	}

	var err diag.Error
	var cn ldap.Connection

	for _, uri := range ci.Addresses() {
		for _, pwd := range acct.Pwd() {
			cn, err = ldap.DefaultConnectionFactory.Connection(
				uri, acct.UserName(), pwd, bind, ci.TrustedCerts, ctxt)
			if err != nil {
				continue
			}
			return cn, nil
		}
	}

	return cn, err
}

func (lcp *ldapConnectionProviderImpl) PooledConnection(
	tenant diag.TenantID, ci types.ConnectionInfo, acct types.Account, ctxt diag.RequestContext) (ldap.Connection, diag.Error) {

	// todo: implement pooling
	return lcp.Connection(tenant, ci, acct, ctxt)
}
