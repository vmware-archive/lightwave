/*
 * Copyright ©2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#include "includes.h"

static
DWORD
VmRESTTestIfHostIPMatch(
    PSTR        pszOrigin,
    BOOLEAN     *pbMatch
    );

DWORD
VmRESTSetCORSHeaders(
    PVDIR_REST_OPERATION    pRestOp,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD   dwError = 0;

    if (!pRestOp || !ppResponse)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //determine if Origin header is present & set CORS headers accordingly
    if (pRestOp->bisValidOrigin)
    {
        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Origin", pRestOp->pszOrigin);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Headers",
                      "Origin, X-Requested-With, Content-Type, Authorization");
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmRESTSetHttpHeader (ppResponse,
                      "Access-Control-Allow-Methods",
                      "GET, OPTIONS, PUT, DELETE, PATCH");
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

DWORD
VmRESTIsValidOrigin(
    PSTR        pszOrigin,
    BOOLEAN     *pbIsValidOrigin
    )
{
    DWORD dwError = 0;
    BOOLEAN  bIsValidOrigin = FALSE;

    if (IsNullOrEmptyString(pszOrigin) || !pbIsValidOrigin)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get the part of origin after "https://"
    if (VmDirStringStartsWith (pszOrigin, HTTP_PROTOCOL_PREFIX, FALSE))
    {
        PSTR  pszOriginValue = pszOrigin + strlen(HTTP_PROTOCOL_PREFIX);

        if (VmDirIsIPAddrFormat(pszOriginValue))
        {
            dwError = VmRESTTestIfHostIPMatch(pszOrigin, &bIsValidOrigin);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringEndsWith(
                    pszOriginValue,
                    gVmdirKrbGlobals.pszRealm,
                    FALSE /* case insensitive */
                    ))
        {
            bIsValidOrigin = TRUE; // Origin is from same domain
        }
    }

    *pbIsValidOrigin = bIsValidOrigin;

cleanup:

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

static
DWORD
VmRESTTestIfHostIPMatch(
    PSTR        pszOrigin,
    BOOLEAN     *pbMatch
    )
{
    DWORD dwError = 0;
    BOOLEAN bMatch = FALSE;
    char pszAddr[INET_ADDRSTRLEN];
    struct ifaddrs *addrs = NULL, *pCur = NULL;
    struct sockaddr_in *pAddr = NULL;

    if (IsNullOrEmptyString(pszOrigin) || !pbMatch)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Compare with current IP addrss
    if (getifaddrs(&addrs) < 0)
    {
        dwError = VMDIR_ERROR_REST_IP_UNKNOWN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pCur = addrs;

    while (pCur)
    {
        if (pCur->ifa_addr  != NULL)
        {
            pAddr = (struct sockaddr_in *)pCur->ifa_addr;
            if (!inet_ntop(AF_INET, &(pAddr->sin_addr), pszAddr, INET_ADDRSTRLEN))
            {
               dwError = VMDIR_ERROR_REST_IP_UNKNOWN;
               BAIL_ON_VMDIR_ERROR(dwError);
            }

            if(!strncasecmp(pszOrigin + strlen(HTTP_PROTOCOL_PREFIX), pszAddr, strlen(pszAddr)))
            {
                bMatch = TRUE;
                break;
            }
        }
        pCur = pCur->ifa_next;
    }

    *pbMatch = bMatch;

cleanup:
    if (addrs)
    {
        freeifaddrs(addrs);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

DWORD
VmDirErrForNsErr(
    int error
)
{
    switch(error)
    {
        case HOST_NOT_FOUND:
            /* Authoritative Answer Host not found. */
            return VMDIR_ERROR_REST_HOST_NOT_FOUND;
        case NO_DATA:
            /* Valid name, no data record of requested type. */
            return VMDIR_ERROR_REST_NO_DATA;
        case TRY_AGAIN:
            /* Non-Authoritative Host not found, or SERVERFAIL. */
            return VMDIR_ERROR_REST_TRY_AGAIN;
        case NO_RECOVERY :
            /* Non recoverable errors, FORMERR, REFUSED, NOTIMP. */
            return VMDIR_ERROR_REST_NO_RECOVERY;
        case NETDB_INTERNAL :
            /* See errno. */
            return LwErrnoToWin32Error(errno);
        default:
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL, "Unexpected ns error (%d)", error);
            return VMDIR_ERROR_REST_UNKNOWN_NS_ERROR;
        }
    }
}

DWORD
VmDirErrForDnsErr(
    int error
)
{
    switch(error)
    {
        case ns_r_formerr:
            /* Format error. */
            return VMDIR_ERROR_REST_DNS_FORM_ERR;
        case ns_r_servfail:
            /* Server failure. */
            return VMDIR_ERROR_REST_DNS_SERV_FAIL;
        case ns_r_nxdomain:
            /* Name error. */
            return VMDIR_ERROR_REST_DNS_NXDOMAIN;
        case ns_r_notimpl:
            /* Unimplemented. */
            return VMDIR_ERROR_REST_DNS_NOTIMPL;
        case ns_r_refused:
            /* Operation refused. */
            return VMDIR_ERROR_REST_DNS_REFUSED;
        default:
        {
            VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL, "Other dns error (%d)", error);
            return VMDIR_ERROR_REST_DNS_OTHER;
        }
    }
}

DWORD
VmDirRESTOidcIssuer(
    PCSTR pszDnsDomain,
    PCSTR pszDomain,
    PCSTR pszSite,
    PSTR* ppszOidcIssuer
)
{
    DWORD dwError = 0;
    PSTR pszIssuer = NULL;
    PSTR pszStsServerLookupName = NULL;
    CHAR stsSrv[NS_MAXDNAME] = {0};

    res_state state = NULL;
    union {
        HEADER hdr;
        u_char buf[NS_PACKETSZ];
    } response = {0};
    int responseLen = 0;
    ns_msg handle = {0};
    ns_rr rr = {0};
//    u_int priority = 0;
//    u_int weight = 0;
    u_int port = 0;
    const u_char * rdata = NULL;

    if ( IsNullOrEmptyString(pszDnsDomain) ||
         IsNullOrEmptyString(pszDomain) ||
         ( ppszOidcIssuer == NULL ) )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
            "%s: Discovering oidc issuer for domain '%s'",
            __FUNCTION__, pszDomain
    );

    dwError = VmDirAllocateMemory(
        sizeof(*state),
        (PVOID*)&state
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (res_ninit(state) < 0 )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: res_ninit() failed with (%d)",
            __FUNCTION__, errno
        );
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

     if (!IsNullOrEmptyString(pszSite))
    {
        dwError = VmDirAllocateStringPrintf(
            &pszStsServerLookupName,
            "_sts._tcp.%s._sites.%s.",
            pszSite,
            pszDnsDomain
        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
            &pszStsServerLookupName,
            "_sts._tcp.%s.",
            pszDnsDomain
        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_DEBUG(
        VMDIR_LOG_MASK_ALL,
        "%s: searching for srv records with name '%s'",
        __FUNCTION__, pszStsServerLookupName
    );

    if ( ( responseLen = res_nquery(
               state,
               pszStsServerLookupName,
               ns_c_in,
               ns_t_srv,
               (u_char *)&response,
               sizeof(response)
           )
         ) < 0 )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: res_nquery() failed with (%d)",
            __FUNCTION__, h_errno
        );

        dwError = VmDirErrForNsErr(h_errno);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ns_initparse(
             response.buf,
             responseLen,
             &handle) < 0 )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: ns_initparse() failed with (%d)",
            __FUNCTION__, errno
        );

        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ns_msg_getflag(
             handle,
             ns_f_rcode
         ) != ns_r_noerror )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: dns query returned error (%d)",
            __FUNCTION__, ns_msg_getflag( handle, ns_f_rcode )
        );

        dwError = VmDirErrForDnsErr(ns_msg_getflag( handle, ns_f_rcode ));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ns_msg_count(
             handle,
             ns_s_an
         ) != 1 ) {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: unexpected number of dns records for sts service: %d",
            __FUNCTION__, ns_msg_count(handle, ns_s_an));

        dwError = VMDIR_ERROR_REST_UNEXPECTED_RECORDS_NUM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ns_parserr(
             &handle,
             ns_s_an,
             0,
             &rr ) )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: ns_parserr() failed with (%d)",
            __FUNCTION__, errno
        );

        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ns_rr_type(rr) != ns_t_srv )
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: unexpected record type for sts service entry %d (expecting srv)",
            __FUNCTION__, ns_rr_type(rr));

        dwError = VMDIR_ERROR_REST_UNEXPECTED_RECORD_TYPE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // https://tools.ietf.org/html/rfc2782: priority weight port target
    if (ns_rr_rdlen(rr) < 3U*NS_INT16SZ)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: unexpected record size %d for an srv record",
            __FUNCTION__, ns_rr_rdlen(rr));

        dwError = VmDirErrForNsErr(NO_RECOVERY); // format error
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    rdata = ns_rr_rdata(rr);
//    NS_GET16(priority, rdata);
//    NS_GET16(weight, rdata);
    NS_GET16(port, rdata);

    if ( dn_expand(
             ns_msg_base(handle),
             ns_msg_end(handle),
             rdata,
             stsSrv,
             NS_MAXDNAME
         ) < 0 )
    {
        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "%s: dn_expand() failed with (%d)",
            __FUNCTION__, errno
        );

        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // todo: once dns URI records are supported
    // this should be switched to read oidc issuer uri
    // from dns, and not construct the issuer
    dwError = VmDirAllocateStringPrintf(
        &pszIssuer,
        "https://%s:%d/%s/idp",
        stsSrv,
        port,
        pszDomain
    );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
            "%s: Oidc issuer for domain '%s' is '%s'",
            __FUNCTION__, pszDomain, pszIssuer
    );

    *ppszOidcIssuer = pszIssuer;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszStsServerLookupName);
    if (state != NULL)
    {
        res_nclose(state);
    }
    VMDIR_SAFE_FREE_MEMORY(state);
    return dwError;

error:
    VMDIR_LOG_ERROR(
       VMDIR_LOG_MASK_ALL,
       "%s failed, error (%d)",
       __FUNCTION__,
       dwError
    );

    VMDIR_SAFE_FREE_MEMORY(pszIssuer);
    goto cleanup;
}
