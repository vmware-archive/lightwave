package mutentcatestsuite

import (
	"crypto/x509"
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type GetRootCASuite struct {
	suite.Suite
	client mutentcaclient.MutentCAClientInterface
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGetRootCASuite(t *testing.T) {
	suite.Run(t, new(GetRootCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *GetRootCASuite) SetupSuite() {
	suite.client = mtcaclient
}

// TearDownSuite will be run after every execution of the Suite
func (suite *GetRootCASuite) TearDownSuite() {
}

func (suite *GetRootCASuite) TestGetRootCACert() {
	caCerts, err := suite.client.GetRootCA()
	if err != nil {
		suite.FailNow("Error while getting root CA", "Error: %+v", err)
	}

	suite.NotNil(caCerts)
	if len(caCerts.Certs) < 1 {
		suite.FailNow("Error while getting root CA", "No valid cert was found")
	}

	cert := caCerts.Certs[0]
	suite.checkRootCACert(cert)
}

func (suite *GetRootCASuite) checkRootCACert(cert string) {
	x509Cert, err := common.ConvertPEMToX509(cert)
	if err != nil {
		suite.FailNow("Error while reading root CA certificate", "Invalid root CA certificate. Cert: %s, Error: %+v", cert, err)
	}

	suite.Truef(x509Cert.IsCA, "CA basic constraint is not enabled in root CA cert")
	bitCertSign := x509Cert.KeyUsage & x509.KeyUsageCertSign
	if bitCertSign == 0 {
		suite.FailNow("Root CA cert does not support certificate sign")
	}

	bitCrlSign := x509Cert.KeyUsage & x509.KeyUsageCRLSign
	if bitCrlSign == 0 {
		suite.FailNow("Root CA cert does not support crl sign")
	}

	/*
		// TODO: Check is currently disabled as mutentca does not have AIA in self-sign cert
		if len(x509Cert.IssuingCertificateURL) < 1 {
			suite.FailNow("Root CA cert does not have Authority information access")
		}
	*/

	suite.Falsef(x509Cert.MaxPathLenZero, "Root CA cert does not allow the creation of root ca")
}
