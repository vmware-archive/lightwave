package oidc

import (
	"bytes"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"sync/atomic"
	"time"
)

// retryableTransportWrapper wraps the http transport so it can automatically retry the original request.
type retryableTransportWrapper struct {
	transport       atomic.Value
	logger          Logger
	interval        int
	retries         int
	certRefreshHook CertRefreshHook
	httpConfig      HTTPClientConfig
}

// NewRetryableTransportWrapper creates a retryable transport wrapper using a fixed interval and retries
func NewRetryableTransportWrapper(httpConfig HTTPClientConfig, retries, retryInterval int, hook CertRefreshHook, logger Logger) *retryableTransportWrapper {
	var httpTransport atomic.Value
	httpTransport.Store(httpTransportFromConfig(httpConfig))

	return &retryableTransportWrapper{
		transport:       httpTransport,
		logger:          logger,
		interval:        retryInterval,
		retries:         retries,
		certRefreshHook: hook,
		httpConfig:      httpConfig,
	}
}

// RoundTrip implements the RoundTripper interface.
// It submits the request object to same destination by calling the original RoundTrip method in Transport.
// It checks status code and finds out whether it is retryable or not. It will automatically
// resend the request if it is retryable.
func (r *retryableTransportWrapper) RoundTrip(req *http.Request) (*http.Response, error) {
	var response *http.Response
	var err error
	var bodyStr []byte

	requestStr := requestToStr(req)

	if req.Body != nil {
		bodyStr, err = ioutil.ReadAll(req.Body)
		if err != nil {
			return nil, err
		}
		req.Body.Close()
	}

	PrintLog(r.logger, LogLevelInfo, "Sending [%s]", requestStr)
	for i := 0; i < r.retries; i++ {
		var shouldRetry bool
		certsRefreshed := false

		if req.Body != nil {
			req.Body = ioutil.NopCloser(bytes.NewBuffer(bodyStr))
		}

		httpTransport := r.transport.Load().(*http.Transport)
		response, err = httpTransport.RoundTrip(req)
		if err != nil {
			PrintLog(r.logger, LogLevelError, "Error in submitting [%s] request. Error: %+v", requestStr, err)

			if _, ok := err.(x509.UnknownAuthorityError); ok && r.certRefreshHook != nil {
				certsRefreshed = r.refreshRootCerts()
			}

			if r.isNetworkError(err) || certsRefreshed {
				shouldRetry = true
			} else {
				return response, err
			}
		} else {
			code := response.StatusCode
			if code == http.StatusOK {
				return response, nil
			}

			shouldRetry = r.isRetryable(code)
		}

		if !shouldRetry {
			break
		}

		PrintLog(r.logger, LogLevelInfo, "[Retry %d] %s failed, retrying", i, requestStr)
		time.Sleep(time.Duration(r.interval) * time.Millisecond)
	}

	PrintLog(r.logger, LogLevelError, "Error: [%s] request failed, err: %+v", requestStr, err)

	return response, err
}

// isRetryable checks whether error codes are retryable. Retryable errors are any server related error codes (5XX)
func (r *retryableTransportWrapper) isRetryable(code int) bool {
	return code/100 == 5
}

func (r *retryableTransportWrapper) isNetworkError(err error) bool {
	if _, ok := err.(net.Error); ok {
		return true
	}

	return false
}

func (r *retryableTransportWrapper) refreshRootCerts() bool {
	PrintLog(r.logger, LogLevelInfo, "Refreshing root certs")
	refreshedCerts, err := r.certRefreshHook()
	if err != nil {
		PrintLog(r.logger, LogLevelError, "Failed to refresh root certs, Error: %+v", err)
		return false
	}

	if refreshedCerts != nil {
		//Build new transport and replace atomically
		transport := httpTransportFromConfig(r.httpConfig)
		transport.TLSClientConfig.RootCAs = refreshedCerts
		transport.TLSClientConfig.BuildNameToCertificate()
		r.transport.Store(transport)

		PrintLog(r.logger, LogLevelInfo, "Root certs successfully updated")
	} else {
		PrintLog(r.logger, LogLevelError, "Returned cert bundle from certRefresh hook is empty")
		return false
	}

	return true
}

func requestToStr(r *http.Request) string {
	if r == nil {
		return ""
	}

	method := r.Method
	url := r.URL
	path := "<nil>"
	scheme := ""
	if url != nil {
		path = url.Path
		scheme = url.Scheme
	}
	return fmt.Sprintf("HTTP %s %s://%s%s", method, scheme, r.Host, path)
}
