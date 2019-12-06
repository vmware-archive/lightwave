package config

import (
	"crypto"
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"encoding/base64"
	"encoding/pem"
	"fmt"
	"net/url"
	"os"
	"strings"
	"sync/atomic"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	yaml "gopkg.in/yaml.v2"
)

type configImpl struct {
	cert         *x509.Certificate
	pk           crypto.PrivateKey
	heads        []HttpHead
	port         int
	shutdownWait time.Duration

	site      string
	tenant    diag.TenantID
	addresses []*url.URL
	updCfg    *atomic.Value

	authType   types.AuthType
	stsaccount string
	pwds       []string

	configLocation string
}

type updateableConfig struct {
	ldapcerts []*x509.Certificate
	certPool  *x509.CertPool
}

type configMarshal struct {
	RawKey              string   `yaml:"key"`
	RawTLSCert          string   `yaml:"tls_cert"`
	RawTLSPK            string   `yaml:"tls_pk"`
	RawShutdownWaitSecs int32    `yaml:"shutdown_wait"`
	RawPort             int32    `yaml:"port"`
	RawHeads            []string `yaml:"heads"`

	RawSite         string   `yaml:"site"`
	RawSystemTenant string   `yaml:"system_tenant"`
	RawVmdir        []string `yaml:"vmdir"`
	RawVmdirCerts   []string `yaml:"vmdir_certs"`
	RawAuthType     string   `yaml:"auth_type"`
	RawStsUname     string   `yaml:"sts_account"`
	RawStsPwd       []string `yaml:"sts_cred"`
	//RawAccountSelfManaged bool     `yaml:"sts_acct_self_managed,omitempty"`
}

func (c *configImpl) ShutdownWait() time.Duration { return c.shutdownWait }

func (c *configImpl) EnabledHttpHeads() []HttpHead { return c.heads }

func (c *configImpl) Port() int { return int(c.port) }

func (c *configImpl) TLSCert() *x509.Certificate { return c.cert }

func (c *configImpl) TLSPrivateKey() crypto.PrivateKey { return c.pk }

func (c *configImpl) Site() string { return c.site }

func (c *configImpl) SystemTenant() diag.TenantID { return c.tenant }

func (c *configImpl) Addresses() []*url.URL { return c.addresses }

func (c *configImpl) Certificates() []*x509.Certificate {
	var updateCfg *updateableConfig
	updateCfg = c.updCfg.Load().(*updateableConfig)
	return updateCfg.ldapcerts
}

func (c *configImpl) AuthType() types.AuthType { return c.authType }
func (c *configImpl) UserName() string         { return c.stsaccount }
func (c *configImpl) Pwd() []string            { return c.pwds }

func (c *configImpl) TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error) {
	l := ctxt.Logger()

	if refresh {
		l.Tracef(diag.CONFIG, "Refreshing ldap trsueted certs from '%v'", c.configLocation)

		if len(c.configLocation) > 0 {

			cm, err := unmarshalConfig(c.configLocation)
			if err != nil {
				l.Errorf(diag.CONFIG, "Unable to unmarshal config from '%v': %v", c.configLocation, err)
				return nil, err
			}

			updConf := &updateableConfig{}

			updConf.ldapcerts, updConf.certPool, err = cm.ldapCerts()
			if err != nil {
				l.Errorf(diag.CONFIG, "Unable to get vmdir certs from config: %v", err)
				return nil, err
			}
			c.updCfg.Store(updConf)
			return updConf.certPool, nil
		}
	}
	var updateCfg *updateableConfig
	updateCfg = c.updCfg.Load().(*updateableConfig)
	return updateCfg.certPool, nil
}

type confingImplBuilder configImpl

func (c *confingImplBuilder) ShutdownWait(wait time.Duration)   { c.shutdownWait = wait }
func (c *confingImplBuilder) EnabledHttpHeads(heads []HttpHead) { c.heads = heads }

func (c *confingImplBuilder) Port(p int) { c.port = p }

func (c *confingImplBuilder) TLSCert(cert *x509.Certificate)     { c.cert = cert }
func (c *confingImplBuilder) TLSPrivateKey(pk crypto.PrivateKey) { c.pk = pk }

func (c *confingImplBuilder) Site(site string) { c.site = site }

// config store
func (c *confingImplBuilder) SystemTenant(t diag.TenantID) { c.tenant = t }
func (c *confingImplBuilder) Addresses(vmdirs []*url.URL)  { c.addresses = vmdirs }
func (c *confingImplBuilder) Certificates(vmdircerts []*x509.Certificate) {
	upd := &updateableConfig{}
	upd.ldapcerts = vmdircerts
	if len(vmdircerts) > 0 {
		if len(upd.ldapcerts) > 0 {
			upd.certPool = x509.NewCertPool()
			for _, cert := range upd.ldapcerts {
				upd.certPool.AddCert(cert)
			}
		}
	}
	c.updCfg.Store(upd)
}

func (c *confingImplBuilder) AuthType(t types.AuthType) { c.authType = t }
func (c *confingImplBuilder) UserName(uname string)     { c.stsaccount = uname }
func (c *confingImplBuilder) Pwd(pwds []string)         { c.pwds = pwds }

func (c *confingImplBuilder) Build() (InstanceConfig, diag.Error) {
	// todo: validations
	return (*configImpl)(c), nil
}

const randomLength = 32

func generateRandom() (string, diag.Error) {
	b := make([]byte, randomLength)
	_, err := rand.Read(b)
	if err != nil {
		return "", diag.MakeError(
			RandomGenerationError,
			fmt.Sprintf("Failed to generate random: %v", err), err)
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}

func unmarshalConfig(cfgLocation string) (*configMarshal, diag.Error) {
	if len(cfgLocation) <= 0 {
		cfgLocation = CfgFile
	}
	file, err := os.Open(cfgLocation)
	if err != nil {
		return nil, diag.MakeError(
			ConfigFileOpenError,
			fmt.Sprintf("Failed opening config file '%s' with error '%v'", cfgLocation, err),
			err)
	}
	defer file.Close()

	d := yaml.NewDecoder(file)

	cm := &configMarshal{}

	err = d.Decode(cm)
	if err != nil {
		return nil, diag.MakeError(
			YamlDecodingError,
			fmt.Sprintf("Failed reading config file with error '%v'", err),
			err)
	}

	return cm, nil
}

func (c *configMarshal) marshalConfig(cfgLocation string) diag.Error {
	cfgFile, err := os.Create(cfgLocation)
	if err != nil {
		return diag.MakeError(
			ConfigFileCreateError,
			fmt.Sprintf("Failed creating config file '%s' with error '%v'", cfgLocation, err),
			err)
	}
	defer cfgFile.Close()

	d := yaml.NewEncoder(cfgFile)
	err = d.Encode(c)
	if err != nil {
		return diag.MakeError(
			YamlEncodingError,
			fmt.Sprintf("Failed writing config file '%s' with error '%v'", cfgLocation, err),
			err)
	}

	return nil
}

func (c *configMarshal) encrypt(cleartext string) (string, diag.Error) {
	if len(cleartext) <= 0 {
		return "", nil
	}

	binKey, err := base64.RawURLEncoding.DecodeString(c.RawKey)
	if err != nil {
		return "", diag.MakeError(
			Base64DecodeError, fmt.Sprintf("Failed to base64 decode key: %v", err), err)
	}

	block, err := aes.NewCipher(binKey)
	if err != nil {
		return "", diag.MakeError(
			NewCipherError, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return "", diag.MakeError(
			NewGCMError, fmt.Sprintf("Failed to create a new GCM: %v", err), err)
	}

	nonce := make([]byte, aesgcm.NonceSize())
	_, err = rand.Read(nonce)
	if err != nil {
		return "", diag.MakeError(
			RandomNonceError, fmt.Sprintf("Failed to create random nonce: %v", err), err)
	}

	ciphertext := aesgcm.Seal(nonce, nonce, []byte(cleartext), nil)

	return base64.RawURLEncoding.EncodeToString(ciphertext), nil
}
func (c *configMarshal) decrypt(encrypted string) (string, diag.Error) {
	if len(encrypted) <= 0 {
		return "", nil
	}

	val, err := base64.RawURLEncoding.DecodeString(encrypted)
	if err != nil {
		return "", diag.MakeError(
			Base64DecodeError, fmt.Sprintf("Failed to base64 decode encrypted: %v", err), err)
	}

	binKey, err := base64.RawURLEncoding.DecodeString(c.RawKey)
	if err != nil {
		return "", diag.MakeError(
			Base64DecodeError, fmt.Sprintf("Failed to base64 decode key: %v", err), err)
	}
	block, err := aes.NewCipher(binKey)
	if err != nil {
		return "", diag.MakeError(
			NewCipherError, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return "", diag.MakeError(
			NewGCMError, fmt.Sprintf("Failed to new gcm: %v", err), err)
	}

	nonce := val[:aesgcm.NonceSize()]
	val = val[aesgcm.NonceSize():]

	cleartext, err := aesgcm.Open(nil, nonce, val, nil)
	if err != nil {
		return "", diag.MakeError(
			GCMOpenError, fmt.Sprintf("Failed to gcm open: %v", err), err)
	}

	return string(cleartext), nil
}

func (c *configMarshal) certAndPK() (*x509.Certificate, crypto.PrivateKey, diag.Error) {
	certificate, err := tls.X509KeyPair([]byte(c.RawTLSCert), []byte(c.RawTLSPK))
	if err != nil {
		return nil, nil, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading tls cert/key from config with error '%v'", err),
			err)
	}

	cert, err := x509.ParseCertificate(certificate.Certificate[0])
	if err != nil {
		return nil, nil, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading tls cert from config with error '%v'", err),
			err)
	}

	return cert, certificate.PrivateKey, nil
}

func (c *configMarshal) authType() (types.AuthType, diag.Error) {
	authType := types.AuthTypeNone
	err := authType.From(c.RawAuthType)
	if err != nil {
		return types.AuthTypeNone, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading authType from config with error '%v'", err),
			err)
	}
	return authType, nil
}

func (c *configMarshal) site() (string, diag.Error) {

	site := strings.TrimSpace(c.RawSite)
	if len(site) <= 0 {
		return "", diag.MakeError(
			ConfigPropertyError, "Missing site from config", nil)
	}

	return site, nil
}

func (c *configMarshal) tenant() (diag.TenantID, diag.Error) {

	tenant := diag.NoneTenantID
	err := tenant.From(c.RawSystemTenant)
	if err != nil {
		return diag.NoneTenantID, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading system tenant from config with error '%v'", err),
			err)
	}

	return tenant, nil
}

func (c *configMarshal) vmdirAddresses() ([]*url.URL, diag.Error) {
	addresses := make([]*url.URL, 0, len(c.RawVmdir))
	for _, a := range c.RawVmdir {
		url, err := url.Parse(a)
		if err != nil {
			return nil, diag.MakeError(
				ConfigPropertyError,
				fmt.Sprintf("Failed reading vmdir from config with error '%v'", err),
				err)
		}
		addresses = append(addresses, url)
	}

	return addresses, nil
}

func (c *configMarshal) heads() ([]HttpHead, diag.Error) {
	heads := make([]HttpHead, 0, len(c.RawHeads))
	for _, h := range c.RawHeads {
		hh := NoneHead
		err := hh.From(h)
		if err != nil {
			return []HttpHead{}, diag.MakeError(
				ConfigPropertyError,
				fmt.Sprintf("Failed reading enabled heads from config: '%v'", err),
				err)
		}
		heads = append(heads, hh)
	}
	return heads, nil
}

func (c *configMarshal) hasStsUname() bool {
	return len(c.RawStsUname) > 0
}
func (c *configMarshal) hasStsPwd() bool {
	return len(c.RawStsPwd) > 0
}

func (c *configMarshal) stsAccount() (string, []string, diag.Error) {

	if c.hasStsUname() && !c.hasStsPwd() ||
		!c.hasStsUname() && c.hasStsPwd() {
		return "", []string{}, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading sts account info from config"),
			nil)
	}
	if !c.hasStsUname() {
		return "", []string{}, nil
	}
	pwds := make([]string, 0, len(c.RawStsPwd))
	for _, p := range c.RawStsPwd {
		pwd, err := c.decrypt(p)
		if err != nil {
			return "", []string{}, err
		}
		pwds = append(pwds, pwd)
	}

	return c.RawStsUname, pwds, nil
}

func (c *configMarshal) ldapCerts() ([]*x509.Certificate, *x509.CertPool, diag.Error) {
	ldapcerts := make([]*x509.Certificate, 0, len(c.RawVmdirCerts))
	var certPool *x509.CertPool
	for _, crt := range c.RawVmdirCerts {
		var certDERBlock *pem.Block
		certDERBlock, _ = pem.Decode([]byte(crt))
		if certDERBlock == nil {
			continue
		}
		if certDERBlock.Type == "CERTIFICATE" {
			x509cert, err1 := x509.ParseCertificate(certDERBlock.Bytes)
			if err1 != nil {
				return nil, nil, diag.MakeError(
					ConfigPropertyError,
					fmt.Sprintf("Failed reading vmdir certs from config with error '%v'", err1),
					err1)
			}
			ldapcerts = append(ldapcerts, x509cert)
		} else {
			return nil, nil, diag.MakeError(
				ConfigPropertyError,
				fmt.Sprintf("Failed reading vmdir certs from config with unexpected type '%s'", certDERBlock.Type),
				nil)
		}
	}
	if len(ldapcerts) > 0 {
		certPool = x509.NewCertPool()
		for _, cert := range ldapcerts {
			certPool.AddCert(cert)
		}
	}

	return ldapcerts, certPool, nil
}

func (c *configMarshal) port() (int, diag.Error) {
	if c.RawPort <= 0 {
		return 0, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading port info from config: invalid port '%v'", c.RawPort),
			nil)
	}

	return int(c.RawPort), nil
}

func (c *configMarshal) shutdownWait() (time.Duration, diag.Error) {
	if c.RawShutdownWaitSecs <= 0 {
		return 0, diag.MakeError(
			ConfigPropertyError,
			fmt.Sprintf("Failed reading shutdown wait from config: invalid '%v'", c.RawShutdownWaitSecs),
			nil)
	}
	return time.Second * time.Duration(c.RawShutdownWaitSecs), nil
}
