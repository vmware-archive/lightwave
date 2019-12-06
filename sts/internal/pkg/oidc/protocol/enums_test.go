package protocol

import (
	"strings"
	"testing"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

func TestResponseType(t *testing.T) {
	var ts ResponseTypeSet
	ts.From("token id_token")
	var hasToken, hasIdToken bool

	ts.Iterate(func(e ResponseType) diag.Error {
		if e == ResponseTypeToken {
			hasToken = true
		} else if e == ResponseTypeIdToken {
			hasIdToken = true
		}

		return nil
	})

	assertEquals(t, hasToken, true, "hasToken")
	assertEquals(t, hasIdToken, true, "hasIdToken")

	str := ts.String()
	assertEquals(t, strings.Index(str, " ") > -1, true, "has spaces")
}

func assertEquals(t *testing.T, actual interface{}, expected interface{}, valueName string) {
	if actual != expected {
		t.Fatalf("%s: actual value '%v' expected value '%v'", valueName, actual, expected)
	}
}
