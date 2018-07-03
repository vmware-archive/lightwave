package oidc

import (
	"crypto/tls"
	"crypto/x509"
	"net"
	"net/http"
	"time"
)

const (
	defaultSkipCertValidation     = false
	defaultClockToleranceSecs     = 60
	defaultRetries                = 3
	defaultRetryIntervalMillisecs = 2000

	defaultHTTPClientTLSHandshakeTimeout    = 10
	defaultHTTPClientMaxIdleConns           = 100
	defaultHTTPClientIdleConnTimeout        = 90
	defaultHTTPClientExpectContinueTimeout  = 1
	defaultHTTPClientResponseHeaderTimeout  = 0
	defaultHTTPClientMaxResponseHeaderBytes = 0
	defaultHTTPClientMaxIdleConnsPerHost    = 0
	defaultHTTPClientDisableKeepAlives      = false
	defaultHTTPClientDisableCompression     = false
	defaultHTTPClientDialTimeout            = 30
	defaultHTTPClientDialKeepAlive          = 30
	defaultHTTPClientDialDualStack          = true
)

type clientConfigImpl struct {
	issuer          string
	clientID        string
	retries         int
	intervalMillis  int
	httpConfig      HTTPClientConfig
	certRefreshHook func() (*x509.CertPool, error)
}

// NewDefaultClientConfig creates a new client config given the issuer and sets the default config values for HTTP Client and retries
func NewDefaultClientConfig() ClientConfig {
	return clientConfigImpl{
		clientID:       "",
		retries:        defaultRetries,
		intervalMillis: defaultRetryIntervalMillisecs,
		httpConfig:     NewHTTPClientConfig(),
	}
}

// NewClientConfigBuilder creates a new client config given the issuer and sets the default config values for HTTP Client and retries
func NewClientConfigBuilder() clientConfigImpl {
	return clientConfigImpl{
		clientID:       "",
		retries:        defaultRetries,
		intervalMillis: defaultRetryIntervalMillisecs,
		httpConfig:     NewHTTPClientConfig(),
	}
}

//ClientID is the client id the oidc client will use
func (c clientConfigImpl) ClientID() string {
	return c.clientID
}

//Retries returns the number of retries when getting metadata
func (c clientConfigImpl) Retries() int {
	return c.retries
}

//RetryIntervalMillis returns the interval between retries
func (c clientConfigImpl) RetryIntervalMillis() int {
	return c.intervalMillis
}

//HTTPConfig returns the HTTP client configuration
func (c clientConfigImpl) HTTPConfig() HTTPClientConfig {
	return c.httpConfig
}

//CertRefreshHook returns the hook to handle root cert refresh
func (c clientConfigImpl) CertRefreshHook() CertRefreshHook {
	return c.certRefreshHook
}

//WithClientID sets the clientID and returns the config
func (c clientConfigImpl) WithClientID(clientID string) clientConfigImpl {
	c.clientID = clientID
	return c
}

//WithRetries sets the number of retries when using HTTP calls and returns the config
func (c clientConfigImpl) WithRetries(retries int) clientConfigImpl {
	c.retries = retries
	return c
}

//WithRetryInterval sets the interval between retries and returns the config
func (c clientConfigImpl) WithRetryInterval(interval int) clientConfigImpl {
	c.intervalMillis = interval
	return c
}

//WithHTTPConfig sets the HTTP client configuration and returns the config
func (c clientConfigImpl) WithHTTPConfig(config HTTPClientConfig) clientConfigImpl {
	c.httpConfig = config
	return c
}

//WithCertRefreshHook sets the hook to refresh root certs on unknown cert error
func (c clientConfigImpl) WithCertRefreshHook(hook CertRefreshHook) clientConfigImpl {
	c.certRefreshHook = hook
	return c
}

//Build returns the ClientConfig object
func (c clientConfigImpl) Build() ClientConfig {
	return c
}

// HTTPClientConfig contains configurable values for the HTTP Transport OIDC client uses
type HTTPClientConfig struct {
	TLSHandshakeTimeout     time.Duration
	DisableKeepAlives       bool
	DisableCompression      bool
	MaxIdleConns            int
	MaxIdleConnsPerHost     int
	IdleConnTimeout         time.Duration
	ResponseHeaderTimeout   time.Duration
	ExpectContinueTimeout   time.Duration
	MaxResponseHeaderBytes  int64
	DialTimeout             time.Duration
	DialKeepAlive           time.Duration
	DialDualStack           bool
	SSLCertField            *x509.CertPool // null default system roots (golang default behavior)
	SkipCertValidationField bool
}

// NewHTTPClientConfig returns an HTTPClientConfig struct with default settings
func NewHTTPClientConfig() HTTPClientConfig {
	return HTTPClientConfig{
		TLSHandshakeTimeout:     defaultHTTPClientTLSHandshakeTimeout * time.Second,
		DisableKeepAlives:       defaultHTTPClientDisableKeepAlives,
		DisableCompression:      defaultHTTPClientDisableCompression,
		MaxIdleConns:            defaultHTTPClientMaxIdleConns,
		MaxIdleConnsPerHost:     defaultHTTPClientMaxIdleConnsPerHost,
		IdleConnTimeout:         defaultHTTPClientIdleConnTimeout * time.Second,
		ResponseHeaderTimeout:   defaultHTTPClientResponseHeaderTimeout * time.Second,
		ExpectContinueTimeout:   defaultHTTPClientExpectContinueTimeout * time.Second,
		MaxResponseHeaderBytes:  defaultHTTPClientMaxResponseHeaderBytes,
		DialTimeout:             defaultHTTPClientDialTimeout * time.Second,
		DialKeepAlive:           defaultHTTPClientDialKeepAlive * time.Second,
		DialDualStack:           defaultHTTPClientDialDualStack,
		SSLCertField:            nil,
		SkipCertValidationField: defaultSkipCertValidation,
	}
}

func httpTransportFromConfig(httpConfig HTTPClientConfig) *http.Transport {
	tlsConf := &tls.Config{
		// Only use curves which have assembly implementations
		// https://github.com/golang/go/tree/master/src/crypto/elliptic
		CurvePreferences: []tls.CurveID{tls.CurveP256},
		// Use modern tls mode https://wiki.mozilla.org/Security/Server_Side_TLS#Modern_compatibility
		NextProtos: []string{"http/1.1", "h2"},
		// https://www.owasp.org/index.php/Transport_Layer_Protection_Cheat_Sheet#Rule_-_Only_Support_Strong_Protocols
		MinVersion: tls.VersionTLS12,
		CipherSuites: []uint16{
			// These ciphersuites support Forward Secrecy: https://en.wikipedia.org/wiki/Forward_secrecy
			tls.TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
			tls.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
			tls.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
			tls.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
			// these are actually enabled on sts
			tls.TLS_RSA_WITH_AES_256_CBC_SHA,
			tls.TLS_RSA_WITH_AES_128_CBC_SHA,
		},
		RootCAs:            httpConfig.SSLCertField,
		InsecureSkipVerify: httpConfig.SkipCertValidationField == true,
	}

	tr := &http.Transport{
		DialContext: (&net.Dialer{
			Timeout:   httpConfig.DialTimeout,
			KeepAlive: httpConfig.DialKeepAlive,
			DualStack: httpConfig.DialDualStack,
		}).DialContext,
		TLSHandshakeTimeout:    httpConfig.TLSHandshakeTimeout,
		DisableKeepAlives:      httpConfig.DisableKeepAlives,
		DisableCompression:     httpConfig.DisableCompression,
		MaxIdleConns:           httpConfig.MaxIdleConns,
		MaxIdleConnsPerHost:    httpConfig.MaxIdleConnsPerHost,
		IdleConnTimeout:        httpConfig.IdleConnTimeout,
		ResponseHeaderTimeout:  httpConfig.ResponseHeaderTimeout,
		ExpectContinueTimeout:  httpConfig.ExpectContinueTimeout,
		MaxResponseHeaderBytes: httpConfig.MaxResponseHeaderBytes,

		TLSClientConfig: tlsConf,
	}

	return tr
}
