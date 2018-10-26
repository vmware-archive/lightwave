package mutentcatestsuite

import (
	"testing"

	"github.com/stretchr/testify/suite"

	"mutentcaclient"
)

type RootCASuite struct {
	suite.Suite
	client mutentcaclient.MutentCAClientInterface
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestRootCASuite(t *testing.T) {
	suite.Run(t, new(RootCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *RootCASuite) SetupSuite() {
	suite.client = mtcaclient
}

// TearDownSuite will be run after every execution of the Suite
func (suite *RootCASuite) TearDownSuite() {
}

// TODO: Add the required tests below

func (suite *RootCASuite) TestGetCAVersion() {
	expectedVersion := "Lightwave Multi-Tenanted CA Version 1.0"

	version, err := suite.client.GetCAVersion()
	if err != nil {
		suite.FailNow("Error getting CA version. Error: " + err.Error())
	}

	suite.Equal(expectedVersion, version.Version, "Unexpected output from rest request. Version output: '%s'", version.Version)
}

func (suite *RootCASuite) TestGetRootCACert() {
	// NOT IMPLEMENTED
}

func (suite *RootCASuite) TestRequestRootCASignedCert() {
	// NOT IMPLEMENTED
}

func (suite *RootCASuite) TestRevokeRootCASignedCert() {
	// NOT IMPLEMENTED
}

func (suite *RootCASuite) TestGetRootCACRL() {
	// NOT IMPLEMENTED
}
