package mutentcatestsuite

import (
	"strconv"
	"testing"
	"time"

	"common"
	"crypto/x509"
	"gen/api/models"
	"github.com/stretchr/testify/suite"
	"mutentcaclient"
)

type CreateIntermediateCASuite struct {
	suite.Suite
	client                     mutentcaclient.MutentCAClientInterface
	testIntermediateCAId       string
	testIntermediateCAToDelete []string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestCreateIntermediateCASuite(t *testing.T) {
	suite.Run(t, new(CreateIntermediateCASuite))
}

// SetupSuite will be run before every execution of the Suite
func (suite *CreateIntermediateCASuite) SetupSuite() {
	suite.client = mtcaclient

	suite.testIntermediateCAId = "IntermediateCA-" + common.RandSeq(8)
	suite.createAndCheckIntermediateCA(suite.testIntermediateCAId, "", nil, nil, nil, nil, nil, nil)
}

// TearDownSuite will be run after every execution of the Suite
func (suite *CreateIntermediateCASuite) TearDownSuite() {
	for _, intermediateCAId := range suite.testIntermediateCAToDelete {
		err := suite.client.DeleteIntermediateCA(intermediateCAId)
		if err != nil {
			suite.Fail("Error while deleting intermediate CA", "IntermediateCA ID:%s, Error: %+v",
				intermediateCAId, err)
		}
	}
}

func (suite *CreateIntermediateCASuite) TestCreateIntermediateCA() {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	OUs := []string{"VMware Inc"}
	state := []string{"CA", "WA"}
	locality := []string{"Bellevue"}
	country := []string{"US"}
	suite.createAndCheckIntermediateCA(testIntermediateCAId, "", country, state, locality, OUs, nil, nil)

	startTime := time.Now()
	endTime := startTime.AddDate(1, 0, 0)
	testIntermediateCAId2 := "IntermediateCA-" + common.RandSeq(8)
	suite.createAndCheckIntermediateCA(testIntermediateCAId2, "", nil, nil, nil, nil, &startTime, &endTime)
}

func (suite *CreateIntermediateCASuite) TestNestedCreateIntermediateCA() {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	OUs := []string{"VMware Inc"}
	state := []string{"WA"}
	locality := []string{"Bellevue"}
	country := []string{"US"}
	suite.createAndCheckIntermediateCA(testIntermediateCAId, suite.testIntermediateCAId, country, state, locality, OUs, nil, nil)
}

func (suite *CreateIntermediateCASuite) TestCreateDuplicateIntermediateCA() {
	_, err := suite.client.CreateIntermediateCA(
		suite.testIntermediateCAId, "", nil, nil, nil, nil, "", nil)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as CA exists")
	suite.Contains(err.Error(), "[409]", "The error returned is not one expected")

	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	suite.createAndCheckIntermediateCA(testIntermediateCAId, "", nil, nil, nil, nil, nil, nil)

	_, err = suite.client.CreateIntermediateCA(
		suite.testIntermediateCAId, testIntermediateCAId, nil, nil, nil, nil, "", nil)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as CA exists")
	suite.Contains(err.Error(), "[409]", "The error returned is not one expected")
}

func (suite *CreateIntermediateCASuite) TestCreateIntermediateCAParentDoesNotExist() {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	_, err := suite.client.CreateIntermediateCA(
		testIntermediateCAId, "dummy", nil, nil, nil, nil, "", nil)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as parent does not exist")
	suite.Contains(err.Error(), "[404]", "The error returned is not one expected")
}

func (suite *CreateIntermediateCASuite) TestCreateIntermediateCAInvalidData() {
	testIntermediateCAId := "IntermediateCA-" + common.RandSeq(8)
	country := []string{"USA"}
	_, err := suite.client.CreateIntermediateCA(testIntermediateCAId, "", country, nil, nil, nil, "", nil)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as input data is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	validity := &models.Validity{
		StartTime: "dummy",
		EndTime:   "dummy",
	}

	_, err = suite.client.CreateIntermediateCA(testIntermediateCAId, "", nil, nil, nil, nil, "", validity)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as input data is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	validity = &models.Validity{
		StartTime: "",
		EndTime:   "",
	}

	_, err = suite.client.CreateIntermediateCA(testIntermediateCAId, "", nil, nil, nil, nil, "", validity)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as input data is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	endTime := time.Now()
	startTime := endTime.AddDate(1, 0, 0)
	validity = &models.Validity{
		StartTime: strconv.FormatInt(startTime.Unix(), 10),
		EndTime:   strconv.FormatInt(endTime.Unix(), 10),
	}
	_, err = suite.client.CreateIntermediateCA(testIntermediateCAId, "", nil, nil, nil, nil, "", validity)
	suite.NotNilf(err, "Error while creating intermediate CA", "Error expected as input data is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")
}

func (suite *CreateIntermediateCASuite) createAndCheckIntermediateCA(caId string, parentCAId string, country []string, state []string, locality []string, orgUnit []string, startTime, endTime *time.Time) {

	var validity *models.Validity
	validity = nil

	if startTime != nil && endTime != nil {
		validity = &models.Validity{
			StartTime: strconv.FormatInt(startTime.Unix(), 10),
			EndTime:   strconv.FormatInt(endTime.Unix(), 10),
		}
	}

	caCerts, err := suite.client.CreateIntermediateCA(
		caId, parentCAId, country, state, locality, orgUnit, "", validity)
	if err != nil {
		suite.FailNow("Error while creating intermediate CA", "Intermediate CA ID: %s, Error: %+v",
			caId, err)
	}
	suite.testIntermediateCAToDelete = append(suite.testIntermediateCAToDelete, caId)
	suite.NotNil(caCerts)
	if len(caCerts.Certs) < 1 {
		suite.FailNow("Error while creating intermediate CA", "No valid cert was found")
	}
	cert := caCerts.Certs[0]

	var parentCACerts *models.CACertificates
	if parentCAId != "" {
		parentCACerts, err = suite.client.GetIntermediateCA(parentCAId)
		if err != nil {
			suite.FailNow("Error while getting intermediate CA", "IntermediateCA: %s, Error: %+v", parentCAId, err)
		}
	} else {
		parentCACerts, err = suite.client.GetRootCA()
		if err != nil {
			suite.FailNow("Error while getting root CA", "Error: %+v", err)
		}
	}
	suite.NotNil(parentCACerts)
	if len(parentCACerts.Certs) < 1 {
		suite.FailNow("Error while getting root CA", "No valid cert was found")
	}
	parentCACert := parentCACerts.Certs[0]

	orgs, err := common.GetCertOrg(parentCACert)
	if err != nil {
		suite.FailNow("Error while reading certificate", "Unable to get org's info from cert. Error: %+v", err)
	}

	keyUsage := int(x509.KeyUsageCRLSign | x509.KeyUsageCertSign)
	certData := common.CertData{
		CommonName:   caId,
		CountryList:  country,
		StateList:    state,
		LocalityList: locality,
		OrgList:      orgs,
		OrgUnitList:  orgUnit,
		IsCA:         true,
		KeyUsage:     keyUsage,
	}

	err = common.CheckCertDetails(cert, certData)
	if err != nil {
		suite.FailNow("Intermediate CA certificate data does not match with input request", "Error: %+v", err)
	}

	if startTime != nil && endTime != nil {
		err = common.CheckCertValidity(cert, *startTime, *endTime)
		if err != nil {
			suite.FailNow("Intermediate CA certificate data does not match with input requested validity", "Error: %+v", err)
		}
	}

	err = common.VerifyCert(cert, parentCACert)
	if err != nil {
		suite.FailNow("Error while verifying intermediate CA cert", "Error: %+v", err)
	}
}
