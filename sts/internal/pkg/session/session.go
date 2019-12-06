package session

import (
	"crypto/rand"
	"encoding/base64"
	"fmt"
	"sync"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	idmconfig "github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type SessionID string

const (
	NoneSessionID SessionID = SessionID("")
)

func (s SessionID) String() string            { return string(s) }
func SessionIDFromString(id string) SessionID { return SessionID(id) }

type ClientIDs map[types.ClientID]struct{}

func (s ClientIDs) Len() int {
	if s == nil {
		return 0
	}
	return len(s)
}

func (s ClientIDs) Contains(v types.ClientID) bool {
	if s == nil {
		return false
	}
	_, ok := s[v]
	return ok
}

type ClientIDsIteratorFunc func(v types.ClientID) diag.Error

func (s ClientIDs) Iterate(f ClientIDsIteratorFunc) diag.Error {
	if s == nil {
		return nil
	}
	for k := range s {
		err := f(k)
		if err != nil {
			return err
		}
	}

	return nil
}

type AuthSession interface {
	ID() SessionID
	UserIdentity() types.UserID
	LoginMethod() types.LoginMethod
	Clients() ClientIDs
}

type AuthSessionsMap interface {
	NewSession(t diag.TenantID,
		userID types.UserID, loginMethod types.LoginMethod, client types.ClientID,
		ctxt diag.RequestContext) (AuthSession, diag.Error)

	SessionCookieName(t diag.TenantID, ctxt diag.RequestContext) (string, diag.Error)

	Get(t diag.TenantID, key SessionID, ctxt diag.RequestContext) (AuthSession, diag.Error)
	Remove(t diag.TenantID, key SessionID, ctxt diag.RequestContext) diag.Error
	Update(t diag.TenantID, key SessionID, loginMethod *types.LoginMethod, client types.ClientID, ctxt diag.RequestContext) (AuthSession, diag.Error)
}

func NewAuthSessionMap(cfg config.InstanceConfig, sc idmconfig.StsConfig, logger diag.Logger) (AuthSessionsMap, diag.Error) {
	return &sessionsMapImpl{
		lock: &sync.RWMutex{},
		ts:   make(map[diag.TenantID]*tenantMap, 100),
	}, nil
}

var exists = struct{}{}

func (s *sessionsMapImpl) NewSession(
	t diag.TenantID, userID types.UserID, loginMethod types.LoginMethod,
	client types.ClientID, ctxt diag.RequestContext) (AuthSession, diag.Error) {
	id, err := generateRandom(randomLength)
	if err != nil {
		ctxt.Logger().Errorf(diag.SESSION, "Failed to generate session id: %v", err)
		return nil, diag.MakeError(GenericSessionError, "Failed to generate session id", err)
	}

	session := &sessionImpl{
		id:          SessionID(id),
		userID:      userID,
		loginMethod: loginMethod,
		clients:     map[types.ClientID]struct{}{client: exists},
	}

	s.lock.Lock()
	tm, found := s.ts[t]
	if !found || tm == nil {
		tm = &tenantMap{
			lock: &sync.RWMutex{},
			as:   make(map[SessionID]AuthSession, 100),
		}
		s.ts[t] = tm
		tm.as[session.id] = session
		s.lock.Unlock()
		return session, nil
	} else {
		s.lock.Unlock()
	}

	tm.lock.Lock()
	tm.as[session.id] = session
	tm.lock.Unlock()
	return session, nil
}

func (s *sessionsMapImpl) SessionCookieName(t diag.TenantID, ctxt diag.RequestContext) (string, diag.Error) {
	// todo: consider cookie based on issuer ?
	return fmt.Sprintf("sts-%s", string(t)), nil
}

// TODO: switch to bigcache or freecache

func (s *sessionsMapImpl) Get(t diag.TenantID, key SessionID, ctxt diag.RequestContext) (AuthSession, diag.Error) {
	s.lock.RLock()
	tm, found := s.ts[t]
	s.lock.RUnlock()
	if !found || tm == nil {
		return nil, nil
	}
	tm.lock.RLock()
	as, ok := tm.as[key]
	tm.lock.RUnlock()
	if !ok || as == nil {
		return nil, nil
	}
	return as, nil
}

func (s *sessionsMapImpl) Update(
	t diag.TenantID, key SessionID, loginMethod *types.LoginMethod, client types.ClientID, ctxt diag.RequestContext) (AuthSession, diag.Error) {
	s.lock.RLock()
	tm, found := s.ts[t]
	s.lock.RUnlock()
	if !found || tm == nil {
		return nil, nil
	}

	tm.lock.Lock()
	currentSession, ok := tm.as[key]
	if !ok || currentSession == nil {
		return nil, nil
	}
	updateSession(currentSession, loginMethod, client)
	tm.lock.Unlock()
	return currentSession, nil
}

func (s *sessionsMapImpl) Remove(t diag.TenantID, key SessionID, ctxt diag.RequestContext) diag.Error {
	s.lock.RLock()
	tm, found := s.ts[t]
	s.lock.RUnlock()
	if !found || tm == nil {
		return nil
	}
	tm.lock.Lock()
	_, ok := tm.as[key]
	if ok {
		delete(tm.as, key)
	}
	tm.lock.Unlock()
	return nil
}

func updateSession(currentSession AuthSession, loginMethod *types.LoginMethod, client types.ClientID) {
	simpl, _ := currentSession.(*sessionImpl)
	if loginMethod != nil {
		simpl.loginMethod = *loginMethod
	}
	simpl.clients[client] = exists
}

type sessionsMapImpl struct {
	lock *sync.RWMutex
	ts   map[diag.TenantID]*tenantMap
}

type tenantMap struct {
	lock *sync.RWMutex
	as   map[SessionID]AuthSession
}

func (s *sessionImpl) ID() SessionID                  { return s.id }
func (s *sessionImpl) UserIdentity() types.UserID     { return s.userID }
func (s *sessionImpl) LoginMethod() types.LoginMethod { return s.loginMethod }
func (s *sessionImpl) Clients() ClientIDs             { return s.clients }

type sessionImpl struct {
	id          SessionID
	userID      types.UserID
	loginMethod types.LoginMethod
	clients     ClientIDs
}

const randomLength = 32

func generateRandom(n int) (string, error) {
	b := make([]byte, n)
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}

/*
func getSha(str string) string {
	sha := sha256.Sum256(([]byte)(str))
	return base64.RawURLEncoding.EncodeToString(sha[:])
}*/
