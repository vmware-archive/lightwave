package mutentcatestsuite

import (
	"testing"

	"github.com/stretchr/testify/suite"

	"mutentcaclient"
)

type IntermediateCASuite struct {
	suite.Suite
	client mutentcaclient.MutentCAClientInterface
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(IntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *IntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient
}

// TearDownSuite will be run after every execution of the Suite
func (suite *IntermediateCASuite) TearDownSuite() {
}

// TODO: Add the required tests below

func (suite *IntermediateCASuite) TestCreateIntermediateCA() {
	// NOT IMPLEMENTED
}

func (suite *IntermediateCASuite) TestGetIntermediateCACert() {
	// NOT IMPLEMENTED
}

func (suite *IntermediateCASuite) TestRequestIntermediateCASignedCert() {
	// NOT IMPLEMENTED
}

func (suite *IntermediateCASuite) TestRevokeIntermediateCASignedCert() {
	// NOT IMPLEMENTED
}

func (suite *IntermediateCASuite) TestGetIntermediateCACRL() {
	// NOT IMPLEMENTED
}

func (suite *IntermediateCASuite) TestRevokeIntermediateCA() {
	// NOT IMPLEMENTED
}
