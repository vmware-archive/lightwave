package diag

import (
	"fmt"
	"io"
	"log"
	"os"
	"sync/atomic"
)

// Component type defines logical components
// which produce logs
type Component string

const (
	// OIDC is used for OpenIDConnect logging
	OIDC Component = "OIDC"
	// SERVER is used for generic http server logging
	SERVER Component = "SERVER"
	// IDM is used for shared idm functionality logging
	IDM Component = "IDM"
	// SESSION is used for session management
	SESSION Component = "SESSION"
	// LDAP
	LDAP Component = "LDAP"
	// REST
	REST Component = "REST"
	// SETUP
	SETUP Component = "SETUP"
	// CONFIG
	CONFIG Component = "CONFIG"
)

// Logger defines logging routines
type Logger interface {
	Tracef(c Component, format string, args ...interface{})
	Infof(c Component, format string, args ...interface{})
	Warningf(c Component, format string, args ...interface{})
	Errorf(c Component, format string, args ...interface{})

	TraceEnabled(c Component) bool
	InfoEnabled(c Component) bool
	WarningEnabled(c Component) bool
}

// NewLogger returns a new Logger
func NewLogger() (Logger, error) {
	lgr := &loggerImpl{
		level:     TraceLvl,
		errLogger: log.New(os.Stderr, "", log.LstdFlags|log.LUTC),
		outLogger: log.New(os.Stdout, "", log.LstdFlags|log.LUTC),
	}
	return lgr, nil
}

// NewCmdLogger returns a new Logger
func NewCmdLogger(level int32, writer io.Writer) (Logger, error) {

	if level < ErrorLvl {
		level = ErrorLvl
	} else if level > TraceLvl {
		level = TraceLvl
	}

	logger := log.New(writer, "", log.LstdFlags|log.LUTC)
	lgr := &loggerImpl{
		level:     level,
		errLogger: logger,
		outLogger: logger,
	}
	return lgr, nil
}

// NewCtxAwareLogger returns a new Logger
func NewCtxAwareLogger(l Logger, ctx DiagContext) Logger {

	impl, ok := l.(*loggerImpl)
	if !ok || ctx == nil {
		return nil
	}

	lgr := &ctxAwareLoggerImpl{
		log: impl,
		ctx: ctx,
	}
	return lgr
}

func SafeString(s fmt.Stringer) string {
	if s == nil {
		return ""
	}
	return s.String()
}

// TODO: swap to ZAP
type loggerImpl struct {
	level     int32
	errLogger *log.Logger
	outLogger *log.Logger
}

type ctxAwareLoggerImpl struct {
	log *loggerImpl
	ctx DiagContext
}

func (l *loggerImpl) Tracef(c Component, format string, args ...interface{}) {
	l.log("", c, TraceLvl, "", format, args...)
}

func (l *loggerImpl) Infof(c Component, format string, args ...interface{}) {
	l.log("", c, InfoLvl, "", format, args...)
}

func (l *loggerImpl) Warningf(c Component, format string, args ...interface{}) {
	l.log("", c, WarningLvl, "", format, args...)
}

func (l *loggerImpl) Errorf(c Component, format string, args ...interface{}) {
	l.log("", c, ErrorLvl, "", format, args...)
}

func (l *loggerImpl) TraceEnabled(c Component) bool {
	return l.isEnabled("", c, TraceLvl)
}

func (l *loggerImpl) InfoEnabled(c Component) bool {
	return l.isEnabled("", c, InfoLvl)
}

func (l *loggerImpl) WarningEnabled(c Component) bool {
	return l.isEnabled("", c, WarningLvl)
}

func (l *loggerImpl) log(
	t TenantID, c Component, level int32, reqID string,
	format string, args ...interface{}) {
	if l.isEnabled(t, c, level) {
		if len(format) > 0 {
			lvlLen := len(logLevelMap[level])
			tenantLen := len(t.String())
			componentLen := len(c)
			reqIDLen := len(reqID)
			formatLen := len(format)
			fmtStr := make([]byte,
				lvlLen+tenantLen+3+componentLen+3+reqIDLen+3+formatLen,
				lvlLen+tenantLen+3+componentLen+3+reqIDLen+3+formatLen)
			copy(fmtStr, logLevelMap[level])
			copy(fmtStr[lvlLen:], "[")
			copy(fmtStr[lvlLen+1:], t.String())
			copy(fmtStr[lvlLen+1+tenantLen:], "] ")
			copy(fmtStr[lvlLen+1+tenantLen+2:], "[")
			copy(fmtStr[lvlLen+1+tenantLen+2+1:], c)
			copy(fmtStr[lvlLen+1+tenantLen+2+1+componentLen:], "] ")
			copy(fmtStr[lvlLen+1+tenantLen+2+1+componentLen+2:], "[")
			copy(fmtStr[lvlLen+1+tenantLen+2+1+componentLen+2+1:], reqID)
			copy(fmtStr[lvlLen+1+tenantLen+2+1+componentLen+2+1+reqIDLen:], "] ")
			copy(fmtStr[lvlLen+1+tenantLen+2+1+componentLen+2+1+reqIDLen+2:], format)
			if level >= InfoLvl {
				l.outLogger.Printf(string(fmtStr), args...)
			} else {
				l.errLogger.Printf(string(fmtStr), args...)
			}
		}
		if level == ErrorLvl {
			//l.errLogger.Println(string(debug.Stack()))
		}
	}
}

// TODO: level per component
func (l *loggerImpl) isEnabled(t TenantID, c Component, level int32) bool {
	return level <= atomic.LoadInt32(&l.level)
}

const (
	ErrorLvl   int32 = 1
	WarningLvl int32 = 2
	InfoLvl    int32 = 3
	TraceLvl   int32 = 4
)

var logLevelMap = map[int32]string{
	ErrorLvl:   "[ERROR] ",
	WarningLvl: "[WARN] ",
	InfoLvl:    "[INFO] ",
	TraceLvl:   "[TRACE] ",
}

func (l *ctxAwareLoggerImpl) Tracef(c Component, format string, args ...interface{}) {
	l.log.log(l.ctx.Tenant(), c, TraceLvl, l.ctx.RequestID(), format, args...)
}

func (l *ctxAwareLoggerImpl) Infof(c Component, format string, args ...interface{}) {
	l.log.log(l.ctx.Tenant(), c, InfoLvl, l.ctx.RequestID(), format, args...)
}

func (l *ctxAwareLoggerImpl) Warningf(c Component, format string, args ...interface{}) {
	l.log.log(l.ctx.Tenant(), c, WarningLvl, l.ctx.RequestID(), format, args...)
}

func (l *ctxAwareLoggerImpl) Errorf(c Component, format string, args ...interface{}) {
	l.log.log(l.ctx.Tenant(), c, ErrorLvl, l.ctx.RequestID(), format, args...)
}

func (l *ctxAwareLoggerImpl) TraceEnabled(c Component) bool {
	return l.log.isEnabled(l.ctx.Tenant(), c, TraceLvl)
}

func (l *ctxAwareLoggerImpl) InfoEnabled(c Component) bool {
	return l.log.isEnabled(l.ctx.Tenant(), c, InfoLvl)
}

func (l *ctxAwareLoggerImpl) WarningEnabled(c Component) bool {
	return l.log.isEnabled(l.ctx.Tenant(), c, WarningLvl)
}
