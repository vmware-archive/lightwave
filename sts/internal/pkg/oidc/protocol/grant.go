package protocol

// Grant - rfc6749: defines grant-type
type Grant interface {
	Type() GrantType
}

// PasswordGrant - rfc6749: Resource Owner Password Credentials Grant
type PasswordGrant interface {
	// Grant - rfc6749: grant_type - REQUIRED. Value MUST be set to "password".
	Grant
	// UserName - rfc6749: REQUIRED. The resource owner username.
	UserName() string
	// Password - rfc6749: REQUIRED. The resource owner password.
	Password() string
}

// AuthzCodeGrant - rfc6749: Authorization Code Grant
type AuthzCodeGrant interface {
	// Grant - rfc6749: grant_type - REQUIRED. Value MUST be set to "authorization_code".
	Grant
	// Code - rfc6749: REQUIRED. The authorization code received from the authorization server.
	Code() string
}

// RefreshTokenGrant - rfc6749: Refreshing an Access Token
type RefreshTokenGrant interface {
	// Grant - rfc6749: grant_type - REQUIRED. Value MUST be set to "refresh_token".
	Grant
	// RefreshToken - rfc6749: REQUIRED. The refresh token issued to the client.
	RefreshToken() string
}

// ClientCredentialsGrant - rfc6749: Client Credentials Grant
type ClientCredentialsGrant interface {
	// Grant - rfc6749: grant_type - REQUIRED. Value MUST be set to "client_credentials".
	Grant
}

// ------------------------
// impl follows rfc6749:

// oidc:"marshal:dec=q"
type pwdGrantImpl struct {
	username string // oidc:"marshal:name=username"
	password string // oidc:"marshal:name=password"
}

func (pg *pwdGrantImpl) Type() GrantType {
	return GrantTypePassword
}

func (pg *pwdGrantImpl) UserName() string {
	if pg == nil {
		return ""
	}
	return pg.username
}
func (pg *pwdGrantImpl) Password() string {
	if pg == nil {
		return ""
	}
	return pg.password
}

// oidc:"marshal:dec=q"
type authzCodeGrantImpl struct {
	code string // oidc:"marshal:name=code"
}

func (az *authzCodeGrantImpl) Type() GrantType {
	return GrantTypeAuthorizationCode
}

func (az *authzCodeGrantImpl) Code() string { return az.code }

// oidc:"marshal:dec=q"
type refreshTokGrantImpl struct {
	refreshToken string // oidc:"marshal:name=refresh_token"
}

func (rt *refreshTokGrantImpl) Type() GrantType {
	return GrantTypeRefreshToken
}

func (rt *refreshTokGrantImpl) RefreshToken() string { return rt.refreshToken }
