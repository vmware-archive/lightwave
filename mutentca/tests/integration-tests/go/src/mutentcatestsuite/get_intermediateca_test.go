package mutentcatestsuite

import (
	"crypto/x509"
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type GetIntermediateCASuite struct {
	suite.Suite
	client                     mutentcaclient.MutentCAClientInterface
	testIntermediateCAId       string
	testIntermediateCAToDelete []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGetIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(GetIntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *GetIntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient

	suite.testIntermediateCAId = "IntermediateCA-" + common.RandSeq(8)

	_, err := suite.client.CreateIntermediateCA(
		suite.testIntermediateCAId, "", nil, nil, nil, nil, "", nil)
	if err != nil {
		suite.FailNow("Error while creating intermediate CA", "Intermediate CA ID: %s, Error: %+v",
			suite.testIntermediateCAId, err)
	}
	suite.testIntermediateCAToDelete = append(suite.testIntermediateCAToDelete, suite.testIntermediateCAId)
}

// TearDownSuite will be run after every execution of the Suite
func (suite *GetIntermediateCASuite) TearDownSuite() {
	for _, intermediateCA := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCA)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCA, err)
		}
	}
}

func (suite *GetIntermediateCASuite) TestGetIntermediateCACert() {
	caCerts, err := suite.client.GetIntermediateCA(suite.testIntermediateCAId)
	if err != nil {
		suite.FailNow("Error while getting intermediateCA", "Error: %+v", err)
	}

	suite.NotNil(caCerts)
	if len(caCerts.Certs) < 1 {
		suite.FailNow("Error while getting intermediate CA", "No valid cert was found")
	}

	cert := caCerts.Certs[0]
	suite.checkIntermediateCACert(cert)
}

func (suite *GetIntermediateCASuite) TestGetIntermediateCACertDoesNotExist() {
	testIntermediateCAId := "IntermediateCA-dummy"
	_, err := suite.client.GetIntermediateCA(testIntermediateCAId)
	suite.NotNilf(err, "Error while get intermediate CA", "Error expected as CA does not exist")
	suite.Contains(err.Error(), "[404]", "The error returned is not one expected")
}

func (suite *GetIntermediateCASuite) checkIntermediateCACert(cert string) {
	x509Cert, err := common.ConvertPEMToX509(cert)
	if err != nil {
		suite.FailNow("Error while reading intermediate CA certificate", "Invalid intermediate CA certificate. Cert: %s, Error: %+v", cert, err)
	}

	suite.Truef(x509Cert.IsCA, "CA basic constraint is not enabled in intermediate CA cert")
	bitCertSign := x509Cert.KeyUsage & x509.KeyUsageCertSign
	if bitCertSign == 0 {
		suite.FailNow("Intermediate CA cert does not support certificate sign")
	}

	bitCrlSign := x509Cert.KeyUsage & x509.KeyUsageCRLSign
	if bitCrlSign == 0 {
		suite.FailNow("Intermediate CA cert does not support crl sign")
	}

	if len(x509Cert.IssuingCertificateURL) < 1 {
		suite.FailNow("Intermediate CA cert does not have Authority information access")
	}

	suite.Falsef(x509Cert.MaxPathLenZero, "Intermediate CA cert does not allow the creation of intermediate ca")
}
