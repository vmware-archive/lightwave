package protocol

import (
	"fmt"
	"io"
	"net/http"
	"net/url"

	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
)

//

// oidc:"marshal:dec=q;err=OidcErrorInvalidRequest"
type logoutRequest struct {
	token       string   // oidc:"marshal:name=id_token_hint"
	redirectURI *url.URL // oidc:"marshal:name=post_logout_redirect_uri;omitempty"
	state       string   // oidc:"marshal:name=state;omitempty"
	session     session.SessionID
	idToken     auth.Token
}

func (r *logoutRequest) TokenHint() auth.Token { return r.idToken }

func (r *logoutRequest) RedirectURI() *url.URL { return r.redirectURI }

func (r *logoutRequest) State() string { return r.state }

func (r *logoutRequest) Session() session.SessionID { return r.session }

func (r *logoutRequest) ClientID() string {
	if r == nil || r.idToken == nil {
		return ""
	}
	return r.idToken.Audience().String() // todo: nicer way to get first string ?
}

func ParseLogoutRequest(
	req *http.Request, tokenValidator TokenValidatorFunc, uriValidator UriValidatorFunc, ctxt diag.RequestContext) (LogoutRequest, diag.Error) {
	log := ctxt.Logger()

	errURL := req.ParseForm()
	if errURL != nil {
		log.Errorf(diag.OIDC, "Failed to parse request url: %v", errURL)
		return nil, diag.MakeError(OidcErrorInvalidRequest, "Invalid query parameter", errURL)
	}
	vals := req.Form

	logout := &logoutRequest{session: session.NoneSessionID}
	var err diag.Error
	var err1 diag.Error

	err = logout.UnMarshalQuery(vals)
	if err != nil {
		log.Errorf(diag.OIDC, "Failed to parse logout request: %v", err)
	}

	if len(logout.token) > 0 {
		logout.idToken, err1 = tokenValidator(logout.token, ctxt)
		if err1 != nil && err == nil {
			err = err1
		}
	}

	if logout.redirectURI != nil { // optional
		_, uriErr := uriValidator(logout, ctxt)
		if uriErr != nil {
			logout.redirectURI = nil
			if err == nil {
				err = uriErr
			}
		}
	}

	if err == nil {
		err = logout.validate(ctxt)
	}
	if err != nil {
		return nil, err
	}

	return logout, err
}

func (r *logoutRequest) validate(ctxt diag.RequestContext) diag.Error {
	// todo: any other validations?
	return nil
}

type logoutResponseImpl struct {
	redirectURI *url.URL
	state       string
	logoutUris  []*url.URL
}

func NewLogoutResponse(ctxt OidcResponseCtxt, logouts []*url.URL) OidcLogoutResponse {
	return &logoutResponseImpl{redirectURI: ctxt.RedirectURI(), state: ctxt.State(), logoutUris: logouts}
}

func (r *logoutResponseImpl) State() string { return r.state }

func (r *logoutResponseImpl) Marshal(w io.Writer, format MarshalFormat) diag.Error {
	if format != MarshalFormatHTML {
		return diag.MakeError(OidcErrorEncodeError,
			fmt.Sprintf("Type 'logoutResponse' does not support serialization format %v", format), nil)
	}

	redirStr := ""
	if r.redirectURI != nil {
		redirStr = r.redirectURI.String()
		if len(r.state) > 0 {
			if len(r.redirectURI.RawQuery) > 0 {
				redirStr = redirStr + "&"
			} else {
				redirStr = redirStr + "?"
			}
			redirStr = redirStr + url.QueryEscape("state") + "=" + url.QueryEscape(r.state)
		}
	}
	_, err := fmt.Fprintf(w, htmlResponseStart, redirStr)
	if err != nil {
		return diag.MakeError(OidcErrorStreamWriteError,
			fmt.Sprintf("Failed writing 'logoutResponse' serialization: %v", err), err)
	}
	for _, link := range r.logoutUris {
		if link != nil {
			_, err = fmt.Fprintf(w, iframe, link.String())
			if err != nil {
				return diag.MakeError(OidcErrorStreamWriteError,
					fmt.Sprintf("Failed writing 'logoutResponse' serialization: %v", err), err)
			}
		}
	}
	_, err = w.Write([]byte(htmlresponseEnd))
	if err != nil {
		return diag.MakeError(OidcErrorStreamWriteError,
			fmt.Sprintf("Failed writing 'logoutResponse' serialization: %v", err), err)
	}

	return nil
}

const (
	htmlResponseStart = "<html>" +
		"    <head>" +
		"        <script type=\"text/javascript\">" +
		"            var postLogoutRedirectUriWithState = \"%s\";" +
		"            if (postLogoutRedirectUriWithState != \"\") {" +
		"                window.onload = function() {" +
		"                    document.location = postLogoutRedirectUriWithState;" +
		"                }" +
		"            }" +
		"        </script>" +
		"    </head>" +
		"    <body>"

	htmlresponseEnd = "    </body>" +
		"</html>"

	iframe = "<iframe src=\"%s\" />"
)
