package credentials

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"time"

	"encoding/base64"
	"github.com/aws/aws-sdk-go/aws"
	awscreds "github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/sts"
)

const (
	// lightwaveSamlMetaDataUrlFormat for creating url to retrieve SAML metadata document from lightwave
	// https://<lw-fqdn>/websso/SAML2/Metadata/<tenant>
	lightwaveEntityIdFormat = "https://%s/websso/SAML2/Metadata/%s"

	// lightwaveSTSUrlFormat for creating url to retrive SAML token
	// https://<lw fqdn>/sts/STSService/<tenant>
	lightwaveSTSUrlFormat = "https://%s/sts/STSService/%s"

	getSAMLTokenRequestTemplate = `<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/">
  <soapenv:Header>
    <wsse:Security xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
      <wsu:Timestamp xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">
        <wsu:Created>%s</wsu:Created>
        <wsu:Expires>%s</wsu:Expires>
      </wsu:Timestamp>
      <wsse:UsernameToken xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
        <wsse:Username>%s</wsse:Username>
        <wsse:Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordText"><![CDATA[%s]]></wsse:Password>
      </wsse:UsernameToken>
    </wsse:Security>
  </soapenv:Header>
  <soapenv:Body xmlns:wst="http://docs.oasis-open.org/ws-sx/ws-trust/200512">
    <wst:RequestSecurityToken xmlns:wst="http://docs.oasis-open.org/ws-sx/ws-trust/200512" xmlns:wsa="http://www.w3.org/2005/08/addressing">
      <wst:TokenType>urn:oasis:names:tc:SAML:2.0:assertion</wst:TokenType>
      <wst:RequestType>http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue</wst:RequestType>
      <wst:KeyType>http://docs.oasis-open.org/ws-sx/ws-trust/200512/Bearer</wst:KeyType>
      <wst:Lifetime xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">
        <wsu:Created>%s</wsu:Created>
        <wsu:Expires>%s</wsu:Expires>
      </wst:Lifetime>
      <wst:Renewing Allow="false" OK="false"/>
      <wst:Delegatable>false</wst:Delegatable>
      <wst:Participants>
      	<wst:Primary>
      		<wsa:EndpointReference>
      			<wsa:Address>https://signin.aws.amazon.com/saml</wsa:Address>
      		</wsa:EndpointReference>
      	</wst:Primary>
      </wst:Participants>
      <wst:Claims xmlns:auth="http://docs.oasis-open.org/wsfed/authorization/200706" Dialect="http://schemas.xmlsoap.org/ws/2005/05/fedclaims">
      	<auth:ClaimType Uri="https://aws.amazon.com/SAML/Attributes/Role" />
      	<auth:ClaimType Uri="https://aws.amazon.com/SAML/Attributes/RoleSessionName" />
      </wst:Claims>
    </wst:RequestSecurityToken>
  </soapenv:Body>
</soapenv:Envelope>`

	beginSamlAssertionTag = `<saml2:Assertion`
	endSamlAssertionTag   = `</saml2:Assertion>`

        SAMLResponseTemplate = `<samlp:Response xmlns:samlp="urn:oasis:names:tc:SAML:2.0:protocol" xmlns:saml="urn:oasis:names:tc:SAML:2.0:assertion">
<saml:Issuer>%s</saml:Issuer>
<samlp:Status>
    <samlp:StatusCode Value="urn:oasis:names:tc:SAML:2.0:status:Success"/>
</samlp:Status>
%s
</samlp:Response>`
)

// DefaultDuration is the default amount of time in minutes that the credentials
// will be valid for.
var DefaultDuration = time.Duration(1) * time.Hour

var netClient = &http.Client{
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
}

// NewLightwavevCredentials returns a pointer to a new Credentials object
// wrapping the environment variable provider.
func NewLightwavevCredentials(lwFqdn, region, tenant, username, password, principalARN, roleARN string) *awscreds.Credentials {
	provider := &LightwaveProvider{
		LightwaveFQDN: lwFqdn,
                Region:        region,
		Tenant:        tenant,
		Username:      username,
		Password:      password,
                PrincipalARN:  principalARN,
                RoleARN:       roleARN,
	}
	return awscreds.NewCredentials(provider)
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

	creds, err := GetAssumeRoleWithSAMLCreds(s, e.LightwaveFQDN, e.Tenant, e.Username, e.Password, e.PrincipalARN, e.RoleARN, e.Duration)
        // set expiration for the credentials
        e.expiration = time.Now().Add(e.Duration)
        return *creds, err
}

// Get AWS temparory credentials by calling AWS AssumeRoleWithSAML API with Lightwave SAML token.
func GetAssumeRoleWithSAMLCreds(s *session.Session, lwFqdn, tenant, username, password, principalARN, roleARN string, duration time.Duration) (*awscreds.Value, error) {
        issuer := fmt.Sprintf(lightwaveEntityIdFormat, lwFqdn, tenant)

	token, err := GetSamlToken(lwFqdn, tenant, username, password)
	if err != nil {
		return &awscreds.Value{ProviderName: issuer}, err
	}

	// The base-64 encoded SAML response
        samlResponse := fmt.Sprintf(SAMLResponseTemplate, issuer, token)
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

// IsExpired returns if the credentials have been expired.
func (e *LightwaveProvider) IsExpired() bool {
	curTime := time.Now()
	return e.expiration.Before(curTime)
}

// GetSamlMetaData retrieve the SAML metadata document from lightwave.
// The result is used to upload to AWS to register lightwave as IdP.
func GetSamlMetaData(lwFqdn, tenant string) (string, error) {
	url := fmt.Sprintf(lightwaveEntityIdFormat, lwFqdn, tenant)
	response, err := netClient.Get(url)
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

// GetSamlToken to retrieve saml token from lightwave STS
func GetSamlToken(lwFqdn, tenant, username, password string) (string, error) {
	url := fmt.Sprintf(lightwaveSTSUrlFormat, lwFqdn, tenant)
	body := makeSamlTokenRequest(username, password)
	req, err := http.NewRequest("POST", url, strings.NewReader(*body))
	if err != nil {
		return "", err
	}
	req.Header.Set("Content-Type", "text/xml;charset=utf-8")
	req.Header.Set("SOAPAction", "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue")
	req.Header.Set("Method", "GetSamlToken")

	response, err := netClient.Do(req)
	if err != nil {
		return "", err
	}

	defer response.Body.Close()
	contents, err := ioutil.ReadAll(response.Body)
	if err != nil {
		return "", err
	}

        if response.StatusCode != http.StatusOK {
		err = SAMLGetTokenError.MakeError(
			fmt.Sprintf("Failed to get SAML token from lightwave [%s] with tenant [%s]", lwFqdn, tenant), string(contents), nil)

		return "", err
        }

	return extractSamlAssertion(contents), nil
}

func makeSamlTokenRequest(username, password string) *string {
	start := time.Now()
	end := start.Add(time.Duration(24) * time.Hour)

	// 2018-05-02T19:35:00.000Z
	// RFC3339     = "2006-01-02T15:04:05Z07:00"
	startTime := start.UTC().Format(time.RFC3339)
	endTime := end.UTC().Format(time.RFC3339)

	req := fmt.Sprintf(getSAMLTokenRequestTemplate, startTime, endTime, username, password, startTime, endTime)
	return &req
}

// extractSamlAssertion to take the content around <saml2:Assertion> and </saml2:Assertion>
func extractSamlAssertion(data []byte) string {
	s := string(data)
	start := strings.Index(s, beginSamlAssertionTag)
	end := strings.Index(s, endSamlAssertionTag) + len(endSamlAssertionTag)
	return s[start:end]
}
