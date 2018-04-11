package oidc

const (
	// LogLevelEmergency checks if the log level is EMERGENCY
	LogLevelEmergency = iota
	// LogLevelAlert checks if the log level is ALERT
	LogLevelAlert
	// LogLevelCritical checks if the log level is CRITICAL
	LogLevelCritical
	// LogLevelError checks if the log level is ERROR
	LogLevelError
	// LogLevelWarn checks if the log level is WARNING
	LogLevelWarn
	// LogLevelNotice checks if the log level is NOTICE
	LogLevelNotice
	// LogLevelInfo checks if the log level is INFORMATIONAL
	LogLevelInfo
	// LogLevelDebug checks if the log level is DEBUG
	LogLevelDebug
)

// Logger defines the interface for logging in the OIDC client
// Defines methods to log debug and error messages
type Logger interface {
	Emergencyf(format string, args ...interface{})
	Alertf(format string, args ...interface{})
	Criticalf(format string, args ...interface{})
	Errorf(format string, args ...interface{})
	Warnf(format string, args ...interface{})
	Noticef(format string, args ...interface{})
	Infof(format string, args ...interface{})
	Debugf(format string, args ...interface{})
}

// Implementation of logger, defines callback functions
type loggerImpl struct {
	emergencyFunc func(string, ...interface{})
	alertFunc     func(string, ...interface{})
	criticalFunc  func(string, ...interface{})
	errorFunc     func(string, ...interface{})
	warnFunc      func(string, ...interface{})
	noticeFunc    func(string, ...interface{})
	infoFunc      func(string, ...interface{})
	debugFunc     func(string, ...interface{})
}

func (l loggerImpl) Emergencyf(format string, args ...interface{}) {
	l.emergencyFunc(format, args...)
}

func (l loggerImpl) Alertf(format string, args ...interface{}) {
	l.alertFunc(format, args...)
}

func (l loggerImpl) Criticalf(format string, args ...interface{}) {
	l.criticalFunc(format, args...)
}

func (l loggerImpl) Errorf(format string, args ...interface{}) {
	l.errorFunc(format, args...)
}

func (l loggerImpl) Warnf(format string, args ...interface{}) {
	l.warnFunc(format, args...)
}

func (l loggerImpl) Noticef(format string, args ...interface{}) {
	l.noticeFunc(format, args...)
}

func (l loggerImpl) Debugf(format string, args ...interface{}) {
	l.debugFunc(format, args...)
}

func (l loggerImpl) Infof(format string, args ...interface{}) {
	l.infoFunc(format, args...)
}

// NewLogger creates a no-op logger
func NewLogger() Logger {
	noop := func(string, ...interface{}) {}

	l := &loggerImpl{
		emergencyFunc: noop,
		alertFunc:     noop,
		criticalFunc:  noop,
		errorFunc:     noop,
		warnFunc:      noop,
		noticeFunc:    noop,
		infoFunc:      noop,
		debugFunc:     noop,
	}

	return l
}

// NewLoggerBuilder creates a logger with logging call back functions set to no-op
func NewLoggerBuilder() loggerImpl {
	noop := func(string, ...interface{}) {}

	l := loggerImpl{
		emergencyFunc: noop,
		alertFunc:     noop,
		criticalFunc:  noop,
		errorFunc:     noop,
		warnFunc:      noop,
		noticeFunc:    noop,
		infoFunc:      noop,
		debugFunc:     noop,
	}

	return l
}

// Register associates a callback logging function with a log level and returns the loggerImpl
func (l loggerImpl) Register(logLevel int, callback func(format string, args ...interface{})) loggerImpl {
	if logLevel == LogLevelEmergency {
		l.emergencyFunc = callback
	}
	if logLevel == LogLevelAlert {
		l.alertFunc = callback
	}
	if logLevel == LogLevelCritical {
		l.criticalFunc = callback
	}
	if logLevel == LogLevelError {
		l.errorFunc = callback
	}
	if logLevel == LogLevelWarn {
		l.warnFunc = callback
	}
	if logLevel == LogLevelNotice {
		l.noticeFunc = callback
	}
	if logLevel == LogLevelDebug {
		l.debugFunc = callback
	}
	if logLevel == LogLevelInfo {
		l.infoFunc = callback
	}

	return l
}

func (l loggerImpl) Build() Logger {
	return l
}

// PrintLog prints to the logger with the correct method based on the log level
func PrintLog(logger Logger, logLevel int, format string, args ...interface{}) {
	if logLevel == LogLevelInfo {
		logger.Infof(format, args...)
	}

	if logLevel == LogLevelDebug {
		logger.Debugf(format, args...)
	}

	if logLevel == LogLevelWarn {
		logger.Warnf(format, args...)
	}

	if logLevel == LogLevelError {
		logger.Errorf(format, args...)
	}

	if logLevel == LogLevelEmergency {
		logger.Emergencyf(format, args...)
	}

	if logLevel == LogLevelAlert {
		logger.Alertf(format, args...)
	}

	if logLevel == LogLevelCritical {
		logger.Criticalf(format, args...)
	}

	if logLevel == LogLevelNotice {
		logger.Noticef(format, args...)
	}
}
