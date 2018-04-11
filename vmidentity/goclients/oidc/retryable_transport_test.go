package oidc

import (
	"github.com/stretchr/testify/assert"
	"net/http"
	"net/http/httptest"
	"net/url"
	"testing"
)

// TestVmDirClient does a basic test to make sure the client can talk to server
func TestRetryableTransportWrapper(t *testing.T) {
	config := NewHTTPClientConfig()
	config.SkipCertValidationField = true
	tr := httpTransportFromConfig(config)

	retryableTransport := NewRetryableTransportWrapper(*tr, defaultRetries, 50, NewLogger())
	httpClient := &http.Client{Transport: retryableTransport}

	testRetryableTransportWrapperRetries(httpClient, 500, defaultRetries, 200, t)
	testRetryableTransportWrapperRetries(httpClient, 500, defaultRetries + 1, 500, t)
	// no retries
	testRetryableTransportWrapperRetries(httpClient, 404, defaultRetries, 404, t)

}

func testRetryableTransportWrapperRetries(client *http.Client, code int, retries int, resultCode int, t *testing.T) {
	resp := testRetryableTransportGet(client, code, retries, t)
	assert.Equal(t, resp.StatusCode, resultCode, "Retries should result in HTTP Status %d", resultCode)

	resp = testRetryableTransportPost(client, code, retries, t)
	assert.Equal(t, resp.StatusCode, resultCode, "Retries should result in HTTP Status %d", resultCode)
}

func testRetryableTransportGet(client *http.Client, code int, retries int, t *testing.T) http.Response {
	count := 0
	ts := httptest.NewTLSServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		count++
		// Write success after retrying times
		if count == retries {
			w.WriteHeader(200)
		} else {
			w.WriteHeader(code)
		}
		w.Write([]byte("OK"))
	}))
	defer ts.Close()

	resp, err := client.Get(ts.URL)
	assert.Nil(t, err, "GET request with retries should succeed")
	assert.NotNil(t, resp, "GET request with retries should have a response")

	return *resp
}

func testRetryableTransportPost(client *http.Client, code int, retries int, t *testing.T) http.Response {
	count := 0
	ts := httptest.NewTLSServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		count++
		// Write success after retrying times
		if count == retries {
			w.WriteHeader(200)
		} else {
			w.WriteHeader(code)
		}
		w.Write([]byte("OK"))
	}))
	defer ts.Close()

	data := url.Values{
		"grant_type": {"password"},
	}

	resp, err := client.PostForm(ts.URL, data)
	assert.Nil(t, err, "POST request with retries should succeed")
	assert.NotNil(t, resp, "POST request with retries should have a response")

	return *resp
}
