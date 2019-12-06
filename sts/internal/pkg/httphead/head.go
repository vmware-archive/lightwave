package httphead

import (
	"net/http"

	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type PathParameters interface {
	ByName(param string) string
}

const (
	TenantParam     = "tenant"
	RequestIDHeader = "x-Request-ID"

	HTTPGET             = "GET"
	HTTPPOST            = "POST"
	HTTPPUT             = "PUT"
	JSONContentType     = "application/json;charset=UTF-8"
	HTMLContentType     = "text/html;charset=UTF-8"
	ContentTypeHeader   = "Content-Type"
	ContentLengthHeader = "Content-Length"

	ResourcesPath = "/resources"
	IssuerPath    = "/idp"
	ConfigPath    = "/config"
)

type RequestHandler func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params PathParameters)
type Middleware func(RequestHandler) RequestHandler

type EndpointInfo interface {
	Methods() []string
	Handler() RequestHandler
}

type EndpointInfoFunc func(ep EndpointInfo)

type EndpointInfos interface {
	Iterate(f EndpointInfoFunc)
}

func NewEP(handler RequestHandler, method ...string) EndpointInfo {
	return &endpointInfoImpl{method: method, handler: handler}
}

func NewEndpointInfo(handler RequestHandler, method ...string) EndpointInfos {
	return endpointInfosImpl([]EndpointInfo{NewEP(handler, method...)})
}

func NewEndpointInfos(ep ...EndpointInfo) EndpointInfos {
	return endpointInfosImpl(ep)
}

type HttpHeadConfig interface {
	Endpoints() map[string]EndpointInfos
	Middlewares() []Middleware
}

type endpointInfoImpl struct {
	method  []string
	handler RequestHandler
}

func (ep *endpointInfoImpl) Methods() []string {
	return ep.method
}
func (ep *endpointInfoImpl) Handler() RequestHandler {
	return ep.handler
}

type endpointInfosImpl []EndpointInfo

func (i endpointInfosImpl) Iterate(f EndpointInfoFunc) {
	for _, ep := range i {
		f(ep)
	}
}

type HostingServer interface {
	GetServerRelativePath(tenant diag.TenantID) string
	AuthServicesPath(tenant diag.TenantID) string
}

type AuthRequestContext interface {
	diag.RequestContext

	Token() auth.Token // token of the request
	IsAuthenticated() bool
}

type authRequestContextImpl struct {
	ctxt  diag.RequestContext
	token auth.Token
}

func (a *authRequestContextImpl) Tenant() diag.TenantID { return a.ctxt.Tenant() }
func (a *authRequestContextImpl) RequestID() string     { return a.ctxt.RequestID() }
func (a *authRequestContextImpl) Logger() diag.Logger   { return a.ctxt.Logger() }
func (a *authRequestContextImpl) Token() auth.Token     { return a.token }
func (a *authRequestContextImpl) IsAuthenticated() bool { return a.token != nil }

func NewAuthRequestContext(ctxt diag.RequestContext, token auth.Token) AuthRequestContext {
	return &authRequestContextImpl{ctxt: ctxt, token: token}
}
