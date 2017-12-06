package afdclient

/*
#cgo CFLAGS: -I/opt/vmware/include -I/opt/likewise/include
#cgo LDFLAGS: -L/opt/vmware/lib64 -lvmafdclient -Wl,-rpath=/opt/vmware/lib64
#include <stdlib.h>
#include <vmafdclient.h>
*/
import "C"

import (
	"errors"
	"unsafe"
)

// goStringToCString returns a C string allocated on the C heap.
// The returned string needs to be freed via freeCString().
func goStringToCString(gostr string) (cstr C.PSTR) {
	if gostr == "" {
		cstr = nil // go's empty string becomes null as that is how we do OPTIONAL parameters in c
	} else {
		cstr = C.CString(gostr)
	}
	return cstr
}

// cStringToGoString converts a C string to a Go string.
func cStringToGoString(cstr C.PSTR) (gostr string) {
	if cstr == nil {
		gostr = ""
	} else {
		gostr = C.GoString(cstr)
	}
	return gostr
}

// vmafdStringToGoString frees the C string after copying it to a Go string.
func vmafdStringToGoString(cstr C.PSTR) (gostr string) {
	if cstr == nil {
		gostr = ""
	} else {
		gostr = C.GoString(cstr)
		C.VmAfdFreeString(cstr)
	}
	return gostr
}

// cErrorToGoError generates a string representation of the VMAFD error code.
func cErrorToGoError(e C.DWORD) (err error) {
	var errCStr C.PSTR
	C.VmAfdGetErrorMsgByCode(e, &errCStr)
	return errors.New(vmafdStringToGoString(errCStr))
}

// freeCString frees a C string from the C heap.
func freeCString(cstr C.PSTR) {
	if cstr != nil {
		C.free(unsafe.Pointer(cstr))
	}
}
