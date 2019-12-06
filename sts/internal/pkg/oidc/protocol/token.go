package protocol

import (
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/auth"
)

func (t *token) ID() string { return t.id }

func (t *token) Issuer() string           { return t.issuer }
func (t *token) Subject() string          { return t.subject }
func (t *token) Audience() auth.StringSet { return &t.audience }
func (t *token) Groups() auth.StringSet   { return &t.groups }

func (t *token) IssuedAt() time.Time  { return time.Unix(t.issuedAt, 0) }
func (t *token) ExpiresAt() time.Time { return time.Unix(t.expiration, 0) }
func (t *token) ValidAt() time.Time   { return time.Unix(t.issuedAt, 0) }
func (t *token) Claim(name string) []string {
	// TODO: implement
	return []string{}
}

func (t *token) TokenType() string { return t.tokenType } //bearer,hotk_pk, saml2-bearer, saml2-hotk_pk etc.

/*
func (t *token) String() string {
	sb := &strings.Builder{}
	EncodeJson(t, sb)
	return sb.String()
}*/

/*
type TokenBuilder interface {
	ID(id string)
	Issuer(iss string)
	Subject(sub string)
	Audience(aud StringSetRO)

	IssuedAt(t time.Time)  // iat
	ExpiresAt(t time.Time) // exp
	//ValidAt(t time.Time)   // nbf

	Groups(gs StringSetRO)

	Claim(name string, val []string) // other claims

	Build() (Token, diag.Error)
}

type tokenBuilderImpl struct {
	tok *token
}

func NewTokenBuilder() TokenBuilder {
	return &tokenBuilderImpl{tok: &token{}}
}

func (b *tokenBuilderImpl) ID(id string)             { b.tok.id = id }
func (b *tokenBuilderImpl) Issuer(iss string)        { b.tok.issuer = iss }
func (b *tokenBuilderImpl) Subject(sub string)       { b.tok.subject = sub }
func (b *tokenBuilderImpl) Audience(aud StringSetRO) { b.tok.audience = aud }

func (b *tokenBuilderImpl) IssuedAt(t time.Time)  { b.tok.issuedAt = t.Unix() }
func (b *tokenBuilderImpl) ExpiresAt(t time.Time) { b.tok.expiration = t.Unix() }

//ValidAt(t time.Time)   // nbf

func (b *tokenBuilderImpl) Groups(gs StringSetRO) { b.tok.groups = gs }

func (b *tokenBuilderImpl) Claim(name string, val []string) { // other claims
	// todo: implement
}

func (b *tokenBuilderImpl) Build() (Token, diag.Error) {
	// TODO validation
	return b.tok, nil
}
*/

// oidc:"marshal:enc=j;dec=j;err=OidcErrorInvalidRequest"
type token struct {
	subject    string        // oidc:"marshal:name=sub"
	issuer     string        // oidc:"marshal:name=iss"
	tokenClass string        // oidc:"marshal:name=lightwave_token_class" // lw specific
	tokenType  string        // oidc:"marshal:name=token_type"
	audience   stringSetImpl // oidc:"marshal:name=aud;omitempty"
	scope      ScopeSet      // oidc:"marshal:name=scope;omitempty"
	expiration int64         // oidc:"marshal:name=exp"
	issuedAt   int64         // oidc:"marshal:name=iat"
	id         string        // oidc:"marshal:name=jti"
	sid        string        // oidc:"marshal:name=sid;omitempty"
	nonce      string        // oidc:"marshal:name=nonce;omitempty"
	atHash     string        // oidc:"marshal:name=at_hash;omitempty"
	cHash      string        // oidc:"marshal:name=c_hash;omitempty"
	groups     stringSetImpl // oidc:"marshal:name=lightwave_groups;omitempty"
	tenant     string        // oidc:"marshal:name=lightwave_tenant"
}
