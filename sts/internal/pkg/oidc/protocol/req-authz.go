package protocol

import (
	"fmt"
	"net/http"
	"net/url"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

// ParseAuthzRequest attempts to construct a auth request from http request
func ParseAuthzRequest(
	req *http.Request, uriValidator UriValidatorFunc, ctxt diag.RequestContext) (AuthzRequest, diag.Error) {
	log := ctxt.Logger()
	errURL := req.ParseForm()
	var err diag.Error

	if errURL != nil {
		log.Errorf(diag.OIDC, "Failed to parse request url: %v", errURL)
		return nil, diag.MakeError(OidcErrorInvalidRequest, "Invalid query parameter", errURL)
	}
	vals := req.Form

	authz := &authzRequest{cliInfo: &clientInfoImpl{}}
	err = authz.cliInfo.UnMarshalQuery(vals)
	if err != nil {
		return nil, err
	}

	normalizedURI, uriErr := uriValidator(authz.cliInfo, ctxt)
	if err != nil {
		authz.cliInfo = nil
		return nil, uriErr
	}
	if authz.cliInfo.redirectURI == nil {
		authz.cliInfo.redirectURI = normalizedURI
	}

	err = authz.UnMarshalQuery(vals)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed to parse client info: %v", err)
	}

	// todo: possibly more complicated logic for AJAX
	authz.loginFormPost = strings.EqualFold(req.Method, "POST")

	if err == nil && authz.loginFormPost {
		// parse optional username info grant
		switch authz.grantType {
		case GrantTypePassword:
			{
				authz.grant = new(pwdGrantImpl)
				err1 := authz.grant.UnMarshalQuery(req.PostForm)
				if err1 != nil {
					log.Errorf(diag.OIDC, "Failed to parse password grant: '%v'", err1)
				}
				if err1 != nil && err == nil {
					err = err1
				}
			}
		default:
			{
				if err == nil {
					err = diag.MakeError(OidcErrorUnsupportedGrantType,
						fmt.Sprintf("grant_type '%s' is unsupported for login form", authz.grantType.String()), nil)
				}
			}
		}
	}

	// if there is no error so far, go onto validation logic
	if err == nil {
		err = authz.validate(ctxt)
	}

	if err == nil {
		if !authz.responseMode.Known() {
			authz.responseMode = getDefaultResponseMode(authz.responseTypes)
		}
	}

	return authz, err
}

func (ar *authzRequest) validate(ctxt diag.RequestContext) diag.Error {

	if ar.RedirectURI() == nil {
		return diag.MakeError(
			OidcErrorInvalidRequest, "redirect_uri is required", nil)
	}

	if ar.responseTypes.Len() == 0 {
		return diag.MakeError(OidcErrorInvalidRequest, "response_type is required", nil)
	}

	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#none
	//    The Response Type none SHOULD NOT be combined with other Response Types.
	if ar.ResponseTypes().Contains(ResponseTypeNone) && ar.ResponseTypes().Len() > 1 {
		return diag.MakeError(OidcErrorInvalidRequest,
			"'none' response_type cannot be used in combination with other response types", nil)
	}

	// https://openid.net/specs/openid-connect-core-1_0.html#ImplicitAuthRequest
	//     NOTE: While OAuth 2.0 also defines the token Response Type value for the Implicit Flow,
	//     OpenID Connect does not use this Response Type, since no ID Token would be returned.
	if !ar.OauthRequest() && ar.ResponseTypes().Contains(ResponseTypeToken) && ar.ResponseTypes().Len() == 1 {
		return diag.MakeError(OidcErrorInvalidRequest,
			"'token' response_type is not valid for OpenID Connect", nil)
	}

	// https://openid.net/specs/openid-connect-core-1_0.html#ImplicitAuthRequest
	//    nonce REQUIRED. String value used to associate a Client session with an ID Token, and to mitigate replay attacks.
	// todo: spec does not state so, but it would seem hybrid flow with id_token returned should requirte nonce as well...
	if !ar.OauthRequest() && ar.ResponseTypes().ImplicitFlow() {
		if len(ar.Nonce()) <= 0 {
			return diag.MakeError(
				OidcErrorInvalidRequest, "nonce is required", nil)
		}
	}

	// query can only be used for code or none response types
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#id_token
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#none
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#Combinations

	if (ar.responseMode == ResponseModeQuery) && (ar.responseTypes.ImplicitFlow() || ar.responseTypes.HybridFlow()) {
		err := diag.MakeError(
			OidcErrorInvalidRequest,
			fmt.Sprintf(
				"'%s' response mode cannot be used with specified responseType '%s'",
				diag.SafeString(ar.responseMode),
				diag.SafeString(ar.responseTypes)),
			nil)
		ctxt.Logger().Errorf(diag.OIDC, err.Error())
		return err
	}
	return nil
}

func getDefaultResponseMode(respTypes ResponseTypeSet) ResponseMode {
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#id_token
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#none
	// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#Combinations
	if respTypes.NoneResponse() || respTypes.AuthzFlow() {
		return ResponseModeQuery
	}
	return ResponseModeFragment
}

func (ar *authzRequest) Grant() Grant { return ar.grant }

func (ar *authzRequest) ClientID() string {
	if ar == nil || ar.cliInfo == nil {
		return ""
	}
	return ar.cliInfo.clientID
}
func (ar *authzRequest) RedirectURI() *url.URL {
	if ar == nil || ar.cliInfo == nil {
		return nil
	}
	return ar.cliInfo.redirectURI
}

func (ar *authzRequest) ResponseTypes() ResponseTypeSet { return ar.responseTypes }
func (ar *authzRequest) Scope() ScopeSet                { return ar.scope }
func (ar *authzRequest) State() string                  { return ar.state }

// oidc
func (ar *authzRequest) ResponseMode() ResponseMode { return ar.responseMode }
func (ar *authzRequest) Nonce() string              { return ar.nonce }
func (ar *authzRequest) Prompt() PromptSet          { return ar.prompt }
func (ar *authzRequest) IDTokenHint() string        { return ar.idTokenHint }
func (ar *authzRequest) OauthRequest() bool {
	if ar == nil || ar.scope == nil {
		return true
	}

	return !ar.scope.Contains(ScopeOpenid)
}

func (ar *authzRequest) LoginFormPost() bool { return ar.loginFormPost }

// oidc:"marshal:dec=q;err=OidcErrorInvalidRequest"
type clientInfoImpl struct {

	// rfc6749: Authorization Request
	// https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest

	// REQUIRED
	clientID string // oidc:"marshal:name=client_id"
	// OPTIONAL (oauth), REQUIRED(oidc)
	redirectURI *url.URL // oidc:"marshal:name=redirect_uri;omitempty"
}

func (ci *clientInfoImpl) ClientID() string {
	if ci == nil {
		return ""
	}
	return ci.clientID
}
func (ci *clientInfoImpl) RedirectURI() *url.URL {
	if ci == nil {
		return nil
	}
	return ci.redirectURI
}

// oidc:"marshal:dec=q;err=OidcErrorInvalidRequest"
type authzRequest struct {
	// rfc6749: Authorization Request
	// https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest

	cliInfo *clientInfoImpl

	grantType GrantType // oidc:"marshal:name=grant_type;omitempty"

	// REQUIRED
	responseTypes ResponseTypeSet // oidc:"marshal:name=response_type"

	// OPTIONAL (oauth), REQUIRED(oidc)
	scope ScopeSet // oidc:"marshal:name=scope;omitempty"

	// RECOMMENDED
	state string // oidc:"marshal:name=state;omitempty"

	// OPTIONAL
	responseMode ResponseMode // oidc:"marshal:name=response_mode;omitempty"
	// OPTIONAL
	nonce string // oidc:"marshal:name=nonce;omitempty"
	// OPTIONAL
	prompt PromptSet // oidc:"marshal:name=prompt;omitempty"
	// OPTIONAL
	idTokenHint string // oidc:"marshal:name=id_token_hint;omitempty"

	loginFormPost bool
	grant         *pwdGrantImpl
}

// oidc:"marshal:enc=qjf,url=redirectURL"
type authzResponse struct {
	state       string // oidc:"marshal:name=state;omitempty"
	redirectURL *url.URL
	code        string // oidc:"marshal:name=code"
}

func (r *authzResponse) Code() string          { return r.code }
func (r *authzResponse) State() string         { return r.state }
func (r *authzResponse) RedirectURI() *url.URL { return r.redirectURL }

func NewAuthzResponse(code string, ctxt OidcResponseCtxt) AuthzResponse {
	r := &authzResponse{
		code: code,
	}
	if ctxt != nil {
		r.state = ctxt.State()
		r.redirectURL = ctxt.RedirectURI()
	}
	return r
}
