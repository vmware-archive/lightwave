package mutentcatestsuite

import (
	"testing"

	"common"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type GetCrlRootCASuite struct {
	suite.Suite
	client     mutentcaclient.MutentCAClientInterface
	rootCACert string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGetCrlRootCASuite(t *testing.T) {
	suite.Run(t, new(GetCrlRootCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *GetCrlRootCASuite) SetupSuite() {
	suite.client = mtcaclient

	rootCACerts, err := suite.client.GetRootCA()
	if err != nil {
		suite.FailNow("Error while getting root CA", "Error: %+v", err)
	}

	suite.NotNil(rootCACerts)
	if len(rootCACerts.Certs) < 1 {
		suite.FailNow("Error while getting root CA", "No valid cert was found")
	}
	suite.rootCACert = rootCACerts.Certs[0]
}

// TearDownSuite will be run after every execution of the Suite
func (suite *GetCrlRootCASuite) TearDownSuite() {
}

func (suite *GetCrlRootCASuite) TestGetCrl() {
	crl, err := suite.client.GetRootCACRL()
	if err != nil {
		suite.FailNow("Error while getting root CA Crl", "Error: %+v", err)
	}

	suite.NotNilf(crl.Crl, "Crl should not be empty")

	err = common.CheckAndVerifyCrl(suite.rootCACert, crl.Crl)
	suite.Nilf(err, "Unable to verify crl")
}
