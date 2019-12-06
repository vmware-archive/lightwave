package provider

import (
	"crypto/x509"
	"fmt"
	"net/url"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

type vmdirProviderImpl struct {
	tenant     diag.TenantID
	cfg        types.IDSConfig
	conn       utils.LdapConnectionProvider
	stsAccount types.Account
	certs      ldap.TrustedCertsFunc
	secDomains types.SecurityDomainSet
}

func newVmDirProvider(tenant diag.TenantID, cfg types.IDSConfig,
	connectionProvider utils.LdapConnectionProvider,
	stsAccount types.Account, lwdomainCerts ldap.TrustedCertsFunc,
	ctxt diag.RequestContext) IdentityProvider {
	vmDir := &vmdirProviderImpl{
		tenant:     tenant,
		cfg:        cfg,
		conn:       connectionProvider,
		stsAccount: stsAccount}
	if len(cfg.Addresses()) == 1 && ldap.LwDomainScheme(cfg.Addresses()[0].Scheme) {
		vmDir.certs = lwdomainCerts
	} else {
		vmDir.certs = cfg.TrustedCerts
	}
	vmDir.secDomains = buildSecurityDomains(cfg)
	return vmDir
}

func (p *vmdirProviderImpl) SecurityDomains() types.SecurityDomainSet {
	return p.secDomains
}

func (p *vmdirProviderImpl) ContainsUserDomain(user types.UserID) bool {
	dom := types.DomainFromUserID(user)
	return p.secDomains.ContainsName(dom)
}

func (p *vmdirProviderImpl) Authenticate(creds types.Credentials, ctxt diag.RequestContext) (types.UserID, diag.Error) {

	unamePwd, ok := creds.(types.UnamePwdCredentials)
	if !ok {
		return types.NoneUserID,
			diag.MakeError(types.IdmErrorInvalidArgument,
				fmt.Sprintf("Unsupported Auth type: '%v'", creds), nil)
	}

	idsAcct := p.effectiveIDSAccount()
	userID := types.NoneUserID
	uname := unamePwd.UserName()
	var err diag.Error

	if idsAcct.AuthType() == types.AuthTypeSimple {
		uname, userID, err = p.retrieveUserID(unamePwd.UserName(), ctxt)
		if err != nil {
			return types.NoneUserID, err
		}
	}

	loginAcct := newAccount(idsAcct.AuthType(), uname, unamePwd.Password())
	conn, err := p.conn.Connection(p.tenant, p, loginAcct, ctxt)
	if err != nil {
		return types.NoneUserID, err
	}
	defer conn.Close()

	if userID == types.NoneUserID {
		_, userID, err = p.retrieveUserID(unamePwd.UserName(), ctxt)
		if err != nil {
			return types.NoneUserID, err
		}
	}

	return userID, nil
}

func (p *vmdirProviderImpl) GetUserAttributes(ctxt diag.RequestContext,
	user types.UserID, attributes ...types.AttributeID) (types.AttributeValues, diag.Error) {

	conn, err := p.conn.PooledConnection(p.tenant, p, p.effectiveIDSAccount(), ctxt)
	if err != nil {
		return nil, err
	}

	ldapAttributes := []string{}
	attrMap := p.cfg.Attributes()

	for _, attr := range attributes {
		a, ok := attrMap.V(attr)
		if ok {
			switch a.AttrType() {
			//constant ldap_attribute attribute
			case types.IDSAttributeTypeAttribute:
				{
					switch attr {
					case types.AttributeIDUserIdentity:
						{
							ldapAttributes = append(ldapAttributes, "userPrincipalName", "samAccountName")
						}
					case types.AttributeIDGroupIdentities:
						{
							ldapAttributes = append(ldapAttributes, "memberOf")
						}
					}
				}
			case types.IDSAttributeTypeLdapAttribute:
				{
					ldapAttributes = append(ldapAttributes, a.AttrValue().String())
				}
			}
		}
	}

	m, err := conn.Search(
		p.userSearchBase(), ldap.ScopeSubTree, p.userByUnameFilter(user.String()),
		ldapAttributes, false, 0, 0, ctxt)
	if err != nil {
		return nil, err
	}
	defer m.Close()
	lene, err := m.Len(ctxt)
	if err != nil {
		return nil, err
	}
	if lene != 1 {
		return nil, diag.MakeError(types.IdmErrorGeneric, "Non unique user", nil)
	}

	avsb := types.NewAttributeValuesBuilder(len(attributes))
	err = m.IterateEntries(func(e ldap.Entry, ctxt diag.RequestContext) diag.Error {

		for _, attr := range attributes {
			a, ok := attrMap.V(attr)
			if ok {
				var avb types.AttributeValueBuilder
				switch a.AttrType() {
				case types.IDSAttributeTypeConstant:
					{
						avb = types.NewAttributeValueBuilder(1)
						avb.ID(attr)
						avb.Add(a.AttrValue().String())
					}
					//constant ldap_attribute attribute
				case types.IDSAttributeTypeAttribute:
					{
						switch attr {
						case types.AttributeIDUserIdentity:
							{
								avb = types.NewAttributeValueBuilder(1)
								avb.ID(attr)
								val, err := p.getUserIdentity(e, ctxt)
								if err != nil {
									return err
								}
								avb.Add(val)
							}
						case types.AttributeIDGroupIdentities:
							{
								mems, err := ldap.GetAttributeValue(e, "memberOf", true, ctxt)
								if err != nil {
									return err
								}
								avb = types.NewAttributeValueBuilder(mems.Len())
								avb.ID(attr)
								err = p.getGroupIdentities(conn, mems, avb, ctxt)
								if err != nil {
									return err
								}
							}
						}
					}
				case types.IDSAttributeTypeLdapAttribute:
					{
						mems, err := ldap.GetAttributeValue(e, a.AttrValue().String(), false, ctxt)
						if err != nil {
							return err
						}
						leng := 0
						if mems != nil {
							leng = mems.Len()
						}
						avb = types.NewAttributeValueBuilder(leng)
						avb.ID(attr)
						if mems != nil {
							mems.IterateString(func(v string) diag.Error {
								avb.Add(v)
								return nil
							})
						}
					}
				}

				val, err := avb.Build()
				if err != nil {
					return err
				}
				avsb.Add(val)
			}
		}
		return nil
	}, ctxt)
	if err != nil {
		return nil, err
	}

	return avsb.Build()
}

func (p *vmdirProviderImpl) getUserIdentity(e ldap.Entry, ctxt diag.RequestContext) (string, diag.Error) {
	sam, err := ldap.GetAttributeValue(e, "sAMAccountName", true, ctxt)
	if err != nil {
		return "", err
	}
	upn, err := ldap.GetAttributeValue(e, "userPrincipalName", false, ctxt)
	if err != nil {
		return "", err
	}
	val := ""
	if upn != nil && upn.Len() > 0 {
		val, err = ldap.StringForValue(upn)
		if err != nil {
			return "", err
		}
	} else {
		val, err = ldap.StringForValue(sam)
		if err != nil {
			return "", err
		}
		val = val + "@" + p.cfg.Domain()
	}
	return val, nil
}

func (p *vmdirProviderImpl) getGroupIdentities(conn ldap.Connection, groupDns ldap.Value, builder types.AttributeValueBuilder, ctxt diag.RequestContext) diag.Error {
	// todo: retrieve via batching
	return groupDns.IterateString(func(v string) diag.Error {
		m, err := conn.Search(v, ldap.ScopeBase, "(objectClass=*)", []string{"cn"}, false, 0, 0, ctxt)
		if err != nil {
			return err
		}
		defer m.Close()

		leng, err := m.Len(ctxt)
		if err != nil {
			return err
		}

		if leng != 1 {
			return diag.MakeError(types.IdmErrorGeneric, "Non unique group", nil)
		}

		var cn string

		err = m.IterateEntries(func(e ldap.Entry, ctxt diag.RequestContext) diag.Error {
			lcn, err := ldap.GetAttributeValue(e, "cn", true, ctxt)
			if err != nil {
				return err
			}

			cn, err = ldap.StringForValue(lcn)
			if err != nil {
				return err
			}
			return nil
		}, ctxt)

		if err != nil {
			return err
		}
		builder.Add(p.cfg.Domain() + "\\" + cn)

		return nil
	})
}

func (p *vmdirProviderImpl) retrieveUserID(userName string, ctxt diag.RequestContext) (string, types.UserID, diag.Error) {
	idsConn, err := p.conn.PooledConnection(
		p.tenant, p, p.effectiveIDSAccount(), ctxt)
	if err != nil {
		return "", types.NoneUserID, err
	}
	defer idsConn.Close()

	m, err := idsConn.Search(
		p.userSearchBase(), ldap.ScopeSubTree, p.userByUnameFilter(userName),
		[]string{"samAccountName", "userPrincipalName"}, false, 0, 0, ctxt)
	if err != nil {
		return "", types.NoneUserID, err
	}
	defer m.Close()
	len, err := m.Len(ctxt)
	if err != nil {
		return "", types.NoneUserID, err
	}
	if len != 1 {
		return "", types.NoneUserID, diag.MakeError(types.IdmErrorGeneric, "Non unique user", nil)
	}

	uID := types.NoneUserID
	dn := ""

	err = m.IterateEntries(func(e ldap.Entry, ctxt diag.RequestContext) diag.Error {

		dn, err = e.Dn(ctxt)
		if err != nil {
			return err
		}
		val, err := p.getUserIdentity(e, ctxt)
		if err != nil {
			return err
		}
		uID, err = types.UserIDFromString(val)
		if err != nil {
			return err
		}
		return nil
	}, ctxt)
	if err != nil {
		return "", types.NoneUserID, err
	}

	return dn, uID, nil
}

func (p *vmdirProviderImpl) userSearchBase() string {
	if len(p.cfg.UserBaseDN()) > 0 {
		return p.cfg.UserBaseDN()
	}

	return types.DnFromDomain(p.cfg.Domain())
}

func (p *vmdirProviderImpl) userByUnameFilter(name string) string {
	parts := strings.Split(name, "@")

	if strings.EqualFold(p.cfg.Domain(), parts[1]) {
		return fmt.Sprintf(userQueryByUpnOrAccount, name, p.tenantizedUPN(name), parts[0], "")
	} else {
		return fmt.Sprintf(userQueryByUpn, name, p.tenantizedUPN(name))
	}
}

func (p *vmdirProviderImpl) tenantizedUPN(user string) string {
	return user + "/" + strings.ToLower(p.tenant.String())
}

func (p *vmdirProviderImpl) effectiveIDSAccount() types.Account {
	acct := types.Account(p.cfg)
	if p.cfg.AuthType() == types.AuthTypeStsAccount {
		acct = p.stsAccount
	}
	return acct
}

func (p *vmdirProviderImpl) Addresses() []*url.URL {
	return p.cfg.Addresses()
}
func (p *vmdirProviderImpl) TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error) {
	return p.certs(refresh, ctxt)
}

const (
	/**
	 * arg1 - userPrincipalName
	 * arg2 - tenantizedUserPrincipalName
	 * arg3 - sAMAccountName
	 * arg4 - additional filter
	 */
	userQueryByUpnOrAccount = "(&(|" +
		"(userPrincipalName=%s)" +
		"(vmwSTSTenantizedUserPrincipalName=%s)" +
		"(sAMAccountName=%s)" +
		")(objectClass=user)(!(vmwSTSSubjectDN=*))%s)"

	/**
	 * arg1 - userPrincipalName
	 * arg2 - tenantizedUserPrincipalName
	 */
	userQueryByUpn = "(&(|" +
		"(userPrincipalName=%s)" +
		"(vmwSTSTenantizedUserPrincipalName=%s)" +
		")(objectClass=user))"
)

func buildSecurityDomains(cfg types.IDSConfig) types.SecurityDomainSet {
	b := types.NewSecurityDomainBuilder()
	b.Name(cfg.Domain())
	b.Alias(cfg.Alias())
	v, _ := b.Build()
	sb := types.NewSecurityDomainSetBuilder(1)
	sb.Add(v)
	s, _ := sb.Build()
	return s
}

func newAccount(authType types.AuthType, uname string, pwd string) types.Account {
	return acctImpl{authType: authType, uname: uname, pwd: []string{pwd}}
}

type acctImpl struct {
	authType types.AuthType
	uname    string
	pwd      []string
}

func (ai acctImpl) AuthType() types.AuthType { return ai.authType }
func (ai acctImpl) UserName() string         { return ai.uname }
func (ai acctImpl) Pwd() []string            { return ai.pwd }
