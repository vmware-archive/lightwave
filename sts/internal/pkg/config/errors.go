package config

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type ConfigError uint32

const (
	InvalidArgumentError ConfigError = ConfigError(diag.ConfigFacility) + iota + 1
	UnsupportedHTTPHeadError
	PrivateKeyMarshallError
	RandomGenerationError
	ConfigFileOpenError
	YamlDecodingError
	ConfigFileCreateError
	YamlEncodingError
	Base64DecodeError
	NewCipherError
	NewGCMError
	RandomNonceError
	GCMOpenError
	ConfigPropertyError
)

const (
	invalidArgError          = "invalid_argument"
	unsupportedHTTPHeadError = "unsupported_http_head"
	privateKeyMarshalError   = "unable_to_marshal_private_key"
	randomGenerationError    = "random_generation_error"
	configFileOpenError      = "config_file_open_error"
	yamlDecodingError        = "yaml_decoding_error"
	configFileCreateError    = "config_file_create_error"
	yamlEncodingError        = "yaml_encoding_error"
	base64DecodeError        = "base64_decode_error"
	newCipherError           = "new_cipher_error"
	newGCMError              = "new_gcm_error"
	randomNonceError         = "random_nonce_error"
	gcmOpenError             = "gcm_open_error"
	configPropertyError      = "config_property_error"
)

var errorText = map[ConfigError]string{
	InvalidArgumentError:     invalidArgError,
	UnsupportedHTTPHeadError: unsupportedHTTPHeadError,
	PrivateKeyMarshallError:  privateKeyMarshalError,
	RandomGenerationError:    randomGenerationError,
	ConfigFileOpenError:      configFileOpenError,
	YamlDecodingError:        yamlDecodingError,
	ConfigFileCreateError:    configFileCreateError,
	YamlEncodingError:        yamlEncodingError,
	Base64DecodeError:        base64DecodeError,
	NewCipherError:           newCipherError,
	NewGCMError:              newGCMError,
	RandomNonceError:         randomNonceError,
	GCMOpenError:             gcmOpenError,
	ConfigPropertyError:      configPropertyError,
}

func (c ConfigError) Code() uint32 {
	return uint32(c)
}

func (c ConfigError) Name() string {
	return errorText[c]
}
