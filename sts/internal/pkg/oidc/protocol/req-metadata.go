package protocol

import jose "gopkg.in/square/go-jose.v2"

// oidc:"marshal:enc=j"
type metadata struct {
	responseTypes         []string // oidc:"marshal:name=response_types_supported"
	jwksEndpoint          string   // oidc:"marshal:name=jwks_uri"
	logoutEndpoint        string   // oidc:"marshal:name=end_session_endpoint"
	subjectTypes          []string // oidc:"marshal:name=subject_types_supported"
	signingAlg            []string // oidc:"marshal:name=id_token_signing_alg_values_supported"
	issuer                string   // oidc:"marshal:name=issuer"
	authorizationEndpoint string   // oidc:"marshal:name=authorization_endpoint"
	tokenEndpoint         string   // oidc:"marshal:name=token_endpoint"

	// https://openid.net/specs/openid-connect-frontchannel-1_0.html#OPLogout

	fcLogoutSupported        bool // oidc:"marshal:name=frontchannel_logout_supported;omitempty"
	fcLogoutSessionSupported bool // oidc:"marshal:name=frontchannel_logout_session_supported;omitempty"
}

// todo: theoretically https://openid.net/specs/openid-connect-session-1_0.html#OPMetadata
// requires check_session_iframe REQUIRED.
// right now we however only implement front-channel logout:
// https://openid.net/specs/openid-connect-frontchannel-1_0.html

func (r *metadata) State() string { return "" }

func NewMetadataResponse(issuer, jwksEP, authzEP, tokenEP, logoutEP string) OidcResponse {
	return &metadata{
		jwksEndpoint:          jwksEP,
		logoutEndpoint:        logoutEP,
		issuer:                issuer,
		authorizationEndpoint: authzEP,
		tokenEndpoint:         tokenEP,
		//https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#Combinations
		//https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#none
		responseTypes: []string{
			ResponseTypeNone.String(), ResponseTypeCode.String(), ResponseTypeToken.String(), ResponseTypeIdToken.String(),
			ResponseTypeCode.String() + " " + ResponseTypeToken.String(),
			ResponseTypeCode.String() + " " + ResponseTypeIdToken.String(),
			ResponseTypeIdToken.String() + " " + ResponseTypeToken.String(),
			ResponseTypeCode.String() + " " + ResponseTypeToken.String() + " " + ResponseTypeIdToken.String(),
		},
		subjectTypes:             []string{"public"},
		signingAlg:               []string{string(jose.RS256)},
		fcLogoutSupported:        true,
		fcLogoutSessionSupported: true,
	}
}
