security plugin for aws kms based encryption
================
aws kms security plugin implements envelope encryption for CA private keys using data keys obtained
from aws key management service.

In addition to providing encryption and decryption, openssl x509 sign, verify and key generation
are done by this implementation so as to keep plain text secrets within the plugin for the duration
of the operation. Plain text key information is not stored or handed out by this implementation.

This implementation relies on capability override for storage of encrypted data.

Configure
--------
1. customer master key id
2. aws credentials in default $HOME/.aws/credentials (this will change in the future)

Details to configure and configure file formats to follow.
