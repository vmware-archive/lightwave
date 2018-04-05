package oidc

import (
	"crypto/rsa"
	"encoding/json"
	"fmt"
	"net/http"
	"net/url"
	"sync/atomic"
	"time"

	jose "gopkg.in/square/go-jose.v2"
	"strings"
)

const (
	metadataEndpointSuffix = "/.well-known/openid-configuration"
)

type idpMetadata struct {
	ResponseTypes         []string `json:"response_types_supported"`
	JwksEndpoint          string   `json:"jwks_uri"`
	LogoutEndpoint        string   `json:"end_session_endpoint"`
	SubjectTypes          []string `json:"subject_types_supported"`
	SigningAlg            []string `json:"id_token_signing_alg_values_supported"`
	Issuer                string   `json:"issuer"`
	AuthorizationEndpoint string   `json:"authorization_endpoint"`
	TokenEndpoint         string   `json:"token_endpoint"`
}

type oidcClientImpl struct {
	metadata      *idpMetadata
	audience      string
	signers       atomic.Value
	retries       int
	retryInterval int
	config        ClientConfig
	httpClient    *http.Client

	logger Logger
}

func newClient(clientConfig ClientConfig, issuer, requestID string, logger Logger) (Client, error) {
	var err error

	if issuer == "" {
		return nil, OIDCInvalidArgError.MakeError("Issuer is required", nil)
	}

	if clientConfig == nil {
		clientConfig = NewDefaultClientConfig()
	}

	if logger == nil {
		logger = NewLogger()
		if err != nil {
			return nil, err
		}
	}

	client := &oidcClientImpl{
		audience:      clientConfig.ClientID(),
		retries:       clientConfig.Retries(),
		retryInterval: clientConfig.RetryIntervalMillis(),
		logger:        logger,
	}

	tr := httpTransportFromConfig(clientConfig.HTTPConfig())
	retryableTransport := NewRetryableTransportWrapper(*tr, client.retries, client.retryInterval, logger)
	client.httpClient = &http.Client{Transport: retryableTransport}

	metadata, err := getProviderMetadata(issuer, client.httpClient, requestID, logger)
	if err != nil {
		return nil, err
	}
	client.metadata = metadata

	keys, err := getSigners(client.httpClient, client.metadata.JwksEndpoint, requestID, logger)
	if err != nil {
		return nil, err
	}

	client.signers.Store(keys)

	return client, nil
}

func getProviderMetadata(issuer string, client *http.Client, requestID string, logger Logger) (*idpMetadata, error) {
	var err error
	uri := issuer + metadataEndpointSuffix
	PrintLog(logger, LogLevelInfo, "Getting Provider Metatadata: %s, reqId=%s", uri, requestID)

	req, err := http.NewRequest(http.MethodGet, uri, nil)
	if len(requestID) > 0 {
		req.Header.Add("X-Request-Id", requestID)
	}

	resp, err := client.Do(req)
	if err != nil {
		PrintLog(logger, LogLevelError, "Unable to retrieve oidc metadata. Error: '%v'", err)
		return nil, OIDCMetadataError.MakeError("Unable to retrieve oidc metadata.", err)
	}
	defer resp.Body.Close()

	var metadata idpMetadata
	jsonDecoder := json.NewDecoder(resp.Body)
	if err := jsonDecoder.Decode(&metadata); err != nil {
		PrintLog(logger, LogLevelError, "Unable to parse oidc metadata. Error: '%v'", err)
		return nil, OIDCMetadataError.MakeError("Unable to parse oidc metadata.", err)
	}

	if !strings.EqualFold(metadata.Issuer, issuer) {
		PrintLog(logger, LogLevelError,
			"Issuer does not match the configured issuer. Configured: '%s', Returned:'%s'",
			issuer, metadata.Issuer)

		return nil, OIDCMetadataError.MakeError(
			fmt.Sprintf("Issuer does not match the configured issuer. Configured: '%s', Returned:'%s'",
				issuer, metadata.Issuer), nil)
	}

	if len(metadata.AuthorizationEndpoint) <= 0 {
		PrintLog(logger, LogLevelError, "Invalid OIDC metadata: authorization endpoint is required")
		return nil, OIDCMetadataError.MakeError("Authorization endpoint is required", nil)
	}

	if len(metadata.JwksEndpoint) <= 0 {
		PrintLog(logger, LogLevelError, "Invalid OIDC metadata: Jwks endpoint is is required")
		return nil, OIDCMetadataError.MakeError("Jwks endpoint is is required", nil)
	}

	if len(metadata.SigningAlg) <= 0 || !contains(metadata.SigningAlg, string(jose.RS256)) {
		PrintLog(logger, LogLevelError, "Invalid OIDC metadata: RS256 signing algorithm is required")
		return nil, OIDCMetadataError.MakeError("RS256 signing algorithm is required", nil)
	}

	return &metadata, nil
}

func getSigners(client *http.Client, jwksEndpoint string, requestID string, logger Logger) (*jose.JSONWebKeySet, error) {
	var err error
	PrintLog(logger, LogLevelInfo, "Getting Signers: %s, reqId=%s", jwksEndpoint, requestID)

	req, err := http.NewRequest(http.MethodGet, jwksEndpoint, nil)
	if len(requestID) > 0 {
		req.Header.Add("X-Request-Id", requestID)
	}

	resp, err := client.Do(req)
	if err != nil {
		PrintLog(logger, LogLevelError, "Unable to retrieve oidc signer keys. Error: '%v'", err)
		return nil, OIDCJwksRetrievalError.MakeError("Unable to retrieve oidc signer keys.", err)
	}
	defer resp.Body.Close()

	var jwkSet jose.JSONWebKeySet
	jsonDecoder := json.NewDecoder(resp.Body)
	if err := jsonDecoder.Decode(&jwkSet); err != nil {
		PrintLog(logger, LogLevelError, "Unable to unmarshal oidc signer keys. Error: '%v'", err)
		return nil, OIDCJwksRetrievalError.MakeError("Unable to unmarshal oidc signer keys.", err)
	}

	return &jwkSet, nil
}

func (c *oidcClientImpl) WithConfig(config ClientConfig) Client {
	c.config = config
	return c
}

func (c *oidcClientImpl) Signers(fetch bool, requestID string) (IssuerSigners, error) {
	if fetch {
		keys, errSigners := getSigners(c.httpClient, c.metadata.JwksEndpoint, requestID, c.logger)
		if errSigners != nil {
			return nil, errSigners
		}

		c.signers.Store(keys)
	}

	return &signersImpl{signers: c.internalSigners()}, nil
}

func (c *oidcClientImpl) Issuer() string {
	return c.metadata.Issuer
}

func (c *oidcClientImpl) ClientID() string {
	return c.audience
}

func (c *oidcClientImpl) AcquireTokensByPassword(username string, password string,
	scope string, requestID string) (Tokens, error) {
	if len(username) <= 0 {
		return nil, OIDCInvalidArgError.MakeError("username must be provided", nil)
	}
	if len(password) <= 0 {
		return nil, OIDCInvalidArgError.MakeError("password must be provided", nil)
	}

	urlVals := url.Values{
		"grant_type": {"password"},
		"username":   {username},
		"password":   {password},
	}
	return c.acquireTokens(urlVals, scope, requestID)
}

func (c *oidcClientImpl) AcquireTokensBySolutionUserCert(certSubjectDn string, privateKey *rsa.PrivateKey,
	scope string, requestID string) (Tokens, error) {

	if len(certSubjectDn) <= 0 {
		return nil, OIDCInvalidArgError.MakeError("certSubjectDn must be provided", nil)
	}
	if privateKey == nil {
		return nil, OIDCInvalidArgError.MakeError("privateKey must be provided", nil)
	}

	slnAuth, err := newSolutionUserAssertion(certSubjectDn, c.metadata.TokenEndpoint)
	if err != nil {
		if c.logger != nil {
			PrintLog(c.logger, LogLevelError, "Unable to create solution user assertion.")
		}
		return nil, OIDCGetTokenError.MakeError("Unable to create solution user assertion.", err)
	}
	assertion, err := signSolutionUserAssertion(slnAuth, privateKey)
	if err != nil {
		if c.logger != nil {
			PrintLog(c.logger, LogLevelError, "Unable to sign solution user assertion.")
		}
		return nil, OIDCGetTokenError.MakeError("Unable to sign solution user assertion.", err)
	}

	urlVals := url.Values{
		"grant_type":              {"urn:vmware:grant_type:solution_user_credentials"},
		"solution_user_assertion": {assertion},
	}
	return c.acquireTokens(urlVals, scope, requestID)
}

func (c *oidcClientImpl) acquireTokens(auth url.Values, scope string, requestID string) (Tokens, error) {

	if len(scope) > 0 {
		auth.Add("scope", scope)
	}
	if len(c.audience) > 0 {
		auth.Add("client_id", c.audience)
	}
	if len(requestID) > 0 {
		auth.Add("correlation_id", requestID)
	}

	resp, err := c.httpClient.PostForm(c.metadata.TokenEndpoint, auth)
	if resp != nil {
		defer resp.Body.Close()
	}
	if err != nil {
		PrintLog(c.logger, LogLevelError, "Unable to obtain oidc token. Error: '%v'", err)
		return nil, OIDCGetTokenError.MakeError("Unable to obtain oidc token.", err)
	}

	if resp == nil {
		PrintLog(c.logger, LogLevelError, "Unable to obtain oidc token. HTTP response is nil.")
		return nil, OIDCGetTokenError.MakeError("Unable to obtain oidc token. HTTP response is nil.", nil)
	}

	if resp.StatusCode != http.StatusOK {
		PrintLog(c.logger, LogLevelError,
			"Unable to obtain oidc token. HttpStatusCode='%v' HttpStatus='%v'",
			resp.StatusCode, resp.Status)
		var errResponse jsonErrorResponse
		jsonDecoder := json.NewDecoder(resp.Body)
		if err := jsonDecoder.Decode(&errResponse); err != nil {
			PrintLog(c.logger, LogLevelError, "Unable to parse oidc error response. Error: '%v'", err)
			return nil,
				OIDCGetTokenError.MakeError(
					fmt.Sprintf(
						"Unable to obtain oidc token. HttpStatusCode='%v' HTTPStatus='%v'",
						resp.StatusCode, resp.Status), err)
		}

		err = errResponse.makeError()
		PrintLog(c.logger, LogLevelError, "Unable to obtain oidc token with '%v'", err)
		return nil, err
	}

	var jsonRsp tokenResponse
	jsonDecoder := json.NewDecoder(resp.Body)
	if err := jsonDecoder.Decode(&jsonRsp); err != nil {
		PrintLog(c.logger, LogLevelError, "Unable to parse oidc token response. Error: '%v'", err)
		return nil, OIDCGetTokenError.MakeError("Unable to parse oidc token response.", err)
	}

	if len(jsonRsp.AccessToken) <= 0 {
		PrintLog(c.logger, LogLevelError, "Access token is empty")
		return nil, OIDCGetTokenError.MakeError("Access token is empty.", err)
	}

	if jsonRsp.ExpiresIn <= 0 {
		PrintLog(c.logger, LogLevelError, "Unexpected token expiration in '%v'", jsonRsp.ExpiresIn)
		return nil, OIDCGetTokenError.MakeError(fmt.Sprintf("Unaxpected token expiration in '%v'", jsonRsp.ExpiresIn), nil)
	}

	tokens := &tokensImpl{
		AccessTokenField:  jsonRsp.AccessToken,
		RefreshTokenField: jsonRsp.RefreshToken,
		IDTokenField:      jsonRsp.IDToken,
		TokenTypeField:    jsonRsp.TokenType,
		ExpiresInField:    jsonRsp.ExpiresIn,
	}

	return tokens, nil
}

func (c *oidcClientImpl) internalSigners() *jose.JSONWebKeySet {
	return c.signers.Load().(*jose.JSONWebKeySet)
}

type tokenResponse struct {
	IDToken      string `json:"id_token"`
	AccessToken  string `json:"access_token"`
	TokenType    string `json:"token_type"`
	ExpiresIn    int    `json:"expires_in"`
	RefreshToken string `json:"refresh_token"`
}

type solutionUserAssertion struct {
	TokenClass string   `json:"token_class"`
	TokenType  string   `json:"token_type"`
	ID         string   `json:"jti"`
	Issuer     string   `json:"iss"`
	Subject    string   `json:"sub"`
	Audience   []string `json:"aud"`
	IssueTime  int64    `json:"iat"`
}

func newSolutionUserAssertion(subjectDn string, tokenEndpoint string) (*solutionUserAssertion, error) {
	randid, err := generateRandom(32)
	if err != nil {
		return nil, OIDCRandomGenError.MakeError(
			fmt.Sprintf("Unable to generate random '%v'", err), err)
	}
	return &solutionUserAssertion{
		TokenClass: "solution_user_assertion",
		TokenType:  BearerTokenType,
		ID:         randid,
		Issuer:     subjectDn,
		Subject:    subjectDn,
		Audience:   []string{tokenEndpoint},
		IssueTime:  time.Now().UTC().Unix(),
	}, nil
}

func signSolutionUserAssertion(assertion *solutionUserAssertion, privateKey *rsa.PrivateKey) (string, error) {
	signer, err := jose.NewSigner(
		jose.SigningKey{
			Algorithm: jose.RS256,
			Key:       privateKey,
		}, nil)
	if err != nil {
		return "", OIDCJWSError.MakeError(
			fmt.Sprintf("Unable to create signer '%v'", err), err)
	}

	payload, err := encodeJSON(assertion)
	if err != nil {
		return "", OIDCJsonError.MakeError(
			fmt.Sprintf("Unable to serialize json '%v'", err), err)
	}
	jws, err := signer.Sign([]byte(payload))
	if err != nil {
		return "", OIDCJWSError.MakeError(
			fmt.Sprintf("Unable to sign '%v'", err), err)
	}

	serialized, err := jws.CompactSerialize()
	if err != nil {
		return "", OIDCJWSError.MakeError(
			fmt.Sprintf("Unable to serialize JWS '%v'", err), err)
	}

	return serialized, nil
}
