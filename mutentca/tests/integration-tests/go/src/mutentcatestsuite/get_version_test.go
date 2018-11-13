package mutentcatestsuite

import (
	"testing"

	"github.com/stretchr/testify/suite"

	"mutentcaclient"
)

type CAVersionSuite struct {
	suite.Suite
	client mutentcaclient.MutentCAClientInterface
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestCAVersionSuite(t *testing.T) {
	suite.Run(t, new(CAVersionSuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *CAVersionSuite) SetupSuite() {
	suite.client = mtcaclient
}

// TearDownSuite will be run after every execution of the Suite
func (suite *CAVersionSuite) TearDownSuite() {
}

func (suite *CAVersionSuite) TestGetCAVersion() {
	expectedVersion := "Lightwave Multi-Tenanted CA Version 1.0"

	version, err := suite.client.GetCAVersion()
	if err != nil {
		suite.FailNow("Error getting CA version. Error: " + err.Error())
	}

	suite.Equal(expectedVersion, version.Version, "Unexpected output from rest request. Version output: '%s'", version.Version)
}
