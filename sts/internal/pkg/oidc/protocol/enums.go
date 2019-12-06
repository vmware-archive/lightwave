package protocol

// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html
// oidc:"enum:vals=none code token id_token,set;err=OidcErrorUnsupportedResponseType"
type ResponseType uint8

// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html
// https://openid.net/specs/oauth-v2-form-post-response-mode-1_0.html
// oidc:"enum:vals=query fragment form_post"
type ResponseMode uint8

// https://openid.net/specs/openid-connect-core-1_0.html#AuthRequest
// oidc:"enum:vals=none login,set"
type Prompt uint8

// oauth: https://tools.ietf.org/pdf/rfc6749.pdf #section 3.3 Access Token Scope
// openidconnect: https://openid.net/specs/openid-connect-core-1_0.html#ScopeClaims
// oidc:"enum:vals=openid offline_access,set;err=OidcErrorInvalidScope"
type Scope string

// https://tools.ietf.org/pdf/rfc6749.pdf
// oidc:"enum:vals=password refresh_token authorization_code client_credentials;err=OidcErrorUnsupportedGrantType"
type GrantType uint8

// https://openid.net/specs/openid-connect-core-1_0.html#Authentication
// OpenID Connect "response_type" Values

func (rts ResponseTypeSet) ImplicitFlow() bool {
	okToken := rts.Contains(ResponseTypeToken)
	okIDToken := rts.Contains(ResponseTypeIdToken)

	l := rts.Len()
	if l == 1 && (okToken || okIDToken) {
		return true
	}

	if l == 2 && (okToken && okIDToken) {
		return true
	}

	return false
}

func (rts ResponseTypeSet) AuthzFlow() bool {
	if rts.Len() != 1 {
		return false
	}
	ok := rts.Contains(ResponseTypeCode)
	return ok

}
func (rts ResponseTypeSet) HybridFlow() bool {
	l := rts.Len()
	containsCode := rts.Contains(ResponseTypeCode)
	if !containsCode || l <= 1 {
		return false
	}

	okToken := rts.Contains(ResponseTypeToken)
	okIDToken := rts.Contains(ResponseTypeIdToken)

	if l == 2 && (okToken || okIDToken) {
		return true
	}

	if l == 3 && (okToken && okIDToken) {
		return true
	}
	return false
}

// https://openid.net/specs/oauth-v2-multiple-response-types-1_0.html#none
func (rts ResponseTypeSet) NoneResponse() bool {
	if rts.Len() != 1 {
		return false
	}
	ok := rts.Contains(ResponseTypeNone)
	return ok
}

func (ss ScopeSet) Oidc() bool {
	return ss.Contains(ScopeOpenid)
}
