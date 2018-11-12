package postclient

import (
	"fmt"
	"httpclient"
	"log"
	"strings"

	postclient "gen/post/client"
	"gen/post/client/ldap"
	"gen/post/models"
	"github.com/go-openapi/runtime"
	openapiclient "github.com/go-openapi/runtime/client"
	"github.com/go-openapi/strfmt"
)

const (
	HeaderAuthorization  = "Authorization"
	AuthTokenBearer      = "Bearer"
	POSTRestPort         = 7578
	CertAuthLdapObjClass = "vmwCertificationAuthority"
	CertLdapObjClass     = "vmwCerts"
	CertAuthContainerDN  = "cn=Certificate-Authority,%s"
	CertAuthConfigDN     = "cn=Configuration,%s"
	LdapCn               = "cn"
	LdapParentId         = "cAParentCAId"
	LdapScopeSub         = "sub"
	LdapCertsContainer   = "Certs"
)

type DbRestClient struct {
	client     *postclient.Post
	LwHost     string
	PostHost   string
	PostPort   int
	PostDomain string
	DcDomain   string
	Token      string
}

func NewDbRestClient(lwHost string, postHost string, postDomain string) (postdbclient *DbRestClient) {
	postdbclient = new(DbRestClient)

	postdbclient.LwHost = lwHost
	postdbclient.PostHost = postHost
	postdbclient.PostPort = POSTRestPort
	postdbclient.PostDomain = postDomain
	postdbclient.DcDomain = fmt.Sprintf("dc=%s", strings.Replace(postDomain, ".", ",dc=", -1))
	postdbclient.client = nil

	return
}

func newDbRestClient(host string, port int) *postclient.Post {
	address := fmt.Sprintf("%s:%d", host, port)
	openapiClient := openapiclient.New(address, postclient.DefaultBasePath, postclient.DefaultSchemes)

	return postclient.New(openapiClient, nil)
}

func (restClient *DbRestClient) BearerToken() runtime.ClientAuthInfoWriter {
	return runtime.ClientAuthInfoWriterFunc(func(r runtime.ClientRequest, _ strfmt.Registry) error {
		authValue := fmt.Sprintf("%s %s", AuthTokenBearer, restClient.Token)
		r.SetHeaderParam(HeaderAuthorization, authValue)
		return nil
	})
}

func (restClient *DbRestClient) Login(username string, password string) error {

	tokenOptions, err := httpclient.GetTokenByPassword(
		fmt.Sprintf("https://%s", restClient.LwHost),
		strings.Split(username, "@")[1],
		username,
		password)

	if err != nil {
		return err
	}

	restClient.Token = tokenOptions.AccessToken
	restClient.client = newDbRestClient(restClient.PostHost, restClient.PostPort)

	return err
}

func (restClient DbRestClient) deleteImpl(dn string) error {
	params := ldap.NewDeletePostLdapParams()
	params.SetDn(dn)

	return restClient.client.Ldap.DeletePostLdap(params, restClient.BearerToken())
}

func (restClient *DbRestClient) deleteCRLs(issuerDn string) error {
	params := ldap.NewGetPostLdapParams()
	params.SetDn(issuerDn)

	attrs := []string{LdapCn}
	params.SetAttrs(attrs)

	certFilter := fmt.Sprintf("(objectClass=%s)", CertLdapObjClass)
	params.SetFilter(&certFilter)

	searchScope := LdapScopeSub
	params.SetScope(&searchScope)

	response, err := restClient.client.Ldap.GetPostLdap(params, restClient.BearerToken())
	if err != nil {
		return err
	}

	result := response.Payload.Result
	resultCount := int(response.Payload.ResultCount)

	for i := 0; i < resultCount; i++ {
		err = restClient.deleteImpl(result[i].Dn)
		if err != nil {
			log.Printf("Error in deleting CRL '%s': '%+v'\n", result[i].Dn, err)
		}
	}

	return err
}

func (restClient *DbRestClient) deleteCAImpl(dn string) error {
	// delete all the CRLs
	err := restClient.deleteCRLs(dn)
	if err != nil {
		log.Printf("Error in deleting CRLs of CA '%s'", dn)
		return err
	}

	// delete the Certs container
	certsDn := fmt.Sprintf("cn=%s,%s", LdapCertsContainer, dn)
	err = restClient.deleteImpl(certsDn)
	if err != nil {
		log.Printf("Error in deleting Certs container '%s': '%+v'\n", dn, err)
		return err
	}

	err = restClient.deleteImpl(dn)
	if err != nil {
		log.Printf("Error in deleting CA '%s': '%+v'\n", dn, err)
	}

	return err
}

func (restClient *DbRestClient) getAttrMap(entry []*models.LDAPAttribute) map[string][]string {

	attrMap := make(map[string][]string)
	for _, attr := range entry {
		attrMap[attr.Type] = attr.Value
	}

	return attrMap
}

func (restClient *DbRestClient) getCA(caID string) ([]*models.LDAPEntry, int, error) {

	params := ldap.NewGetPostLdapParams()

	attrs := []string{LdapCn, LdapParentId}
	params.SetAttrs(attrs)

	caDN := fmt.Sprintf(CertAuthContainerDN, restClient.DcDomain)
	params.SetDn(caDN)

	searchScope := LdapScopeSub
	params.SetScope(&searchScope)

	caFilter := fmt.Sprintf("(&(objectClass=%s)(cn=%s))", CertAuthLdapObjClass, caID)
	params.SetFilter(&caFilter)

	response, err := restClient.client.Ldap.GetPostLdap(params, restClient.BearerToken())
	if err != nil {
		return nil, -1, err
	}
	result := response.Payload.Result
	resultCount := int(response.Payload.ResultCount)

	return result, resultCount, nil
}

func (restClient *DbRestClient) getCert(issuerDn string, crlNumber string) ([]*models.LDAPEntry, int, error) {

	params := ldap.NewGetPostLdapParams()

	attrs := []string{LdapCn}
	params.SetAttrs(attrs)

	searchScope := LdapScopeSub
	params.SetScope(&searchScope)

	certDN := fmt.Sprintf("cn=%s,%s", LdapCertsContainer, issuerDn)
	params.SetDn(certDN)

	certFilter := fmt.Sprintf("(&(objectClass=%s)(cn=%s))", CertLdapObjClass, crlNumber)
	params.SetFilter(&certFilter)

	response, err := restClient.client.Ldap.GetPostLdap(params, restClient.BearerToken())
	if err != nil {
		return nil, -1, err
	}

	result := response.Payload.Result
	resultCount := int(response.Payload.ResultCount)

	return result, resultCount, nil
}

func (restClient *DbRestClient) DeleteCA(caID string) error {
	configDN := fmt.Sprintf(CertAuthConfigDN, restClient.DcDomain)

	result, resultCount, err := restClient.getCA(caID)
	if err != nil {
		return err
	}

	for i := 0; i < resultCount; i++ {
		attrMap := restClient.getAttrMap(result[i].Attributes)
		if _, ok := attrMap[LdapParentId]; !ok {
			cnValue := attrMap[LdapCn][0]
			restClient.deleteImpl(fmt.Sprintf("cn=%s,%s", cnValue, configDN))
		}

		restClient.deleteCAImpl(result[i].Dn)
	}

	return nil
}

func (restClient *DbRestClient) DeleteCert(caID string, crlNumber string) error {

	result, resultCount, err := restClient.getCA(caID)
	if err != nil {
		return err
	}

	for i := 0; i < resultCount; i++ {
		certs, certCount, err := restClient.getCert(result[i].Dn, crlNumber)
		if err != nil {
			return err
		}

		for j := 0; j < certCount; j++ {
			restClient.deleteImpl(certs[j].Dn)
		}
	}

	return nil
}
