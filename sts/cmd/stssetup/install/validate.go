package install

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"net"
	"net/url"
	"os/exec"
	"os/user"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"golang.org/x/crypto/ssh/terminal"
)

func (c *setupCmd) validate(ctxt diag.RequestContext) error {

	logger := ctxt.Logger()
	logger.Infof(diag.SETUP, "Validating parameters for '%s'", c.Name())

	if len(c.adminUname) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", adminUnameParam)
		return fmt.Errorf("%s is required", adminUnameParam)
	}
	c.adminAccount = account(c.adminUname)

	if len(c.adminPwd) <= 0 {
		fmt.Printf("enter %s:", adminPwdParam)
		bytePassword, err := terminal.ReadPassword(int(syscall.Stdin))
		if err != nil {
			return err
		}
		c.adminPwd = string(bytePassword)
	}
	if len(c.adminPwd) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", adminPwdParam)
		return fmt.Errorf("%s is required", adminPwdParam)
	}

	if len(c.cfgFile) <= 0 {
		c.cfgFile = config.CfgFile
	} else {
		file, err := filepath.Abs(c.cfgFile)
		if err != nil {
			logger.Errorf(diag.SETUP, "%s is invalid: Unable to get absolute path from '%s' provided: %v", cfgFileParam, c.cfgFile, err)
			return fmt.Errorf("%s is invalid: Unable to get absolute path from '%s' provided: %v", cfgFileParam, c.cfgFile, err)
		}
		c.cfgFile = file
	}
	logger.Infof(diag.SETUP, "Config file location is '%s'", c.cfgFile)

	usr, err := user.Lookup(c.daemonUName)
	if err != nil {
		logger.Errorf(diag.SETUP, "%s is invalid: %v", daemonUnameParam, err)
		return fmt.Errorf("%s is invalid: %v", daemonUnameParam, err)
	}
	pid, err := strconv.ParseInt(usr.Uid, 10, strconv.IntSize)
	if err != nil {
		logger.Errorf(diag.SETUP, "%s is invalid: %v", daemonUnameParam, err)
		return fmt.Errorf("%s is invalid: %v", daemonUnameParam, err)
	}
	c.pid = int(pid)

	grp, err := user.LookupGroup(c.daemonGroup)
	if err != nil {
		logger.Errorf(diag.SETUP, "%s is invalid: %v", daemonGroupParam, err)
		return fmt.Errorf("%s is invalid: %v", daemonGroupParam, err)
	}
	gid, err := strconv.ParseInt(grp.Gid, 10, strconv.IntSize)
	if err != nil {
		logger.Errorf(diag.SETUP, "%s is invalid: %v", daemonGroupParam, err)
		return fmt.Errorf("%s is invalid: %v", daemonGroupParam, err)
	}
	c.gid = int(gid)

	if len(c.tlscert) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", tlsCertParam)
		return fmt.Errorf("%s is required", tlsCertParam)
	}

	if len(c.tlskey) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", tlsKeyParam)
		return fmt.Errorf("%s is required", tlsKeyParam)
	}

	cert, err := tls.LoadX509KeyPair(c.tlscert, c.tlskey)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed loading tls cert/key pair: %v", err)
		return fmt.Errorf("Failed loading tls cert/key pair: %v", err)
	}

	c.httpskey = cert.PrivateKey
	c.httpscert, err = x509.ParseCertificate(cert.Certificate[0])
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed parsing x509 cert from tls cert/key pair: %v", err)
		return fmt.Errorf("Failed parsing x509 cert from tls cert/key pair: %v", err)
	}

	if c.firstInstance {
		if len(c.publicEndpoint) <= 0 {
			logger.Errorf(diag.SETUP, "%s is required", publicEndpointParam)
			return fmt.Errorf("%s is required", publicEndpointParam)
		}

		u, err := url.Parse("https://" + c.publicEndpoint)
		if err != nil {
			logger.Errorf(diag.SETUP, "Invalid %s : %v", publicEndpointParam, err)
			return fmt.Errorf("Invalid %s : %v", publicEndpointParam, err)
		}
		if u.Scheme != "https" || u.User != nil || len(u.RawQuery) != 0 ||
			len(u.RawPath) != 0 || len(u.Fragment) != 0 || len(u.Opaque) != 0 {
			logger.Errorf(diag.SETUP, "Invalid %s : expected format is host[:port]", publicEndpointParam)
			return fmt.Errorf("Invalid %s : expected format is host[:port]", publicEndpointParam)
		}

		if strings.Contains(c.publicEndpoint, ":") {
			if strings.Contains(c.publicEndpoint, ":") {
				c.publicHost, c.publicPort, err = net.SplitHostPort(c.publicEndpoint)
				if err != nil {
					logger.Errorf(diag.SETUP, "Invalid %s : expected format is host[:port]", publicEndpointParam)
					return fmt.Errorf("Invalid %s : expected format is host[:port]", publicEndpointParam)
				}
			}
		} else {
			c.publicHost = c.publicEndpoint
			c.publicPort = "443"
		}
	} else {
		if len(c.publicEndpoint) > 0 {
			logger.Errorf(diag.SETUP, "%s is only accepted for primary instance", publicEndpointParam)
			return fmt.Errorf("%s is only accepted for primary instance", publicEndpointParam)
		}
	}

	if len(c.site) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", siteParam)
		return fmt.Errorf("%s is required", siteParam)
	}

	if len(c.systemTenant) <= 0 {
		logger.Errorf(diag.SETUP, "%s is required", systemTenantParam)
		return fmt.Errorf("%s is required", systemTenantParam)
	}

	err = c.tenant.From(c.systemTenant)
	if err != nil {
		logger.Errorf(diag.SETUP, "Invalid system tenant '%s': %v", c.systemTenant, err)
		return fmt.Errorf("Invalid system tenant '%s': %v", c.systemTenant, err)
	}

	if len(c.ldapUrls) <= 0 {
		c.ldapUrls = []*url.URL{ldap.LwDomainURI(c.systemTenant, c.site, true)}
	}
	for _, lu := range c.ldapUrls {
		if !ldap.SupportedScheme(lu.Scheme) ||
			lu.User != nil || len(lu.RawQuery) != 0 ||
			len(lu.Fragment) != 0 || len(lu.Opaque) != 0 {
			logger.Errorf(diag.SETUP, "%s is invalid: %s", ldapUrlsParam, lu.String())
			return fmt.Errorf("%s is invalid: %s", ldapUrlsParam, lu.String())
		}
	}

	c.vmdirCerts = make([]*x509.Certificate, 0)
	if len(c.ldapCerts) > 0 {
		certBytes, err := ioutil.ReadFile(c.ldapCerts)
		if err != nil {
			logger.Errorf(diag.SETUP, "Unable to read '%s': %v", c.ldapCerts, err)
			return fmt.Errorf("Unable to process '%s' parameter. Unable to read '%s': %v", ldapUrlsParam, c.ldapCerts, err)
		}

		var block *pem.Block
		for {
			block, certBytes = pem.Decode(certBytes)
			if block == nil {
				break
			}
			if block.Type == "CERTIFICATE" {
				cert, err := x509.ParseCertificate(block.Bytes)
				if err != nil {
					logger.Errorf(diag.SETUP, "Unable to parse ldap cert: %v", err)
					return fmt.Errorf("Unable to process '%s' parameter. Unable to parse cert from '%s': %v", ldapUrlsParam, c.ldapCerts, err)
				}
				c.vmdirCerts = append(c.vmdirCerts, cert)
			}
		}
	}
	if len(c.vmdirCerts) > 0 {
		c.trustedCerts = x509.NewCertPool()
		for _, cert := range c.vmdirCerts {
			c.trustedCerts.AddCert(cert)
		}
	}

	if len(c.stsAccount) > 0 {
		if len(c.stsAccountPwd) <= 0 {
			fmt.Printf("enter %s:", stsAccountPwdParam)
			bytePassword, err := terminal.ReadPassword(int(syscall.Stdin))
			if err != nil {
				return err
			}
			c.stsAccountPwd = string(bytePassword)
		}
	} else if len(c.stsAccountPwd) > 0 {
		logger.Errorf(diag.SETUP, "%s is required", stsAccountParam)
		return fmt.Errorf("%s is required", stsAccountParam)
	}

	checkedTenantSite := false
	// validate ldapUrls & creds
	for _, lu := range c.ldapUrls {
		if lu != nil {
			conn, err := ldap.DefaultConnectionFactory.Connection(
				lu, unameDn(c.adminUname), c.adminPwd, ldap.BindTypeSimple, c.trustedCertsFunc, ctxt)
			if err != nil {
				logger.Errorf(diag.SETUP, "Unable to bind '%s' and '%s': %v", lu.String(), unameDn(c.adminUname), err)
				return fmt.Errorf("Unable to process '%s' parameter. Unable to validate connection to '%s' for '%s': %v", ldapUrlsParam, lu.String(), unameDn(c.adminUname), err)
			}

			if !checkedTenantSite {
				exists, err := c.ldapObjectExists(conn, types.DnFromDomain(c.systemTenant), "(objectClass=DCObject)", ctxt)
				if err != nil {
					conn.Close()
					logger.Errorf(diag.SETUP, "Unable to verify object DCObject existence for '%s': %v", types.DnFromDomain(c.systemTenant), err)
					return fmt.Errorf("Unable to process '%s' parameter. Failed to validate domain '%s' existence: %v", systemTenantParam, c.systemTenant, err)
				}
				if !exists {
					conn.Close()
					logger.Errorf(diag.SETUP, "DCObject for '%s' does not exist", types.DnFromDomain(c.systemTenant))
					return fmt.Errorf("Unable to process '%s' parameter. Failed to validate domain '%s' does not exist", systemTenantParam, c.systemTenant)
				}

				exists, err = c.ldapObjectExists(conn,
					fmt.Sprintf("cn=%s,cn=Sites,cn=Configuration,%s", c.site, types.DnFromDomain(c.systemTenant)),
					"(objectClass=container)", ctxt)
				if err != nil {
					conn.Close()
					logger.Errorf(diag.SETUP, "Unable to verify site existence for '%s': %v", c.site, err)
					return fmt.Errorf("Unable to process '%s' parameter. Failed to validate site '%s' existence: %v", siteParam, c.site, err)
				}
				if !exists {
					conn.Close()
					logger.Errorf(diag.SETUP, "Site '%s' does not exist", c.site)
					return fmt.Errorf("Unable to process '%s' parameter. Site '%s' does not exist", siteParam, c.site)
				}
				checkedTenantSite = true
			}
			conn.Close()

			if len(c.stsAccount) > 0 {
				conn, err := ldap.DefaultConnectionFactory.Connection(
					lu, unameDn(c.stsAccount), c.stsAccountPwd, ldap.BindTypeSimple, c.trustedCertsFunc, ctxt)
				if err != nil {
					logger.Errorf(diag.SETUP, "Unable to bind '%s' and '%s': %v", lu.String(), unameDn(c.stsAccount), err)
					return fmt.Errorf("Unable to process '%s' parameter. Unable to validate connection to '%s' for '%s': %v", ldapUrlsParam, lu.String(), unameDn(c.stsAccount), err)
				}
				conn.Close()
			}
		}
	}

	err = c.setVmdirHost(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Unable to identify a dc: %v", err)
		return fmt.Errorf("Unable to identify an available DC: %v", err)
	}
	logger.Infof(diag.SETUP, "Successfully validated parameters for '%s'", c.Name())

	return nil
}

func (c *setupCmd) setVmdirHost(ctxt diag.RequestContext) error {

	logger := ctxt.Logger()
	logger.Tracef(diag.SETUP, "identifying vmdir host")

	command := exec.Command(c.VmAfdCli(), "get-dc-name", "--server-name", "localhost")
	out, err := command.Output()
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed running:'%s get-dc-name ...' with:%v", c.VmAfdCli(), err)
		logger.Errorf(diag.SETUP, "Command output %s'", string(out))
		return fmt.Errorf("unable to identify vmdir host: %v", err)
	}

	c.vmdirHost = strings.Trim(string(out), "\n")
	if len(c.vmdirHost) <= 0 {
		logger.Errorf(diag.SETUP, "unable to identify vmdir host")
		return fmt.Errorf("unable to identify vmdir host")
	}
	logger.Infof(diag.SETUP, "identified vmdir host as '%s'", c.vmdirHost)
	return nil
}

func (c *setupCmd) trustedCertsFunc(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error) {
	return c.trustedCerts, nil
}

func account(uname string) string {
	parts := strings.Split(uname, "@")
	if len(parts) == 2 {
		return parts[0]
	}
	if strings.HasPrefix(strings.ToLower(uname), "cn=") {
		index := strings.Index(uname, ",")
		if index != -1 {
			return uname[3:index]
		}
	}
	return uname
}
