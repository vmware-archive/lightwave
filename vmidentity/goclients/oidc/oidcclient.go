package oidc

import (
	"crypto/rsa"
	"crypto/x509"
	"time"

	jose "gopkg.in/square/go-jose.v2"
)

// Client represents an oidc client
type Client interface {
	Signers(fetch bool, requestID string) (IssuerSigners, error)
	Issuer() string
	ClientID() string

	AcquireTokensByPassword(username string, password string, scope string, requestID string) (Tokens, error)
	AcquireTokensBySolutionUserCert(certSubjectDn string, privateKey *rsa.PrivateKey, scope string, requestID string) (Tokens, error)
}

// NewOidcClient creates a new oidc client. It requires the issuer endpoint argument and optional arguments clientConfig,
// logger, requestID. If not passed in, default config and no-op logging will be used.
func NewOidcClient(issuer string, config ClientConfig, logger Logger, requestID string) (Client, error) {
	return newClient(config, issuer, requestID, logger)
}

// ParseAndValidateAccessToken parses the given access token as a string, checks the signature, and validates the claims
// The audience parameter and logger may be empty
// It will return a verified access token
func ParseAndValidateAccessToken(
	token string, issuer string, audience string, signers IssuerSigners, logger Logger) (AccessToken, error) {

	return parseToken(token, issuer, audience, "", signers, AccessTokenClass, logger)
}

// ParseAndValidateIDToken parses the given access token as a string, checks the signature, and validates the claims
// Audience and nonce are optional arguments, but will be used for validation if passed in. Logger is optional as well
// It will return a verified ID Token
func ParseAndValidateIDToken(
	token string, issuer string, audience string, nonce string, signers IssuerSigners, logger Logger) (IDToken, error) {

	return parseToken(token, issuer, audience, nonce, signers, IDTokenClass, logger)
}

// ParseAndValidateAccessTokenMulti parses the given access token as a string,
// checks the signature, and validates the claims against each of the elements
// of the providerInfo array.
// Logger is optional.
// It will return a verified access token and index of the ProviderInfo array
// that verified successfully.
func ParseAndValidateAccessTokenMulti(
	token string, audience string, providerInfo []ProviderInfo, logger Logger) (AccessToken, int, error) {

	return parseTokenMulti(token, audience, "", providerInfo, AccessTokenClass, logger)
}

// ParseAndValidateIDTokenMulti parses the given access token as a string,
// checks the signature, and validates the claims against each of the elements
// of the providerInfo array.
// Logger is optional.
// It will return a verified ID token and index of the ProviderInfo array
// that verified successfully.
func ParseAndValidateIDTokenMulti(
	token string, audience string, nonce string, providerInfo []ProviderInfo, logger Logger) (IDToken, int, error) {

	return parseTokenMulti(token, audience, nonce, providerInfo, IDTokenClass, logger)
}

// ParseTenantInToken parses a token string and returns the tenant claim.
// It should only be used when necessary, as this method does not verify the signature in the token. It is recommended to
// use ParseAndValidateToken() first and get the tenant claim.
// ** This api is exposed temporarily, for a short transition period **
func ParseTenantInToken(token string) (string, error) {
	return parseTenantInToken(token)
}

// ParseSignedToken parses a token string and returns an unvalidated JWT.
func ParseSignedToken(token string) (JWT, error) {
	jwt, err := parseSignedToken(token, nil)
	return jwt, err
}

// Tokens represents successful acquire token response
type Tokens interface {
	AccessToken() string
	RefreshToken() string
	IDToken() string
	TokenType() string
	ExpiresIn() int
}

// AccessToken represents a valid access token (usually used by resource server for authn/authz)
type AccessToken interface {
	Issuer() string
	Groups() ([]string, bool)
	Type() string
	Subject() string
	Expiration() time.Time
	IssuedAt() time.Time
	Audience() ([]string, bool)
	Hotk() (*jose.JSONWebKeySet, error)
	Claim(claimName string) (interface{}, bool)
}

// IDToken represents a valid id token usually used by clients for auth
type IDToken interface {
	Issuer() string
	Nonce() (string, bool)
	Type() string
	Subject() string
	Expiration() time.Time
	IssuedAt() time.Time
	IdpSessionID() (string, bool)
	Audience() ([]string, bool)
	Claim(claimName string) (interface{}, bool)
}

// ClientConfig provides configuration for the client
type ClientConfig interface {
	ClientID() string
	Retries() int
	RetryIntervalMillis() int
	HTTPConfig() HTTPClientConfig
	CertRefreshHook() CertRefreshHook
}

// IssuerSigners represents a set of Issuer signing certificates
type IssuerSigners interface {
	Combine(signers ...IssuerSigners) IssuerSigners
}

// CertRefreshHook can be registered with the client to be be called upon any unknown cert errors
// The registered hook should be thread safe as multiple threads can try to refresh certs
type CertRefreshHook func() (*x509.CertPool, error)

// ProviderInfo contains client information needed to validate a token. An array of ProviderInfo
// structs be used in calls to ParseAndValidateAccessTokenMulti() or ParseAndValidateIDTokenMulti()
type ProviderInfo struct {
	Issuer  string
	Signers IssuerSigners
}

// NewProviderInfo creates a new ProviderInfo structure
func NewProviderInfo(issuer string, signers IssuerSigners) (ProviderInfo, error) {
	return newProviderInfo(issuer, signers)
}
