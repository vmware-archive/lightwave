package main

import (
	"flag"
	"fmt"
	"errors"
	"os"
	"github.com/vmware/lightwave/vmidentity/goclients/oidc"
	"io/ioutil"
	"gopkg.in/yaml.v2"
)

var configPath = flag.String("config", "", "Path to file containing config in yaml format")

type Config struct {
	// DN of solution user to test with
	SolutionUserDn string `yaml:"SolutionUserDn"`
	// SolutionUser username
	SolutionUserName string `yaml:"SolutionUserName"`
	// Issuer to get token from
	Issuer string `yaml:"Issuer1"`
	// Username to use to get token
	Username string `yaml:"Username"`
	// Password to use to get token
	Password string `yaml:"Password"`
}

func setup() (*Config, error) {
	flag.Parse()
	if *configPath == "" {
		fmt.Printf("Usage: go run -config <path to config in yaml>\n")
		return nil, errors.New("Invalid Arguments")
	}

	yamlFile, err := ioutil.ReadFile(*configPath)
	if err != nil {
		return nil, err
	}

	var config Config

	err = yaml.Unmarshal(yamlFile, &config)
	if err != nil {
		return nil, err
	}

	return &config, nil

}

// Sets up a logger for OIDC and registers some callbacks
func getLogger() oidc.Logger {
	errorFunc := func(format string, args ...interface{}) {
		msg := fmt.Sprintf("[Error - SampleOidc] ")
		fmt.Printf(msg + format, args...)
	}

	infoFunc := func(format string, args ...interface{}) {
		msg := fmt.Sprintf("[Info - SampleOidc] ")
		fmt.Printf(msg + format, args...)
	}

	return oidc.NewLoggerBuilder().Register(oidc.LogLevelInfo, infoFunc).Register(oidc.LogLevelError, errorFunc).Build()
}

func main() {
	config, err := setup()
	if err != nil {
		fmt.Printf("Error setting up test suite: %+v", err)
		os.Exit(1)
	}

	// scope to use for requesting tokens
	scope := "openid at_groups rs_admin_server rs_vmdir"
	// General request ID to track this sample flow in logs
	reqID := "sample-flows"

	// First, Build OIDC Client. You will need a Client config and Logger to pass in

	// To create client config, pass in an issuer. The other arguments are configurable, but set to default values
	// To configure other parameters, see the With*() methods defined in the interface
	// For this sample, we will create an HTTP config, set skipSSLVerify to true, and pass it to the config. For basic
	// uses, NewClientConfig(issuer) is fine.
	httpConfig := oidc.NewHTTPClientConfig()
	httpConfig.SkipCertValidationField = true
	clientConfig := oidc.NewClientConfigBuilder().WithHTTPConfig(httpConfig).Build()

	// Set a logger to be used by client. The logger will have callbacks which will need to be registered for certain
	// log levels. If no callback is registered for a log level, nothing will be logged or printed.
	logger := getLogger()

	// Create OIDC client using config, handle errors
	client, err := oidc.NewOidcClient(config.Issuer, clientConfig, logger, reqID)
	if err != nil {
		fmt.Printf("Error occurred when building client: %+v", err)
		return
	}

	tokens, err := client.AcquireTokensByPassword(config.Username, config.Password, scope, reqID)
	if err != nil {
		fmt.Printf("Error occurred when getting tokens by username/password: %+v", err)
		return
	}

	fmt.Printf("[AcquireTokensByPassword]\nAccess Token: %s\nID Token: %s\nToken Type: %s\nExpires In: %d\n",
		tokens.AccessToken(), tokens.IDToken(), tokens.TokenType(), tokens.ExpiresIn())

	// Parse and get validated Access Token from result
	signers, err := client.Signers(true, reqID)
	if err != nil {
		fmt.Printf("Error occurred when getting signing cert: %+v", err)
		return
	}
	accessToken, err := oidc.ParseAndValidateAccessToken(tokens.AccessToken(), client.Issuer(), "", signers, logger)
	if err != nil {
		fmt.Printf("Error occurred when parsing and validating Access Token: %+v", err)
		return
	}

	// Access Token valid, get Tenant and groups
	tenant, ok:= accessToken.Claim("tenant")
	if !ok {
		fmt.Printf("Error occurred when parsing Access Token for tenant: %+v", err)
		return
	}
	if s, ok := tenant.(string); ok {
		fmt.Printf("\tTenant: %s\n", s)
	}

	groups, ok:= accessToken.Claim("groups")
	if !ok {
		fmt.Printf("Error occurred when parsing Access Token for groups: %+v", err)
		return
	}
	if s, ok := groups.([]string); ok {
		fmt.Printf("\tGroups: %+v\n", s)
	}

}
