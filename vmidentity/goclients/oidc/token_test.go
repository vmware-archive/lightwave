package oidc

import (
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"testing"
	"time"
)

func TestValidateExpiration(t *testing.T) {
	claims := make(map[string]interface{})

	claims[ClaimExpiration] = time.Now()
	claims[ClaimIssuedAt] = time.Now()
	err := validateExpiration(&claims, defaultClockToleranceSecs)
	assert.Nil(t, err, "Claim should be valid: %+v", err)

	claims[ClaimExpiration] = time.Now().Add(-time.Second * (defaultClockToleranceSecs + 1))
	claims[ClaimIssuedAt] = time.Now()
	err = validateExpiration(&claims, defaultClockToleranceSecs)
	if assert.NotNil(t, err, "Claim should be expired") {
		assert.Contains(t, err.Error(), OIDCTokenExpiredError.Name(), "Error should be TokenExpired: %+v", err)
	}

	claims[ClaimExpiration] = time.Now()
	claims[ClaimIssuedAt] = time.Now().Add(time.Second * (defaultClockToleranceSecs + 1))
	err = validateExpiration(&claims, defaultClockToleranceSecs)
	if assert.NotNil(t, err, "Claim should be invalid") {
		assert.Contains(t, err.Error(), OIDCTokenNotYetValidError.Name(), "Error should be NotYetValid: %+v", err)
	}
}

func TestValidateAudienceClaim(t *testing.T) {
	claims := make(map[string]interface{})
	assert.NotNil(t, validateAudienceClaim(&claims), "Claim is empty, error expected")

	claims[ClaimAudience] = "1"
	assert.Nil(t, validateAudienceClaim(&claims), "Audience is valid")

	claims[ClaimAudience] = 0
	err := validateAudienceClaim(&claims)
	if assert.NotNil(t, err, "Audience should be String") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name(), "InvalidToken Error expected")
	}

	claims[ClaimAudience] = []int{1}
	err = validateAudienceClaim(&claims)
	if assert.NotNil(t, err, "Audience should be String") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name(), "InvalidToken Error expected")
	}

	claims[ClaimAudience] = []string{""}
	assert.Nil(t, validateAudienceClaim(&claims), "Audience is valid")

	claims[ClaimAudience] = ""
	assert.Nil(t, validateAudienceClaim(&claims), "Audience is valid")

	claims[ClaimAudience] = []string{"1", "2", "3"}
	assert.Nil(t, validateAudienceClaim(&claims), "Audience is valid")
}

func TestParseSignedToken(t *testing.T) {
	token := "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJodHRwczovL2V4YW1wbGUuY29tIiwic3ViIjoic3ViSjEiLCJuYmYiOjE1MjE3NzMwOTgsImV4cCI6MTUyMTc3NjY5OCwiaWF0IjoxNTIxNzczMDk4LCJqdGkiOiJpZDEyMzQ1NiJ9.wf8E82CGm_saE8gGnoz7aX1COSzkc5ZbcO2H7xJSgIQ"
	jwt, err := parseSignedToken(token, "issuer1", nil, defaultClockToleranceSecs)
	assert.Nil(t, jwt, "Token should be nil")
	if assert.NotNil(t, err, "Error is expected when using unsupported signature algo") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name())
	}

	jwt, err = parseSignedToken("", "issuer1", nil, defaultClockToleranceSecs)
	assert.Nil(t, jwt, "Token should be nil")
	if assert.NotNil(t, err, "Error is expected when using bad token") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name())
	}

	// get a token and keyset
	client, err := buildOidcClient(config.Issuer1, "", "", NewLogger())
	require.Nil(t, err, "Error when building OIDC client: %+v", err)
	tok, err := client.AcquireTokensByPassword(config.Username, config.Password, scope, "")
	require.Nil(t, err, "Error when getting tokens by password: %+v", err)
	strTok := tok.AccessToken()
	signers, err := client.Signers(true, "")
	require.Nil(t, err, "Error when getting signers: %+v", err)
	s, ok := signers.(*signersImpl)
	require.True(t, ok, "Error when getting keyset from IssuerSigners")
	require.NotNil(t, s, "KeySet is nil")

	jwt, err = parseSignedToken(strTok, client.Issuer(), s.signers, defaultClockToleranceSecs)
	assert.Nil(t, err, "No error expected: %+v", err)
	assert.NotNil(t, jwt, "Token should not be nil")

	jwt, err = parseSignedToken("  "+strTok+" ", client.Issuer(), s.signers, defaultClockToleranceSecs)
	assert.Nil(t, err, "No error expected: %+v", err)
	assert.NotNil(t, jwt, "Token should not be nil")

	jwt, err = parseSignedToken(strTok+"a", client.Issuer(), s.signers, defaultClockToleranceSecs)
	if assert.NotNil(t, err, "Error expected when parsing malformed token") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidSignatureError.Name(), "Wrong Error code: %+v", err)
	}
	assert.Nil(t, jwt, "No token expected")

	jwt, err = parseSignedToken(strTok, "wrongIssuer", s.signers, defaultClockToleranceSecs)
	if assert.NotNil(t, err, "Error expected when token from incorrect issuer") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name(), "Wrong Error code: %+v", err)
	}
	assert.Nil(t, jwt, "No token expected")
}

func TestParseTenantInToken(t *testing.T) {
	testInvalidTokens(t, "")
	testInvalidTokens(t, "  .  .  ")
	testInvalidTokens(t, "..")
	testInvalidTokens(t, "aaaaa.bbbbb.cccc")
	// Multi tenant claim
	testInvalidTenantInToken(t, "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJPbmxpbmUgSldUIEJ1aWxkZXIiLCJpYXQiOjE1MjI0MzczNDgsImV4cCI6MTU1Mzk3MzM0OCwiYXVkIjoid3d3LmV4YW1wbGUuY29tIiwic3ViIjoiZXhhbXBsZSIsIkdpdmVuTmFtZSI6IkpvaG5ueSIsInRlbmFudCI6WyJ0ZW5hbnQxIiwidGVuYW50MiJdLCJFbWFpbCI6Impyb2NrZXRAZXhhbXBsZS5jb20iLCJSb2xlIjpbIk1hbmFnZXIiLCJQcm9qZWN0IEFkbWluaXN0cmF0b3IiXX0.GYpTmNd1jLsBZhRBmMvaHNbqW-nZJuUWsXPbBt6j2Bc")
	// No tenant claim
	testInvalidTenantInToken(t, "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJPbmxpbmUgSldUIEJ1aWxkZXIiLCJpYXQiOjE1MjI0MzczNDgsImV4cCI6MTU1Mzk3MzM0OCwiYXVkIjoid3d3LmV4YW1wbGUuY29tIiwic3ViIjoiZXhhbXBsZSIsIkdpdmVuTmFtZSI6IkpvaG5ueSIsIkVtYWlsIjoianJvY2tldEBleGFtcGxlLmNvbSIsIlJvbGUiOlsiTWFuYWdlciIsIlByb2plY3QgQWRtaW5pc3RyYXRvciJdfQ.ID96zlMw6_JtRz91QZSPg9iwQRiNRXEjo43WYOCpt-8")
	// Valid tenant claim
	tenant, err := ParseTenantInToken("eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJPbmxpbmUgSldUIEJ1aWxkZXIiLCJpYXQiOjE1MjI0MzczNDgsImV4cCI6MTU1Mzk3MzM0OCwiYXVkIjoid3d3LmV4YW1wbGUuY29tIiwic3ViIjoiZXhhbXBsZSIsIkdpdmVuTmFtZSI6IkpvaG5ueSIsIkVtYWlsIjoianJvY2tldEBleGFtcGxlLmNvbSIsIlJvbGUiOlsiTWFuYWdlciIsIlByb2plY3QgQWRtaW5pc3RyYXRvciJdLCJ0ZW5hbnQiOiJ0ZXN0LXRlbmFudCJ9.-7E3nnZOARCWbgFdxlYYw61Atddnc-3MxwvTdNQqc5I")
	require.Nil(t, err, "No error is expected when parsing valid token")
	assert.Equal(t, "test-tenant", tenant, "Tenant in claim does not match")
}

func testInvalidTokens(t *testing.T, token string) {
	testInvalidTenantInToken(t, token)

	jwt, err := parseSignedToken(token, "issuer1", nil, defaultClockToleranceSecs)
	assert.Nil(t, jwt, "Token should be nil")
	if assert.NotNil(t, err, "Error is expected when using bad token") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name())
	}
}

func testInvalidTenantInToken(t *testing.T, token string) {
	tenant, err := ParseTenantInToken(token)
	assert.Empty(t, tenant, "Tenant should not be returned")
	if assert.NotNil(t, err, "Error is expected when using bad token") {
		assert.Contains(t, err.Error(), OIDCTokenInvalidError.Name())
	}
}
