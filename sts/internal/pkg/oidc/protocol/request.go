package protocol

import (
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type UriValidatorFunc func(ci ClientInfo, ctxt diag.RequestContext) (*url.URL, diag.Error)
type TokenValidatorFunc func(token string, ctxt diag.RequestContext) (auth.Token, diag.Error)

// ClientInfo - rfc6749: Authorization Request
// https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest
type ClientInfo interface {
	// ClientID - rfc6749: REQUIRED. The client identifier as described in Section 2.2.
	ClientID() string
	// RedirectURI - rfc6749: OPTIONAL (oauth), REQUIRED(oidc)
	RedirectURI() *url.URL
}

// OidcRequest common parameters for oidc requests
type OidcRequest interface {
	ClientInfo

	// Scope - OPTIONAL. The scope of the access request as described by Section 3.3.
	// REQUIRED(oidc) - openid scope
	Scope() ScopeSet
}

// AuthzRequest - represents request to authorization endpoint
type AuthzRequest interface {
	OidcRequest

	// Nonce - https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest:
	// 	OPTIONAL. String value used to associate a Client session with an ID Token, and to mitigate replay attacks.
	Nonce() string

	// ResponseTypes oauth&oidc - REQUIRED. Determines the flow and response content.
	ResponseTypes() ResponseTypeSet

	// State - rfc6749: RECOMMENDED. An opaque value used by the client to maintain
	//    state between the request and callback.
	State() string

	// ResponseMode - : how response is to be delivered: query, fragment, form_post
	ResponseMode() ResponseMode

	// IDTokenHint - https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest
	IDTokenHint() string
	// Prompt - https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest
	Prompt() PromptSet

	Grant() Grant // optional grant
	LoginFormPost() bool

	// OauthRequest defines if this is an oauth request vs oidc
	OauthRequest() bool
}

// Token request - represents request to token endpoint
type TokenRequest interface {
	OidcRequest
	// Grant - rfc6749: REQUIRED. Password, client creds etc.
	Grant() Grant
}

// https://openid.net/specs/openid-connect-frontchannel-1_0.html
type LogoutRequest interface {
	// RECOMMENDED. Previously issued ID Token passed to the logout endpoint as a hint about the End-User's current authenticated session with the Client.
	TokenHint() auth.Token
	// OPTIONAL. URL to which the RP is requesting that the End-User's User Agent be redirected after a logout has been performed.
	RedirectURI() *url.URL
	// OPTIONAL. Opaque value used by the RP to maintain state between the logout request and the callback to the endpoint specified by the post_logout_redirect_uri query parameter.
	State() string

	ClientID() string
}
