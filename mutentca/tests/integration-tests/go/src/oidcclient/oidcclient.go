// Copyright (c) 2018 VMware, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache License, Version 2.0 (the "License").
// You may not use this product except in compliance with the License.
//
// This product may include a number of subcomponents with separate copyright notices and
// license terms. Your use of these subcomponents is subject to the terms and conditions
// of the subcomponent's license, as noted in the LICENSE file.

package oidcclient

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"strings"
)

const tokenScope string = "openid offline_access"

// OIDCClient is client for OIDC
type OIDCClient struct {
	httpClient *http.Client
	logger     *log.Logger
	Endpoint   string
	Options    *OIDCClientOptions
}

// OIDCClientOptions is OIDC client options
type OIDCClientOptions struct {
	// Whether or not to ignore any TLS errors when talking to photon,
	// false by default.
	IgnoreCertificate bool

	// List of root CA's to use for server validation
	// nil by default.
	RootCAs *x509.CertPool

	// The scope values to use when requesting tokens
	TokenScope string
}

// NewOIDCClient creates an instance of OIDCClient
func NewOIDCClient(endpoint string, options *OIDCClientOptions, logger *log.Logger) (c *OIDCClient, err error) {
	if logger == nil {
		logger = log.New(ioutil.Discard, "", log.LstdFlags)
	}

	if options == nil {
		err = errors.New("cannot build OIDC client with empty options")
		return
	}

	options, err = buildOptions(options)
	if err != nil {
		return
	}

	tr := &http.Transport{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: options.IgnoreCertificate,
			RootCAs:            options.RootCAs},
	}

	c = &OIDCClient{
		httpClient: &http.Client{Transport: tr},
		logger:     logger,
		Endpoint:   strings.TrimRight(endpoint, "/"),
		Options:    options,
	}

	return
}

func buildOptions(options *OIDCClientOptions) (*OIDCClientOptions, error) {
	if options == nil {
		return nil, errors.New("cannot build options with nil argument")
	}
	ret := &OIDCClientOptions{
		TokenScope: tokenScope,
	}

	ret.IgnoreCertificate = options.IgnoreCertificate

	if options.RootCAs != nil {
		ret.RootCAs = options.RootCAs
	}

	if options.TokenScope != "" {
		ret.TokenScope = options.TokenScope
	}

	return ret, nil
}

func (client *OIDCClient) buildURL(path string) (url string) {
	return fmt.Sprintf("%s%s", client.Endpoint, path)
}

func (client *OIDCClient) setTransport(tr http.RoundTripper) {
	client.httpClient.Transport = tr
}

// Toke request helpers

const tokenPath string = "/openidconnect/token/%s"
const passwordGrantFormatString = "grant_type=password&username=%s&password=%s&scope=%s"

// OIDCTokenResponse is the response for OIDC request
type OIDCTokenResponse struct {
	AccessToken  string `json:"access_token"`
	ExpiresIn    int    `json:"expires_in"`
	RefreshToken string `json:"refresh_token,omitempty"`
	IDToken      string `json:"id_token"`
	TokenType    string `json:"token_type"`
}

// GetTokenByPasswordGrant gets OIDC tokens by password
func (client *OIDCClient) GetTokenByPasswordGrant(tenant string, username string, password string) (tokens *OIDCTokenResponse, err error) {
	username = url.QueryEscape(username)
	password = url.QueryEscape(password)
	scope := url.QueryEscape(client.Options.TokenScope)
	body := fmt.Sprintf(passwordGrantFormatString, username, password, scope)

	return client.getToken(body, tenant)
}

func (client *OIDCClient) getToken(body string, tenant string) (tokens *OIDCTokenResponse, err error) {
	request, err := http.NewRequest("POST", client.buildURL(fmt.Sprintf(tokenPath, tenant)), strings.NewReader(body))
	if err != nil {
		return nil, err
	}
	request.Header.Add("Content-Type", "application/x-www-form-urlencoded")

	resp, err := client.httpClient.Do(request)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	err = client.checkResponse(resp)
	if err != nil {
		return nil, err
	}

	tokens = &OIDCTokenResponse{}
	err = json.NewDecoder(resp.Body).Decode(tokens)
	if err != nil {
		return nil, err
	}

	return
}

// OIDCError is OIDC error
type OIDCError struct {
	Code    string `json:"error"`
	Message string `json:"error_description"`
}

func (e OIDCError) Error() string {
	return fmt.Sprintf("%v: %v", e.Code, e.Message)
}

func (client *OIDCClient) checkResponse(response *http.Response) (err error) {
	if response.StatusCode/100 == 2 {
		return
	}
	respBody, readErr := ioutil.ReadAll(response.Body)
	if readErr != nil {
		return fmt.Errorf(
			"Status: %v, Body: %v [%v]", response.Status, string(respBody[:]), readErr)
	}
	var oidcErr OIDCError
	err = json.Unmarshal(respBody, &oidcErr)
	if err != nil || oidcErr.Code == "" {
		return fmt.Errorf(
			"Status: %v, Body: %v [%v]", response.Status, string(respBody[:]), readErr)
	}
	return oidcErr
}
