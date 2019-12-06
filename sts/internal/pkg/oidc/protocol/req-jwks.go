package protocol

import (
	"io"

	"github.com/francoispqt/gojay"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	jose "gopkg.in/square/go-jose.v2"
)

type jwksResponse jose.JSONWebKeySet

func NewJWKSResponse(ks *jose.JSONWebKeySet) OidcResponse {
	return (*jwksResponse)(ks)
}
func (r *jwksResponse) State() string { return "" }

func (r *jwksResponse) Marshal(w io.Writer, format MarshalFormat) diag.Error {
	if format != MarshalFormatJSON {
		return diag.MakeError(OidcErrorEncodeError, "Type 'jwks response' only supports JSON serialization", nil)
	}

	// todo: this will fall back to reflection, consider re-implementing
	bytes, err := gojay.MarshalAny(r)
	if err != nil {
		return diag.MakeError(OidcErrorEncodeError, "Failed to serialize Json", err)
	}
	_, err = w.Write(bytes)
	if err != nil {
		return diag.MakeError(OidcErrorStreamWriteError, "Failed writing to stream", err)
	}

	return nil
}
