package mutentcatestsuite

import (
	"strconv"
	"testing"
	"time"

	"github.com/stretchr/testify/suite"

	"common"
	"gen/api/models"
	"mutentcaclient"
)

type GetSignCertRootCA struct {
	suite.Suite
	client     mutentcaclient.MutentCAClientInterface
	rootCACert string
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGetSignCertRootCA(t *testing.T) {
	suite.Run(t, new(GetSignCertRootCA))
}

// SetupSuite will be run before every execution of the Suite
func (suite *GetSignCertRootCA) SetupSuite() {
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
func (suite *GetSignCertRootCA) TearDownSuite() {
}

func (suite *GetSignCertRootCA) TestGetSignCert() {
	commonName := "localhost"
	domainName := "lightwave.local"
	orgList := []string{"VMware Inc"}
	orgUnitList := []string{"VMware Inc"}
	countryList := []string{"US"}
	stateList := []string{"WA"}
	ipAddressList := []string{"127.0.0.1"}
	emailAddresses := []string{"abc@xyz"}
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName:     commonName,
		DomainName:     domainName,
		OrgList:        orgList,
		OrgUnitList:    orgUnitList,
		CountryList:    countryList,
		StateList:      stateList,
		IPAddresses:    ipAddressList,
		EmailAddresses: emailAddresses,
		KeyUsage:       keyUsage,
		IsCA:           false,
	}

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	cert, err := suite.client.GetRootCASignedCert(csr, nil, "")
	if err != nil {
		suite.FailNow("Error while requesting sign certificate from root CA", "Error: %+v", err)
	}
	suite.NotNil(cert.Cert)

	certData := common.CertData{
		CommonName:     commonName,
		DomainName:     domainName,
		OrgList:        orgList,
		OrgUnitList:    orgUnitList,
		CountryList:    countryList,
		StateList:      stateList,
		IPAddresses:    ipAddressList,
		EmailAddresses: emailAddresses,
		KeyUsage:       keyUsage,
		IsCA:           false,
	}

	err = common.CheckCertDetails(cert.Cert, certData)
	if err != nil {
		suite.FailNow("Issued sign certificate data does not match with csr", "Error: %+v", err)
	}

	err = common.VerifyCert(cert.Cert, suite.rootCACert)
	if err != nil {
		suite.FailNow("Error while verifying issued sign certificate", "Error: %+v", err)
	}
}

func (suite *GetSignCertRootCA) TestGetSignCertWithValidity() {
	commonName := "localhost"
	domainName := "lightwave.local"
	orgList := []string{"VMware Inc"}
	orgUnitList := []string{"VMware Inc"}
	countryList := []string{"US"}
	stateList := []string{"WA"}
	dnsNames := []string{"localhost"}
	emailAddresses := []string{"abc@xyz"}
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName:     commonName,
		DomainName:     domainName,
		OrgList:        orgList,
		OrgUnitList:    orgUnitList,
		CountryList:    countryList,
		DNSList:        dnsNames,
		EmailAddresses: emailAddresses,
		StateList:      stateList,
		KeyUsage:       keyUsage,
	}

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error generating csr", "Error: %+v", err)
	}

	startTime := time.Now()
	endTime := startTime.AddDate(0, 0, 10)

	validity := models.Validity{
		StartTime: strconv.FormatInt(startTime.Unix(), 10),
		EndTime:   strconv.FormatInt(endTime.Unix(), 10),
	}

	cert, err := suite.client.GetRootCASignedCert(csr, &validity, "")
	if err != nil {
		suite.FailNow("Error while requesting sign certificate from root CA", "Error: %+v", err)
	}
	suite.NotNil(cert.Cert)

	certData := common.CertData{
		CommonName:     commonName,
		DomainName:     domainName,
		OrgList:        orgList,
		OrgUnitList:    orgUnitList,
		CountryList:    countryList,
		StateList:      stateList,
		DNSList:        dnsNames,
		EmailAddresses: emailAddresses,
		KeyUsage:       keyUsage,
		IsCA:           false,
	}

	err = common.CheckCertDetails(cert.Cert, certData)
	if err != nil {
		suite.FailNow("Issued sign certificate data does not match with csr", "Error: %+v", err)
	}

	err = common.CheckCertValidity(cert.Cert, startTime, endTime)
	if err != nil {
		suite.FailNow("Issued sign certificate data does not match with requested valid period", "Error: %+v", err)
	}

	err = common.VerifyCert(cert.Cert, suite.rootCACert)
	if err != nil {
		suite.FailNow("Error while verifying issued sign certificate from root CA", "Error: %+v", err)
	}
}

func (suite *GetSignCertRootCA) TestGetSignCertInvalidCSR() {
	csr := "Invalid CSR"

	_, err := suite.client.GetRootCASignedCert(csr, nil, "")
	suite.NotNilf(err, "Error while requesting sign certificate from root CA", "Error expected as csr is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	commonName := "Test-" + common.RandSeq(8)
	certSignRequest := common.CertSignRequest{
		CommonName: commonName,
	}

	csr, err = common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	endTime := time.Now()
	startTime := endTime.AddDate(1, 0, 0)

	validity := models.Validity{
		StartTime: strconv.FormatInt(startTime.Unix(), 10),
		EndTime:   strconv.FormatInt(endTime.Unix(), 10),
	}

	_, err = suite.client.GetRootCASignedCert(csr, &validity, "")
	suite.NotNilf(err, "Error while requesting sign certificate from root CA", "Error expected as requested valid period is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")

	validity = models.Validity{
		StartTime: "",
		EndTime:   "",
	}

	_, err = suite.client.GetRootCASignedCert(csr, &validity, "")
	suite.NotNilf(err, "Error while requesting sign certificate from root CA", "Error expected as requested valid period is invalid")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")
}

func (suite *GetSignCertRootCA) TestGetSignCertPolicyViolation() {
	commonName := "localhost"
	domainName := "lightwave.local"
	orgList := []string{"VMware Inc"}
	orgUnitList := []string{"VMware Inc"}
	countryList := []string{"US"}
	stateList := []string{"WA"}
	ipAddressList := []string{"127.0.0.1"}
	emailAddresses := []string{"abc@xyz"}
	keyUsage := 0
	certSignRequest := common.CertSignRequest{
		CommonName:     commonName,
		DomainName:     domainName,
		OrgList:        orgList,
		OrgUnitList:    orgUnitList,
		CountryList:    countryList,
		StateList:      stateList,
		IPAddresses:    ipAddressList,
		EmailAddresses: emailAddresses,
		KeyUsage:       keyUsage,
		IsCA:           true,
	}

	csr, err := common.CreateCertSignRequest(certSignRequest)
	if err != nil {
		suite.FailNow("Error while generating csr", "Error: %+v", err)
	}

	_, err = suite.client.GetRootCASignedCert(csr, nil, "")
	suite.NotNilf(err, "Error while requesting sign certificate from root CA", "Error expected as requested csr is not allowed")
	suite.Contains(err.Error(), "[400]", "The error returned is not one expected")
}
