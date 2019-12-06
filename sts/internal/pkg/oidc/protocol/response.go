package protocol

import (
	"io"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type MarshalFormat uint8

const (
	MarshalFormatJSON MarshalFormat = iota
	MarshalFormatQuery
	MarshalFormatFragment
	MarshalFormatForm
	MarshalFormatHTML
)

type Marshaler interface {
	Marshal(w io.Writer, format MarshalFormat) diag.Error
}

type OidcResponse interface {
	Marshaler
	State() string
}

type AuthzResponse interface {
	Code() string // must
	OidcResponse
}

type ErrorResponse interface {
	Name() string  // short name
	Error() string // description
	OidcResponse
}

type OidcTokenResponse interface {
	AccessToken() string
	TokenType() string
	IDToken() string
	RefreshToken() string
	ExpiresInSecs() int
	Scope() ScopeSet
	Code() string
	OidcResponse
}

type OidcLogoutResponse interface {
	OidcResponse
}

type OidcResponseCtxt interface {
	RedirectURI() *url.URL
	State() string
}

func NewErrorResponse(e diag.Error, ctxt OidcResponseCtxt) ErrorResponse {
	e = ensureOidcError(e)

	r := &errorResponse{
		errName:     e.Code().Name(),
		description: e.Error(),
		err:         e,
	}
	if ctxt != nil {
		r.state = ctxt.State()
		r.redirectURL = ctxt.RedirectURI()
	}
	return r
}

// oidc:"marshal:enc=qjf,url=redirectURL;err=OidcErrorInvalidRequest" // enc to query, form, json; no dec
type errorResponse struct {
	state       string // oidc:"marshal:name=state;omitempty"
	redirectURL *url.URL
	errName     string // oidc:"marshal:name=error"
	description string // oidc:"marshal:name=error_description;omitempty"
	err         diag.Error
}

func (r *errorResponse) Name() string  { return r.errName }
func (r *errorResponse) Error() string { return r.description }

func (r *errorResponse) State() string         { return r.state }
func (r *errorResponse) RedirectURI() *url.URL { return r.redirectURL }
