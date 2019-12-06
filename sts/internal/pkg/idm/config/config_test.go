package config

import (
	"crypto"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"fmt"
	"math/big"
	mathrand "math/rand"
	"net/url"
	"testing"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

func TestConfigOps(t *testing.T) {

	lgr, err := diag.NewLogger()
	assertEquals(t, err, nil, "error")

	configCfg := &cfgInfoImpl{}
	err = configCfg.genSigner()
	if err != nil {
		t.Fatalf("Unable to generate signer key: %v", err)
	}

	connProv := utils.NewConnectionProvider()
	cfg, err := NewConfigurator(configCfg, connProv, lgr)
	assertEquals(t, err, nil, "error")

	ctxt := diag.NewRequestContext(configCfg.SystemTenant(), "12345", lgr)

	tenant, err := getSystemTenant(configCfg)
	assertEquals(t, err, nil, "error")

	systemDomain, err := getSystemDomain(configCfg)
	assertEquals(t, err, nil, "error")

	secondDomain, err := getSecondDomain(configCfg)
	assertEquals(t, err, nil, "error")

	deploymentInfo, err := getDeploymentInfo()
	assertEquals(t, err, nil, "error")

	oidcClient, err := getOidcClient()
	assertEquals(t, err, nil, "error")

	defer func() {
		conn, _ := connProv.Connection(
			configCfg.SystemTenant(), configCfg, configCfg, ctxt)
		defer conn.Close()
		ldap.DeleteSubtree(conn, "cn="+stsContainer+","+types.DnFromDomain(configCfg.SystemTenant().String()), true, ctxt)
	}()

	success := t.Run("Create system tenant", func(t *testing.T) {
		err = cfg.CreateTenant(tenant, ctxt)
		assertEquals(t, err, nil, "error")
	})

	if success {
		success = t.Run("Get system tenant", func(t *testing.T) {
			t1, err := cfg.GetTenant(tenant.Name(), ctxt)
			assertEquals(t, err, nil, "error")

			assertEquals(t, t1.Name(), tenant.Name(), "Name()")
			assertEquals(t, t1.Domain(), tenant.Domain(), "Domain()")
		})
	}

	tenantSuccess := success
	if tenantSuccess {
		success = t.Run("Create system domain", func(t *testing.T) {
			err := cfg.CreateIDS(tenant.Name(), systemDomain, ctxt)
			assertEquals(t, err, nil, "error")
		})
	}

	if success {
		success = t.Run("Get system domain", func(t *testing.T) {
			ids1, err := cfg.GetIDS(tenant.Name(), systemDomain.Name(), ctxt)
			assertEquals(t, err, nil, "error")

			assertEquals(t, ids1.Name(), systemDomain.Name(), "Name()")
			assertEquals(t, ids1.Domain(), systemDomain.Domain(), "Domain()")

		})
	}

	if tenantSuccess {
		success = t.Run("Create second domain", func(t *testing.T) {
			err := cfg.CreateIDS(tenant.Name(), secondDomain, ctxt)
			assertEquals(t, err, nil, "error")

			ids2, err := cfg.GetIDS(tenant.Name(), secondDomain.Name(), ctxt)
			assertEquals(t, err, nil, "error")

			assertEquals(t, ids2.Name(), secondDomain.Name(), "Name()")
			assertEquals(t, ids2.Domain(), secondDomain.Domain(), "Domain()")

			assertEquals(t, ids2.AuthType(), secondDomain.AuthType(), "AuthType()")
			assertEquals(t, ids2.UserName(), secondDomain.UserName(), "UserName()")

			assertArrayEquals(t, ids2.Pwd(), secondDomain.Pwd(), "Pwd()")

			err = cfg.DeleteIDS(tenant.Name(), secondDomain.Name(), ctxt)
			assertEquals(t, err, nil, "error")
		})
	}

	success = t.Run("Create deployment info", func(t *testing.T) {
		err := cfg.CreateDeploymentInfo(deploymentInfo, ctxt)
		assertEquals(t, err, nil, "error")

		deplInfo1, err := cfg.GetDeploymentInfo(ctxt)
		assertEquals(t, err, nil, "error")

		assertEquals(t, deplInfo1.PublicEndpoint(), deploymentInfo.PublicEndpoint(), "PublicEndpoint")
		assertEquals(t, deplInfo1.SchemaVersion(), deploymentInfo.SchemaVersion(), "SchemaVersion")
	})

	if tenantSuccess {
		success = t.Run("Create oidc client", func(t *testing.T) {
			err := cfg.CreateOidcClient(tenant.Name(), oidcClient, ctxt)
			assertEquals(t, err, nil, "error")

			cli1, err := cfg.GetOidcClient(tenant.Name(), oidcClient.ID(), ctxt)
			assertEquals(t, err, nil, "error")

			assertEquals(t, cli1.ID(), oidcClient.ID(), "ID")
			assertUrlPtrArrayEquals(t, cli1.RedirectURIs(), oidcClient.RedirectURIs(), "RedirectURIs")
		})
	}

}

func assertEquals(t *testing.T, actual interface{}, expected interface{}, valueName string) {
	if actual != expected {
		t.Fatalf("%s: actual value '%v' expected value '%v'", valueName, actual, expected)
	}
}

func assertArrayEquals(t *testing.T, actual []string, expected []string, valueName string) {
	if len(actual) != len(expected) {
		t.Fatalf("%s: actual value len='%v' expected value len'%v'", valueName, len(actual), len(expected))
	}

	for _, av := range actual {
		found := false
		for _, ae := range expected {
			if av == ae {
				found = true
				break
			}
		}

		if !found {
			t.Fatalf("%s: actual value '%v' not found in expected", valueName, av)
		}
	}
}

func assertUrlPtrArrayEquals(t *testing.T, actual []*url.URL, expected []*url.URL, valueName string) {
	if len(actual) != len(expected) {
		t.Fatalf("%s: actual value len='%v' expected value len'%v'", valueName, len(actual), len(expected))
	}

	for _, av := range actual {
		found := false
		for _, ae := range expected {
			if av == ae {
				found = true
				break
			} else if av != nil && ae != nil {
				if av.String() == ae.String() {
					found = true
					break
				}
			}
		}

		if !found {
			t.Fatalf("%s: actual value '%v' not found in expected", valueName, av)
		}
	}
}

type cfgInfoImpl struct {
	signer *x509.Certificate
	pk     crypto.PrivateKey
}

func (c *cfgInfoImpl) SystemTenant() diag.TenantID {
	return "lw-testdom.com"
}

func (c *cfgInfoImpl) Addresses() []*url.URL {
	u, _ := url.Parse("ldaps://192.168.114.3:636")
	return []*url.URL{u}
}

func (c *cfgInfoImpl) Certificates() []*x509.Certificate {
	return []*x509.Certificate{}
}

func (c *cfgInfoImpl) AuthType() types.AuthType {
	return types.AuthTypeSimple
}
func (c *cfgInfoImpl) UserName() string {
	return "cn=Administrator,cn=users,dc=lw-testdom,dc=com"
}
func (c *cfgInfoImpl) Pwd() []string {
	return []string{"Ca$hc0w1"}
}

func (c *cfgInfoImpl) genSigner() error {
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
		return fmt.Errorf("Failed to gen private key: %v", err)
	}

	c.pk = privatekey

	// create a self-signed certificate. template = parent
	var parent = template
	cert, err := x509.CreateCertificate(rand.Reader, template, parent, &privatekey.PublicKey, privatekey)
	if err != nil {
		return fmt.Errorf("Failed to gen cert: %v", err)
	}

	c.signer, err = x509.ParseCertificate(cert)
	if err != nil {
		return fmt.Errorf("Failed to parse cert: %v", err)
	}

	return nil
}

func getSystemTenant(configCfg *cfgInfoImpl) (types.Tenant, diag.Error) {
	tb := types.NewTenantBuilder()
	tb.Name(configCfg.SystemTenant())
	tb.Domain(configCfg.SystemTenant().String())

	tb.SignerCert(configCfg.signer)
	tb.SignerKey(configCfg.pk)
	tb.SignerCerts([]*x509.Certificate{configCfg.signer})

	tb.ClockTolerance(time.Duration(10) * time.Minute)

	tb.MaxBearerLifetime(time.Duration(1) * time.Hour)
	tb.MaxHOKLifetime(time.Duration(30*24) * time.Hour)
	tb.MaxBearerRefreshLifetime(time.Duration(8) * time.Hour)
	tb.MaxHOKRefreshLifetime(time.Duration(8) * time.Hour)

	tb.SessionLifetime(time.Duration(8) * time.Hour)

	tb.DelegationCount(10)
	tb.RenewCount(10)

	var lms types.LoginMethodSet
	lms.From([]string{types.LoginMethodPassword.String()})
	tb.LoginMethods(lms)

	return tb.Build()
}

func getSystemDomain(configCfg *cfgInfoImpl) (types.IDSConfig, diag.Error) {
	tb := types.NewIDSConfigBuilder()
	tb.Name(configCfg.SystemTenant().String())
	tb.Domain(configCfg.SystemTenant().String())

	tb.AuthType(types.AuthTypeStsAccount)
	tb.Addresses(configCfg.Addresses())
	tb.Certificates(configCfg.Certificates())

	tb.Provider(types.ProviderTypeVmdir)

	// user_identity first_name last_name group_identities
	idsb := types.NewIDSAttributeMapBuilder()
	ab := types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDUserIdentity)
	ab.AttrType(types.IDSAttributeTypeAttribute)
	attr, err := ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)
	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDGroupIdentities)
	ab.AttrType(types.IDSAttributeTypeAttribute)
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)

	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDFirstName)
	ab.AttrType(types.IDSAttributeTypeLdapAttribute)
	ab.AttrValue("givenName")
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)
	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDLastName)
	ab.AttrType(types.IDSAttributeTypeLdapAttribute)
	ab.AttrValue("lastName")
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)

	am, err := idsb.Build()
	if err != nil {
		return nil, err
	}

	tb.Attributes(am)

	var lms types.LoginMethodSet
	lms.From([]string{types.LoginMethodPassword.String()})
	tb.LoginMethods(lms)

	return tb.Build()
}

func getSecondDomain(configCfg *cfgInfoImpl) (types.IDSConfig, diag.Error) {
	tb := types.NewIDSConfigBuilder()
	tb.Name("ex-" + configCfg.SystemTenant().String())
	tb.Domain("ex-" + configCfg.SystemTenant().String())

	tb.AuthType(configCfg.AuthType())
	tb.UserName(configCfg.UserName())
	tb.Pwd(configCfg.Pwd())
	tb.Addresses(configCfg.Addresses())
	tb.Certificates(configCfg.Certificates())

	tb.Provider(types.ProviderTypeVmdir)

	// user_identity first_name last_name group_identities
	idsb := types.NewIDSAttributeMapBuilder()
	ab := types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDUserIdentity)
	ab.AttrType(types.IDSAttributeTypeAttribute)
	attr, err := ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)
	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDGroupIdentities)
	ab.AttrType(types.IDSAttributeTypeAttribute)
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)

	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDFirstName)
	ab.AttrType(types.IDSAttributeTypeLdapAttribute)
	ab.AttrValue("givenName")
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)
	ab = types.NewIDSAttributeBuilder()
	ab.ID(types.AttributeIDLastName)
	ab.AttrType(types.IDSAttributeTypeLdapAttribute)
	ab.AttrValue("lastName")
	attr, err = ab.Build()
	if err != nil {
		return nil, err
	}
	idsb.Add(attr)

	am, err := idsb.Build()
	if err != nil {
		return nil, err
	}

	tb.Attributes(am)

	var lms types.LoginMethodSet
	lms.From([]string{types.LoginMethodPassword.String()})
	tb.LoginMethods(lms)

	return tb.Build()
}

func getDeploymentInfo() (types.DeploymentInfo, diag.Error) {
	b := types.NewDeploymentInfoBuilder()
	b.PublicEndpoint("10.62.21.242:5555")
	b.SchemaVersion("0.0.0.1")

	return b.Build()
}

func getOidcClient() (types.OidcClient, diag.Error) {

	b := types.NewOidcClientBuilder()
	redirUrl, _ := url.Parse("https://10.62.21.242/oidccli/auth")
	postLogoutUrl, _ := url.Parse("https://10.62.21.242/oidccli")
	logoutUrl, _ := url.Parse("https://10.62.21.242/oidccli/logout")

	b.RedirectURIs([]*url.URL{redirUrl})
	b.AuthMethod(types.OidcClientAuthMethodNone)
	b.PostLogoutRedirectURIs([]*url.URL{postLogoutUrl})
	b.LogoutURI(logoutUrl)

	return b.Build()
}
