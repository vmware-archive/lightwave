package protocol

import (
	"fmt"
	"net/http"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

// ParseTokenRequest attempts to construct a token request from http request
func ParseTokenRequest(
	req *http.Request, uriValidator UriValidatorFunc, ctxt diag.RequestContext) (TokenRequest, diag.Error) {

	log := ctxt.Logger()
	errURL := req.ParseForm()
	if errURL != nil {
		log.Errorf(diag.OIDC, "Failed to parse request form: %v", errURL)
		return nil, diag.MakeError(OidcErrorInvalidRequest, "Invalid requestForm", errURL)
	}

	var err diag.Error
	tr := &tokenRequest{}

	err = tr.UnMarshalQuery(req.PostForm)
	if err != nil {
		return nil, err
	}

	if len(tr.clientID) > 0 {
		normalizedURI, uriErr := uriValidator(tr, ctxt)
		if err != nil {
			return nil, uriErr
		}
		if tr.redirectURI == nil {
			tr.redirectURI = normalizedURI
		}
	}

	switch tr.grantType {
	case GrantTypePassword:
		{
			g := new(pwdGrantImpl)
			err := g.UnMarshalQuery(req.PostForm)
			if err != nil {
				log.Errorf(diag.OIDC, "Failed to parse %s: '%v'", tr.grantType.String(), err)
				return nil, err
			}
			tr.grant = g
		}
	case GrantTypeRefreshToken:
		{
			g := new(refreshTokGrantImpl)
			err := g.UnMarshalQuery(req.PostForm)
			if err != nil {
				log.Errorf(diag.OIDC, "Failed to parse %s: '%v'", tr.grantType.String(), err)
				return nil, err
			}
			tr.grant = g
		}
	case GrantTypeAuthorizationCode:
		{
			g := new(authzCodeGrantImpl)
			err := g.UnMarshalQuery(req.PostForm)
			if err != nil {
				log.Errorf(diag.OIDC, "Failed to parse %s: '%v'", tr.grantType.String(), err)
				return nil, err
			}
			tr.grant = g
		}
		//	case GrantTypeClientCredentials:
		//		{
		//			// TODO: ////
		//		}
	default:
		{
			return nil,
				diag.MakeError(OidcErrorUnsupportedGrantType,
					fmt.Sprintf("grant_type '%s' is unsupported", tr.grantType.String()), nil)
		}
	}

	err = tr.validate(ctxt)
	if err != nil {
		return nil, err
	}

	return tr, nil
}

func (r *tokenRequest) validate(ctxt diag.RequestContext) diag.Error {
	if r.grantType == GrantTypeAuthorizationCode {
		if len(r.clientID) <= 0 {
			return diag.MakeError(
				OidcErrorInvalidRequest, "client_id is required", nil)
		}

		if r.RedirectURI() == nil {
			return diag.MakeError(
				OidcErrorInvalidRequest, "redirect_uri is required", nil)
		}

		if r.Scope().Len() > 0 {
			return diag.MakeError(
				OidcErrorInvalidRequest, "scope parameter is not suported in authorzation code grant", nil)
		}
	}

	return nil
}

func (r *tokenRequest) Grant() Grant          { return r.grant }
func (r *tokenRequest) Scope() ScopeSet       { return r.scope }
func (r *tokenRequest) ClientID() string      { return r.clientID }
func (r *tokenRequest) RedirectURI() *url.URL { return r.redirectURI }

// oidc:"marshal:dec=q;err=OidcErrorInvalidRequest"
type tokenRequest struct {
	grantType GrantType // oidc:"marshal:name=grant_type"

	// authz code
	clientID    string   // oidc:"marshal:name=client_id;omitempty"
	redirectURI *url.URL // oidc:"marshal:name=redirect_uri;omitempty"

	scope ScopeSet // oidc:"marshal:name=scope;omitempty" OPTIONAL (oauth) REQUIRED (oidc). OpenID Connect requests MUST contain the openid scope value

	grant Grant
}

// oidc:"marshal:enc=qjf,url=redirectURL;err=OidcErrorInvalidRequest" // marshal to query, form, json; no unmarshal
type tokenResponse struct {
	state         string // oidc:"marshal:name=state;omitempty"
	redirectURL   *url.URL
	accessToken   string   // oidc:"marshal:name=access_token;omitempty"
	tokenType     string   // oidc:"marshal:name=token_type;omitempty"
	expiresInSecs int      // oidc:"marshal:name=expires_in;omitempty"
	refreshToken  string   // oidc:"marshal:name=refresh_token;omitempty"
	scope         ScopeSet // oidc:"marshal:name=scope;omitempty"

	// oidc
	idToken string // oidc:"marshal:name=id_token;omitempty"
	code    string // oidc:"marshal:name=code;omitempty"
}

func (r *tokenResponse) AccessToken() string   { return r.accessToken }
func (r *tokenResponse) TokenType() string     { return r.tokenType }
func (r *tokenResponse) IDToken() string       { return r.idToken }
func (r *tokenResponse) RefreshToken() string  { return r.refreshToken }
func (r *tokenResponse) ExpiresInSecs() int    { return r.expiresInSecs }
func (r *tokenResponse) Scope() ScopeSet       { return r.scope }
func (r *tokenResponse) Code() string          { return r.code }
func (r *tokenResponse) State() string         { return r.state }
func (r *tokenResponse) RedirectURI() *url.URL { return r.redirectURL }

func NewTokenResponse(
	accessToken string, tokenType string, idToken string, refreshToken string, code string,
	expiresInSecs int, scope ScopeSet, ctxt OidcResponseCtxt) OidcTokenResponse {
	tr := &tokenResponse{
		accessToken:   accessToken,
		tokenType:     tokenType,
		expiresInSecs: expiresInSecs,
		refreshToken:  refreshToken,
		scope:         scope,
		idToken:       idToken,
		code:          code,
	}

	if ctxt != nil {
		tr.state = ctxt.State()
		tr.redirectURL = ctxt.RedirectURI()
	}

	return tr
}
