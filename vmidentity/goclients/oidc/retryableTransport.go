package oidc

import (
	"fmt"
	"net"
	"net/http"
	"time"
)

// retryableTransportWrapper wraps the http transport so it can automatically retry the original request.
type retryableTransportWrapper struct {
	transport http.Transport
	logger    Logger
	interval  int
	retries   int
}

// NewRetryableTransportWrapper creates a retryable transport wrapper using a fixed interval and retries
func NewRetryableTransportWrapper(transport http.Transport, retries, retryInterval int, logger Logger) retryableTransportWrapper {
	return retryableTransportWrapper{
		transport: transport,
		logger:    logger,
		interval:  retryInterval,
		retries:   retries,
	}
}

// RoundTrip implements the RoundTripper interface.
// It submits the request object to same destination by calling the original RoundTrip method in Transport.
// It checks status code and finds out whether it is retryable or not. It will automatically
// resend the request if it is retryable.
func (r retryableTransportWrapper) RoundTrip(req *http.Request) (*http.Response, error) {
	var response *http.Response
	var err error

	requestStr := requestToStr(req)
	PrintLog(r.logger, LogLevelInfo, "Sending [%s]", requestStr)
	for i := 0; i < r.retries; i++ {
		var shouldRetry bool
		response, err = r.transport.RoundTrip(req)
		if err != nil {
			PrintLog(r.logger, LogLevelError, "Error in submitting [%s] request. Error: %+v", requestStr, err)

			if !r.isNetworkError(err) {
				return response, err
			}

			shouldRetry = true
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
func (r retryableTransportWrapper) isRetryable(code int) bool {
	return code/100 == 5
}

func (r retryableTransportWrapper) isNetworkError(err error) bool {
	if _, ok := err.(net.Error); ok {
		return true
	}

	return false
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
