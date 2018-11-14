package httpclient

import (
	"oidcclient"
)

const (
	ignoreCertificate = true
	tokenScope        = "rs_mutentca at_groups openid"
)

// TokenOptions represents Tokens
type TokenOptions struct {
	AccessToken  string `json:"access_token"`
	ExpiresIn    int    `json:"expires_in"`
	RefreshToken string `json:"refresh_token,omitempty"`
	IDToken      string `json:"id_token"`
	TokenType    string `json:"token_type"`
}

// GetTokenByPassword gets tokens using username and password
func GetTokenByPassword(endpoint, tenant, username, password string) (*TokenOptions, error) {
	oidcClient, err := buildOIDCClient(endpoint)
	if err != nil {
		return nil, err
	}

	tokenResponse, err := oidcClient.GetTokenByPasswordGrant(tenant, username, password)
	if err != nil {
		return nil, err
	}

	return toTokenOptions(tokenResponse), nil
}

func buildOIDCClient(authEndPoint string) (client *oidcclient.OIDCClient, err error) {
	options := &oidcclient.OIDCClientOptions{
		IgnoreCertificate: ignoreCertificate,
		RootCAs:           nil,
		TokenScope:        tokenScope,
	}

	client, err = oidcclient.NewOIDCClient(authEndPoint, options, nil)

	return
}

func toTokenOptions(response *oidcclient.OIDCTokenResponse) *TokenOptions {
	return &TokenOptions{
		AccessToken:  response.AccessToken,
		ExpiresIn:    response.ExpiresIn,
		RefreshToken: response.RefreshToken,
		IDToken:      response.IDToken,
		TokenType:    response.TokenType,
	}
}
