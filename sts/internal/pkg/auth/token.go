package auth

import (
	"fmt"
	"time"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type StringSetIteratorFunc func(v string) diag.Error

type StringSet interface {
	fmt.Stringer
	Len() uint
	Contains(val string) bool
	Iterate(f StringSetIteratorFunc) diag.Error
}

// Token generic token
type Token interface {
	ID() string
	Issuer() string
	Subject() string
	Audience() StringSet

	IssuedAt() time.Time  // iat
	ExpiresAt() time.Time // exp
	ValidAt() time.Time   // nbf

	Groups() StringSet

	Claim(name string) []string // other claims

	TokenType() string // bearer,hotk_pk, saml2-bearer, saml2-hotk_pk etc.
}

type Validator interface {
	ValidateToken(token string, ctxt diag.RequestContext) (Token, diag.Error)
	TokenKind() string // such as saml2, or jwt
}

const (
	TokenKindJWT  = "jwt"
	TokenKindSaml = "saml2"
)
