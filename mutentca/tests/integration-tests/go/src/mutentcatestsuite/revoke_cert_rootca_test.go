package mutentcatestsuite

import (
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type RevokeCertRootCASuite struct {
	suite.Suite
	client                     mutentcaclient.MutentCAClientInterface
	testRevokedCert            string
	rootCACert                 string
	testIntermediateCAToDelete []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestRevokeCertRootCASuite(t *testing.T) {
	suite.Run(t, new(RevokeCertRootCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *RevokeCertRootCASuite) SetupSuite() {
	suite.client = mtcaclient
	commonName := "localhost"
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName: commonName,
		KeyUsage:   keyUsage,
		IsCA:       false,
	}

	rootCACerts, err := suite.client.GetRootCA()
	if err != nil {
		suite.FailNow("Error while getting root CA", "Error: %+v", err)
	}

	suite.NotNil(rootCACerts)
	if len(rootCACerts.Certs) < 1 {
		suite.FailNow("Error while getting root CA", "No valid cert was found")
	}
	suite.rootCACert = rootCACerts.Certs[0]

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	cert, err := suite.client.GetRootCASignedCert(csr, nil, "")
	if err != nil {
		suite.FailNow("Error while requesting sign certificate from root CA", "Error: %+v", err)
	}
	suite.NotNil(cert.Cert)

	err = suite.client.RevokeRootCASignedCert(cert.Cert)
	if err != nil {
		suite.FailNow("Error while revoking certificate", "Error: %+v", err)
	}

	crl, err := suite.client.GetRootCACRL()
	if err != nil {
		suite.FailNow("Error while getting requesting crl from root CA", "Error: %+v", err)
	}

	err = common.CheckAndVerifyCrl(suite.rootCACert, crl.Crl)
	if err != nil {
		suite.FailNow("Unable to verify crl", "Error: %+v", err)
	}

	isCertRevoked, err := common.CheckIfSignedCertIsRevoked(cert.Cert, crl.Crl)
	suite.Nil(err)
	if !isCertRevoked {
		suite.FailNow("Crl does not have revoke certificate info", "Error: %+v", err)
	}

	suite.testRevokedCert = cert.Cert
}

// TearDownSuite will be run after every execution of the Suite
func (suite *RevokeCertRootCASuite) TearDownSuite() {
	for _, intermediateCA := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCA)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCA, err)
		}
	}
}

func (suite *RevokeCertRootCASuite) TestAlreadyRevokeCertificate() {
	err := suite.client.RevokeRootCASignedCert(suite.testRevokedCert)
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking already revoked certificate")
	suite.Contains(err.Error(), "[409]", "The error returned is not one expected")
}

func (suite *RevokeCertRootCASuite) TestRevokeCertificateInvalid() {
	cert := suite.CreateSignCertificateFromIntermediateCA()

	err := suite.client.RevokeRootCASignedCert(cert)
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking certificate which is not issued by CA")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	err = suite.client.RevokeRootCASignedCert("dummycert")
	suite.NotNilf(err, "Error while revoking certificate", "Error expected while revoking invalid certificate")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")
}

func (suite *RevokeCertRootCASuite) CreateSignCertificateFromIntermediateCA() string {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	commonName := "localhost"
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName: commonName,
		KeyUsage:   keyUsage,
		IsCA:       false,
	}

	_, err := suite.client.CreateIntermediateCA(
		testIntermediateCAId, "", nil, nil, nil, nil, "", nil)
	if err != nil {
		suite.FailNow("Error while creating intermediate CA", "Intermediate CA ID: %s, Error: %+v",
			testIntermediateCAId, err)
	}
	suite.testIntermediateCAToDelete = append(suite.testIntermediateCAToDelete, testIntermediateCAId)

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	cert, err := suite.client.GetIntermediateCASignedCert(testIntermediateCAId, csr, nil, "")
	if err != nil {
		suite.FailNow("Error while requesting sign certificate from intermediate CA", "IntermediateCA ID:%s, Error: %+v", testIntermediateCAId, err)
	}
	suite.NotNil(cert.Cert)
	return cert.Cert
}
