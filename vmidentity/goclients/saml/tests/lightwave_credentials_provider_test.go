package tests

import (
	"errors"
	"flag"
	"fmt"
	"github.com/stretchr/testify/assert"
	"os"
	"testing"

	"crypto/tls"
	"github.com/aws/aws-sdk-go/aws"
	awscreds "github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/kinesis"
	"github.com/vmware/lightwave/vmidentity/goclients/saml/credentials"
	"io/ioutil"
	"net/http"
	"os/exec"
)

const (
	streamName         = "saml-federation-test"
	specstr            = `{"name":"kube-qQyhk","networking":{"containerNetworkCidr":"10.2.0.0/16"},"orgName":"BVT-Org-cLQch","projectName":"project-tDSJd","serviceLevel":"DEVELOPER","size":{"count":1},"version":"1.8.1-4"}`
	awsSamlProfile     = "saml-test"
	awsProfileTemplate = `
[profile %s]
output = %s
region = %s
aws_access_key_id = %s
aws_secret_access_key = %s
aws_session_token = %s
`
)

var username = flag.String("username", "", "Username")
var password = flag.String("password", "", "Password")
var lwFqdn = flag.String("lwFqdn", "", "Lightwave FQDN")
var tenant = flag.String("tenant", "", "Lightwave tenant")
var region = flag.String("region", "", "aws region")
var principalARN = flag.String("principalARN", "", "aws principal ARN")
var roleARN = flag.String("roleARN", "", "aws role ARN")
var certPath = flag.String("cert", "", "Path to cert file")
var privateKeyPath = flag.String("privateKey", "", "Path to file containing private key")
var httpClient *http.Client

func TestMain(m *testing.M) {
	err := parseArgs()
	if err != nil {
		fmt.Printf("Error setting up test suite: %+v", err)
		os.Exit(1)
	}

	httpClient = getHTTPClient()

	os.Exit(m.Run())
}

func parseArgs() error {
	var err error

	flag.Parse()
	if *username == "" || *password == "" || *lwFqdn == "" || *tenant == "" {
		flag.Usage()
		return errors.New("Invalid Arguments")
	}

	if *region == "" {
		*region = "us-west-2"
	}

	return err
}

func getHTTPClient() *http.Client {
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}
	return &http.Client{Transport: tr}
}

func TestLightwaveMetadataDoc(t *testing.T) {
	data, err := credentials.GetSamlMetaData(*lwFqdn, *tenant, httpClient)
	assert.Nil(t, err)
	// The result must be greater than 1024 byte
	assert.True(t, len(data) > 1024)
}

func TestLightwaveSAMLTokenByPassword(t *testing.T) {
	data, err := credentials.GetSamlTokenByPassword(*lwFqdn, *tenant, *username, *password, httpClient)
	assert.Nil(t, err)
	// The result must be greater than 1024 byte
	assert.True(t, len(data) > 1024)
}

func TestLightwaveSAMLTokenInvalidCredentials(t *testing.T) {
	data, err := credentials.GetSamlTokenByPassword(*lwFqdn, *tenant, *username, "123", httpClient)
	assert.Empty(t, data)
	if assert.NotNil(t, err) {
		assert.Contains(t, err.Error(), "FailedAuthentication")
	}
}

func TestLightwaveSAMLTokenByCert(t *testing.T) {
	if *privateKeyPath == "" || *certPath == "" {
		t.Skipf("Private Key or cert path not provided, skipping GetTokenByCert test")
	}

	cert, err := credentials.ParseCertificate(*certPath)
	assert.Nil(t, err)

	privateKey, err := credentials.ParsePrivateKey(*privateKeyPath)
	assert.Nil(t, err)

	data, err := credentials.GetSamlTokenByCert(*lwFqdn, *tenant, cert, privateKey, httpClient)
	assert.Nil(t, err)

	// The result must be greater than 1024 byte
	assert.True(t, len(data) > 1024)
}

func TestAssumeRoleCLI(t *testing.T) {
	if *principalARN == "" || *roleARN == "" {
		t.Skipf("role/principal ARN not provided, skipping AWS credential tests")
	}

	provider := credentials.LightwaveProvider{
		LightwaveFQDN:  *lwFqdn,
		Region:         *region,
		Tenant:         *tenant,
		Username:       *username,
		Password:       *password,
		PrincipalARN:   *principalARN,
		RoleARN:        *roleARN,
		IsSolutionUser: false,
		HTTPClient:     httpClient,
	}
	awsConfig := os.Getenv("HOME") + "/.aws/config"

	awsCreds, err := provider.Retrieve()
	assert.Nil(t, err)

	profile := fmt.Sprintf(awsProfileTemplate, awsSamlProfile, "json", "us-west-2",
		awsCreds.AccessKeyID, awsCreds.SecretAccessKey, awsCreds.SessionToken)

	config, err := ioutil.ReadFile(awsConfig)
	assert.Nil(t, err)

	// append profile "saml-test" and verify
	f, err := os.OpenFile(awsConfig, os.O_APPEND|os.O_WRONLY, 0600)
	assert.Nil(t, err)
	f.WriteString(profile)
	f.Close()

	// test cli
	cmd := exec.Command("aws", "kinesis", "describe-stream", "--stream-name", streamName, "--profile", awsSamlProfile)
	err = cmd.Run()
	assert.Nil(t, err)

	// restore contents in config file
	err = ioutil.WriteFile(awsConfig, config, 0600)
	assert.Nil(t, err)
}

func TestPublishToKinesisWithLightwaveSAML(t *testing.T) {
	if *principalARN == "" || *roleARN == "" {
		t.Skipf("role/principal ARN not provided, skipping AWS credential tests")
	}

	provider := credentials.LightwaveProvider{
		LightwaveFQDN:  *lwFqdn,
		Region:         *region,
		Tenant:         *tenant,
		Username:       *username,
		Password:       *password,
		PrincipalARN:   *principalARN,
		RoleARN:        *roleARN,
		IsSolutionUser: false,
		HTTPClient:     httpClient,
	}

	s := session.New(&aws.Config{Region: aws.String(*region), Credentials: awscreds.NewCredentials(&provider)})
	kc := kinesis.New(s)
	for i := 0; i < 10; i++ {
		partitionKey := string(i)
		data := []byte(specstr)
		_, err := kc.PutRecord(&kinesis.PutRecordInput{
			Data:         data,
			StreamName:   aws.String(streamName),
			PartitionKey: aws.String(partitionKey),
		})
		if err != nil {
			t.Logf("Error in publishing data to %s/%s. Error: %+v", streamName, partitionKey, err)
		}
	}
}
