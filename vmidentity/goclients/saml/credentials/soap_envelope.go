package credentials

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/x509"
	"encoding/base64"
	"fmt"
	"io"
	"time"

        "github.com/sabertail/signedxml"
)

func getSoapRequestWithPassword(username, password string) string {
	start := time.Now()
	end := start.Add(time.Duration(24) * time.Hour)

	// 2018-05-02T19:35:00.000Z
	// RFC3339     = "2006-01-02T15:04:05Z07:00"
	startTime := start.UTC().Format(time.RFC3339)
	endTime := end.UTC().Format(time.RFC3339)

	credential := fmt.Sprintf(usernameTokenRequestTemplate, username, password)
	header := fmt.Sprintf(soapSecurityHeaderTemplate, fmt.Sprintf(wsuTimestampTemplate, "", startTime, endTime), credential, "")
	body := fmt.Sprintf(soapBodyTemplate, "", keyTypeBearer, "", startTime, endTime, "")

	return fmt.Sprintf(soapEnvTemplate, header, body)
}

func getSoapRequestWithCert(cert *x509.Certificate, privateKey *rsa.PrivateKey) (string, error) {
	start := time.Now()
	end := start.Add(time.Duration(24) * time.Hour)

	// 2018-05-02T19:35:00.000Z
	// RFC3339     = "2006-01-02T15:04:05Z07:00"
	startTime := start.UTC().Format(time.RFC3339)
	endTime := end.UTC().Format(time.RFC3339)
	timestampId, err := newUUID()
	if err != nil {
		return "", err
	}
	timestampToken := fmt.Sprintf(wsuTimestampTemplate, fmt.Sprintf(wsuIdTemplate, timestampId), startTime, endTime)

	bstId, err := newUUID()
	if err != nil {
		return "", err
	}

	binarySecurityToken := fmt.Sprintf(binarySecurityTokenRequestTemplate, fmt.Sprintf(wsuIdTemplate, bstId), base64.StdEncoding.EncodeToString(cert.Raw))
	signatureId, err := newUUID()
	if err != nil {
		return "", err
	}
	bodyId, err := newUUID()
	if err != nil {
		return "", err
	}
	body := fmt.Sprintf(soapBodyTemplate, fmt.Sprintf(wsuIdTemplate, bodyId), keyTypePublicKey, sigAlg, startTime, endTime, fmt.Sprintf(useKeyTemplate, signatureId))

	signedInfo := fmt.Sprintf(signedInfoTemplate, bodyId, timestampId)
	signature := fmt.Sprintf(signatureTemplate, signatureId, signedInfo, bstId)
	header := fmt.Sprintf(soapSecurityHeaderTemplate, timestampToken, binarySecurityToken, signature)
	unsignedEnvelope := fmt.Sprintf("%s%s", header, body)

	signer, err := signedxml.NewSigner(unsignedEnvelope)
	if err != nil {
		return "", err
	}

	signedRequest, err := signer.Sign(privateKey)
	if err != nil {
		return "", err
	}

	return fmt.Sprintf(soapEnvTemplate, signedRequest, ""), nil
}

// Generate a random UUID with a "_" prefix to be of type NCName
func newUUID() (string, error) {
	uuid := make([]byte, 16)
	n, err := io.ReadFull(rand.Reader, uuid)
	if n != len(uuid) || err != nil {
		return "", err
	}

	uuid[8] = uuid[8]&^0xc0 | 0x80
	uuid[6] = uuid[6]&^0xf0 | 0x40
	return "_" + fmt.Sprintf("%x-%x-%x-%x-%x", uuid[0:4], uuid[4:6], uuid[6:8], uuid[8:10], uuid[10:]), nil
}
