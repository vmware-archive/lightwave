/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#ifndef DCCACLIENT_H_
#define DCCACLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Enables client affinity
 *
 * @param[in]  pServer Host server struct
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnableClientAffinity(
        PVMAFD_SERVER pServer
        );

/*
 * @brief Disable client affinity feature
 *
 * @param[in]  pServer Host server struct
 *
 * @return Returns 0 for success
 */
DWORD
CdcDisableClientAffinity(
        PVMAFD_SERVER pServer
        );

/*
 * @brief Returns the affinitized DC for the node
 *
 * @param[in]           pServer Host server struct
 * @param[in,optional]  pszDomainName Domain Name
 * @param[in,optional]  pDomainGuid
 * @param[in,optional]  pszSiteName Site Name
 * @param[in,optional]  dwFlags Flags to filter the result
 * @param[out]          ppDomainControllerInfo Pointer to a struct to hold DC info
 *
 * @return Returns 0 for success
 */
DWORD
CdcGetDCNameA(
        PVMAFD_SERVER  pServer,
        PCSTR          pszDomainName,
        GUID_A         pDomainGuid,
        PCSTR          pszSiteName,
        DWORD          dwFlags,
        PCDC_DC_INFO_A *ppDomainControllerInfo
        );

/*
 * @brief Returns the affinitized DC for the node
 *
 * @param[in]           pServer Host server struct
 * @param[in,optional]  pwszDomainName Domain Name
 * @param[in,optional]  pDomainGuid
 * @param[in,optional]  pwszSiteName Site Name
 * @param[in,optional]  dwFlags Flags to filter the result
 * @param[out]          ppDomainControllerInfo Pointer to a struct to hold DC info
 *
 * @return Returns 0 for success
 */

DWORD
CdcGetDCNameW(
        PVMAFD_SERVER  pServer,
        PCWSTR         pszDomainName,
        GUID_W         pDomainGuid,
        PCWSTR         pszSiteName,
        DWORD          dwFlags,
        PCDC_DC_INFO_W *ppDomainControllerInfo
        );

/*
 * @brief Lists entries in client side cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          ppszDCEntries String array to hold DC entries
 * @param[out]          pdwCount Returns the count of entries
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnumDCEntriesA(
        PVMAFD_SERVER pServer,
        PSTR          **ppszDCEntries,
        PDWORD        pdwCount
        );

/*
 * @brief Lists entries in client side cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          ppszDCEntries String array to hold DC entries
 * @param[out]          pdwCount Returns the count of entries
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnumDCEntriesW(
        PVMAFD_SERVER pServer,
        PWSTR         **ppszDCEntries,
        PDWORD        pdwCount
        );

/*
 * @brief Gets the current state of the client domain controller cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          pcdcState Returns the state of cdc
 *
 * @return Returns 0 for success
 */
DWORD
CdcGetCurrentState(
        PVMAFD_SERVER pServer,
        PCDC_DC_STATE pcdcState
        );

/*
 * @brief Frees an array of strings
 *
 * @param[in] ppszStringArray Array of strings
 * @param[in] dwCount Count of DC entries
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeStringArrayA(
    PSTR *pszStringArray,
    DWORD dwCount
    );

/*
 * @brief Frees an array of strings
 *
 * @param[in] pwszStringArray Array of strings
 * @param[in] dwCount Count of DC entries
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeStringArrayW(
    PWSTR *pwszStringArray,
    DWORD dwCount
    );

/*
 * @brief Frees Domain Controller Info structure
 *
 * @param[in] pDomainControllerInfoA
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeDomainControllerInfoA(
    PCDC_DC_INFO_A pDomainControllerInfoA
    );

/*
 * @brief Frees Domain Controller Info structure
 *
 * @param[in] pDomainControllerInfoW
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeDomainControllerInfoW(
    PCDC_DC_INFO_W pDomainControllerInfoW
    );

#ifdef __cplusplus
}
#endif

#endif /* DCCACLIENT_H_ */



