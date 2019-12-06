package rest

import (
	"fmt"
	"net/http"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/httphead"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

func New(hs httphead.HostingServer, cfg config.InstanceConfig, sc idmconfig.Configurator,
	tis []auth.Validator, logger diag.Logger) (httphead.HttpHeadConfig, diag.Error) {

	rh := &restHeadImpl{hs: hs, config: sc, tis: make(map[string]auth.Validator, len(tis))}
	for _, ti := range tis {
		rh.tis[ti.TokenKind()] = ti
	}

	return rh, nil
}

const (
	policyEp     = httphead.ConfigPath + "/policy"
	providerEp   = httphead.ConfigPath + "/providers"
	oidcClientEp = httphead.ConfigPath + "/oidcclients"

	providerParam   = "provider"
	oidcClientParam = "oidcclient"
)

type restHeadImpl struct {
	hs     httphead.HostingServer
	tis    map[string]auth.Validator
	config idmconfig.Configurator
}

func (h *restHeadImpl) Endpoints() map[string]httphead.EndpointInfos {
	eps := make(map[string]httphead.EndpointInfos, 20)
	eps[policyEp] = httphead.NewEndpointInfo(h.policy, httphead.HTTPGET)
	// todo: set/update policies
	eps[providerEp] = httphead.NewEndpointInfos(
		httphead.NewEP(authUser(h.listProviders), httphead.HTTPGET),
		httphead.NewEP(admin(h.createProvider), httphead.HTTPPOST))
	eps[providerEp+"/:"+providerParam] = httphead.NewEndpointInfos(
		httphead.NewEP(authUser(h.getProvider), httphead.HTTPGET),
		httphead.NewEP(admin(h.updateProvider), httphead.HTTPPUT))

	eps[oidcClientEp] = httphead.NewEndpointInfos(
		httphead.NewEP(authUser(h.listOidcClient), httphead.HTTPGET),
		httphead.NewEP(admin(h.createOidcClient), httphead.HTTPPOST))
	eps[oidcClientEp+"/:"+oidcClientParam] = httphead.NewEndpointInfos(
		httphead.NewEP(authUser(h.getOidcClient), httphead.HTTPGET),
		httphead.NewEP(admin(h.updateOidcClient), httphead.HTTPPUT))

	return eps
}

func (h *restHeadImpl) Middlewares() []httphead.Middleware {
	return []httphead.Middleware{h.authMw}
}

func (h *restHeadImpl) policy(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling policy request : '%v' ...", req.URL.Path)

	tp, err := h.config.TokenPolicy(ctxt.Tenant(), ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	sb := &strings.Builder{}

	err = MarshalTokenPolicy(tp, sb)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	h.sendJSON(sb.String(), rw, ctxt)
}

func (h *restHeadImpl) listProviders(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling listProviders request : '%v' ...", req.URL.Path)

	ps, err := h.config.ListIDSs(ctxt.Tenant(), ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	sb := &strings.Builder{}

	err = MarshalIDSConfigList(ps, sb)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	h.sendJSON(sb.String(), rw, ctxt)
}

func (h *restHeadImpl) createProvider(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling createProvider request : '%v' ...", req.URL.Path)

	// todo: implement
	rw.WriteHeader(http.StatusBadRequest)
	rw.Write([]byte("not yet implemented"))
}

func (h *restHeadImpl) getProvider(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling getProvider request : '%v' ...", req.URL.Path)

	provName := params.ByName(providerParam)
	if len(provName) <= 0 {
		err := diag.MakeError(types.IdmErrorInvalidArgument, "Provider name is required.", nil)
		handleError(err, rw, ctxt)
		return
	}

	prov, err := h.config.GetIDS(ctxt.Tenant(), provName, ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	sb := &strings.Builder{}

	err = MarshalIDSConfig(prov, sb)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	h.sendJSON(sb.String(), rw, ctxt)
}

func (h *restHeadImpl) updateProvider(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling getProvider request : '%v' ...", req.URL.Path)

	// todo: implement
	rw.WriteHeader(http.StatusBadRequest)
	rw.Write([]byte("not yet implemented"))
}

func (h *restHeadImpl) listOidcClient(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling listOidcClient request : '%v' ...", req.URL.Path)

	ocs, err := h.config.ListOidcClients(ctxt.Tenant(), ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	sb := &strings.Builder{}

	err = MarshalOidcClientList(ocs, sb)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	h.sendJSON(sb.String(), rw, ctxt)
}

func (h *restHeadImpl) createOidcClient(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling createOidcClient request : '%v' ...", req.URL.Path)

	clib := types.NewOidcClientBuilder()
	err := UnMarshalOidcClientMeta(clib, req.Body)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	oidcClient, err := clib.Build()
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	err = h.config.CreateOidcClient(ctxt.Tenant(), oidcClient, ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	rw.WriteHeader(http.StatusCreated)
}

func (h *restHeadImpl) getOidcClient(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling getOidcClient request : '%v' ...", req.URL.Path)

	cliid := params.ByName(oidcClientParam)
	if len(cliid) <= 0 {
		err := diag.MakeError(types.IdmErrorInvalidArgument, "Client id is required.", nil)
		handleError(err, rw, ctxt)
		return
	}

	oc, err := h.config.GetOidcClient(ctxt.Tenant(), cliid, ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	sb := &strings.Builder{}

	err = MarshalOidcClient(oc, sb)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	h.sendJSON(sb.String(), rw, ctxt)
}

func (h *restHeadImpl) updateOidcClient(
	rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
	ctxt.Logger().Infof(diag.REST, "Handling getOidcClient request : '%v' ...", req.URL.Path)

	cliid := params.ByName(oidcClientParam)
	if len(cliid) <= 0 {
		err := diag.MakeError(types.IdmErrorInvalidArgument, "Client id is required.", nil)
		handleError(err, rw, ctxt)
		return
	}

	clib := types.NewOidcClientBuilder()
	err := UnMarshalOidcClientMeta(clib, req.Body)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	clib.ID(cliid)
	oidcClient, err := clib.Build()
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	err = h.config.UpdateOidcClient(ctxt.Tenant(), oidcClient, ctxt)
	if err != nil {
		handleError(err, rw, ctxt)
		return
	}

	rw.WriteHeader(http.StatusNoContent)
}

func handleError(err diag.Error, rw http.ResponseWriter, ctxt diag.RequestContext) {
	// todo: real impl
	rw.WriteHeader(http.StatusBadRequest)
	rw.Write([]byte(err.Error()))
}
func (h *restHeadImpl) sendJSON(resp string, rw http.ResponseWriter, ctxt diag.RequestContext) {
	rw.Header().Set(httphead.ContentTypeHeader, httphead.JSONContentType)
	rw.WriteHeader(http.StatusOK)
	_, err := rw.Write([]byte(resp))
	if err != nil {
		ctxt.Logger().Errorf(diag.REST, "Failed writing server response: '%v'", err)
	}
}

func (h *restHeadImpl) authMw(handler httphead.RequestHandler) httphead.RequestHandler {
	return func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
		ctxt.Logger().Tracef(diag.REST, "rest.authMw mw before handler...")

		var token auth.Token
		var err diag.Error

		// look at auth header and validate token
		authz := req.Header.Get("Authorization")
		if len(authz) > 0 {
			parts := strings.Fields(authz)
			if len(parts) < 2 {
				err := diag.MakeError(
					types.IdmErrorInvalidArgument,
					"un authorized: unexpected authorization header format", nil)
				handleError(err, rw, ctxt)
				return
			}
			kind := auth.TokenKindJWT
			parts[0] = strings.ToLower(parts[0])

			if strings.HasPrefix(parts[0], auth.TokenKindSaml) {
				kind = auth.TokenKindSaml
			}

			validator, ok := h.tis[kind]
			if !ok {
				err := diag.MakeError(
					types.IdmErrorInvalidArgument,
					fmt.Sprintf("un authorized: unknown token type '%s'", parts[0]), nil)
				handleError(err, rw, ctxt)
				return
			}
			if kind == auth.TokenKindSaml {
				// base64 decode token
			}
			token, err = validator.ValidateToken(parts[1], ctxt)
			if err != nil {
				handleError(err, rw, ctxt)
				return
			}
		}

		authCtxt := httphead.NewAuthRequestContext(ctxt, token)
		handler(rw, req, authCtxt, params)
		ctxt.Logger().Tracef(diag.REST, "rest.authMw mw after handler...")
	}
}

func admin(handler httphead.RequestHandler) httphead.RequestHandler {
	return func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
		ctxt.Logger().Tracef(diag.REST, "rest.admin before handler...")

		authCtxt, ok := ctxt.(httphead.AuthRequestContext)
		if !ok || !authCtxt.IsAuthenticated() {
			err := diag.MakeError(types.IdmErrorInvalidArgument, "un authorized", nil)
			handleError(err, rw, ctxt)
			return
		}

		token := authCtxt.Token()
		// todo: proper, better impl - admin server role?
		if !token.Groups().Contains(ctxt.Tenant().String() + "\\" + "Administrators") {
			// un-authorized
			err := diag.MakeError(types.IdmErrorInvalidArgument, "un authorized", nil)

			handleError(err, rw, ctxt)
			return
		}
		handler(rw, req, authCtxt, params)
		ctxt.Logger().Tracef(diag.REST, "rest.authMw mw after handler...")
	}
}

func authUser(handler httphead.RequestHandler) httphead.RequestHandler {
	return func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
		ctxt.Logger().Tracef(diag.REST, "rest.admin before handler...")

		authCtxt, ok := ctxt.(httphead.AuthRequestContext)
		if !ok || !authCtxt.IsAuthenticated() {
			// todo: proper impl
			err := diag.MakeError(types.IdmErrorInvalidArgument, "un authorized", nil)
			handleError(err, rw, ctxt)
			return
		}

		handler(rw, req, authCtxt, params)
		ctxt.Logger().Tracef(diag.REST, "rest.authMw mw after handler...")
	}
}
