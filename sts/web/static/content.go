package static

import (
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"net/http"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/web/static/assets"
	"github.com/vmware/lightwave/sts/web/static/templates"
)

var compiledTemplates map[string]*template.Template

var loginPageDefaults = &LoginPageParameters{
	LoginTitle:          "login",
	WelcomeTo:           "Welcome to",
	TenantBrandName:     "Lightwave Authentication Services",
	SignInText:          "Please sign-in here",
	UserNamePlaceHolder: "user@lightwave.local",
	Login:               "Login",
	EnablePasswordAuth:  true,
	Protocol:            "openidconnect",
	ResponseMode:        "form_post",
	GenericError:        "Login failed: ",
	ResponsePostForm:    "responsePostForm",
	ResourcesPath:       "/resources",
}

const (
	loginPage = "login.html"
)

type LoginPageParameters struct {
	LoginTitle          string
	WelcomeTo           string
	TenantBrandName     string
	SignInText          string
	UserNamePlaceHolder string
	Login               string
	EnablePasswordAuth  bool
	Protocol            string
	ResponseMode        string
	GenericError        string
	ResponsePostForm    string
	ResourcesPath       string
}

func Init(l diag.Logger) diag.Error {
	compiledTemplates = make(map[string]*template.Template, 2)

	err := initPageTemplate(loginPage)
	if err != nil {
		return err
	}
	return nil
}
func initPageTemplate(tname string) diag.Error {
	file, err := templates.Templates.Open(tname)
	if err != nil {
		return diag.MakeError(templateReadError,
			fmt.Sprintf("Failed opening template: '%s': %v", tname, err), err)
	}
	content, err := ioutil.ReadAll(file)
	file.Close()
	if err != nil {
		return diag.MakeError(templateReadError,
			fmt.Sprintf("Failed reading template: '%s': %v", tname, err), err)
	}
	t, err := template.New(tname).Parse(string(content))
	if err != nil {
		return diag.MakeError(templateParseError,
			fmt.Sprintf("Failed parsing template: '%s': %v", tname, err), err)
	}
	compiledTemplates[tname] = t

	return nil
}

func ServeLoginPage(writer io.Writer, params *LoginPageParameters) diag.Error {
	if params == nil {
		params = loginPageDefaults
	}
	t := compiledTemplates[loginPage]
	err := t.Execute(writer, params)
	if err != nil {
		return diag.MakeError(templateExecuteError,
			fmt.Sprintf("Failed executing template: '%s': %v", loginPage, err), err)
	}
	return nil
}

func StaticContentHandler() http.Handler {
	return http.FileServer(assets.Assets)
}

type staticsError uint32

const (
	templateReadError    staticsError = staticsError(diag.StaticContentFacility) + 0x001
	templateParseError   staticsError = staticsError(diag.StaticContentFacility) + 0x002
	templateExecuteError staticsError = staticsError(diag.StaticContentFacility) + 0x003

	readError    = "read_error"
	parseError   = "parse_error"
	executeError = "execute_error"
)

var errorText = map[staticsError]string{
	templateReadError:    readError,
	templateParseError:   parseError,
	templateExecuteError: executeError,
}

func (c staticsError) Code() uint32 {
	return uint32(c)
}

func (c staticsError) Name() string {
	return errorText[c]
}
