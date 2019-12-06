package config

import (
	"crypto"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"net/url"
	"strings"
	"sync/atomic"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type HttpHead string

const (
	OIDCHead HttpHead = "oidc"
	RESTHead HttpHead = "rest"
	NoneHead HttpHead = ""
)

func (e HttpHead) String() string {
	return string(e)
}

func (e *HttpHead) From(s string) diag.Error {
	if e == nil {
		return diag.MakeError(
			InvalidArgumentError, "Unable to unmarshal 'HttpHead' into nil", nil)
	}

	s = strings.TrimSpace(s)
	switch s {
	case string(OIDCHead):
		*e = OIDCHead
	case string(RESTHead):
		*e = RESTHead
	case string(NoneHead):
		*e = NoneHead
	default:
		return diag.MakeError(
			UnsupportedHTTPHeadError,
			fmt.Sprintf("Unexpected value of HttpHead: '%s'", s), nil)
	}
	return nil
}

const (
	CfgFile string = "/opt/lightwave/config/stssrv.conf"
)

type InstanceConfig interface {
	ShutdownWait() time.Duration
	EnabledHttpHeads() []HttpHead

	Port() int

	TLSCert() *x509.Certificate
	TLSPrivateKey() crypto.PrivateKey

	// config store
	Site() string
	SystemTenant() diag.TenantID
	Addresses() []*url.URL
	Certificates() []*x509.Certificate // ldaps
	TrustedCerts(refresh bool, ctxt diag.RequestContext) (*x509.CertPool, diag.Error)
	AuthType() types.AuthType
	UserName() string
	Pwd() []string
}

type InstanceConfigBuilder interface {
	ShutdownWait(wait time.Duration)
	EnabledHttpHeads(heads []HttpHead)

	Port(p int)

	TLSCert(cert *x509.Certificate)
	TLSPrivateKey(pk crypto.PrivateKey)

	// config store
	Site(site string)
	SystemTenant(t diag.TenantID)
	Addresses(vmdirs []*url.URL)
	Certificates(vmdircerts []*x509.Certificate) // ldaps
	AuthType(t types.AuthType)
	UserName(string)
	Pwd(pwds []string)

	Build() (InstanceConfig, diag.Error)
}

func NewInstanceConfigBuilder() InstanceConfigBuilder {
	upd := &atomic.Value{}
	upd.Store(&updateableConfig{})
	return &confingImplBuilder{
		heads:        []HttpHead{OIDCHead, RESTHead},
		port:         443,
		shutdownWait: time.Duration(10) * time.Second,
		authType:     types.AuthTypeSimple, // todo: switch when srp is there
		updCfg:       upd,
	}
}

func ReadConfig(cfgLocation string, l diag.Logger) (InstanceConfig, diag.Error) {

	// todo: we should probably use a config builder here
	l.Tracef(diag.CONFIG, "Unmarshalling InstanceConfig from '%v'", cfgLocation)
	cm, err := unmarshalConfig(cfgLocation)
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to unmarshal config from '%v': %v", cfgLocation, err)
		return nil, err
	}

	c := &configImpl{updCfg: &atomic.Value{}}
	updConf := &updateableConfig{}

	c.cert, c.pk, err = cm.certAndPK()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get TLS cert and private key from config: %v", err)
		return nil, err
	}

	c.site, err = cm.site()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get site from config: %v", err)
		return nil, err
	}

	c.tenant, err = cm.tenant()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get system tenant from config: %v", err)
		return nil, err
	}

	c.heads, err = cm.heads()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get enabled heads from config: %v", err)
		return nil, err
	}

	c.addresses, err = cm.vmdirAddresses()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get vmdir addersses from config: %v", err)
		return nil, err
	}

	updConf.ldapcerts, updConf.certPool, err = cm.ldapCerts()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get vmdir certs from config: %v", err)
		return nil, err
	}
	c.updCfg.Store(updConf)

	c.authType, err = cm.authType()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get AuthType from config: %v", err)
		return nil, err
	}

	c.stsaccount, c.pwds, err = cm.stsAccount()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get sts account info from config: %v", err)
		return nil, err
	}

	c.port, err = cm.port()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get port info from config: %v", err)
		return nil, err
	}

	c.shutdownWait, err = cm.shutdownWait()
	if err != nil {
		l.Errorf(diag.CONFIG, "Unable to get shutdown wait info from config: %v", err)
		return nil, err
	}

	c.configLocation = cfgLocation

	l.Tracef(diag.CONFIG, "Successfully unmarshalled InstanceConfig from '%v'", c.configLocation)

	return c, nil
}

func SaveConfig(cfg InstanceConfig, file string, l diag.Logger) diag.Error {

	l.Tracef(diag.CONFIG, "Saving instance config to '%v'", file)

	rstr, err := generateRandom()
	if err != nil {
		l.Errorf(diag.CONFIG, "Failed generating random: %v", err)
		return err
	}
	cm := &configMarshal{
		RawKey: rstr,
	}

	certBlock := &pem.Block{
		Type:  "CERTIFICATE",
		Bytes: cfg.TLSCert().Raw,
	}

	cm.RawTLSCert = string(pem.EncodeToMemory(certBlock))
	pkBlock := &pem.Block{
		Type: "PRIVATE KEY",
	}
	var err1 error
	pkBlock.Bytes, err1 = x509.MarshalPKCS8PrivateKey(cfg.TLSPrivateKey())
	if err1 != nil {
		l.Errorf(diag.CONFIG, "Failed writing tls key to config with error '%v'", err1)
		return diag.MakeError(
			PrivateKeyMarshallError,
			fmt.Sprintf("Failed writing tls key to config with error '%v'", err1),
			err1)
	}
	cm.RawTLSPK = string(pem.EncodeToMemory(pkBlock))

	cm.RawAuthType = cfg.AuthType().String()
	cm.RawSystemTenant = cfg.SystemTenant().String()
	cm.RawSite = cfg.Site()

	cm.RawVmdir = make([]string, 0, len(cfg.Addresses()))
	for _, u := range cfg.Addresses() {
		cm.RawVmdir = append(cm.RawVmdir, u.String())
	}

	cm.RawHeads = make([]string, 0, len(cfg.EnabledHttpHeads()))
	for _, h := range cfg.EnabledHttpHeads() {
		cm.RawHeads = append(cm.RawHeads, string(h))
	}

	cm.RawVmdirCerts = make([]string, 0, len(cfg.Certificates()))
	for _, crt := range cfg.Certificates() {

		certBlock = &pem.Block{
			Type:  "CERTIFICATE",
			Bytes: crt.Raw,
		}

		cm.RawVmdirCerts = append(cm.RawVmdirCerts, string(pem.EncodeToMemory(certBlock)))
	}

	cm.RawStsPwd = make([]string, 0, len(cfg.Pwd()))
	for _, p := range cfg.Pwd() {
		pwd, err := cm.encrypt(p)
		if err != nil {
			return err
		}
		cm.RawStsPwd = append(cm.RawStsPwd, pwd)
	}

	cm.RawPort = int32(cfg.Port())

	cm.RawShutdownWaitSecs = int32(cfg.ShutdownWait() / time.Second)
	cm.RawStsUname = cfg.UserName()

	err = cm.marshalConfig(file)
	if err != nil {
		l.Errorf(diag.CONFIG, "Failed marshalling config to '%v' with error: %v", err)
		return err
	}

	l.Tracef(diag.CONFIG, "Successfully saved instance config to '%v'", file)

	return nil
}
