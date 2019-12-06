package install

import (
	"bufio"
	"crypto"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/utils"
)

func (c *setupCmd) stsSystemTenant(ctxt diag.RequestContext) error {

	logger := ctxt.Logger()

	logger.Infof(diag.SETUP, "Configuring system tenant for sts")
	if c.firstInstance {
		var err error

		deplInfo, err := c.getDeploymentInfo()
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed constructing deployment info with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (constructing deployment info): %v", err)
		}

		cert, pk, err := c.createStsSigner(ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed generating signer cert/pk with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (creating signer cert/pk): %v", err)
		}
		st, err := c.getSystemTenant(cert, pk)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed constructing system tenant info with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (constructing system tenant config): %v", err)
		}

		systemDomain, err := c.getSystemDomain()
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed constructing system domain info with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (constructing system domain config): %v", err)
		}

		connProv := utils.NewConnectionProvider()
		// check if anything exists under config, if it does prompt and delete....

		cfg, err := idmconfig.NewConfigurator(c.cfg, connProv, ctxt.Logger())
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed obtaining idm configurator: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (creating configurator): %v", err)
		}

		err = c.cleanupSTSConfigIfNeeded(ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed cleaning up sts config container: %v", err)
			return err
		}

		err = cfg.CreateDeploymentInfo(deplInfo, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed registering deployment info in ldap with: %v", err.Error())
			return fmt.Errorf("Failed configuring system tenant for sts (registering deployment info with ldap): %v", err)
		}
		err = cfg.CreateTenant(st, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed registering system tenant in ldap with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (registering system tenant with ldap): %v", err)
		}
		err = cfg.CreateIDS(c.tenant, systemDomain, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed registering system domain info in ldap with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (registering system domain with ldap): %v", err)
		}
	} else {
		connProv := utils.NewConnectionProvider()
		cfg, err := idmconfig.NewConfigurator(c.cfg, connProv, ctxt.Logger())
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed obtaining idm configurator with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (constucting configurator): %v", err)
		}

		deplInfo, err := cfg.GetDeploymentInfo(ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed retrieving deployment info from ldap with: %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (retrieving deployment info from ldap): %v", err)
		}
		if deplInfo.SchemaVersion() != c.schemaVersion {
			logger.Errorf(diag.SETUP, "Existing schema version '%s' does not match schema version of current node '%s'", deplInfo.SchemaVersion(), c.schemaVersion)
			return fmt.Errorf("Ldap schema version '%s' of existing deployment does not match the current instance '%s'",
				deplInfo.SchemaVersion(), c.schemaVersion)
		}
		_, err = cfg.GetTenant(c.tenant, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed retrieving tenant info from ldap with: %v", err)
			return fmt.Errorf("It looks like system tenant is not configured. Check if this is indeed a secondary instance")
		}
		_, err = cfg.GetIDS(c.tenant, c.systemTenant, ctxt)
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed retrieving system domain info from ldap with: %v", err)
			return fmt.Errorf("It looks like system tenant is not properly configured. Check if this is indeed a secondary instance")
		}
	}

	logger.Infof(diag.SETUP, "Successfully configured system tenant for sts")

	return nil
}

func (c *setupCmd) createStsSigner(ctxt diag.RequestContext) (*x509.Certificate, crypto.PrivateKey, error) {
	logger := ctxt.Logger()

	privkeyfile, err := ioutil.TempFile("", "stssigpriv*.prv")
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed creating temporary file for signer private key with: %v", err)
		return nil, nil, err
	}
	logger.Tracef(diag.SETUP, "created temp file for signer pk '%s'", privkeyfile.Name())
	err = privkeyfile.Close()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed closing private key file '%s': %v", privkeyfile.Name(), err)
		return nil, nil, err
	}
	defer func() {
		err = os.Remove(privkeyfile.Name())
		if err != nil {
			logger.Warningf(diag.SETUP, "Failed removing private key file '%s': %v", privkeyfile.Name(), err)
		}
	}()
	pubkeyfile, err := ioutil.TempFile("", "stssigpub*.pub")
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed creating temporary file for signer public key with: %v", err)
		return nil, nil, err
	}
	logger.Tracef(diag.SETUP, "created temp file for signer pub key '%s'", pubkeyfile.Name())
	err = pubkeyfile.Close()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed closing signer public key file '%s' with: %v", pubkeyfile.Name(), err)
		return nil, nil, err
	}
	defer func() {
		err = os.Remove(pubkeyfile.Name())
		if err != nil {
			logger.Warningf(diag.SETUP, "Failed removing private key file '%s': %v", pubkeyfile.Name(), err)
		}
	}()
	certfile, err := ioutil.TempFile("", "stssigcert*.crt")
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed creating temporary file for signer certificate with: %v", err)
		return nil, nil, err
	}
	logger.Tracef(diag.SETUP, "created temp file for signer cert '%s'", certfile.Name())
	err = certfile.Close()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed closing signer cert file '%s' with: %v", certfile.Name(), err)
		return nil, nil, err
	}
	defer func() {
		err = os.Remove(certfile.Name())
		if err != nil {
			logger.Warningf(diag.SETUP, "Failed removing cert file '%s': %v", certfile.Name(), err)
		}
	}()
	configfile, err := ioutil.TempFile("", "stssigconfig*.conf")
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed creating temporary file for signer cert config: %v", err)
		return nil, nil, err
	}
	logger.Tracef(diag.SETUP, "created temp file for signer cert config '%s'", configfile.Name())
	defer func() {
		err = configfile.Close()
		if err != nil {
			logger.Warningf(diag.SETUP, "Failed closing signer cert config file '%s': %v", configfile.Name(), err)
		}
		err = os.Remove(configfile.Name())
		if err != nil {
			logger.Warningf(diag.SETUP, "Failed removing signing cert config file '%s': %v", configfile.Name(), err)
		}
	}()
	_, err = configfile.Write([]byte("Name=STS\nDomainComponent=" + c.systemTenant))
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed writing signer cert config file content: %v", err)
		return nil, nil, err
	}
	logger.Tracef(diag.SETUP, "written signer cert config to '%s'", configfile.Name())
	// gen keys
	cmd := exec.Command(
		c.CertoolCli(), "--genkey",
		"--privkey="+privkeyfile.Name(),
		"--pubkey="+pubkeyfile.Name())
	out, err := cmd.CombinedOutput()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed calling '%s --genkey...': %v", c.CertoolCli(), err)
		logger.Errorf(diag.SETUP, "command output:%s", string(out))
		return nil, nil, err
	}
	// gen cert
	cmd = exec.Command(
		c.CertoolCli(), "--gencert",
		"--privkey="+privkeyfile.Name(),
		"--pubkey="+pubkeyfile.Name(),
		"--cert="+certfile.Name(),
		"--config="+configfile.Name(),
		"--server="+c.vmdirHost)
	out, err = cmd.CombinedOutput()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed calling '%s --gencert...': %v", c.CertoolCli(), err)
		logger.Errorf(diag.SETUP, "command output:%s", string(out))
		return nil, nil, err
	}

	// read pk + cert
	crt, err := tls.LoadX509KeyPair(certfile.Name(), privkeyfile.Name())
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed loading x509 keypair from '%s', '%s': %v", certfile.Name(), privkeyfile.Name(), err)
		return nil, nil, err
	}

	signingcert, err := x509.ParseCertificate(crt.Certificate[0])
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed parsing x509 cert from loaded key pair: %v", err)
		return nil, nil, err
	}

	return signingcert, crt.PrivateKey, nil
}

func (c *setupCmd) getSystemTenant(signer *x509.Certificate, pk crypto.PrivateKey) (types.Tenant, diag.Error) {
	tb := types.NewTenantBuilder()
	tb.Name(c.tenant)
	tb.Domain(c.systemTenant)

	tb.SignerCert(signer)
	tb.SignerKey(pk)
	tb.SignerCerts([]*x509.Certificate{signer})

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

func (c *setupCmd) getSystemDomain() (types.IDSConfig, diag.Error) {
	tb := types.NewIDSConfigBuilder()
	tb.Name(c.systemTenant)
	tb.Domain(c.systemTenant)

	tb.AuthType(types.AuthTypeStsAccount)
	tb.Addresses(c.cfg.Addresses())
	tb.Certificates(c.cfg.Certificates())

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

func (c *setupCmd) getDeploymentInfo() (types.DeploymentInfo, diag.Error) {
	b := types.NewDeploymentInfoBuilder()
	b.PublicEndpoint(c.publicEndpoint)
	b.SchemaVersion(c.schemaVersion)

	return b.Build()
}

func (c *setupCmd) cleanupSTSConfigIfNeeded(ctxt diag.RequestContext) error {
	var err error
	logger := ctxt.Logger()

	conn, err := ldap.DefaultConnectionFactory.Connection(
		c.ldapUrls[0], unameDn(c.adminUname), c.adminPwd, ldap.BindTypeSimple, c.trustedCertsFunc, ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Opening connection: (%s, %s) faled with: %v", c.ldapUrls[0], unameDn(c.adminUname), err)
		return fmt.Errorf("Failed configuring system tenant for sts (failing to connect to ldap server): %v", err)
	}
	defer conn.Close()

	stsConfigDn := fmt.Sprintf("cn=%s,cn=%s,%s", idmconfig.StsConfigContainer, idmconfig.StsContainer, types.DnFromDomain(c.systemTenant))
	exists, err := c.ldapObjectExists(
		conn, stsConfigDn, "(objectClass=container)", ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Checking object existence: (%s, %s) faled with: %v",
			stsConfigDn, "(objectClass=container)", err)
		return fmt.Errorf("Failed configuring system tenant for sts (failed to check sts config container): %v", err)
	}

	if !exists {
		return nil
	}
	// config container already exists, warn
	cleanup := false
	if !c.quiet {
		fmt.Printf("Sts configuration seems to already exists, please confirm it's cleanup (Y/N):")
		reader := bufio.NewReader(os.Stdin)
		char, _, err := reader.ReadRune()
		if err != nil {
			logger.Errorf(diag.SETUP, "Failed configuring system tenant for sts (to confirm sts config container deletion): %v", err)
			return fmt.Errorf("Failed configuring system tenant for sts (to confirm sts config container deletion): %v", err)
		}
		cleanup = (char == 'y' || char == 'Y')
	} else {
		cleanup = true
	}

	if !cleanup {

	}
	err = ldap.DeleteSubtree(conn, stsConfigDn, true, ctxt)

	if err != nil {
		logger.Errorf(diag.SETUP, "Failed cleaning up sts config container %s with: %v",
			stsConfigDn, err)
		return fmt.Errorf("Failed configuring system tenant for sts (failed to cleanup sts config container): %v", err)
	}

	return nil
}
