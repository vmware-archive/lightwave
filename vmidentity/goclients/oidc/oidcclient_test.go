package oidc

import (
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"errors"
	"flag"
	"fmt"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"gopkg.in/yaml.v2"
	"io/ioutil"
	"os"
	"strings"
	"testing"
	"time"
)

const (
	scope = "openid at_groups rs_admin_server rs_vmdir"
	// Stress test config
	threads = 20
	runs    = 40
)

type TestConfig struct {
	// DN of solution user to test with
	SolutionUserDn string `yaml:"SolutionUserDn"`
	// SolutionUser username
	SolutionUserName string `yaml:"SolutionUserName"`
	// Client Id to test client with
	ClientID string `yaml:"ClientID"`
	// Issuer to get token from
	Issuer1 string `yaml:"Issuer1"`
	// Second Issuer to get token from. Must be different from issuer 1 and oidc metadata issuer
	Issuer2 string `yaml:"Issuer2"`
	// Issuer endpoint with tenant that does not exist
	FakeIssuer string `yaml:"FakeIssuer"`
	// Username to use to get token
	Username string `yaml:"Username"`
	// Password to use to get token
	Password string `yaml:"Password"`
	// Private Key of Solution User
	PrivateKey *rsa.PrivateKey
	// No-op Logger to used for testing
	NoopLogger Logger
}

var privateKeyPath = flag.String("privateKey", "", "Path to file containing private key")
var configPath = flag.String("config", "", "Path to file containing config in yaml format")
var runStress = flag.Bool("stress", false, "Run the stress test")

var config *TestConfig

func TestMain(m *testing.M) {
	err := setup()
	if err != nil {
		fmt.Printf("Error setting up test suite: %+v", err)
		os.Exit(1)
	}

	os.Exit(m.Run())
}

func setup() error {
	var err error

	flag.Parse()
	if *privateKeyPath == "" || *configPath == "" {
		fmt.Printf("Usage: go test -privateKey <path to private key> -config <path to config in yaml>\n")
		return errors.New("Invalid Arguments")
	}

	config, err = loadConfig(*configPath)
	if err != nil {
		return err
	}

	config.PrivateKey, err = getPrivateKey(*privateKeyPath)
	if err != nil {
		fmt.Printf("Unable to get private key from pem: '%v'\n", err)
		return err
	}

	config.NoopLogger = NewLogger()

	return err
}

func TestAcquireTokensByPassword(t *testing.T) {
	reqID := "Test-AcquireTokensByPassword"
	logger := getLogger(reqID)

	oidcClient, err := buildOidcClient(config.Issuer1, config.ClientID, "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	checkAcquireTokensByPassword(t, oidcClient, config.ClientID, "", config.Username, config.Password, logger)

	oidcClient, err = buildOidcClient(config.Issuer1, "", "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	checkAcquireTokensByPassword(t, oidcClient, "", "", config.Username, config.Password, logger)
}

func checkAcquireTokensByPassword(t *testing.T, oidcClient Client, clientID, reqID, username, password string, logger Logger) {
	tokens, err := oidcClient.AcquireTokensByPassword(username, password, scope, reqID)
	require.Nil(t, err, "Error in acquiring tokens by password: %+v", err)
	checkToken(t, tokens)

	idToken, err := BuildIDToken(oidcClient, tokens, reqID, logger)
	require.Nil(t, err, "Error building ID Token: %+v", err)
	checkIDToken(t, idToken, config.Username, BearerTokenType, clientID, oidcClient.Issuer())

	signers, _ := oidcClient.Signers(false, reqID)
	accessToken, err := ParseAndValidateAccessToken(tokens.AccessToken(), oidcClient.Issuer(), clientID, signers, logger)
	require.Nil(t, err, "Error in parsing access token: %+v", err)
	checkAccessToken(t, accessToken, config.Username, BearerTokenType, clientID, oidcClient.Issuer())
}

func TestAcquireTokensInvalidCreds(t *testing.T) {
	oidcClient, err := buildOidcClient(config.Issuer1, "", "", config.NoopLogger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	tokens, err := oidcClient.AcquireTokensByPassword("abcdef", "xyz", scope, "")
	assert.NotNil(t, err, nil, "error should be returned")
	assert.Nil(t, tokens, nil, "tokens should be nil")
}

func TestAcquireTokensBySolutionUserCert(t *testing.T) {
	reqID := "TestAcquireTokensBySolutionUserCert"
	logger := getLogger(reqID)

	oidcClient, err := buildOidcClient(config.Issuer1, "", "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	checkAcquireTokensBySolutionUser(t, oidcClient, "", "", logger)

	oidcClient, err = buildOidcClient(config.Issuer2, config.ClientID, "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	checkAcquireTokensBySolutionUser(t, oidcClient, config.ClientID, "", logger)
}

func checkAcquireTokensBySolutionUser(t *testing.T, oidcClient Client, clientID, reqID string, logger Logger) {
	tokens, err := oidcClient.AcquireTokensBySolutionUserCert(config.SolutionUserDn, config.PrivateKey, scope, "")
	require.Nil(t, err, "Error in AcquiringTokens: %+v", err)
	checkToken(t, tokens)

	idToken, err := BuildIDToken(oidcClient, tokens, reqID, logger)
	require.Nil(t, err, "Error in Building ID Token: %+v", err)
	checkIDToken(t, idToken, config.SolutionUserName, HOKTokenType, clientID, oidcClient.Issuer())

	signers, _ := oidcClient.Signers(false, reqID)
	accessToken, err := ParseAndValidateAccessToken(
		tokens.AccessToken(), oidcClient.Issuer(), clientID, signers, logger)
	require.Nil(t, err, "Error in parsing access token: %+v", err)
	checkAccessToken(t, accessToken, config.SolutionUserName, HOKTokenType, clientID, oidcClient.Issuer())
}

func TestAcquireTokensSolutionUserInvalidCreds(t *testing.T) {
	oidcClient, err := buildOidcClient(config.Issuer1, "", "", config.NoopLogger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	tokens, err := oidcClient.AcquireTokensBySolutionUserCert(config.SolutionUserDn+"1", config.PrivateKey, scope, "")
	assert.NotNil(t, err, nil, "error should be returned")
	assert.Nil(t, tokens, nil, "tokens should be nil")
}

func TestMultipleCertsValidation(t *testing.T) {
	reqID := "TestMultipleCertsValidation"
	logger := getLogger(reqID)

	oidcClient, err := buildOidcClient(config.Issuer1, "", "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	oidcClient2, err := buildOidcClient(config.Issuer2, "", "", logger)
	if err != nil {
		t.Fatalf("Error in building Oidc Client")
	}

	tokens, err := oidcClient.AcquireTokensByPassword(config.Username, config.Password, scope, "")
	require.Nil(t, err, "Error in AcquiringTokens: %+v", err)
	checkToken(t, tokens)

	signers, _ := oidcClient.Signers(false, "")

	tokens2, err := oidcClient2.AcquireTokensBySolutionUserCert(config.SolutionUserDn, config.PrivateKey, scope, "")
	require.Nil(t, err, "Error in AcquiringTokens: %+v", err)
	checkToken(t, tokens2)

	signers2, _ := oidcClient2.Signers(false, "")
	combinedSigners := signers.Combine(signers2)

	accessToken1CombinedSigners, err := ParseAndValidateAccessToken(
		tokens.AccessToken(), oidcClient.Issuer(), "", combinedSigners, logger)
	require.Nil(t, err, "Error in ParsingAccessTokens: %+v", err)
	assert.NotNil(t, accessToken1CombinedSigners, "parsed access token check")
	checkAccessToken(t, accessToken1CombinedSigners, config.Username, BearerTokenType, "", oidcClient.Issuer())

	accessToken1Signer2, err := ParseAndValidateAccessToken(
		tokens.AccessToken(), oidcClient.Issuer(), "", signers2, config.NoopLogger)
	assert.NotNil(t, err, "Error expected when parsing token with different signer", err)
	assert.Nil(t, accessToken1Signer2, "parsed access token with different signer should be nil")

	accessToken1Signer2, err = ParseAndValidateAccessToken(
		tokens.AccessToken(), oidcClient2.Issuer(), "", signers, config.NoopLogger)
	assert.NotNil(t, err, "Error expected when parsing token with different issuer", err)
	assert.Nil(t, accessToken1Signer2, "parsed access token with different issuer should be nil")

	accessToken2CombinedSigners, err := ParseAndValidateAccessToken(
		tokens2.AccessToken(), oidcClient2.Issuer(), "", combinedSigners, logger)
	require.Nil(t, err, "Error in ParsingAccessTokens: %+v", err)
	assert.NotNil(t, accessToken2CombinedSigners, "parsed access token check")
	checkAccessToken(t, accessToken2CombinedSigners, config.SolutionUserName, HOKTokenType, "", oidcClient2.Issuer())

	accessToken2Signers1, err := ParseAndValidateAccessToken(
		tokens2.AccessToken(), oidcClient2.Issuer(), "", signers, config.NoopLogger)
	assert.NotNil(t, err, "Error expected when parsing token with different signer", err)
	assert.Nil(t, accessToken2Signers1, "parsed access token with different signer should be nil")

	accessToken2Signers1, err = ParseAndValidateAccessToken(
		tokens2.AccessToken(), oidcClient.Issuer(), "", signers2, config.NoopLogger)
	assert.NotNil(t, err, "Error expected when parsing token with different issuer", err)
	assert.Nil(t, accessToken2Signers1, "parsed access token with different issuer should be nil")
}

func TestAcquireTokensConcurrent(t *testing.T) {
	if runStress == nil || *runStress == false {
		return
	}

	done := make(chan bool)
	for i := 0; i < threads; i++ {
		go func(t *testing.T, id int) {
			for run := 0; run < runs; run++ {
				errorFunc := func(format string, args ...interface{}) {
					msg := fmt.Sprintf("\n\t[%d-run%d] ", id, run)
					fmt.Printf(msg+format, args...)
				}
				logger := NewLoggerBuilder().Register(LogLevelError, errorFunc).Build()

				oidcClient, err := buildOidcClient(config.Issuer1, config.ClientID, "", logger)
				if err != nil {
					t.Errorf("[Thread %d] Error in building Oidc Client: %+v", id, err)
					continue
				}

				checkAcquireTokensByPassword(t, oidcClient, "", "",
					config.Username, config.Password, NewLoggerBuilder().Register(LogLevelError, errorFunc).Build())
			}
			done <- true
		}(t, i)
		go func(t *testing.T, id int) {
			for run := 0; run < runs; run++ {
				errorFunc := func(format string, args ...interface{}) {
					msg := fmt.Sprintf("\n\t[%d-run%d] ", id, run)
					fmt.Printf(msg+format, args...)
				}
				logger := NewLoggerBuilder().Register(LogLevelError, errorFunc).Build()

				oidcClient, err := buildOidcClient(config.Issuer2, config.ClientID, "", logger)
				if err != nil {
					t.Errorf("[Thread %d] Error in building Oidc Client: %+v", id, err)
					continue
				}

				checkAcquireTokensBySolutionUser(t, oidcClient, config.ClientID, "",
					NewLoggerBuilder().Register(LogLevelError, errorFunc).Build())
			}
			done <- true
		}(t, i)
	}

	for i := 0; i < threads; i++ {
		// CheckAcquireTokensByPassword
		<-done
		// CheckAcquireTokensBySolutionUser
		<-done
	}
}

func TestBuildClientIssuer(t *testing.T) {
	reqID := "Test-ClientBuildIssuers"
	logger := getLogger(reqID)

	oidcClient, err := buildOidcClient(config.Issuer2, "", reqID, logger)
	require.Nil(t, err, "Build OIDC client should succeed and take metadata's issuer, err: %+v", err)

	assert.NotEqual(t, oidcClient.Issuer(), config.Issuer2, "Issuer 2 in config should not equal metadata issuer")

	// Test Errors
	oidcClient, err = buildOidcClient(config.FakeIssuer, "", reqID, config.NoopLogger)
	if assert.NotNil(t, err, "Error expected with fake issuer") {
		assert.Contains(t, err.Error(), OIDCInvalidRequestError.Name(), "Invalid Request error expected")
	}
	assert.Nil(t, oidcClient, "Client returned on error should be nil")
}

func BuildIDToken(oidcClient Client, tokens Tokens, reqID string, logger Logger) (IDToken, error) {
	signers, _ := oidcClient.Signers(false, reqID)
	return ParseAndValidateIDToken(
		tokens.IDToken(), oidcClient.Issuer(), oidcClient.ClientID(), "", signers, logger)
}

func checkToken(t *testing.T, tokens Tokens) {
	assert.NotEmpty(t, tokens.AccessToken(), "AccessToken check")
	assert.NotEmpty(t, tokens.IDToken(), "IDToken check")
}

func checkIDToken(t *testing.T, idToken IDToken, expectedSubject, expectedTokenType, audience, issuer string) {
	assert.True(t, strings.EqualFold(idToken.Subject(), expectedSubject),
		"ID token subject (%s) should be equal to %s", idToken.Subject(), expectedSubject)
	assert.Equal(t, expectedTokenType, idToken.Type(), "ID token type check")

	aud, audExists := idToken.Audience()
	assert.True(t, audExists, "id token must have audience")
	if audience != "" {
		assert.True(t, contains(aud, audience), "id token audience (%+v) must have client ID %s", aud, audience)
	}

	assert.Equal(t, issuer, idToken.Issuer(), "Issuer should match from client")
	assert.True(t, idToken.Expiration().After(idToken.IssuedAt()), "Expiration should be after issued time")
	assert.True(t, idToken.Expiration().After(time.Now()), "Token should not be expired")

	tenant, err := idToken.Claim("tenant")
	if assert.NotNil(t, err, "Token should have tenant") {
		if s, ok := tenant.(string); ok {
			assert.NotEmpty(t, s, "Token should have tenant")
		}
	}
}

func checkAccessToken(t *testing.T, accessToken AccessToken, expectedSubject, expectedTokenType, audience, issuer string) {
	groupsList, hasGroups := accessToken.Groups()
	assert.True(t, hasGroups, "hasGroups check")
	assert.NotEmpty(t, groupsList, "groupsList check")
	assert.Equal(t, strings.EqualFold(accessToken.Subject(), expectedSubject), true,
		"access token token subject (%s) should be admin username %s", accessToken.Subject(), expectedSubject)
	assert.Equal(t, expectedTokenType, accessToken.Type(), "Access Token type check")

	aud, audExists := accessToken.Audience()
	assert.True(t, audExists, "access token must have audience")
	if audience != "" {
		assert.True(t, contains(aud, audience), "access token audience (%+v) must have client ID %s", aud, audience)
	}

	assert.Equal(t, issuer, accessToken.Issuer(), "Issuer should match from client")
	assert.True(t, accessToken.Expiration().After(accessToken.IssuedAt()), "Expiration should be after issued time")
	assert.True(t, accessToken.Expiration().After(time.Now()), "Token should not be expired")

	groups, ok := accessToken.Claim("groups")
	if assert.True(t, ok, "Group should be in token") {
		if s, ok := groups.([]string); ok {
			assert.NotEmpty(t, s, "Token should have groups")
		}
	}

	tenant, ok := accessToken.Claim("tenant")
	if assert.True(t, ok, "Token should have tenant") {
		if s, ok := tenant.(string); ok {
			assert.NotEmpty(t, s, "Token should have tenant")
		}
	}
}

func buildOidcClient(issuer, clientID, requestID string, logger Logger) (Client, error) {
	httpConfig := NewHTTPClientConfig()
	// skip cert validation for tests
	httpConfig.SkipCertValidationField = true

	clientcfg := NewClientConfigBuilder().WithClientID(clientID).WithHTTPConfig(httpConfig).Build()

	oidcClient, err := NewOidcClient(issuer, clientcfg, logger, requestID)
	if err != nil {
		logger.Errorf("Unable to create OIDC client error: '%v'", err)
	}

	return oidcClient, err
}

func getPrivateKey(filename string) (*rsa.PrivateKey, error) {
	privateKeyPem, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	keyDERBlock, _ := pem.Decode([]byte(privateKeyPem))
	if keyDERBlock != nil &&
		(keyDERBlock.Type == "PRIVATE KEY" || strings.HasSuffix(keyDERBlock.Type, " PRIVATE KEY")) {

	} else {
		return nil, fmt.Errorf("Unable to get privateKey from pem")
	}

	if key, err := x509.ParsePKCS1PrivateKey(keyDERBlock.Bytes); err == nil {
		return key, nil
	}
	if key, err := x509.ParsePKCS8PrivateKey(keyDERBlock.Bytes); err == nil {
		switch key := key.(type) {
		case *rsa.PrivateKey:
			return (*rsa.PrivateKey)(key), nil
		default:
			return nil, fmt.Errorf("Unable to get privateKey from pem")
		}
	}

	return nil, fmt.Errorf("Unable to get privateKey from pem")
}

func loadConfig(filename string) (*TestConfig, error) {
	yamlFile, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	var config TestConfig

	err = yaml.Unmarshal(yamlFile, &config)
	if err != nil {
		return nil, err
	}

	return &config, nil
}

func getLogger(reqID string) Logger {
	errorFunc := func(format string, args ...interface{}) {
		msg := fmt.Sprintf("[Error] req=%s, ", reqID)
		fmt.Printf(msg+format+"\n", args...)
	}

	return NewLoggerBuilder().Register(LogLevelError, errorFunc).Build()
}
