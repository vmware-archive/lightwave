package oidc

import (
	"crypto/rand"
	"encoding/base64"
	"fmt"
	"net/url"
	"sync"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/oidc/protocol"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
)

type AuthzCodeEntry interface {
	Code() string

	protocol.ClientInfo

	Scope() protocol.ScopeSet
	Nonce() string

	SessionID() session.SessionID
}

type AuthzCodeMap interface {
	Add(
		t diag.TenantID,
		clientInfo protocol.ClientInfo, scope protocol.ScopeSet,
		nonce string, session session.SessionID,
		ctxt diag.RequestContext) (AuthzCodeEntry, diag.Error)

	// will remove and return if found
	Remove(t diag.TenantID, key string, ctxt diag.RequestContext) (AuthzCodeEntry, diag.Error)
}

func NewAuthzCodeMap(cfg config.InstanceConfig, sc idmconfig.StsConfig, logger diag.Logger) (AuthzCodeMap, diag.Error) {
	return &authzCodeMapImpl{
		lock: &sync.RWMutex{},
		ts:   make(map[diag.TenantID]*tenantMap, 100),
	}, nil
}

// todo: switch to bigcache
func (m *authzCodeMapImpl) Add(
	t diag.TenantID,
	clientInfo protocol.ClientInfo, scope protocol.ScopeSet,
	nonce string, session session.SessionID,
	ctxt diag.RequestContext) (AuthzCodeEntry, diag.Error) {

	id, err := generateRandom()
	if err != nil {
		de := diag.MakeError(protocol.OidcErrorServerError,
			fmt.Sprintf("Failed to generate authz code: %v", err), err)
		ctxt.Logger().Errorf(diag.OIDC, de.Error())
		return nil, de
	}
	az := &authzCodeEntryImpl{
		code:        id,
		clientID:    clientInfo.ClientID(),
		redirectURI: clientInfo.RedirectURI(),
		scope:       scope,
		nonce:       nonce,
		sessionID:   session,
	}

	m.lock.Lock()
	tm, found := m.ts[t]
	if !found || tm == nil {
		tm = &tenantMap{
			lock: &sync.Mutex{},
			az:   make(map[string]AuthzCodeEntry, 100),
		}
		m.ts[t] = tm
		tm.az[az.Code()] = az
		m.lock.Unlock()
		return az, nil
	} else {
		m.lock.Unlock()
	}

	tm.lock.Lock()
	tm.az[az.Code()] = az
	tm.lock.Unlock()
	return az, nil
}

func (m *authzCodeMapImpl) Remove(t diag.TenantID, key string, ctxt diag.RequestContext) (AuthzCodeEntry, diag.Error) {
	m.lock.RLock()
	tm, found := m.ts[t]
	m.lock.RUnlock()
	if !found || tm == nil {
		return nil, nil
	}
	tm.lock.Lock()
	az, ok := tm.az[key]
	if ok {
		delete(tm.az, key)
	}
	tm.lock.Unlock()
	return az, nil
}

type authzCodeMapImpl struct {
	lock *sync.RWMutex
	ts   map[diag.TenantID]*tenantMap
}

type tenantMap struct {
	lock *sync.Mutex
	az   map[string]AuthzCodeEntry
}

func (az *authzCodeEntryImpl) Code() string                 { return az.code }
func (az *authzCodeEntryImpl) ClientID() string             { return az.clientID }
func (az *authzCodeEntryImpl) RedirectURI() *url.URL        { return az.redirectURI }
func (az *authzCodeEntryImpl) Scope() protocol.ScopeSet     { return az.scope }
func (az *authzCodeEntryImpl) Nonce() string                { return az.nonce }
func (az *authzCodeEntryImpl) SessionID() session.SessionID { return az.sessionID }

type authzCodeEntryImpl struct {
	code        string
	clientID    string
	redirectURI *url.URL
	scope       protocol.ScopeSet
	nonce       string
	sessionID   session.SessionID
}

const randomLength = 32

func generateRandom() (string, diag.Error) {
	b := make([]byte, randomLength)
	_, err := rand.Read(b)
	if err != nil {
		return "", diag.MakeError(
			protocol.OidcErrorRandomGenError, fmt.Sprintf("Failed to generate random: %v", err), err)
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}
