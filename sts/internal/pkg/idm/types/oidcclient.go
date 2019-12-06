package types

import (
	"crypto/rand"
	"encoding/base64"
	"fmt"
	"net/url"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

// idm:"enum:vals=none"
type OidcClientAuthMethod uint8

type OidcClientMeta interface {
	RedirectURIs() []*url.URL
	AuthMethod() OidcClientAuthMethod
	PostLogoutRedirectURIs() []*url.URL
	LogoutURI() *url.URL
	AssertionLifetime() time.Duration
	CrossTenant() bool
}
type OidcClient interface {
	ID() string
	OidcClientMeta
}

type OidcClientBuilder interface {
	ID(id string)
	RedirectURIs(urls []*url.URL)
	AuthMethod(authMethod OidcClientAuthMethod)
	PostLogoutRedirectURIs(uris []*url.URL)
	LogoutURI(uri *url.URL)
	AssertionLifetime(lifetime time.Duration)
	CrossTenant(ct bool)

	Build() (OidcClient, diag.Error)
	BuildOidcClientMeta() (OidcClientMeta, diag.Error)
}

func NewOidcClientBuilder() OidcClientBuilder {
	return &oidcClientBuilderImpl{}
}

// idm:"ldap:oc=lightwaveSTSOidcClient;ccn=OidcClients;t=OidcClient;subt=OidcClientMeta"
// idm:"marshal:enc=j;dec=j;t=OidcClient;subt=OidcClientMeta"
type oidcClientImpl struct {
	// idm:"marshal:name=id;m=ID"
	id string // idm:"ldap:name=cn;u=false;m=ID"

	// idm:"marshal:name=redirect_uris;m=RedirectURIs;subt=OidcClientMeta"
	redirectURIs []*url.URL // idm:"ldap:name=lightwaveSTSRedirectURIs;m=RedirectURIs;subt=OidcClientMeta"

	// idm:"marshal:name=auth_method;m=AuthMethod;subt=OidcClientMeta"
	authMethod OidcClientAuthMethod // idm:"ldap:name=lightwaveSTSOidcClientAuthMethod;m=AuthMethod;subt=OidcClientMeta"

	// idm:"marshal:name=post_logout_redirect_uris;m=PostLogoutRedirectURIs;subt=OidcClientMeta"
	postLogoutRedirectURIs []*url.URL // idm:"ldap:name=lightwaveSTSPostLogoutRedirectURI;m=PostLogoutRedirectURIs;subt=OidcClientMeta"

	// idm:"marshal:name=logout_uri;m=LogoutURI;subt=OidcClientMeta"
	logoutURI *url.URL // idm:"ldap:name=lightwaveSTSLogoutURI;m=LogoutURI;subt=OidcClientMeta"

	// idm:"marshal:name=assertion_lifetime;m=AssertionLifetime;subt=OidcClientMeta"
	assertionLifetime time.Duration // idm:"ldap:name=lightwaveSTSClientAssertionLifetimeMS;m=AssertionLifetime;subt=OidcClientMeta"

	// idm:"marshal:name=cross_tenant;m=CrossTenant;subt=OidcClientMeta"
	crossTenant bool // idm:"ldap:name=lightwaveSTSCrossTenant;m=CrossTenant;subt=OidcClientMeta"
}

func (c *oidcClientImpl) ID() string                         { return c.id }
func (c *oidcClientImpl) RedirectURIs() []*url.URL           { return c.redirectURIs }
func (c *oidcClientImpl) AuthMethod() OidcClientAuthMethod   { return c.authMethod }
func (c *oidcClientImpl) PostLogoutRedirectURIs() []*url.URL { return c.postLogoutRedirectURIs }
func (c *oidcClientImpl) LogoutURI() *url.URL                { return c.logoutURI }
func (c *oidcClientImpl) AssertionLifetime() time.Duration   { return c.assertionLifetime }
func (c *oidcClientImpl) CrossTenant() bool                  { return c.crossTenant }

type oidcClientBuilderImpl oidcClientImpl

func (b *oidcClientBuilderImpl) ID(id string)                               { b.id = id }
func (b *oidcClientBuilderImpl) RedirectURIs(urls []*url.URL)               { b.redirectURIs = urls }
func (b *oidcClientBuilderImpl) AuthMethod(authMethod OidcClientAuthMethod) { b.authMethod = authMethod }
func (b *oidcClientBuilderImpl) PostLogoutRedirectURIs(uris []*url.URL) {
	b.postLogoutRedirectURIs = uris
}
func (b *oidcClientBuilderImpl) LogoutURI(uri *url.URL) { b.logoutURI = uri }
func (b *oidcClientBuilderImpl) AssertionLifetime(lifetime time.Duration) {
	b.assertionLifetime = lifetime
}
func (b *oidcClientBuilderImpl) CrossTenant(ct bool) { b.crossTenant = ct }

func (b *oidcClientBuilderImpl) Build() (OidcClient, diag.Error) {
	// todo: validations
	if len(b.id) <= 0 {
		rid, err := generateRandom()
		if err != nil {
			return nil, err
		}
		b.id = rid
	}

	return (*oidcClientImpl)(b), nil
}

func (b *oidcClientBuilderImpl) BuildOidcClientMeta() (OidcClientMeta, diag.Error) {
	// todo: validations
	return (*oidcClientImpl)(b), nil
}

const randomLength = 32

func generateRandom() (string, diag.Error) {
	b := make([]byte, randomLength)
	_, err := rand.Read(b)
	if err != nil {
		return "", diag.MakeError(
			IdmErrorGeneric, fmt.Sprintf("Failed to generate random: %v", err), err)
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}
