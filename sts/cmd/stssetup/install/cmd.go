package install

import (
	"crypto"
	"crypto/x509"
	"flag"
	"fmt"
	"os"
	"path"
	"strings"

	"github.com/vmware/lightwave/sts/cmd/stssetup/param"
	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

func NewSetupCmd() param.Command {
	return &setupCmd{
		buildInfo: config.GetBuildInfo(),
	}
}

type setupCmd struct {
	setupSet *flag.FlagSet

	help bool

	adminUname string
	adminPwd   string

	cfgFile             string
	port                int
	shutdownTimeoutSecs int
	enabledHeads        param.HttpHeadsParam
	tlscert             string
	tlskey              string
	daemon              bool
	daemonUName         string
	daemonGroup         string

	firstInstance  bool
	publicEndpoint string
	systemTenant   string
	site           string
	ldapUrls       param.UrlsParam
	ldapCerts      string

	stsAccount    string
	stsAccountPwd string

	logLvl      int
	logLocation string

	quiet bool

	// calculated
	httpscert     *x509.Certificate
	httpskey      crypto.PrivateKey
	vmdirCerts    []*x509.Certificate
	trustedCerts  *x509.CertPool
	adminAccount  string
	vmdirHost     string
	tenant        diag.TenantID
	cfg           config.InstanceConfig
	pid           int
	gid           int
	schemaVersion string
	publicHost    string
	publicPort    string

	buildInfo config.BuildInfo
}

func (c *setupCmd) Name() string { return "install" }
func (c *setupCmd) ShortDescription() string {
	return "configures sts instance post (rpm) package install"
}
func (c *setupCmd) LogLvl() int32 {
	return int32(c.logLvl)
}

func (c *setupCmd) LogLocation() string {
	return c.logLocation
}

func (c *setupCmd) RegisterParams() {

	c.setupSet = flag.NewFlagSet(c.Name(), flag.ContinueOnError)
	c.setupSet.SetOutput(os.Stdout)

	c.setupSet.IntVar(&c.logLvl, "log-level", int(diag.ErrorLvl), "log level (defaults to error)")
	c.setupSet.StringVar(&c.logLocation, "log-file", "", "log file location")

	c.setupSet.BoolVar(&c.help, helpParam, false, "print out usage details")
	c.setupSet.BoolVar(&c.quiet, quietParam, false, "whether to suppress any confirmations")

	c.setupSet.StringVar(&c.adminUname, adminUnameParam, "", "system tenant admin account")
	c.setupSet.StringVar(&c.adminPwd, adminPwdParam, "", "system tenant admin account pwd")

	c.setupSet.StringVar(&c.cfgFile, cfgFileParam, config.CfgFile, "full path to config file")
	c.setupSet.IntVar(&c.port, portParam, -1, "port sts process should listen on")
	c.setupSet.IntVar(&c.shutdownTimeoutSecs, shutdownParam, -1, "gracefull timeout for sts shutdown in seconds")
	c.setupSet.Var(&c.enabledHeads, headsParam, "enabled heads such as oidc,rest. Comma separated.")
	c.setupSet.StringVar(&c.tlscert, tlsCertParam, "", "location of the pem encoded tls cert")
	c.setupSet.StringVar(&c.tlskey, tlsKeyParam, "", "location of the pem encoded tls key")
	c.setupSet.BoolVar(&c.daemon, daemonParam, false, "configure sts process to run as daemon")
	c.setupSet.StringVar(&c.daemonUName, daemonUnameParam, defaultDaemonUname, "user to run the sts process as")
	c.setupSet.StringVar(&c.daemonGroup, daemonGroupParam, defaultDaemonGroup, "group of the user to run sts process as")

	c.setupSet.StringVar(&c.publicEndpoint, publicEndpointParam, "", "publicly accessible sts endpoint")
	c.setupSet.BoolVar(&c.firstInstance, firstInstanceParam, false, "whether first sts instance")
	c.setupSet.StringVar(&c.systemTenant, systemTenantParam, "", "system tenant")
	c.setupSet.StringVar(&c.site, siteParam, "", "site name")
	c.setupSet.Var(&c.ldapUrls, ldapUrlsParam, "comma separated vmdir url(s)")
	c.setupSet.StringVar(&c.ldapCerts, ldapCertsParam, "", "location of the file containing vmdir ssl cert(s)")

	c.setupSet.StringVar(&c.stsAccount, stsAccountParam, "", "stsaccount (if omitted a new account will be provisioned)")
	c.setupSet.StringVar(&c.stsAccountPwd, stsAccountPwdParam, "", "sts account password")
}

func (c *setupCmd) Parse(args []string) error {
	err := c.setupSet.Parse(args)
	if err != nil {
		fmt.Printf("Invalid parameters: %v", err)
		fmt.Println("")
		c.setupSet.Usage()
		return err
	}

	return nil
}

func (c *setupCmd) Process(ctxt diag.RequestContext) error {

	logger := ctxt.Logger()

	if c.help {
		logger.Tracef(diag.SETUP, "'%s' usage requested.", c.Name())
		c.setupSet.Usage()
		return nil
	}

	logger.Infof(diag.SETUP, "Validating '%s' parameters", c.Name())
	err := c.validate(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "'%s' parameters invalid: %v", c.Name(), err)
		fmt.Printf("Invalid parameters: %v", err)
		fmt.Println("")
		c.setupSet.Usage()
		return err
	}

	logger.Infof(diag.SETUP, "Processing ldap schema")
	err = c.ldapSchema(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed processing ldap schema: %v", err)
		fmt.Printf("Unable to register schema: %v", err)
		fmt.Println("")
		return err
	}

	logger.Infof(diag.SETUP, "Setting up sts account")
	err = c.ensureStsAccount(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed setting up sts account: %v", err)
		fmt.Printf("Unable to handle sts account: %v", err)
		fmt.Println("")
		return err
	}

	logger.Infof(diag.SETUP, "Preparing local config file")
	err = c.localConfig(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed preparing local config: %v", err)
		fmt.Printf("Unable to save local sts config: %v", err)
		fmt.Println("")
		return err
	}

	logger.Infof(diag.SETUP, "Configuring sts process")
	err = c.configProcess(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed configuring sts process: %v", err)
		fmt.Printf("Unable to configure sts daemon: %v", err)
		fmt.Println("")
		return err
	}

	logger.Infof(diag.SETUP, "Setting up sts tenant config")
	err = c.stsSystemTenant(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed registering system tenant with sts: %v", err)
		fmt.Printf("Unable to register system tenant with sts: %v", err)
		fmt.Println("")
		return err
	}

	err = c.registerStsWithDns(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "Failed registering Sts service in Dns: %v", err)
		fmt.Printf("Unable to register Sts service in Dns : %v", err)
		fmt.Println("")
		return err
	}

	logger.Infof(diag.SETUP, "Successfully setup STS")
	fmt.Println("Successfully setup STS")
	return nil
}

func (c *setupCmd) VmDNSCli() string {
	return path.Join(c.buildInfo.LwBinDir(), "vmdns-cli")
}

func (c *setupCmd) VmDirCli() string {
	return path.Join(c.buildInfo.LwBinDir(), "dir-cli")
}

func (c *setupCmd) VdcSchemaCli() string {
	return path.Join(c.buildInfo.LwBinDir(), "vdcschema")
}

func (c *setupCmd) CertoolCli() string {
	return path.Join(c.buildInfo.LwBinDir(), "certool")
}

func (c *setupCmd) VmAfdCli() string {
	return path.Join(c.buildInfo.LwBinDir(), "vmafd-cli")
}

func (c *setupCmd) StsExec() string {
	return path.Join(c.buildInfo.StsSbinDir(), stsbinName)
}

// todo: this should be removed once we support srp
func unameDn(uname string) string {
	parts := strings.Split(uname, "@")
	if len(parts) == 2 {
		uname = fmt.Sprintf("cn=%s,cn=Users,%s", parts[0], types.DnFromDomain(parts[1]))
	}
	return uname
}

func (c *setupCmd) ldapObjectExists(
	conn ldap.Connection, objectDn string, filter string, ctxt diag.RequestContext) (bool, error) {

	logger := ctxt.Logger()

	logger.Tracef(diag.SETUP, "checking ldap object existence: dn='%s', filter='%s'", objectDn, filter)

	msg, err := conn.Search(
		objectDn, ldap.ScopeBase, filter, []string{"dn"}, false, 0, 0, ctxt)
	if err != nil {
		if ldap.IsNoSuchObjectError(err) {
			return false, nil
		}
		logger.Errorf(diag.SETUP, "checking ldap object existence: dn='%s', filter='%s' search failed: %v", objectDn, filter, err)
		return false, err
	}
	defer msg.Close()

	len, err := msg.Len(ctxt)
	if err != nil {
		logger.Errorf(diag.SETUP, "checking ldap object existence: dn='%s', filter='%s' getting result length failed: %v", objectDn, filter, err)
		return false, err
	}

	if len > 1 {
		logger.Errorf(diag.SETUP, "checking ldap object existence: dn='%s', filter='%s', number returned results='%v'", objectDn, filter, len)
		return false, fmt.Errorf("Unexpected number of '%s' objects found", objectDn)
	}

	return len == 1, nil
}

const (
	helpParam = "help"

	adminUnameParam = "admin-uname"
	adminPwdParam   = "admin-pwd"

	cfgFileParam     = "cfg-file"
	portParam        = "port"
	shutdownParam    = "shutdown-timeout"
	headsParam       = "enabled-heads"
	daemonParam      = "run-as-daemon"
	daemonUnameParam = "daemon-user"
	daemonGroupParam = "daemon-group"

	tlsCertParam = "tls-cert-file"
	tlsKeyParam  = "tls-key-file"

	publicEndpointParam = "sts-endpoint"
	systemTenantParam   = "system-tenant"
	siteParam           = "site"
	firstInstanceParam  = "primary"

	ldapUrlsParam  = "dir-urls"
	ldapCertsParam = "dir-certs-file"

	stsAccountParam    = "sts-account"
	stsAccountPwdParam = "sts-account-pwd"

	quietParam = "quiet"

	schemaFile = "/configs/schema.ldif"

	//
	daemonTemplate     = "/init/lightwave-stsd.service.tmpl"
	daemonFileName     = "lightwave-stsd.service"
	daemonFileLocation = "/lib/systemd/system"

	stsbinName = "stssrv"

	defaultDaemonUname = "root"
	defaultDaemonGroup = "root"
)
