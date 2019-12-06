package sts

import (
	"context"
	"crypto/tls"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"sync"

	"github.com/julienschmidt/httprouter"
	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/httphead"
	"github.com/vmware/lightwave/sts/internal/pkg/idm"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/oidc"
	"github.com/vmware/lightwave/sts/internal/pkg/oidc/protocol"
	"github.com/vmware/lightwave/sts/internal/pkg/rest"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
	"github.com/vmware/lightwave/sts/web/static"
)

type StsServer interface {
	Serve() error
	Close() error
}

func NewServer(cfg config.InstanceConfig, sc idmconfig.Configurator,
	auth idm.Authenticator, logger diag.Logger) (StsServer, error) {

	var err diag.Error

	sts := &stsServerImpl{cfg: cfg, wg: &sync.WaitGroup{}, r: httprouter.New(), log: logger, sc: sc, auth: auth}
	sts.sessions, err = session.NewAuthSessionMap(cfg, sc, logger)
	if err != nil {
		return nil, err
	}

	sts.ti, err = protocol.NewTokenIssuer(sc, auth, sts.AuthServicesPath, logger)
	if err != nil {
		return nil, err
	}

	for _, h := range cfg.EnabledHttpHeads() {
		httpHead, err := sts.getHead(h)
		if err != nil {
			return nil, err
		}
		middlewares := chainMiddlewares(httpHead.Middlewares()...)
		for path, ep := range httpHead.Endpoints() {
			tenantizedPath := fmt.Sprintf("%s%s", sts.GetServerRelativePath(":"+httphead.TenantParam), path)
			logger.Infof(diag.SERVER, "registering endpoint: %v", tenantizedPath)
			ep.Iterate(func(inf httphead.EndpointInfo) {
				handler := inf.Handler()
				methods := inf.Methods()
				f := func(rw http.ResponseWriter, req *http.Request, params httprouter.Params) {
					ctxt := req.Context().Value(reqCtxtKey).(diag.RequestContext)
					h := middlewares(handler)
					h(rw, req, ctxt, params)
				}
				for _, method := range methods {
					sts.r.Handle(method, tenantizedPath, f)
				}
			})
		}
	}

	sts.statics = static.StaticContentHandler()
	sts.r.GET(
		fmt.Sprintf("%s%s", sts.GetServerRelativePath(":"+httphead.TenantParam), httphead.ResourcesPath+"/*filepath"),
		func(w http.ResponseWriter, req *http.Request, ps httprouter.Params) {
			req.URL.Path = "/" + ps.ByName("filepath")
			sts.statics.ServeHTTP(w, req)
		})

	srv := &http.Server{
		Addr: fmt.Sprintf(":%v", cfg.Port()),
		TLSConfig: &tls.Config{
			Certificates: []tls.Certificate{tls.Certificate{
				Certificate: [][]byte{cfg.TLSCert().Raw},
				PrivateKey:  cfg.TLSPrivateKey(),
				Leaf:        cfg.TLSCert(),
			},
			},
		},
		ErrorLog: log.New(&logWrapper{logger}, "", 0),
		Handler:  sts,
	}

	sts.srv = srv

	return sts, nil
}

type stsServerImpl struct {
	srv      *http.Server
	cfg      config.InstanceConfig
	sc       idmconfig.Configurator
	auth     idm.Authenticator
	sessions session.AuthSessionsMap
	ti       protocol.Issuer
	wg       *sync.WaitGroup
	r        *httprouter.Router
	statics  http.Handler
	log      diag.Logger
}

type logWrapper struct {
	log diag.Logger
}

func (w *logWrapper) Write(p []byte) (n int, err error) {
	w.log.Errorf(diag.SERVER, string(p))
	return len(p), nil
}

func (s *stsServerImpl) Serve() error {

	var srvErr error
	var shutdownErr error
	s.wg.Add(2)

	stopChan := make(chan os.Signal)
	cancelChan := make(chan bool)
	defer close(stopChan)
	defer close(cancelChan)

	signal.Notify(stopChan, os.Interrupt)

	go func() {
		defer s.wg.Done()
		srvErr = s.srv.ListenAndServeTLS("", "")
		if srvErr == http.ErrServerClosed {
			srvErr = nil
		}
		select {
		case cancelChan <- true:
		default:
		}
	}()

	go func() {
		defer s.wg.Done()
		select {
		case <-stopChan:
			shutdownErr = s.shutdown()
			if shutdownErr != nil {
				s.log.Errorf(diag.SERVER, "Server shutdown failed: %v", shutdownErr)
			}
		case <-cancelChan:
		}
	}()

	s.wg.Wait()
	if srvErr != nil {
		return srvErr
	} else {
		return shutdownErr
	}
}

func (s *stsServerImpl) shutdown() error {
	// shut down gracefully, but wait no longer than ShutdownWait before halting
	ctx, cancel := context.WithTimeout(context.Background(), s.cfg.ShutdownWait())
	defer cancel()
	err := s.srv.Shutdown(ctx)
	return err
}

func (s *stsServerImpl) Close() error {
	var err error
	if s.srv != nil {
		if err = s.srv.Close(); err == nil {
			s.srv = nil
		}
	}
	return err
}

func (s *stsServerImpl) ServeHTTP(rw http.ResponseWriter, req *http.Request) {
	handler, params, _ := s.r.Lookup(req.Method, req.URL.Path)
	if handler == nil {
		s.log.Errorf(diag.SERVER, "Unable to match url path '%s'", req.URL.Path)
		s.serve404(rw)
		return
	}
	tenant := params.ByName(httphead.TenantParam)

	ti, err := s.sc.LookupTenant(diag.TenantIDFromString(tenant), s)
	if err != nil {
		// todo: distinguish non-existent tenant vs other internal errors and serve different pages
		s.log.Errorf(diag.SERVER, "Unable to validate tenant '%s': %v", tenant, err)
		s.serve404(rw)
		return
	}
	reqID := req.Header.Get(httphead.RequestIDHeader)
	if len(reqID) <= 0 {
		reqID = "<gen guid>"
	}
	reqCtx := diag.NewRequestContext(ti.Name(), reqID, s.log)
	ctxt := req.Context()
	ctxt = context.WithValue(ctxt, reqCtxtKey, reqCtx)
	req = req.WithContext(ctxt)

	handler(rw, req, params)
}

func (s *stsServerImpl) GetServerRelativePath(tenant diag.TenantID) string {
	return fmt.Sprintf("/%s", tenant)
}

func (s *stsServerImpl) AuthServicesPath(tenant diag.TenantID) string {
	return s.GetServerRelativePath(tenant) + httphead.IssuerPath
}

func (s *stsServerImpl) Logger() diag.Logger { return s.log }

func (s *stsServerImpl) Tenant() diag.TenantID { return diag.NoneTenantID }

func (s *stsServerImpl) RequestID() string { return "" }

func (s *stsServerImpl) serve404(rw http.ResponseWriter) {
	// todo:serve better 404
	rw.WriteHeader(404)
	rw.Write([]byte(http.StatusText(http.StatusNotFound)))
}

type requestContextKey string

const (
	reqCtxtKey requestContextKey = "requestContext"
)

func (s *stsServerImpl) getHead(h config.HttpHead) (httphead.HttpHeadConfig, error) {
	if h == config.OIDCHead {
		return oidc.New(s, s.sessions, s.cfg, s.sc, s.auth, s.ti, s.log)
	} else if h == config.RESTHead {
		return rest.New(s, s.cfg, s.sc, []auth.Validator{s.ti}, s.log)
	} else {
		return nil, fmt.Errorf("unsupported head: %v", h)
	}
}

func chainMiddlewares(mws ...httphead.Middleware) httphead.Middleware {
	return func(handler httphead.RequestHandler) httphead.RequestHandler {
		return func(rw http.ResponseWriter, req *http.Request, ctxt diag.RequestContext, params httphead.PathParameters) {
			call := handler
			for i := len(mws) - 1; i >= 0; i-- {
				call = mws[i](call)
			}
			call(rw, req, ctxt, params)
		}
	}
}
