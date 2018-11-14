package mutentcaclient

import (
	"log"
	"math/rand"
	"net/http"
	"strings"
	"time"
)

const (
	defaultPollingIntervalUnit = time.Second
	defaultPollingInterval     = 3
	defaultPollingTimeoutUnit  = time.Second
	defaultPollingTimeout      = 10

	errorDNSTempLookupFailure = "Temporary failure in name resolution"
)

// RetryableTransportWrapper wraps the original ClientTransport. It augments the swagger-generated transport with the
// ability to automatically retry the original request.
type RetryableTransportWrapper struct {
	transport    http.Transport
	IntervalUnit time.Duration
	Interval     int
	TimeoutUnit  time.Duration
	Timeout      int
}

// NewRetryableTransportWrapper creates a retryable transport wrapper using fix interval plus jitter
func NewRetryableTransportWrapper(transport http.Transport) *RetryableTransportWrapper {
	return &RetryableTransportWrapper{
		transport:    transport,
		IntervalUnit: defaultPollingIntervalUnit,
		Interval:     defaultPollingInterval,
		TimeoutUnit:  defaultPollingTimeoutUnit,
		Timeout:      defaultPollingTimeout,
	}
}

// RoundTrip implements the RoundTripper interface.
// It submits the request object to same destination by calling the original RoundTrip method in Transport.
// It checks status code and finds out whether it is retryable or not. It will automatically
// resend the request if it is retryable.
func (r *RetryableTransportWrapper) RoundTrip(req *http.Request) (*http.Response, error) {
	var response *http.Response
	var err error

	sleepTime := time.Duration(r.Interval) * r.IntervalUnit
	deadline := time.Now().Add(time.Duration(r.Timeout) * r.TimeoutUnit)
	count := 0

	for time.Now().Before(deadline) {
		response, err = r.transport.RoundTrip(req)

		// Return directly if there is any error
		if err != nil {
			if strings.Contains(err.Error(), errorDNSTempLookupFailure) {
				log.Printf("DNS Lookup failed. Error: %+v. Will retry after %v seconds.", err, sleepTime)
				time.Sleep(sleepTime)
				continue
			}

			log.Printf("Error in submitting request. Error: %+v", err)
			return response, err
		}

		code := response.StatusCode

		if code/100 == 2 {
			return response, nil
		}

		// Return the error directly if this is not a status unauthorized error
		if !r.isRetryable(code) {
			return response, err
		}

		count++
		jitter := time.Duration(rand.Float64()*5) * time.Second
		delay := sleepTime + jitter
		log.Printf("Status Code: %+v Sleeping %v before retry. Round: %v", code, delay, count)
		time.Sleep(delay)
	}

	log.Printf("Timeout after retrying on submitting request. Error: %+v", err)
	return response, err
}

// isRetryable decides whether the error is retryable.
func (r *RetryableTransportWrapper) isRetryable(code int) bool {
	// Should retry original requests that receive server (5xx) or throttling errors. However, client errors (4xx)
	// indicate that you need to revise the request to correct the problem before trying again.
	return code/100 == 5 || code == 429
}
