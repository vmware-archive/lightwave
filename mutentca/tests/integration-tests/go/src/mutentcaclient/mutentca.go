package mutentcaclient

import (
	"fmt"
	"log"
	"strings"

	"github.com/go-openapi/runtime"
	openapiclient "github.com/go-openapi/runtime/client"
	"github.com/go-openapi/strfmt"

	mutentcaclient "gen/api/client"
	"gen/api/client/certificates"
	"gen/api/client/crl"
	"gen/api/client/intermediateca"
	"gen/api/client/rootca"
	"gen/api/models"
	"httpclient"
)

const (
	MutentCARestPort = 7878
)

// MutentCAClient structure to encapsulate Multi Tenant CA Client
type MutentCAClient struct {
	client   *mutentcaclient.APIClient
	LWHost   string
	MTCAHost string
	MTCAPort int
	Token    string
}

// newMutentCAClient is helper function for unit testing
func newMutentCAClient(host string, port int) *mutentcaclient.APIClient {
	address := fmt.Sprintf("%s:%d", host, port)
	openapiClient := openapiclient.New(address, mutentcaclient.DefaultBasePath, mutentcaclient.DefaultSchemes)

	return mutentcaclient.New(openapiClient, strfmt.Default)
}

// NewMutentCAClient - creates a new API client
func NewMutentCAClient(lwhost string, mtcahost string) (mtcaclient *MutentCAClient) {
	mtcaclient = new(MutentCAClient)
	mtcaclient.client = nil
	mtcaclient.LWHost = lwhost
	mtcaclient.MTCAHost = mtcahost
	mtcaclient.MTCAPort = MutentCARestPort

	return
}

// Login - takes username and password and requests a token
func (mtcaclient *MutentCAClient) Login(username, password string) error {
	tokenOptions, err := httpclient.GetTokenByPassword(
		fmt.Sprintf("https://%s", mtcaclient.LWHost),
		strings.Split(username, "@")[1],
		username,
		password)
	if err != nil {
		return err
	}

	mtcaclient.Token = tokenOptions.AccessToken
	mtcaclient.client = newMutentCAClient(mtcaclient.MTCAHost, mtcaclient.MTCAPort)

	return nil
}

func (mtcaclient *MutentCAClient) BearerToken() runtime.ClientAuthInfoWriter {
	return runtime.ClientAuthInfoWriterFunc(func(r runtime.ClientRequest, _ strfmt.Registry) error {
		r.SetHeaderParam("Authorization", "Bearer "+mtcaclient.Token)
		return nil
	})
}

// GetCAVersion - returns certificate authority version
func (mtcaclient *MutentCAClient) GetCAVersion() (*models.Version, error) {
	params := rootca.NewGetCAVersionParams()

	result, err := mtcaclient.client.Rootca.GetCAVersion(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting CA version: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// GetRootCA - returns root CA certificates
func (mtcaclient *MutentCAClient) GetRootCA() (*models.CACertificates, error) {
	params := rootca.NewGetRootCAParams()

	result, err := mtcaclient.client.Rootca.GetRootCA(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting root CA certificates: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// CreateIntermediateCA - creates intermediate CA and returns intermediate CA certs
func (mtcaclient *MutentCAClient) CreateIntermediateCA(caId string, parentCAId string, country []string, state []string, locality []string, orgUnit []string, policy string, startTime string, endTime string) (*models.CACertificates, error) {
	validity := &models.Validity{
		StartTime: startTime,
		EndTime:   endTime,
	}

	createCASpec := &models.IntermediateCACreateSpec{
		CaID:               &caId,
		ParentCaID:         parentCAId,
		Country:            country,
		State:              state,
		Locality:           locality,
		OrganizationalUnit: orgUnit,
		Policy:             policy,
		Validity:           validity,
	}

	params := intermediateca.NewCreateIntermediateCAParams().WithBody(createCASpec)

	result, err := mtcaclient.client.Intermediateca.CreateIntermediateCA(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error creating intermediate CA: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// GetIntermediateCA - returns intermediate CA certificates
func (mtcaclient *MutentCAClient) GetIntermediateCA(caId string) (*models.CACertificates, error) {
	params := intermediateca.NewGetIntermediateCAParams().WithCaID(caId)

	result, err := mtcaclient.client.Intermediateca.GetIntermediateCA(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting intermediate CA certificates: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// RevokeIntermediateCA - revokes intermediate CA
func (mtcaclient *MutentCAClient) RevokeIntermediateCA(caId string) error {
	params := intermediateca.NewRevokeIntermediateCAParams().WithCaID(caId)

	_, err := mtcaclient.client.Intermediateca.RevokeIntermediateCA(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error revoking intermediate CA: %s\n", err)
		return err
	}

	return nil
}

// GetRootCACRL - returns root CA crl
func (mtcaclient *MutentCAClient) GetRootCACRL() (*models.CRL, error) {
	params := crl.NewGetRootCACRLParams()

	result, err := mtcaclient.client.Crl.GetRootCACRL(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting root CA CRL: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// GetIntermediateCACRL - returns intermediate CA crl
func (mtcaclient *MutentCAClient) GetIntermediateCACRL(caId string) (*models.CRL, error) {
	params := crl.NewGetIntermediateCACRLParams().WithCaID(caId)

	result, err := mtcaclient.client.Crl.GetIntermediateCACRL(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting intermediate CA CRL: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// GetRootCASignedCert - returns root CA signed certificate
func (mtcaclient *MutentCAClient) GetRootCASignedCert(csr string, startTime string, endTime string, signatureAlgo string) (*models.Certificate, error) {
	validity := &models.Validity{
		StartTime: startTime,
		EndTime:   endTime,
	}

	createSignedCertSpec := &models.CreateSignedCertSpec{
		Csr:                &csr,
		Validity:           validity,
		SignatureAlgorithm: signatureAlgo,
	}

	params := certificates.NewAddCACertificateParams().WithBody(createSignedCertSpec)

	result, err := mtcaclient.client.Certificates.AddCACertificate(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting root CA signed certificate: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// RevokeRootCASignedCert - revokes root CA signed certificate
func (mtcaclient *MutentCAClient) RevokeRootCASignedCert(cert string) error {
	revokeSignedCertSpec := &models.Certificate{
		Cert: cert,
	}

	params := certificates.NewRevokeCACertificateParams().WithBody(revokeSignedCertSpec)

	_, err := mtcaclient.client.Certificates.RevokeCACertificate(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error revoking root CA signed certificate: %s\n", err)
		return err
	}

	return nil
}

// GetIntermediateCASignedCert - returns intermediate CA signed certificate
func (mtcaclient *MutentCAClient) GetIntermediateCASignedCert(caId string, csr string, startTime string, endTime string, signatureAlgo string) (*models.Certificate, error) {
	validity := &models.Validity{
		StartTime: startTime,
		EndTime:   endTime,
	}

	createSignedCertSpec := &models.CreateSignedCertSpec{
		Csr:                &csr,
		Validity:           validity,
		SignatureAlgorithm: signatureAlgo,
	}

	params := certificates.NewAddIntermediateCACertificateParams().WithCaID(caId).WithBody(createSignedCertSpec)

	result, err := mtcaclient.client.Certificates.AddIntermediateCACertificate(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error getting intermediate CA signed certificate: %s\n", err)
		return nil, err
	}

	return result.Payload, nil
}

// RevokeIntermediateCASignedCert - revokes intermediate CA signed certificate
func (mtcaclient *MutentCAClient) RevokeIntermediateCASignedCert(caId string, cert string) error {
	revokeSignedCertSpec := &models.Certificate{
		Cert: cert,
	}

	params := certificates.NewRevokeIntermediateCACertificateParams().WithCaID(caId).WithBody(revokeSignedCertSpec)

	_, err := mtcaclient.client.Certificates.RevokeIntermediateCACertificate(params, mtcaclient.BearerToken())
	if err != nil {
		log.Printf("Error revoking intermediate CA signed certificate: %s\n", err)
		return err
	}

	return nil
}
