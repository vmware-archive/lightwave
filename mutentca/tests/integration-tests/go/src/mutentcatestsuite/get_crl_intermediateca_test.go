package mutentcatestsuite

import (
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type GetCrlIntermediateCASuite struct {
	suite.Suite
	client                     mutentcaclient.MutentCAClientInterface
	testintermediateCACert     string
	testIntermediateCAId       string
	testIntermediateCAToDelete []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGetCrlIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(GetCrlIntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *GetCrlIntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient

	suite.testIntermediateCAId = "IntermediateCA-" + common.RandSeq(8)

	certs, err := suite.client.CreateIntermediateCA(
		suite.testIntermediateCAId, "", nil, nil, nil, nil, "", nil)
	if err != nil {
		suite.FailNow("Error while creating intermediate CA", "Intermediate CA ID: %s, Error: %+v",
			suite.testIntermediateCAId, err)
	}
	suite.testIntermediateCAToDelete = append(suite.testIntermediateCAToDelete, suite.testIntermediateCAId)
	suite.NotNil(certs)
	if len(certs.Certs) < 1 {
		suite.FailNow("Error while creating intermediate CA", "No valid cert was found")
	}
	suite.testintermediateCACert = certs.Certs[0]
}

// TearDownSuite will be run after every execution of the Suite
func (suite *GetCrlIntermediateCASuite) TearDownSuite() {
	for _, intermediateCA := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCA)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCA, err)
		}
	}
}

func (suite *GetCrlIntermediateCASuite) TestGetCrl() {
	crl, err := suite.client.GetIntermediateCACRL(suite.testIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while getting  intermediate CA Crl", "Error: %+v", err)
	}

	suite.NotNilf(crl.Crl, "Crl should not be empty")

	err = common.CheckAndVerifyCrl(suite.testintermediateCACert, crl.Crl)
	suite.Nilf(err, "Unable to verify crl")
}

func (suite *GetCrlIntermediateCASuite) TestGetCrlCADoesNotExist() {
	testIntermediateCAId := "dummy"
	_, err := suite.client.GetIntermediateCACRL(testIntermediateCAId)
	suite.NotNilf(err, "No error returned while getting intermediate CA Crl", "Error expected as CA does not exist")
	suite.Contains(err.Error(), "[404]", "The error returned is not one expected")
}
