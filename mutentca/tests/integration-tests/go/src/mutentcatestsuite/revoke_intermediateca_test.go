package mutentcatestsuite

import (
	"testing"

	"common"
	"mutentcaclient"

	"github.com/stretchr/testify/suite"
)

type RevokeIntermediateCASuite struct {
	suite.Suite
	client                      mutentcaclient.MutentCAClientInterface
	testRevokedIntermediateCAId string
	testIntermediateCAToDelete  []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestRevokeIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(RevokeIntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *RevokeIntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient

	testIntermediateCAId, testIntermediateCACert := suite.CreateIntermediateCA("")

	err := suite.client.RevokeIntermediateCA(testIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while revoking certificate", "Error: %+v", err)
	}

	parentCACrl, err := suite.client.GetRootCACRL()
	if err != nil {
		suite.FailNow("Error while getting requesting parentCACrl from intermediate CA", "Error: %+v", err)
	}

	isCertRevoked, err := common.CheckIfSignedCertIsRevoked(testIntermediateCACert, parentCACrl.Crl)
	suite.Nil(err)
	if !isCertRevoked {
		suite.FailNow("Crl does not have revoke intermediate cert info", "Error: %+v", err)
	}

	suite.testRevokedIntermediateCAId = testIntermediateCAId
}

// TearDownSuite will be run after every execution of the Suite
func (suite *RevokeIntermediateCASuite) TearDownSuite() {
	for _, intermediateCA := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCA)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCA, err)
		}
	}
}

func (suite *RevokeIntermediateCASuite) TestRevokeNestedIntermediateCA() {
	testIntermediateCAId, _ := suite.CreateIntermediateCA("")

	testNestedIntermediateCAId, testNestedIntermediateCACert := suite.CreateIntermediateCA(testIntermediateCAId)

	err := suite.client.RevokeIntermediateCA(testNestedIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while revoking certificate", "Error: %+v", err)
	}

	parentCACrl, err := suite.client.GetIntermediateCACRL(testIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while getting requesting parentCACrl from intermediate CA", "Error: %+v", err)
	}

	isCertRevoked, err := common.CheckIfSignedCertIsRevoked(testNestedIntermediateCACert, parentCACrl.Crl)
	suite.Nil(err)
	if !isCertRevoked {
		suite.FailNow("Crl does not have revoke intermediate cert info", "Error: %+v", err)
	}
}

func (suite *RevokeIntermediateCASuite) TestRevokeIntermediateCADoesNotExist() {
	testIntermediateCAId := "dummy"
	err := suite.client.RevokeIntermediateCA(testIntermediateCAId)
	suite.NotNilf(err, "No error returned while revoking intermediate CA", "Error expected as intermediate CA does not exist")
	suite.Contains(err.Error(), "[404]", "The error returned is not one expected")
}

func (suite *RevokeIntermediateCASuite) TestAlreadyRevokedIntermediateCA() {
	err := suite.client.RevokeIntermediateCA(suite.testRevokedIntermediateCAId)
	suite.NotNilf(err, "Error while revoking intermediate CA", "Error expected while revoking already revoked intermediate CA")
	suite.Contains(err.Error(), "[409]", "The error returned is not one expected")

	_, err = suite.client.GetIntermediateCA(suite.testRevokedIntermediateCAId)
	suite.NotNilf(err, "No error returned while rqeuesting intermediate CA certificate", "Error expected as intermediate CA is revoked")
	suite.Contains(err.Error(), "[403]", "The error returned is not one expected")

	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	_, err = suite.client.CreateIntermediateCA(
		testIntermediateCAId, suite.testRevokedIntermediateCAId, nil, nil, nil, nil, "", nil)
	suite.NotNilf(err, "No error returned while creating intermediate CA", "Error expected as intermediate CA is revoked")
	suite.Contains(err.Error(), "[403]", "The error returned is not one expected")

	commonName := "localhost"
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName: commonName,
		KeyUsage:   keyUsage,
		IsCA:       false,
	}

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	_, err = suite.client.GetIntermediateCASignedCert(suite.testRevokedIntermediateCAId, csr, nil, "")
	suite.NotNilf(err, "No error returned while requesting sign certificate from intermediate CA", "Error expected as intermediate CA is revoked")
	suite.Contains(err.Error(), "[403]", "The error returned is not one expected")
}

func (suite *RevokeIntermediateCASuite) CreateIntermediateCA(parentCAId string) (string, string) {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	certs, err := suite.client.CreateIntermediateCA(
		testIntermediateCAId, parentCAId, nil, nil, nil, nil, "", nil)
	if err != nil {
		suite.FailNow("Error while creating intermediate CA", "Intermediate CA ID: %s, Error: %+v",
			testIntermediateCAId, err)
	}
	suite.testIntermediateCAToDelete = append(suite.testIntermediateCAToDelete, testIntermediateCAId)
	suite.NotNil(certs)
	if len(certs.Certs) < 1 {
		suite.FailNow("Error while creating intermediate CA", "No valid cert was found")
	}
	return testIntermediateCAId, certs.Certs[0]
}
