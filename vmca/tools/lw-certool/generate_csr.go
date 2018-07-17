package main

import (
	"bytes"
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/pem"
	"io/ioutil"
	"log"
	"os"
	"strings"
)

// GenerateCSR generates a certificate signing request
func GenerateCSR(encodedKeyBytes []byte) (string, error) {
	keyBlock, _ := pem.Decode(encodedKeyBytes)
	if keyBlock == nil {
		log.Printf("[ERROR] Invalid private key that cannot be decoded for generating certificate signing request")
		return "", nil
	}
	if !strings.EqualFold(keyBlock.Type, RsaPrivateKeyPemType) {
		log.Printf("[ERROR] Invalid private key type %s for generating certificate signing request", keyBlock.Type)
		return "", nil
	}
	privKey, err := x509.ParsePKCS1PrivateKey(keyBlock.Bytes)
	if err != nil {
		log.Printf("[ERROR] Failed to parse private key. Error: '%s'\n", err.Error())
		return "", err
	}

	subj := pkix.Name{
		CommonName:         conf.CommonName,
		Country:            conf.Country,
		Province:           conf.State,
		Locality:           conf.Locality,
		Organization:       conf.Organization,
		OrganizationalUnit: conf.OrgUnit,
	}
	rawSubj := subj.ToRDNSequence()

	asn1Subj, _ := asn1.Marshal(rawSubj)
	template := x509.CertificateRequest{
		RawSubject:         asn1Subj,
		SignatureAlgorithm: x509.SHA256WithRSA,
		DNSNames:           conf.AltDomains,
	}

	csrBytes, err := x509.CreateCertificateRequest(rand.Reader, &template, privKey)
	if err != nil {
		log.Printf("[ERROR] Failed to create certificate signing request. Error: '%s'\n", err.Error())
		return "", err
	}

	var b bytes.Buffer
	err = pem.Encode(&b, &pem.Block{Type: CertificateRequestPemType, Bytes: csrBytes})
	if err != nil {
		log.Printf("[ERROR] Failed to encode certificate signing request. Error '%s'\n", err.Error())
		return "", err
	}

	return b.String(), nil
}

// GeneratePrivateKey generates a private key
func GeneratePrivateKey(filePath string) ([]byte, error) {
	key, err := rsa.GenerateKey(rand.Reader, keySize)
	if err != nil {
		log.Printf("[ERROR] Failed to generate private key. Error: '%s'\n", err.Error())
		return nil, err
	}

	encodedKeyBytes := pem.EncodeToMemory(&pem.Block{Type: RsaPrivateKeyPemType, Bytes: x509.MarshalPKCS1PrivateKey(key)})

	privKeyFileHandle, err := os.Create(filePath)
	if err != nil {
		log.Printf("[ERROR] Failed to create private key file. File Location: '%s', Error: '%s'\n", filePath, err.Error())
		return nil, err
	}
	defer privKeyFileHandle.Close()

	_, err = privKeyFileHandle.Write(encodedKeyBytes)
	if err != nil {
		log.Printf("[ERROR] Failed to write private key to file. File Location: '%s', Error '%s'\n", filePath, err.Error())
		return nil, err
	}

	os.Chmod(filePath, 0400)

	return encodedKeyBytes, nil
}

// ReadPrivateKey reads from given a private key
func ReadPrivateKey(filePath string) ([]byte, error) {
	encodedKeyBytes, err := ioutil.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	return encodedKeyBytes, nil
}
