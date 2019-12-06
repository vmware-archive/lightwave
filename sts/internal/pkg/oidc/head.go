package oidc

import (
	"fmt"
	"net/http"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/httphead"
	"github.com/vmware/lightwave/sts/internal/pkg/idm"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/oidc/protocol"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
	"github.com/vmware/lightwave/sts/web/static"
)

func New(hs httphead.HostingServer, sessions session.AuthSessionsMap,
	cfg config.InstanceConfig, sc idmconfig.StsConfig,
	auth idm.Authenticator, ti protocol.Issuer, logger diag.Logger) (httphead.HttpHeadConfig, diag.Error) {

	codes, err := NewAuthzCodeMap(cfg, sc, logger)
	if err != nil {
		return nil, err
	}

	oh := &oidcHeadImpl{hs: hs, sc: sc, sessions: sessions, authzCodes: codes}
	oh.ti = ti
	rp, err := NewRequestProcessor(sc, sessions, codes, auth, ti, logger)
	if err != nil {
		return nil, err
	}
	oh.reqProcessor = rp

	return oh, nil
}

const (
	metadataEp  = httphead.IssuerPath + "/.well-known/openid-configuration"
	jwksEp      = httphead.IssuerPath + "/" + string(config.OIDCHead) + "/jwks"
	authorizeEp = httphead.IssuerPath + "/" + string(config.OIDCHead) + "/authorize"
	tokenEp     = httphead.IssuerPath + "/" + string(config.OIDCHead) + "/token"
	logoutEp    = httphead.IssuerPath + "/" + string(config.OIDCHead) + "/logout"
)

type oidcHeadImpl struct {
	hs         httphead.HostingServer
	sc         idmconfig.StsConfig
	sessions   session.AuthSessionsMap
	authzCodes AuthzCodeMap
	//	auth         idm.Authenticator
	reqProcessor requestProcessor
	ti           protocol.Issuer
}

func (h *oidcHeadImpl) Endpoints() map[string]httphead.EndpointInfos {
	eps := make(map[string]httphead.EndpointInfos, 5)
	eps[metadataEp] = httphead.NewEndpointInfo(h.metadata, httphead.HTTPGET)
	eps[jwksEp] = httphead.NewEndpointInfo(h.jwks, httphead.HTTPGET)
	eps[authorizeEp] = httphead.NewEndpointInfo(h.authorize, httphead.HTTPGET, httphead.HTTPPOST)
	eps[tokenEp] = httphead.NewEndpointInfo(h.token, httphead.HTTPPOST)
	eps[logoutEp] = httphead.NewEndpointInfo(h.logout, httphead.HTTPGET)
	return eps
}

func (h *oidcHeadImpl) Middlewares() []httphead.Middleware {
	return []httphead.Middleware{h.noCacheMw}
}

func (h *oidcHeadImpl) metadata(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.OIDC, "Handling metadata...")

	var err diag.Error
	var resp protocol.OidcResponse
	var binding HttpBinding

	t := ctxt.Tenant()

	jwksEndpoint, err := h.absUrl(t, jwksEp, ctxt)
	if err != nil {
		resp, binding = h.reqProcessor.errorResponse(err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}
	logoutEndpoint, err := h.absUrl(t, logoutEp, ctxt)
	if err != nil {
		resp, binding = h.reqProcessor.errorResponse(err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}
	issuer, err := h.sc.Issuer(t, h.hs.AuthServicesPath, ctxt)
	if err != nil {
		resp, binding = h.reqProcessor.errorResponse(err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}
	authorizationEndpoint, err := h.absUrl(t, authorizeEp, ctxt)
	if err != nil {
		resp, binding = h.reqProcessor.errorResponse(err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}
	tokenEndpoint, err := h.absUrl(t, tokenEp, ctxt)
	if err != nil {
		resp, binding = h.reqProcessor.errorResponse(err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	resp, binding = h.reqProcessor.processMetadataRequest(
		issuer, jwksEndpoint, authorizationEndpoint, tokenEndpoint, logoutEndpoint, ctxt)
	h.handleResponse(resp, binding, rw, ctxt)
}

func (h *oidcHeadImpl) jwks(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.OIDC, "Handling jwks...")

	resp, binding := h.reqProcessor.processJWKSRequest(ctxt)
	h.handleResponse(resp, binding, rw, ctxt)
}

func (h *oidcHeadImpl) authorize(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.OIDC, "Handling authorize...")

	var cookieName string
	var err diag.Error
	var resp protocol.OidcResponse
	var binding HttpBinding
	var issuer string

	log := ctxt.Logger()

	existingSid := session.NoneSessionID
	createdSid := session.NoneSessionID

	authzReq, err := protocol.ParseAuthzRequest(req, h.reqProcessor.validateRedirectURI, ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorAuthzResponse(authzReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	cookieName, err = h.sessions.SessionCookieName(ctxt.Tenant(), ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorAuthzResponse(authzReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	issuer, err = h.sc.Issuer(ctxt.Tenant(), h.hs.AuthServicesPath, ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorAuthzResponse(authzReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	existingSid = h.readSessionCookie(cookieName, req, ctxt)
	resp, createdSid, binding = h.reqProcessor.processAuthzRequest(
		authzReq, existingSid, ctxt)

	if resp == nil { // show login page
		loginPageParams := &static.LoginPageParameters{
			LoginTitle:          "login",
			WelcomeTo:           "Welcome to",
			TenantBrandName:     "Lightwave Authentication Services",
			SignInText:          "Please sign-in here",
			UserNamePlaceHolder: "user@lightwave.local",
			Login:               "Login",
			EnablePasswordAuth:  true,
			Protocol:            "openidconnect",
			ResponseMode:        authzReq.ResponseMode().String(),
			GenericError:        "Login failed: ",
			ResponsePostForm:    protocol.PostFormID,
			ResourcesPath:       h.serverRelUrl(ctxt.Tenant(), httphead.ResourcesPath),
		}
		err = static.ServeLoginPage(rw, loginPageParams)
		if err != nil {
			log.Errorf(diag.OIDC, "Failed serving login page: %v", err)
			resp, binding := h.reqProcessor.errorAuthzResponse(authzReq, err, ctxt)
			h.handleResponse(resp, binding, rw, ctxt)
			return
		}
		return
	}

	ctxt.Logger().Tracef(diag.OIDC, "Session ID: %v", createdSid)
	if createdSid != session.NoneSessionID {
		cookie := h.newSessionCookie(cookieName, createdSid.String(), issuer)
		ctxt.Logger().Tracef(diag.OIDC, "setting cookie: %s=%s", cookieName, cookie.String())
		http.SetCookie(rw, cookie)
	}

	h.handleResponse(resp, binding, rw, ctxt)
	return
}

func (h *oidcHeadImpl) token(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.OIDC, "Handling token...")

	var err diag.Error

	tokenReq, err := protocol.ParseTokenRequest(req, h.reqProcessor.validateRedirectURI, ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorTokenResponse(tokenReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	resp, binding := h.reqProcessor.processTokenRequest(tokenReq, ctxt)
	h.handleResponse(resp, binding, rw, ctxt)
	return
}

func (h *oidcHeadImpl) logout(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.OIDC, "Handling logout...")

	var resp protocol.OidcResponse
	var binding HttpBinding
	var cookieName string
	var issuer string
	var err diag.Error
	var existingSid session.SessionID

	logoutReq, err := protocol.ParseLogoutRequest(req, h.ti.ValidateToken, h.reqProcessor.validatePostLogoutURI, ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorLogoutResponse(logoutReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	cookieName, err = h.sessions.SessionCookieName(ctxt.Tenant(), ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorLogoutResponse(logoutReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	issuer, err = h.sc.Issuer(ctxt.Tenant(), h.hs.AuthServicesPath, ctxt)
	if err != nil {
		resp, binding := h.reqProcessor.errorLogoutResponse(logoutReq, err, ctxt)
		h.handleResponse(resp, binding, rw, ctxt)
		return
	}

	existingSid = h.readSessionCookie(cookieName, req, ctxt)

	resp, binding = h.reqProcessor.processLogoutRequest(
		logoutReq, existingSid, ctxt)

	// clear off cookie
	if existingSid != session.NoneSessionID {
		cookie := h.newSessionCookie(cookieName, "", issuer)
		ctxt.Logger().Tracef(diag.OIDC, "setting cookie: %s=%s", cookieName, cookie.String())
		http.SetCookie(rw, cookie)
	}

	h.handleResponse(resp, binding, rw, ctxt)
	return
}

func (h *oidcHeadImpl) newSessionCookie(name string, value string, issuer string) *http.Cookie {
	return &http.Cookie{
		Name: name, Value: value, HttpOnly: true, Path: issuer,
		Secure: true}
}

func (h *oidcHeadImpl) readSessionCookie(name string, req *http.Request, ctxt diag.RequestContext) session.SessionID {
	sid := session.NoneSessionID
	sc, errC := req.Cookie(name)
	if sc != nil && errC == nil && len(sc.Value) > 0 {
		sid = session.SessionIDFromString(sc.Value)
	}
	if errC != nil && errC != http.ErrNoCookie {
		ctxt.Logger().Warningf(diag.OIDC, "Failed to get session cookie with '%v'", errC)
	}
	return sid
}

func (h *oidcHeadImpl) absUrl(tenant diag.TenantID, rel string, rc diag.RequestContext) (string, diag.Error) {
	serverRelPath := h.hs.GetServerRelativePath(tenant)
	ep, err := h.sc.PublicEndpoint(tenant, rc)
	if err != nil {
		return "", err
	}
	return fmt.Sprintf("%s%s%s", ep, serverRelPath, rel), nil
}

func (h *oidcHeadImpl) serverRelUrl(tenant diag.TenantID, rel string) string {
	serverRelPath := h.hs.GetServerRelativePath(tenant)
	return fmt.Sprintf("%s%s", serverRelPath, rel)
}

func (h *oidcHeadImpl) handleResponse(
	resp protocol.OidcResponse, binding HttpBinding,
	rw http.ResponseWriter, ctxt diag.RequestContext) {
	log := ctxt.Logger()

	sb := &strings.Builder{}
	err := resp.Marshal(sb, binding.Format())
	if err != nil {
		log.Errorf(diag.OIDC, "Failed serializing server response: '%v'", err)
		sb.Reset()
		fmt.Fprintf(sb,
			"{ \"error\" : \"%s\", \"error_description\" : \"%s\" }",
			err.Code().Name(), "Failed serializing server response")
		rw.Header().Set(httphead.ContentTypeHeader, httphead.JSONContentType)
		rw.WriteHeader(http.StatusInternalServerError)
		_, err1 := rw.Write([]byte(sb.String()))
		if err1 != nil {
			log.Errorf(diag.OIDC, "Failed writing server response: '%v'", err1)
		}
		return
	}

	switch binding.Format() {
	case protocol.MarshalFormatJSON:
		{
			rw.Header().Set(httphead.ContentTypeHeader, httphead.JSONContentType)
			rw.WriteHeader(binding.Status())
			_, err1 := rw.Write([]byte(sb.String()))
			if err1 != nil {
				log.Errorf(diag.OIDC, "Failed writing server response: '%v'", err1)
			}
			return
		}
	case protocol.MarshalFormatQuery, protocol.MarshalFormatFragment:
		{
			if binding.Status() == http.StatusFound {
				rw.Header().Set("Location", sb.String())
			}
			rw.WriteHeader(binding.Status())
			if binding.Status() != http.StatusFound {
				_, err1 := rw.Write([]byte(sb.String()))
				if err1 != nil {
					log.Errorf(diag.OIDC, "Failed writing server response: '%v'", err1)
				}
			}
			return
		}
	case protocol.MarshalFormatHTML, protocol.MarshalFormatForm:
		{
			rw.Header().Set(httphead.ContentTypeHeader, httphead.HTMLContentType)
			_, err1 := rw.Write([]byte(sb.String()))
			if err1 != nil {
				log.Errorf(diag.OIDC, "Failed writing server response: '%v'", err1)
			}
			return
		}
	}

}

func (h *oidcHeadImpl) noCacheMw(handler httphead.RequestHandler) httphead.RequestHandler {
	return func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
		ctxt.Logger().Tracef(diag.OIDC, "oidc.noCacheMw mw before handler...")
		rw.Header().Set("Cache-Control", "no-cache")
		rw.Header().Add("Cache-Control", "no-store")
		rw.Header().Set("Pragma", "no-cache")
		handler(rw, req, ctxt, params)
		ctxt.Logger().Tracef(diag.OIDC, "oidc.noCacheMw mw after handler...")
	}
}
