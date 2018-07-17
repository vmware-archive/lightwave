package main

import (
	"crypto/tls"
	"fmt"
	"io"
	"net/http"
	"time"
)

const (
	urlFormat = "https://%s:%s%s"

	authHeaderKey    = "Authorization"
	authHeaderFormat = "Bearer %s"
)

func RestAPIRequest(token string, host string, method string, endpoint string, body io.Reader) (*http.Response, error) {
	url := fmt.Sprintf(urlFormat, host, VmcaHTTPSPort, endpoint)

	tr := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: disableSSL},
	}

	netClient := &http.Client{
		Timeout:   time.Second * 10,
		Transport: tr,
	}

	req, err := http.NewRequest(method, url, body)
	if err != nil {
		return nil, err
	}

	authToken := fmt.Sprintf(authHeaderFormat, token)
	req.Header.Set(authHeaderKey, authToken)

	response, err := netClient.Do(req)
	if err != nil {
		return nil, err
	}

	return response, nil
}
