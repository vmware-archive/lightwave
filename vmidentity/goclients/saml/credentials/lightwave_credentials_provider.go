package credentials

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"time"

	"crypto/rsa"
	"crypto/x509"
	"encoding/base64"
	"encoding/pem"
	"github.com/aws/aws-sdk-go/aws"
	awscreds "github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/sts"
	"github.com/beevik/etree"
	"strings"
)

const (
	// lightwaveSamlMetaDataUrlFormat for creating url to retrieve SAML metadata document from lightwave
	lightwaveEntityIDFormat = "https://%s/websso/SAML2/Metadata/%s"

	// lightwaveSTSUrlFormat for creating url to retrive SAML token
	lightwaveSTSUrlFormat = "https://%s/sts/STSService/%s"

	// DefaultDuration is the default amount of time in minutes that the credentials
	// will be valid for.
	DefaultDuration = time.Duration(1) * time.Hour
)

var defaultClient = &http.Client{
	Timeout: time.Second * 10,
}

// AssumeWithSAMLRoler represents the minimal subset of the STS client API used by this provider.
type AssumeWithSAMLRoler interface {
	AssumeRoleWithSAML(input *sts.AssumeRoleWithSAMLInput) (*sts.AssumeRoleWithSAMLOutput, error)
}

// A LightwaveProvider federates with AWS using SAML-based federation.
// https://docs.aws.amazon.com/IAM/latest/UserGuide/id_roles_providers_saml.html
//
// A valid lightwave user can use this package to retrieve temporary security
// credential for AWS.
//
type LightwaveProvider struct {
	expiration time.Time

	// Flag to mark if the credentials are from a solution user
	IsSolutionUser bool

	// STS client to make assume role request with.
	Client AssumeWithSAMLRoler

	// The Amazon Resource Name (ARN) of the SAML provider in IAM that describes the IdP.
	PrincipalARN string

	// The Amazon Resource Name (ARN) of the role that the caller is assuming.
	RoleARN string

	// Expiry duration of the STS credentials. Defaults to 15 minutes if not set.
	Duration time.Duration

	// Lightwave FQDN to send the requests.
	LightwaveFQDN string

	// aws region
	Region string

	// Lightwave tenant name.
	Tenant string

	// Lightwave account's username in UPN format.
	Username string

	// Lightwav account's password.
	Password string

	// Certificate of the solution user
	Certificate *x509.Certificate

	// Private key of the solution user
	PrivateKey *rsa.PrivateKey

	// HTTPClient is the http client to use to get credentials
	HTTPClient *http.Client
}

// NewLightwaveCredentials returns a pointer to a new Credentials object
// wrapping the environment variable provider.
func NewLightwaveCredentials(lwFqdn, region, tenant, username, password,
	principalARN, roleARN, certPath, privateKeyPath string, isSolutionUser bool) (*awscreds.Credentials, error) {
	var privateKey *rsa.PrivateKey = nil
	var cert *x509.Certificate = nil
	var err error = nil

	if isSolutionUser {
		privateKey, err = ParsePrivateKey(privateKeyPath)
		if err != nil {
			return nil, err
		}

		cert, err = ParseCertificate(certPath)
		if err != nil {
			return nil, err
		}
	}

	provider := &LightwaveProvider{
		LightwaveFQDN:  lwFqdn,
		Region:         region,
		Tenant:         tenant,
		PrincipalARN:   principalARN,
		RoleARN:        roleARN,
		Username:       username,
		Password:       password,
		Certificate:    cert,
		PrivateKey:     privateKey,
		IsSolutionUser: isSolutionUser,
		HTTPClient:     defaultClient,
	}

	return awscreds.NewCredentials(provider), nil
}

// IsExpired returns if the credentials have been expired.
func (e *LightwaveProvider) IsExpired() bool {
	curTime := time.Now()
	return e.expiration.Before(curTime)
}

// Retrieve retrieves the keys from the AWS using SAML federation.
func (e *LightwaveProvider) Retrieve() (awscreds.Value, error) {
	if e.Duration == 0 {
		// Expire as often as AWS permits.
		e.Duration = DefaultDuration
	}

	s, err := session.NewSession(&aws.Config{Region: aws.String(e.Region)})
	if err != nil {
		return awscreds.Value{}, err
	}

	var creds *awscreds.Value
	if e.IsSolutionUser {
		creds, err = e.getAssumeRoleWithSAMLByCert(s)
	} else {
		creds, err = e.getAssumeRoleWithSAMLByPassword(s)
	}

	// set expiration for the credentials
	e.expiration = time.Now().Add(e.Duration)
	return *creds, err
}

// Get AWS temporary credentials by calling AWS AssumeRoleWithSAML API with Lightwave SAML token.
func (e *LightwaveProvider) getAssumeRoleWithSAMLByPassword(s *session.Session) (*awscreds.Value, error) {
	issuer := fmt.Sprintf(lightwaveEntityIDFormat, e.LightwaveFQDN, e.Tenant)

	token, err := GetSamlTokenByPassword(e.LightwaveFQDN, e.Tenant, e.Username, e.Password, e.HTTPClient)
	if err != nil {
		return &awscreds.Value{ProviderName: issuer}, err
	}

	return getAssumeRoleWithSAML(s, token, issuer, e.PrincipalARN, e.RoleARN, e.Duration)
}

func (e *LightwaveProvider) getAssumeRoleWithSAMLByCert(s *session.Session) (*awscreds.Value, error) {
	issuer := fmt.Sprintf(lightwaveEntityIDFormat, e.LightwaveFQDN, e.Tenant)

	token, err := GetSamlTokenByCert(e.LightwaveFQDN, e.Tenant, e.Certificate, e.PrivateKey, e.HTTPClient)
	if err != nil {
		return &awscreds.Value{ProviderName: issuer}, err
	}

	return getAssumeRoleWithSAML(s, token, issuer, e.PrincipalARN, e.RoleARN, e.Duration)
}

func getAssumeRoleWithSAML(s *session.Session, token, issuer, principalARN, roleARN string, duration time.Duration) (*awscreds.Value, error) {
	// The base-64 encoded SAML response
	samlResponse := fmt.Sprintf(samlResponseTemplate, issuer, token)
	base64SamlResponse := base64.StdEncoding.EncodeToString([]byte(samlResponse))

	input := &sts.AssumeRoleWithSAMLInput{
		DurationSeconds: aws.Int64(int64(duration / time.Second)),
		PrincipalArn:    aws.String(principalARN),
		RoleArn:         aws.String(roleARN),
		SAMLAssertion:   aws.String(base64SamlResponse),
	}

	client := sts.New(s, aws.NewConfig().WithLogLevel(aws.LogDebugWithRequestRetries))
	roleOutput, err := client.AssumeRoleWithSAML(input)
	if err != nil {
		return &awscreds.Value{ProviderName: issuer}, err
	}

	creds := roleOutput.Credentials

	return &awscreds.Value{
		AccessKeyID:     aws.StringValue(creds.AccessKeyId),
		SecretAccessKey: aws.StringValue(creds.SecretAccessKey),
		SessionToken:    aws.StringValue(creds.SessionToken),
		ProviderName:    issuer,
	}, nil
}

// GetSamlMetaData retrieve the SAML metadata document from lightwave.
// The result is used to upload to AWS to register lightwave as IdP.
func GetSamlMetaData(lwFqdn, tenant string, httpClient *http.Client) (string, error) {
	if httpClient == nil {
		// use default client
		httpClient = defaultClient
	}

	url := fmt.Sprintf(lightwaveEntityIDFormat, lwFqdn, tenant)
	response, err := httpClient.Get(url)
	if err != nil {
		return "", err
	}

	defer response.Body.Close()
	contents, err := ioutil.ReadAll(response.Body)
	if err != nil {
		return "", err
	}

	if response.StatusCode != http.StatusOK {
		err = SAMLMetadataRetrievalError.MakeError(
			fmt.Sprintf("Failed to retrieve metadata from lightwave [%s] with tenant [%s]", lwFqdn, tenant), string(contents), nil)
		return "", err
	}

	return string(contents), nil
}

// GetSamlTokenByPassword retrieves saml token from lightwave STS with username/password auth. HttpClient param can be nil
func GetSamlTokenByPassword(lwFqdn, tenant, username, password string, httpClient *http.Client) (string, error) {
	if lwFqdn == "" || tenant == "" || username == "" || password == "" {
		return "", SAMLInvalidArgError.MakeError("Invalid arguments", "Function: GetSamlTokenByPassword", nil)
	}

	body := getSoapRequestWithPassword(username, password)
	return getSamlToken(lwFqdn, tenant, &body, httpClient)
}

// GetSamlTokenByCert retrieves a saml token using a solution user's certificate and private key
func GetSamlTokenByCert(lwFqdn, tenant string, cert *x509.Certificate, privateKey *rsa.PrivateKey, httpClient *http.Client) (string, error) {
	if lwFqdn == "" || tenant == "" || cert == nil || privateKey == nil {
		return "", SAMLInvalidArgError.MakeError("Invalid arguments", "", nil)
	}

	body, err := getSoapRequestWithCert(cert, privateKey)
	if err != nil {
		return "", SAMLInvalidRequestError.MakeError("Failed to construct SOAP request", "", err)
	}

	return getSamlToken(lwFqdn, tenant, &body, httpClient)
}

func getSamlToken(lwFqdn, tenant string, body *string, httpClient *http.Client) (string, error) {
	errMsg := fmt.Sprintf("Failed to get SAML token from url [%s]", lwFqdn)
	if httpClient == nil {
		// use default client
		httpClient = defaultClient
	}

	url := fmt.Sprintf(lightwaveSTSUrlFormat, lwFqdn, tenant)
	req, err := http.NewRequest("POST", url, strings.NewReader(*body))
	if err != nil {
		return "", err
	}
	req.Header.Set("Content-Type", "text/xml;charset=utf-8")
	req.Header.Set("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue")
	req.Header.Set("Method", "GetSamlToken")

	response, err := httpClient.Do(req)
	if err != nil {
		return "", SAMLGetTokenError.MakeError(errMsg, "Request failed to send", err)
	}

	defer response.Body.Close()
	contents, err := ioutil.ReadAll(response.Body)
	if err != nil {
		return "", err
	}

	if response.StatusCode != http.StatusOK {
		details := parseXmlError(string(contents))
		err = SAMLGetTokenError.MakeError(fmt.Sprintf("HTTP %d: %s", response.StatusCode, errMsg), details, nil)
		return "", err
	}

	return extractSamlAssertion(contents), nil
}

func parseXmlError(xml string) string {
	doc := etree.NewDocument()
	err := doc.ReadFromString(xml)
	if err != nil {
		// If error cant be parsed, return xml to keep old behavior
		return xml
	}

	xmlCode := doc.FindElement("//faultcode")
	xmlMessage := doc.FindElement("//faultstring")

	if xmlCode != nil && xmlMessage != nil {
		return fmt.Sprintf("[%s]: %s", xmlCode.Text(), xmlMessage.Text())
	}

	return xml
}

// extractSamlAssertion to take the content around <saml2:Assertion> and </saml2:Assertion>
func extractSamlAssertion(data []byte) string {
	s := string(data)
	start := strings.Index(s, beginSamlAssertionTag)
	end := strings.Index(s, endSamlAssertionTag) + len(endSamlAssertionTag)
	return s[start:end]
}

// ParseCertificate parses the x509 cert from the path
func ParseCertificate(filename string) (*x509.Certificate, error) {
	pemBytes, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	pemBlock, _ := pem.Decode(pemBytes)
	if pemBlock != nil {
		cert, err := x509.ParseCertificate(pemBlock.Bytes)
		if err != nil {
			return nil, err
		}

		return cert, nil
	}

	return nil, fmt.Errorf("Unable to parse certificate")
}

// ParsePrivateKey parses the private key from the pem file
func ParsePrivateKey(filename string) (*rsa.PrivateKey, error) {
	pemBytes, err := ioutil.ReadFile(filename)
	if err != nil {
		return nil, err
	}

	keyDERBlock, _ := pem.Decode(pemBytes)
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
