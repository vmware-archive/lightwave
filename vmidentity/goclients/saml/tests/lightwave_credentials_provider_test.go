package tests

import (
	"github.com/stretchr/testify/assert"
        "errors"
        "flag"
        "fmt"
        "os"
	"testing"

        "github.com/aws/aws-sdk-go/aws"
        awscreds "github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/kinesis"
        "github.com/vmware/lightwave/vmidentity/goclients/saml/credentials"
)

const (
	streamName = "saml-federation-test"
	specstr = `{"name":"kube-qQyhk","networking":{"containerNetworkCidr":"10.2.0.0/16"},"orgName":"BVT-Org-cLQch","projectName":"project-tDSJd","serviceLevel":"DEVELOPER","size":{"count":1},"version":"1.8.1-4"}`
)

var username      = flag.String("username", "", "Username")
var password      = flag.String("password", "", "Password")
var lwFqdn        = flag.String("lwFqdn", "", "Lightwave FQDN")
var tenant        = flag.String("tenant", "", "Lightwave tenant")
var region        = flag.String("region", "", "aws region")
var principalARN  = flag.String("principalARN", "", "aws principal ARN")
var roleARN       = flag.String("roleARN", "", "aws role ARN")

func TestMain(t *testing.T) {
	err := parseArgs()
	if err != nil {
		fmt.Printf("Error setting up test suite: %+v", err)
		os.Exit(1)
	}
}

func parseArgs() error {
	var err error

	flag.Parse()
	if *username == "" || *password == "" || *lwFqdn == "" || *tenant == "" || *principalARN == "" || *roleARN == "" {
		fmt.Printf("Usage: go test -lwFqdn <fqdn> -tenant <tenant> -username <upn> -password <password> -principalARN <principal arn> -roleARN <role arn> (-region <region>)\n")
		return errors.New("Invalid Arguments")
	}

        if *region == "" {
                *region = "us-west-2"
        }

        return err
}

func TestLightwaveMetadataDoc(t *testing.T) {
	data, err := credentials.GetSamlMetaData(*lwFqdn, *tenant)

	assert.Nil(t, err)

	t.Log(data)
	t.Logf("size=%d", len(data))
	// The result must be greater than 1024 byte
	assert.True(t, len(data) > 1024)
}

func TestLightwaveSAMLToken(t *testing.T) {
	data, err := credentials.GetSamlToken(*lwFqdn, *tenant, *username, *password)

	assert.Nil(t, err)

	t.Log(data)
	t.Logf("size=%d", len(data))
	// The result must be greater than 1024 byte
	assert.True(t, len(data) > 1024)
}

func TestPulishToKinesisWithLightwaveSAML(t *testing.T) {
	provider := credentials.LightwaveProvider{
		LightwaveFQDN: *lwFqdn,
                Region:        *region,
		Tenant:        *tenant,
		Username:      *username,
		Password:      *password,
                PrincipalARN:  *principalARN,
                RoleARN:       *roleARN,
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
		        t.Log("Error in publishing data to %s/%s. Error: %+v", streamName, partitionKey, err)
	        }
	}
}
