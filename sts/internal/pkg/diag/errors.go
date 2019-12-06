package diag

// ErrorCode is unified definition of numberic error codes
type ErrorCode interface {
	Code() uint32
	Name() string // short name
}

// HTTPStatusMappedErrorCode can be implemented if an error is associated
// with a specific http status
type HTTPStatusMappedErrorCode interface {
	ErrorCode
	HttpStatus() int
}

type Facility int32

// pre-defined error codes
const (
	// facilities/components
	ServerFacility        Facility = 1 << 16
	OidcFacility          Facility = 2 << 16
	IdmFacility           Facility = 3 << 16
	SessionFacility       Facility = 4 << 16
	StaticContentFacility Facility = 5 << 16
	LdapFacility          Facility = 6 << 16
	ConfigFacility        Facility = 7 << 16
)

type Error interface {
	error
	Code() ErrorCode
	Cause() error
	//Stack() *[]byte
}

// Error is unified error
type errorImpl struct {
	C   ErrorCode
	Msg string
	Err error
	//S   *[]byte
}

// Error implements error
func (e *errorImpl) Error() string {
	return e.Msg
}

func (e *errorImpl) Code() ErrorCode { return e.C }
func (e *errorImpl) Cause() error    { return e.Err }

//func (e *errorImpl) Stack() *[]byte  { return e.S }

// IsErrorWithCode checks whether the specified error is error with specifier ErrorCode
func IsErrorWithCode(err error, c ErrorCode) bool {
	e, ok := err.(Error)
	if !ok {
		return false
	}

	return e.Code() == c
}

// IsErrorWithFacility checks whether the specified error is error with specifier ErrorCode
func IsErrorWithFacility(err error, f Facility) bool {
	e, ok := err.(Error)
	if !ok {
		return false
	}

	code := e.Code().Code()
	code = code & 0xFFFF0000
	return code == uint32(f)
}

// IsError checks whether the specified error is diag.Error
func IsError(err error) bool {
	_, ok := err.(Error)
	if !ok {
		return false
	}

	return true
}

// IsHTTPStatusMappedError checks whether the specified error is diag.Error
func IsHTTPStatusMappedError(err error) bool {
	e, ok := err.(Error)
	if !ok {
		return false
	}

	_, statusOK := e.Code().(HTTPStatusMappedErrorCode)
	return statusOK
}

// MakeError makes an error from errorcode, message, and optional error
func MakeError( /*log Logger, */ c ErrorCode, msg string, err error) Error {
	return &errorImpl{
		C:   c,
		Msg: msg,
		Err: err,
	}
}
