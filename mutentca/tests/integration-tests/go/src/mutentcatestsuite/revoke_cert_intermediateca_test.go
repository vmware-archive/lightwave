package mutentcatestsuite

import (
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type RevokeCertIntermediateCASuite struct {
	suite.Suite
	client                     mutentcaclient.MutentCAClientInterface
	testRevokedCert            string
	testIntermediateCAId       string
	testIntermediateCACert     string
	testIntermediateCAToDelete []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestRevokeCertIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(RevokeCertIntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *RevokeCertIntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient

	suite.testIntermediateCAId, suite.testIntermediateCACert = suite.CreateIntermediateCA()

	cert := suite.CreateSignCertificate(suite.testIntermediateCAId)

	err := suite.client.RevokeIntermediateCASignedCert(suite.testIntermediateCAId, cert)
	if err != nil {
		suite.FailNow("Error while revoking certificate", "Error: %+v", err)
	}

	crl, err := suite.client.GetIntermediateCACRL(suite.testIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while getting requesting crl from intermediate CA", "Error: %+v", err)
	}

	err = common.CheckAndVerifyCrl(suite.testIntermediateCACert, crl.Crl)
	if err != nil {
		suite.FailNow("Unable to verify crl", "Error: %+v", err)
	}

	isCertRevoked, err := common.CheckIfSignedCertIsRevoked(cert, crl.Crl)
	suite.Nil(err)
	if !isCertRevoked {
		suite.FailNow("Crl does not have revoke certificate info", "Error: %+v", err)
	}

	suite.testRevokedCert = cert
}

// TearDownSuite will be run after every execution of the Suite
func (suite *RevokeCertIntermediateCASuite) TearDownSuite() {
	for _, intermediateCA := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCA)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCA, err)
		}
	}
}

func (suite *RevokeCertIntermediateCASuite) TestRevokeCertificateCADoesNotExist() {
	err := suite.client.RevokeIntermediateCASignedCert("dummyCA", suite.testRevokedCert)
	suite.NotNilf(err, "Error while revoking certificate", "Error expected as CA does not exist")
	suite.Contains(err.Error(), "[404]", "The error returned is not one expected")
}

func (suite *RevokeCertIntermediateCASuite) TestAlreadyRevokeCertificate() {
	err := suite.client.RevokeIntermediateCASignedCert(suite.testIntermediateCAId, suite.testRevokedCert)
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking already revoked certificate")
	suite.Contains(err.Error(), "[409]", "The error returned is not one expected")
}

func (suite *RevokeCertIntermediateCASuite) TestRevokeCertificateInvalid() {
	testIntermediateCAId, _ := suite.CreateIntermediateCA()
	cert := suite.CreateSignCertificate(testIntermediateCAId)

	err := suite.client.RevokeIntermediateCASignedCert(suite.testIntermediateCAId, cert)
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking certificate which is not issued by CA")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	err = suite.client.RevokeIntermediateCASignedCert(suite.testIntermediateCAId, "dummycert")
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking invalid certificate")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")
}

func (suite *RevokeCertIntermediateCASuite) CreateIntermediateCA() (string, string) {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	certs, err := suite.client.CreateIntermediateCA(
		testIntermediateCAId, "", nil, nil, nil, nil, "", nil)
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

func (suite *RevokeCertIntermediateCASuite) CreateSignCertificate(caId string) string {
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

	cert, err := suite.client.GetIntermediateCASignedCert(caId, csr, nil, "")
	if err != nil {
		suite.FailNow("Error while requesting sign certificate from intermediate CA", "IntermediateCA ID:%s, Error: %+v", caId, err)
	}
	suite.NotNil(cert.Cert)

	caCerts, err := suite.client.GetIntermediateCA(caId)
	if err != nil {
		suite.FailNow("Error while getting intermediateCA", "Error: %+v", err)
	}

	suite.NotNil(caCerts)
	if len(caCerts.Certs) < 1 {
		suite.FailNow("Error while getting intermediate CA", "No valid cert was found")
	}

	return cert.Cert
}
