/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */



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
