package common

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/pem"
	"net"
	"reflect"
	"strconv"
	"time"
)

var (
	oidDomainName      = asn1.ObjectIdentifier{0, 9, 2342, 19200300, 100, 1, 25}
	oidKeyUsage        = asn1.ObjectIdentifier{2, 5, 29, 15}
	oidBasicConstraint = asn1.ObjectIdentifier{2, 5, 29, 19}
)

type CertSignRequest struct {
	CommonName     string
	DomainName     string
	OrgList        []string
	OrgUnitList    []string
	CountryList    []string
	StateList      []string
	LocalityList   []string
	DNSList        []string
	IPAddresses    []string
	EmailAddresses []string
	KeyUsage       int
	IsCA           bool
}

type CertData struct {
	CommonName     string
	DomainName     string
	OrgList        []string
	OrgUnitList    []string
	CountryList    []string
	StateList      []string
	LocalityList   []string
	DNSList        []string
	IPAddresses    []string
	EmailAddresses []string
	KeyUsage       int
	IsCA           bool
}

func ConvertPEMToX509(cert string) (*x509.Certificate, error) {
	certBytes := []byte(cert)
	block, _ := pem.Decode(certBytes)

	if block == nil || block.Type != "CERTIFICATE" {
		return nil, LwCAInvalidCert.MakeErr()
	}

	x509Cert, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		return nil, LwCAInvalidCert.MakeErr().WithDetail("Unable to parse certificate")
	}

	return x509Cert, nil
}

func convertToIPAddress(in []string) []net.IP {
	var ipaddresses []net.IP
	for _, ip := range in {
		ipaddresses = append(ipaddresses, net.ParseIP(ip))
	}
	return ipaddresses
}

func convertIpToString(ip []net.IP) []string {
	var ipaddresses []string
	for _, ip := range ip {
		ipaddresses = append(ipaddresses, ip.String())
	}
	return ipaddresses
}

func CreateCertSignRequest(certRequest CertSignRequest) (string, error) {
	// step: generate a keypair
	keys, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return "", LwCAKeyGeneration.MakeErr()
	}

	var extraNames []pkix.AttributeTypeAndValue

	if certRequest.DomainName != "" {
		domainName := pkix.AttributeTypeAndValue{
			Type:  oidDomainName,
			Value: certRequest.DomainName,
		}
		extraNames = append(extraNames, domainName)
	}

	names := pkix.Name{
		CommonName:         certRequest.CommonName,
		Organization:       certRequest.OrgList,
		OrganizationalUnit: certRequest.OrgUnitList,
		Locality:           certRequest.LocalityList,
		Country:            certRequest.CountryList,
		Province:           certRequest.StateList,
		ExtraNames:         extraNames,
	}

	//TODO Need to set URIList

	var extensions []pkix.Extension

	if certRequest.KeyUsage != 0 {
		keyBytes := []byte(strconv.Itoa(certRequest.KeyUsage))
		keybitstr, err := asn1.Marshal(
			asn1.BitString{Bytes: keyBytes, BitLength: len(keyBytes) * 8})
		if err != nil {
			return "", LwCAInvalidCSR.MakeError("Unable to add keyusage extension", err)
		}
		keyUsageExtension := pkix.Extension{
			Id:       oidKeyUsage,
			Critical: false,
			Value:    keybitstr,
		}
		extensions = append(extensions, keyUsageExtension)
	}

	type basicConstraints struct {
		IsCA       bool `asn1:"optional"`
		MaxPathLen int  `asn1:"optional,default:-1"`
	}

	basicCon := basicConstraints{IsCA: certRequest.IsCA}
	bitstr, err := asn1.Marshal(basicCon)
	if err != nil {
		return "", LwCAInvalidCSR.MakeError("Unable to add basic constraint extension", err)
	}

	basicConstraintsExtension := pkix.Extension{
		Id:       oidBasicConstraint,
		Critical: false,
		Value:    bitstr,
	}
	extensions = append(extensions, basicConstraintsExtension)

	// step: generate a csr template
	var csrTemplate = x509.CertificateRequest{
		Subject:            names,
		DNSNames:           certRequest.DNSList,
		IPAddresses:        convertToIPAddress(certRequest.IPAddresses),
		EmailAddresses:     certRequest.EmailAddresses,
		SignatureAlgorithm: x509.SHA256WithRSA,
		ExtraExtensions:    extensions,
	}

	// step: generate the csr request
	csrCertificate, err := x509.CreateCertificateRequest(rand.Reader, &csrTemplate, keys)
	if err != nil {
		return "", LwCAInvalidCSR.MakeError("Unable to generate csr", err)
	}

	csr := pem.EncodeToMemory(&pem.Block{
		Type: "CERTIFICATE REQUEST", Bytes: csrCertificate,
	})

	return string(csr), nil
}

func getCertDomainName(subject pkix.RDNSequence) string {
	domainName := ""
	for _, s := range subject {
		for _, i := range s {
			if i.Type.String() == oidDomainName.String() {
				if v, ok := i.Value.(string); ok {
					if domainName != "" {
						domainName = v + "." + domainName
					} else {
						domainName = v
					}
				}
			}
		}
	}
	return domainName
}

func GetCertOrg(cert string) ([]string, error) {
	x509Cert, err := ConvertPEMToX509(cert)
	if err != nil {
		return nil, LwCAInvalidCert.MakeErr().WithDetail("Unable to convert cert to X509 cert")
	}
	return x509Cert.Subject.Organization, nil
}

func CheckCertDetails(cert string, certData CertData) error {
	x509Cert, err := ConvertPEMToX509(cert)
	if err != nil {
		return LwCAInvalidCert.MakeErr().WithDetail("Unable to convert cert to X509 cert")
	}

	var subject pkix.RDNSequence
	if _, err := asn1.Unmarshal(x509Cert.RawSubject, &subject); err != nil {
		return LwCAInvalidCert.MakeErr().WithDetail("Unable to parse X509 cert subject")
	}

	if x509Cert.Subject.CommonName != certData.CommonName {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate common name does not match as expected")
	}

	if getCertDomainName(subject) != certData.DomainName {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate domain name does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.Subject.Country, certData.CountryList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate country does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.Subject.Province, certData.StateList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate state does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.Subject.Locality, certData.LocalityList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate locality does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.Subject.OrganizationalUnit, certData.OrgUnitList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate OU's does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.Subject.Organization, certData.OrgList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate org's does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.EmailAddresses, certData.EmailAddresses) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate email addresses's does not match as expected")
	}

	if !reflect.DeepEqual(x509Cert.DNSNames, certData.DNSList) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate dns names does not match as expected")
	}

	if !reflect.DeepEqual(convertIpToString(x509Cert.IPAddresses), certData.IPAddresses) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate ip addresses1 does not match as expected")
	}

	if x509Cert.IsCA != certData.IsCA {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate CA status does not match as expected")
	}

	if certData.KeyUsage != int(x509Cert.KeyUsage) {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate key usage does not match as expected")
	}

	if len(x509Cert.IssuingCertificateURL) < 1 {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate does not have authority information access")
	}

	if len(x509Cert.AuthorityKeyId) <= 0 {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate does not have authority key identifier")
	}

	return nil
}

func CheckCertValidity(cert string, startTime, endTime time.Time) error {
	x509Cert, err := ConvertPEMToX509(cert)
	if err != nil {
		return LwCAInvalidCert.MakeErr().WithDetail("Unable to convert cert to X509 cert")
	}

	if x509Cert.NotBefore.Unix() != startTime.Unix() {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate start time does not match")
	}

	if x509Cert.NotAfter.Unix() != endTime.Unix() {
		return LwCAInvalidCert.MakeErr().WithDetail("Certificate end time does not match")
	}

	return nil
}

func VerifyCert(cert, CACert string) error {
	roots := x509.NewCertPool()
	ok := roots.AppendCertsFromPEM([]byte(CACert))
	if !ok {
		return LwCAInvalidCert.MakeErr().WithDetail("Invalid CA Cert. Unable to add cert to certpool")
	}

	x509Cert, err := ConvertPEMToX509(cert)
	if err != nil {
		return LwCAInvalidCert.MakeErr().WithDetail("Unable to convert cert to X509 cert")
	}

	opts := x509.VerifyOptions{
		Roots: roots,
	}

	_, err = x509Cert.Verify(opts)
	if err != nil {
		return LwCAInvalidCert.MakeErr().MakeError("Cert verification failed", err)
	}

	return nil
}
