package protocol

import (
	"net/http"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

// OidcError represents an oidc error code
// it implements diag.HTTPStatusMappedErrorCode
type OidcError uint32

const (

	// OidcErrorInvalidRequest - rfc6749: invalid_request
	//    The request is missing a required parameter, includes an
	//    unsupported parameter value (other than grant type),
	//    repeats a parameter, includes multiple credentials,
	//    utilizes more than one mechanism for authenticating the
	//    client, or is otherwise malformed.
	OidcErrorInvalidRequest OidcError = OidcError(diag.OidcFacility) + iota + 1

	// OidcErrorUnauthorizedClient - rfc6749: unauthorized_client
	//    The client is not authorized to request an authorization
	//    code using this method.
	OidcErrorUnauthorizedClient //      OidcError = OidcError(diag.OidcFacility) + 0x002

	// OidcErrorAccessDenied - rfc6749: access_denied
	//    The resource owner or authorization server denied the
	//    request.
	OidcErrorAccessDenied //     OidcError = OidcError(diag.OidcFacility) + 0x003

	// OidcErrorUnsupportedResponseType - rfc6749: unsupported_response_type
	//    The authorization server does not support obtaining an
	//    authorization code using this method.
	OidcErrorUnsupportedResponseType //OidcError = OidcError(diag.OidcFacility) + 0x004

	// OidcErrorInvalidScope - rfc6749: invalid_scope
	//    The requested scope is invalid, unknown, or malformed.
	OidcErrorInvalidScope // OidcError = OidcError(diag.OidcFacility) + 0x005

	// OidcErrorServerError - rfc6749: server_error
	//    The authorization server encountered an unexpected
	//    condition that prevented it from fulfilling the request.
	OidcErrorServerError // OidcError = OidcError(diag.OidcFacility) + 0x006

	// OidcErrorTemporarilyUnavailable - rfc6749: temporarily_unavailable
	//    The authorization server is currently unable to handle
	//    the request due to a temporary overloading or maintenance
	//    of the server.
	OidcErrorTemporarilyUnavailable // OidcError = OidcError(diag.OidcFacility) + 0x007

	// OidcErrorInvalidClient - rfc6749: invalid_client
	//    Client authentication failed (e.g., unknown client, no
	//    client authentication included, or unsupported
	//    authentication method). The authorization server MAY
	//    return an HTTP 401 (Unauthorized) status code to indicate
	//    which HTTP authentication schemes are supported. If the
	//    client attempted to authenticate via the "Authorization"
	//    request header field, the authorization server MUST
	//    respond with an HTTP 401 (Unauthorized) status code and
	//    include the "WWW-Authenticate" response header field
	//    matching the authentication scheme used by the client.
	OidcErrorInvalidClient // OidcError = OidcError(diag.OidcFacility) + 0x008

	// OidcErrorInvalidGrant - rfc6749: invalid_grant
	//    The provided authorization grant (e.g., authorization
	//    code, resource owner credentials) or refresh token is
	//    invalid, expired, revoked, does not match the redirection
	//    URI used in the authorization request, or was issued to
	//    another client.
	OidcErrorInvalidGrant // OidcError = OidcError(diag.OidcFacility) + 0x009

	// OidcErrorUnsupportedGrantType - rfc6749: unsupported_grant_type
	//   The authorization grant type is not supported by the
	//   authorization server.
	OidcErrorUnsupportedGrantType // OidcError = OidcError(diag.OidcFacility) + 0x0010

	// OidcErrorInteractionRequired - https://openid.net/specs/openid-connect-core-1_0.html#AuthError: interaction_required
	//    The Authorization Server requires End-User interaction of some form to proceed.
	//    This error MAY be returned when the prompt parameter value in the Authentication Request
	//    is none, but the Authentication Request cannot be completed without displaying a user
	//    interface for End-User interaction.
	OidcErrorInteractionRequired // OidcError = OidcError(diag.OidcFacility) + 0x0011

	// OidcErrorLoginRequired - https://openid.net/specs/openid-connect-core-1_0.html#AuthError: login_required
	//    The Authorization Server requires End-User authentication. This error MAY be returned
	//    when the prompt parameter value in the Authentication Request is none,
	//    but the Authentication Request cannot be completed without displaying a user interface
	//    for End-User authentication.
	OidcErrorLoginRequired // OidcError = OidcError(diag.OidcFacility) + 0x0012

	//
	// additional more granular error codes they would map to ne of the error above
	//

	// OidcErrorEncodeError : server_error
	//   Error while serializing an object
	OidcErrorEncodeError

	// OidcErrorRandomGenError : server_error
	//   Error generating a random value
	OidcErrorRandomGenError

	// OidcErrorJWSError : server_error
	//	Json web signature error
	OidcErrorJWSError

	// OidcErrorStreamWriteError : server_error
	//	  Error writing marshalled data to stream
	OidcErrorStreamWriteError

	// OidcErrorTokenFormatError : invalid_request
	//	  Invalid format of JWT
	OidcErrorTokenFormatError

	// OidcErrorSignatureValidationError : invalid_request
	//	  Error validating JWS
	OidcErrorSignatureValidationError
)

func (c OidcError) Code() uint32 {
	return uint32(c)
}

func (c OidcError) Name() string {
	return errorText[c]
}

func (c OidcError) HttpStatus() int {
	return httpStatus[c]
}

func IsOidcError(err error) bool {
	return diag.IsErrorWithFacility(err, diag.OidcFacility)
}

func ensureOidcError(err error) diag.Error {
	if err == nil {
		return nil
	}
	if IsOidcError(err) {
		return err.(diag.Error)
	}
	return diag.MakeError(OidcErrorServerError, "Internal server error occurred", err)
}

const (
	invalidRequest          = "invalid_request"
	unauthorizedClient      = "unauthorized_client"
	accessDenied            = "access_denied"
	unsupportedResponseType = "unsupported_response_type"
	invalidScope            = "invalid_scope"
	serverError             = "server_error"
	temporarilyUnavailable  = "temporarily_unavailable"
	invalidClient           = "invalid_client"
	invalidGrant            = "invalid_grant"
	unsupportedGrantType    = "unsupported_grant_type"

	interactionRequired = "interaction_required"
	loginRequired       = "login_required"
)

var errorText = map[OidcError]string{
	OidcErrorInvalidRequest:          invalidRequest,
	OidcErrorUnauthorizedClient:      unauthorizedClient,
	OidcErrorAccessDenied:            accessDenied,
	OidcErrorUnsupportedResponseType: unsupportedResponseType,
	OidcErrorInvalidScope:            invalidScope,
	OidcErrorServerError:             serverError,
	OidcErrorTemporarilyUnavailable:  temporarilyUnavailable,
	OidcErrorInvalidClient:           invalidClient,
	OidcErrorInvalidGrant:            invalidGrant,
	OidcErrorUnsupportedGrantType:    unsupportedGrantType,
	OidcErrorInteractionRequired:     interactionRequired,
	OidcErrorLoginRequired:           loginRequired,

	OidcErrorEncodeError:              serverError,
	OidcErrorRandomGenError:           serverError,
	OidcErrorJWSError:                 serverError,
	OidcErrorStreamWriteError:         serverError,
	OidcErrorTokenFormatError:         invalidRequest,
	OidcErrorSignatureValidationError: invalidRequest,
}

var httpStatus = map[OidcError]int{
	OidcErrorInvalidRequest:          http.StatusBadRequest,
	OidcErrorUnauthorizedClient:      http.StatusUnauthorized,
	OidcErrorAccessDenied:            http.StatusUnauthorized,
	OidcErrorUnsupportedResponseType: http.StatusBadRequest,
	OidcErrorInvalidScope:            http.StatusBadRequest,
	OidcErrorServerError:             http.StatusInternalServerError,
	OidcErrorTemporarilyUnavailable:  http.StatusServiceUnavailable,
	OidcErrorInvalidClient:           http.StatusBadRequest,
	OidcErrorInvalidGrant:            http.StatusBadRequest,
	OidcErrorUnsupportedGrantType:    http.StatusBadRequest,
	OidcErrorInteractionRequired:     http.StatusBadRequest,
	OidcErrorLoginRequired:           http.StatusBadRequest,

	OidcErrorEncodeError:              http.StatusInternalServerError,
	OidcErrorRandomGenError:           http.StatusInternalServerError,
	OidcErrorJWSError:                 http.StatusInternalServerError,
	OidcErrorStreamWriteError:         http.StatusInternalServerError,
	OidcErrorTokenFormatError:         http.StatusBadRequest,
	OidcErrorSignatureValidationError: http.StatusBadRequest,
}
