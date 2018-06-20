package main

import (
	"fmt"

	"github.com/vmware/lightwave/vmidentity/goclients/oidc"
)

const (
	issuerUrlFormat = "https://%s/openidconnect/%s"
	usernameFormat  = "%s@%s"

	vmcaOIDCScope = "openid offline_access id_groups at_groups rs_vmca"
)

// GetBearerToken returns the Bearer Token for the given domain
func GetBearerToken(host string, domain string, username string, password string) (string, error) {
	user := fmt.Sprintf(usernameFormat, username, domain)

	oidcClient, err := buildOidcClient(host, domain, disableSSL)
	if err != nil {
		return "", err
	}

	tenantTokenResp, err := oidcClient.AcquireTokensByPassword(user, password, vmcaOIDCScope, "")
	if err != nil {
		return "", err
	}

	return tenantTokenResp.AccessToken(), nil
}

// buildOidcClient is a wrapper to the VMIdentity Go client to build an OIDC client.
func buildOidcClient(host string, domain string, disableSSL bool) (oidc.Client, error) {
	issuer := fmt.Sprintf(issuerUrlFormat, host, domain)

	httpConfig := oidc.NewHTTPClientConfig()
	httpConfig.SkipCertValidationField = disableSSL

	config := oidc.NewClientConfigBuilder().
		WithHTTPConfig(httpConfig).
		Build()

	client, err := oidc.NewOidcClient(issuer, config, nil, "")
	if err != nil {
		return nil, err
	}

	return client, nil
}
