package protocol

import (
	"fmt"
	"net/url"
	"strings"

	"github.com/francoispqt/gojay"
	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type stringMarshaler interface {
	String() string
}
type stringUnMarshaler interface {
	From(str string) diag.Error
}

/*
type QueryMarshaler interface {
	MarshalQuery(w io.Writer) diag.Error
}
type QueryUnMarshaler interface {
	UnMarshalQuery(vals url.Values) diag.Error
}
*/

func getSingleQueryParam(name string, params url.Values, required bool, code OidcError) (string, diag.Error) {
	vals, ok := params[name]
	var val string
	var err diag.Error
	if !ok && required {
		err = diag.MakeError(
			code,
			fmt.Sprintf("'%s' parameter is required", name),
			nil)
		return "", err
	}
	if required && len(vals) == 0 {
		err = diag.MakeError(
			code,
			fmt.Sprintf("'%s' parameter is required", name),
			nil)
		return "", err
	}
	if len(vals) > 1 {
		err = diag.MakeError(
			code,
			fmt.Sprintf("expecting single '%s' parameter", name),
			nil)
		return "", err
	}

	if len(vals) > 0 {
		val = vals[0]
	}

	if required && len(val) <= 0 {
		err = diag.MakeError(
			code,
			fmt.Sprintf("'%s' parameter is required", name),
			nil)
		return "", err
	}
	return val, nil
}

type modifiableStringSet interface {
	stringUnMarshaler
	auth.StringSet
	Add(val string)
}

type stringSetImpl map[string]struct{}

func (s *stringSetImpl) Len() uint {
	if s == nil || (*s == nil) {
		return 0
	}
	return uint(len(*s))
}

func (s *stringSetImpl) Contains(v string) bool {
	if s == nil || (*s == nil) {
		return false
	}
	_, ok := (*s)[v]
	return ok
}

func (s *stringSetImpl) String() string {
	if s == nil || s.Len() <= 0 {
		return ""
	}
	sb := &strings.Builder{}
	first := true
	for k := range *s {
		if !first {
			sb.WriteString(" ")
		} else {
			first = false
		}
		sb.WriteString(k)
	}
	return sb.String()
}

func (s *stringSetImpl) Iterate(f auth.StringSetIteratorFunc) diag.Error {
	if s == nil || *s == nil {
		return nil
	}
	var err diag.Error
	for k := range *s {
		err = f(k)
		if err != nil {
			return err
		}
	}
	return nil
}

func (s *stringSetImpl) From(text string) diag.Error {
	if s == nil {
		return diag.MakeError(OidcErrorServerError, "Unable to unmarshal 'StringSet' into nil", nil)
	}
	arr := strings.Fields(text)
	*s = make(map[string]struct{}, len(arr))
	for _, v := range arr {
		(*s)[v] = exists
	}
	return nil
}

func (s *stringSetImpl) Add(v string) {
	if s == nil || (*s == nil) {
		return
	}
	(*s)[v] = exists
}

func (s *stringSetImpl) UnmarshalJSONArray(dec *gojay.Decoder) error {
	if (*s) == nil {
		(*s) = make(stringSetImpl)
	}
	var str string
	err := dec.String(&str)
	if err != nil {
		return err
	}
	s.Add(str)
	return nil
}

func (s *stringSetImpl) MarshalJSONArray(enc *gojay.Encoder) {
	if s.Len() > 0 {
		for k := range *s {
			enc.String(k)
		}
	}
}
func (s *stringSetImpl) IsNil() bool {
	return s.Len() == 0
}
