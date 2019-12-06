package oidc

import (
	"fmt"
	"net/http"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/oidc/protocol"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
)

// move to protocol package
type requestProcessor interface {

	// nil response means show login page
	processAuthzRequest(req protocol.AuthzRequest, sid session.SessionID, ctxt diag.RequestContext) (
		protocol.OidcResponse, session.SessionID, HttpBinding)

	errorAuthzResponse(req protocol.AuthzRequest, err diag.Error, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	processTokenRequest(req protocol.TokenRequest, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	errorTokenResponse(req protocol.TokenRequest, err diag.Error, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	processLogoutRequest(req protocol.LogoutRequest, sid session.SessionID, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	errorLogoutResponse(req protocol.LogoutRequest, err diag.Error, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	errorResponse(err diag.Error, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	processMetadataRequest(issuer, jwksEP, authzEP, tokenEP, logoutEP string, ctxt diag.RequestContext) (
		protocol.OidcResponse, HttpBinding)

	processJWKSRequest(ctxt diag.RequestContext) (protocol.OidcResponse, HttpBinding)

	validateRedirectURI(ci protocol.ClientInfo, ctxt diag.RequestContext) (*url.URL, diag.Error)

	validatePostLogoutURI(ci protocol.ClientInfo, ctxt diag.RequestContext) (*url.URL, diag.Error)
}

func NewRequestProcessor(sc config.StsConfig, sessions session.AuthSessionsMap,
	authzCodes AuthzCodeMap, auth idm.Authenticator, ti protocol.Issuer, logger diag.Logger) (requestProcessor, diag.Error) {
	return &requestProcessorImpl{sc: sc, sessions: sessions, authzCodes: authzCodes, auth: auth, ti: ti}, nil
}

func (p *requestProcessorImpl) processAuthzRequest(
	req protocol.AuthzRequest, sid session.SessionID, ctxt diag.RequestContext) (
	protocol.OidcResponse, session.SessionID, HttpBinding) {

	// todo: auth client

	var user types.UserID = types.NoneUserID
	var err diag.Error
	var sess session.AuthSession

	log := ctxt.Logger()

	if req.LoginFormPost() {

		grant := req.Grant()
		if grant == nil {
			log.Errorf(diag.OIDC, "Request: grant is nil")
			err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Invalid user name or password", nil)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}

		if grant.Type() != protocol.GrantTypePassword {
			log.Errorf(diag.OIDC, "Unsupported grant type: %s", grant.Type().String())
			err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Unsupported authentication method", nil)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}

		user, err = p.auth.Authenticate(ctxt.Tenant(), grant, ctxt)
		if err != nil {
			log.Errorf(diag.OIDC, "User auth failed: %v", err)
			err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Invalid user name or password", err)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}

		// create session
		// TODO: if we created session, but failed the function, we should probably clean it up
		sess, err = p.sessions.NewSession(ctxt.Tenant(), user, types.LoginMethodPassword, types.ClientIDFromString(req.ClientID()), ctxt)
		if err != nil {
			log.Errorf(diag.OIDC, "creting session failed: %v", err)
			err = diag.MakeError(protocol.OidcErrorServerError, "Unable to create an auth session", err)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}
	} else {
		// get request
		if sid != session.NoneSessionID {
			sess, err = p.sessions.Get(ctxt.Tenant(), sid, ctxt)
			if err != nil {
				log.Warningf(diag.OIDC, "Failed looking up session: %v", err)
				sess = nil
				err = nil
			}
		}

		// we don;t have a session, or we are required to re-login
		if sess == nil || (!req.OauthRequest() && req.Prompt().Contains(protocol.PromptLogin)) {
			if !req.OauthRequest() && req.Prompt().Contains(protocol.PromptNone) {
				log.Errorf(diag.OIDC, "No session, but prompt=none")
				err = diag.MakeError(protocol.OidcErrorLoginRequired, "Unable to complete request silently", nil)
				r, b := p.errorAuthzResponse(req, err, ctxt)
				return r, session.NoneSessionID, b
			}
			return nil, session.NoneSessionID, nil // serve login page
		}

		sess, err = p.sessions.Update(ctxt.Tenant(), sess.ID(), nil, types.ClientIDFromString(req.ClientID()), ctxt)
		if err != nil {
			log.Errorf(diag.OIDC, "Failed updating session info: %v", err)
			err = diag.MakeError(protocol.OidcErrorServerError, "Unable to update session", err)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}

		user = sess.UserIdentity()
	}

	var authz AuthzCodeEntry

	// we have user
	if req.ResponseTypes().AuthzFlow() || req.ResponseTypes().HybridFlow() {
		// need an authz code
		authz, err = p.authzCodes.Add(
			ctxt.Tenant(), req, req.Scope(), req.Nonce(), sess.ID(), ctxt)

		if err != nil {
			log.Errorf(diag.OIDC, "Failed creating authz code: %v", err)
			err = diag.MakeError(protocol.OidcErrorServerError, "Unable to create authz code", err)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}
	}

	var resp protocol.OidcResponse

	if req.ResponseTypes().AuthzFlow() {
		resp = protocol.NewAuthzResponse(authz.Code(), req)
	} else if req.ResponseTypes().NoneResponse() {
		resp = protocol.NewAuthzResponse("", req)
	} else { // implicit or hybrid
		code := ""
		if authz != nil {
			code = authz.Code()
		}
		resp, err = p.ti.IssueTokens(req, req.Scope(), req.Nonce(), user, code, sess.ID(), ctxt)
		if err != nil {
			log.Errorf(diag.OIDC, "Failed creating authz code: %v", err)
			err = diag.MakeError(protocol.OidcErrorServerError, "Unable to issue tokens", err)
			r, b := p.errorAuthzResponse(req, err, ctxt)
			return r, session.NoneSessionID, b
		}
	}
	// todo: only return session id, if sesion was created, not if it existed
	return resp, sess.ID(), p.authzResponseInfo(req, req.LoginFormPost(), nil)
}

func (p *requestProcessorImpl) errorAuthzResponse(req protocol.AuthzRequest, err diag.Error, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {
	return protocol.NewErrorResponse(err, req), p.authzResponseInfo(req, req.LoginFormPost(), err)
}

func (p *requestProcessorImpl) processTokenRequest(
	req protocol.TokenRequest, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {

	// auth client
	user := types.NoneUserID
	var err diag.Error
	var resp protocol.OidcResponse
	log := ctxt.Logger()

	scope := req.Scope()
	nonce := ""

	switch req.Grant().Type() {
	case protocol.GrantTypePassword:
		{
			user, err = p.auth.Authenticate(ctxt.Tenant(), req.Grant(), ctxt)
			if err != nil {
				log.Errorf(diag.OIDC, "User auth failed: %v", err)
				err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Invalid user name or password", err)
				return p.errorTokenResponse(req, err, ctxt)
			}
		}
	case protocol.GrantTypeAuthorizationCode:
		{
			var authz AuthzCodeEntry

			authzGrant := req.Grant().(protocol.AuthzCodeGrant)

			authz, err := p.authzCodes.Remove(
				ctxt.Tenant(), authzGrant.Code(), ctxt)
			if err != nil {
				log.Errorf(diag.OIDC, "Failed removing authz code: %v", err)
				err = diag.MakeError(protocol.OidcErrorServerError, "Unable to validate authz code", err)
				return p.errorTokenResponse(req, err, ctxt)
			}

			if authz == nil {
				log.Errorf(diag.OIDC, "Failed looking up authz code '%s': %v", authzGrant.Code(), err)
				err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Invalid authz code", nil)
				return p.errorTokenResponse(req, err, ctxt)
			}

			if authz.ClientID() != req.ClientID() {
				log.Errorf(diag.OIDC, "Client ID mismatch authz code '%s': token request '%s'", authz.ClientID(), req.ClientID())
				err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Client id mismatch", nil)
				return p.errorTokenResponse(req, err, ctxt)
			}
			if authz.RedirectURI().String() != req.RedirectURI().String() {
				log.Errorf(diag.OIDC, "Redirect Uri mismatch authz code '%s': token request '%s'", authz.RedirectURI().String(), req.RedirectURI().String())
				err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Client redirect uri mismatch", nil)
				return p.errorTokenResponse(req, err, ctxt)
			}

			authSess, err := p.sessions.Get(ctxt.Tenant(), authz.SessionID(), ctxt)
			if err != nil {
				log.Errorf(diag.OIDC, "Failed identifying session: %v", err)
				err = diag.MakeError(protocol.OidcErrorServerError, "Session lookup failed", err)
				return p.errorTokenResponse(req, err, ctxt)
			}

			if authSess == nil {
				log.Errorf(diag.OIDC, "Failed looking up session from authzcode '%s': %v", authzGrant.Code(), err)
				err = diag.MakeError(protocol.OidcErrorInvalidGrant, "Invalid authz code", nil)
				return p.errorTokenResponse(req, err, ctxt)
			}
			user = authSess.UserIdentity()
			scope = authz.Scope()
			nonce = authz.Nonce()
		}
	default:
		{
			err = diag.MakeError(
				protocol.OidcErrorUnsupportedGrantType,
				fmt.Sprintf("Unsupported grant_type: '%s'", req.Grant().Type()), nil)
			return p.errorTokenResponse(req, err, ctxt)
		}
	}

	resp, err = p.ti.IssueTokens(req, scope, nonce, user, "", session.NoneSessionID, ctxt)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed creating authz code: %v", err)
		err = diag.MakeError(protocol.OidcErrorServerError, "Unable to issue tokens", err)
		return p.errorTokenResponse(req, err, ctxt)
	}

	return resp, &httpBindingImpl{status: http.StatusOK, format: protocol.MarshalFormatJSON}
}

func (p *requestProcessorImpl) errorTokenResponse(req protocol.TokenRequest, err diag.Error, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {

	return p.errorResponse(err, ctxt)
}

func (p *requestProcessorImpl) processLogoutRequest(
	req protocol.LogoutRequest, sid session.SessionID, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {
	// auth client

	var err diag.Error
	log := ctxt.Logger()

	as, err := p.sessions.Get(ctxt.Tenant(), sid, ctxt)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed looking up session: %v", err)
		err = diag.MakeError(protocol.OidcErrorServerError, "Session lookup failed", err)
		return p.errorLogoutResponse(req, err, ctxt)
	}
	err = p.sessions.Remove(ctxt.Tenant(), sid, ctxt)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed removing session: %v", err)
		err = diag.MakeError(protocol.OidcErrorServerError, "Session removal failed", err)
		return p.errorLogoutResponse(req, err, ctxt)
	}

	issuer, err := p.ti.Issuer(ctxt)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed getting issuer: %v", err)
		err = diag.MakeError(protocol.OidcErrorServerError, "Failed obtaining issuer", err)
		return p.errorLogoutResponse(req, err, ctxt)
	}

	var logouts []*url.URL

	logouts = make([]*url.URL, 0, as.Clients().Len())
	err = as.Clients().Iterate(func(v types.ClientID) diag.Error {
		if v.String() != req.ClientID() {
			lu, err := p.getLogoutURI(v, issuer, sid, ctxt)
			if err != nil {
				return err
			}
			if lu != nil {
				logouts = append(logouts, lu)
			}
		}
		return nil
	})
	if err != nil {
		log.Errorf(diag.OIDC, "Failed producing logout response: %v", err)
		err = diag.MakeError(protocol.OidcErrorServerError, "Logout response creation failed", err)
		return p.errorLogoutResponse(req, err, ctxt)
	}

	return protocol.NewLogoutResponse(req, logouts),
		&httpBindingImpl{status: http.StatusOK, format: protocol.MarshalFormatHTML}
}

func (p *requestProcessorImpl) errorLogoutResponse(req protocol.LogoutRequest, err diag.Error, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {

	binding := &httpBindingImpl{
		status: httpStatus(err.Code()),
		format: protocol.MarshalFormatJSON,
	}
	if req.RedirectURI() != nil {
		binding.status = http.StatusFound
		binding.format = protocol.MarshalFormatQuery
	}
	ctxt.Logger().Infof(diag.OIDC, "Error reponse: %v", err)
	return protocol.NewErrorResponse(err, req), binding
}

func (p *requestProcessorImpl) errorResponse(err diag.Error, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {
	return protocol.NewErrorResponse(err, nil),
		&httpBindingImpl{
			status: httpStatus(err.Code()),
			format: protocol.MarshalFormatJSON}
}

func (p *requestProcessorImpl) processMetadataRequest(issuer, jwksEP, authzEP, tokenEP, logoutEP string, ctxt diag.RequestContext) (
	protocol.OidcResponse, HttpBinding) {

	resp := protocol.NewMetadataResponse(issuer, jwksEP, authzEP, tokenEP, logoutEP)
	return resp, successBinding
}

func (p *requestProcessorImpl) processJWKSRequest(ctxt diag.RequestContext) (protocol.OidcResponse, HttpBinding) {

	ks, err := p.ti.Signers(ctxt)
	if err != nil {
		return p.errorResponse(err, ctxt)
	}

	return protocol.NewJWKSResponse(ks), successBinding
}

func (p *requestProcessorImpl) validateRedirectURI(
	ci protocol.ClientInfo, ctxt diag.RequestContext) (*url.URL, diag.Error) {

	if ci == nil {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client info unavailable", nil)
	}

	if len(ci.ClientID()) <= 0 {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client_id unavailable", nil)
	}

	cli, err := p.sc.LookupOidcClient(ctxt.Tenant(), ci.ClientID(), ctxt)
	if err != nil {
		// todo: proper idm->oidc error mapping
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client info unavailable", err)
	}

	// if redirect uri is omitted, and registration has 1 redirect uri use that uri (oauth spec)
	if (ci.RedirectURI() == nil) && len(cli.RedirectURIs()) == 1 {
		return cli.RedirectURIs()[0], nil
	}

	if ci.RedirectURI() == nil {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "redirect url is unavailable", nil)
	}

	if !contains(cli.RedirectURIs(), ci.RedirectURI()) {
		return nil, diag.MakeError(
			protocol.OidcErrorInvalidRequest,
			fmt.Sprintf("Invalid redirect uri '%v'", ci.RedirectURI()),
			nil)
	}

	return nil, nil
}

func (p *requestProcessorImpl) validatePostLogoutURI(
	ci protocol.ClientInfo, ctxt diag.RequestContext) (*url.URL, diag.Error) {

	if ci == nil {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client info unavailable", nil)
	}

	if len(ci.ClientID()) <= 0 {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client_id unavailable", nil)
	}

	cli, err := p.sc.LookupOidcClient(ctxt.Tenant(), ci.ClientID(), ctxt)
	if err != nil {
		// todo: proper idm->oidc error mapping
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client info unavailable", err)
	}

	if ci.RedirectURI() == nil {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "redirect url is unavailable", nil)
	}

	if !contains(cli.PostLogoutRedirectURIs(), ci.RedirectURI()) {
		return nil, diag.MakeError(
			protocol.OidcErrorInvalidRequest,
			fmt.Sprintf("Invalid post logout redirect uri '%v'", ci.RedirectURI()),
			nil)
	}

	return nil, nil
}

func (p *requestProcessorImpl) authzResponseInfo(req protocol.AuthzRequest, loginFormPost bool, err diag.Error) HttpBinding {

	binding := &httpBindingImpl{
		status: http.StatusOK,
		format: protocol.MarshalFormatJSON,
	}

	if req == nil || req.RedirectURI() == nil || (err != nil && loginFormPost) {
		binding.status = http.StatusOK
		binding.format = protocol.MarshalFormatJSON
		if err != nil {
			binding.status = httpStatus(err.Code())
		}
		return binding
	}

	switch req.ResponseMode() {
	case protocol.ResponseModeQuery:
		{
			binding.format = protocol.MarshalFormatQuery
			if loginFormPost {
				binding.status = http.StatusOK
			} else {
				binding.status = http.StatusFound
			}
		}
	case protocol.ResponseModeFragment:
		{
			binding.format = protocol.MarshalFormatFragment
			if loginFormPost {
				binding.status = http.StatusOK
			} else {
				binding.status = http.StatusFound
			}
		}
	case protocol.ResponseModeFormPost:
		{
			binding.status = http.StatusOK
			if loginFormPost {
				binding.format = protocol.MarshalFormatForm
			} else {
				binding.format = protocol.MarshalFormatHTML
			}
		}
	}
	return binding
}

func (p *requestProcessorImpl) getLogoutURI(
	client types.ClientID, issuer string, sid session.SessionID, ctxt diag.RequestContext) (*url.URL, diag.Error) {

	cli, err := p.sc.LookupOidcClient(ctxt.Tenant(), client.String(), ctxt)
	if err != nil {
		// todo: proper idm->oidc error mapping
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "client info unavailable", err)
	}

	// todo: for now we assume issues & sid required; this could be configured on oidc client in future
	// https://openid.net/specs/openid-connect-frontchannel-1_0.html#OPLogout

	uri := cli.LogoutURI()
	query, err1 := url.ParseQuery(uri.RawQuery)
	if err1 != nil {
		return nil, diag.MakeError(protocol.OidcErrorInvalidRequest, "invalid logout uri", err1)
	}

	query.Add("iss", issuer)
	query.Add("sid", sid.String())

	uri.RawQuery = query.Encode()
	return uri, nil
}

func contains(uris []*url.URL, uri *url.URL) bool {
	strURI := uri.String()
	for _, u := range uris {
		if u.String() == strURI {
			return true
		}
	}
	return false
}

type requestProcessorImpl struct {
	sc         config.StsConfig
	sessions   session.AuthSessionsMap
	authzCodes AuthzCodeMap
	auth       idm.Authenticator
	ti         protocol.Issuer
}

type HttpBinding interface {
	Status() int
	Format() protocol.MarshalFormat
}

type httpBindingImpl struct {
	status int
	format protocol.MarshalFormat
}

var successBinding = &httpBindingImpl{
	status: http.StatusOK,
	format: protocol.MarshalFormatJSON,
}

func (hb *httpBindingImpl) Status() int                    { return hb.status }
func (hb *httpBindingImpl) Format() protocol.MarshalFormat { return hb.format }

func httpStatus(c diag.ErrorCode) int {
	st, statusOK := c.(diag.HTTPStatusMappedErrorCode)
	if statusOK {
		return st.HttpStatus()
	}
	return http.StatusInternalServerError
}
