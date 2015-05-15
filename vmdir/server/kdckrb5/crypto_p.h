DWORD
VmKdcInitKrb5(
    OUT PVMKDC_KRB5_CONTEXT *ppRetKrb5
    );

DWORD
VmKdcInitCrypto(
    IN PVMKDC_KRB5_CONTEXT pKrb5,
    IN PVMKDC_KEY pKey,
    OUT PVMKDC_CRYPTO *ppRetCrypto
    );

VOID
VmKdcDestroyKrb5(
    IN PVMKDC_KRB5_CONTEXT pKrb5
    );

VOID
VmKdcDestroyCrypto(
    IN PVMKDC_CRYPTO pCrypto
    );

DWORD
VmKdcCryptoEncrypt(
    IN PVMKDC_CRYPTO pCrypto,
    IN VMKDC_KEY_USAGE keyUsage,
    IN PVMKDC_DATA pPlainText,
    OUT PVMKDC_DATA *ppVmKdcCipherText
    );

DWORD
VmKdcCryptoDecrypt(
    IN PVMKDC_CRYPTO pCrypto,
    IN VMKDC_KEY_USAGE keyUsage,
    IN PVMKDC_DATA pCipherText,
    OUT PVMKDC_DATA *pPlainText
    );
