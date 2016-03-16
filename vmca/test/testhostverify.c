#include "includes.h"

DWORD
MakeCSR(
    PCSTR szCNName,
    PCSTR szSANHostName,
    PCSTR szSANIPAddress
    PVMCA_CSR* pCSR
    )
{
    VMCA_PKCS_10_REQ_DATAA certRequestData = {0};
    PVMCA_KEY pPrivKey = NULL, pPubKey = NULL;
    PVMCA_CSR  pAllocatedCSR = NULL;
    DWORD dwError = ERROR_SUCCESS;

    if (pCSR)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    certRequestData.pszName = szCNName;
    certRequestData.pszDNSName = szSANHostName;
    certRequestData.pszIPAddress = szSANIPAddress;

    PVMCA_KEY pPrivKey = NULL, pPubKey = NULL;
    dwError = VMCACreatePrivateKeyA( NULL, 1024, &, &pPubKey );
    BAIL_ON_ERROR(dwError);

    dwError =  VMCACreateSigningRequest(
                &certRequestData,
                pPrivKey,
                NULL,
                &pAllocatedCSR);
    BAIL_ON_ERROR(dwError);

    *pCSR = pAllocatedCSR;

cleanup:
    VMCAFreeKey(pPrivKey);
    VMCAFreeKey(pPubKey);

    return dwError;
error:

    if (pCSR)
    {
        *pCSR = NULL;
    }
    VMCAFreeCSR(pAllocatedCSR);

    goto cleanup;
}

DWORD
TestVerifyHostName(
    PCSTR szHostName,
    PCSTR szHostIP,
    PCSTR szCN,
    PCSTR szDnsName,
    PCSTR szIpAddress
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMCA_CSR* pCSR = NULL;

    dwError = MakeCSR(szCN, szDnsName, szIpAddress, &pCSR);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAVerifyHostName(szHostName, szHostIP, pCSR);
    BAIL_ON_ERROR(dwError);

cleanup:
    VMCAFreeCSR(pCSR);

    return dwError;
error:

    goto cleanup;
}


void main()
{
    DWORD dwError = 0;
    dwError = TestVerifyHostName("sureshch-dev.eng.vmware.com", NULL, "sureshch-dev.eng.vmware.com", NULL, NULL);
}