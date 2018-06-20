package main

import (
	"encoding/json"
	"io/ioutil"
)

const (
	VmcaHTTPSPort = "7778"

	SignedCertEndpoint = "/v1/vmca/certificates"
	RootCertEndpoint   = "/v1/vmca/root"

	RsaPrivateKeyPemType      = "RSA PRIVATE KEY"
	CertificateRequestPemType = "CERTIFICATE REQUEST"
)

var (
	// Default hostname
	host = "localhost"

	// Used to get/save private key and certificate if provided
	privKeyPath = ""
	certPath    = ""

	// Default key size used to generate private key
	keySize = 2048

	// Used to skip certificate validation
	disableSSL = false

	// Variable used to store config
	conf *Config
)

// Structure of config json file
type Config struct {
	CertDuration string   `json:"cert_duration"`
	CommonName   string   `json:"common_name"`
	Country      []string `json:"country"`
	State        []string `json:"state"`
	Locality     []string `json:"locality"`
	Organization []string `json:"organization"`
	OrgUnit      []string `json:"org_unit"`
	AltDomains   []string `json:"alt_domains"`
}

// Structure of certificate received from server
type Certificate struct {
	Cert string `json:"cert"`
}

func getConfig(filename string) (*Config, error) {
	raw, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	var c Config
	err = json.Unmarshal(raw, &c)
	if err != nil {
		return nil, err
	}

	return &c, nil
}
