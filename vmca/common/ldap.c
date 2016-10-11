/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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



#include "includes.h"

#define ATTR_KRB_UPN  "userPrincipalName"
#define ATTR_MEMBEROF "memberOf"

static
DWORD
VMCACheckCAObject(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR    pszCAContainerDN,
    PCSTR    pszCADN,
    PCSTR    pszCAIssuerDN,
    PSTR     *ppszObjectDN
    );

static
DWORD
VMCACreateCAObject(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR   pszCACN,
    PCSTR   pszServerNameDN
    );

static
BOOLEAN
VMCAIsIPV6AddrFormat(
    PCSTR   pszAddr
    );

static
int
_VMCASASLSRPInteraction(
    LDAP *      pLd,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    )
{
    sasl_interact_t*                pInteract = (sasl_interact_t*)pIn;
    PVMCA_SASL_INTERACTIVE_DEFAULT pDef = (PVMCA_SASL_INTERACTIVE_DEFAULT)pDefaults;

    while( (pDef != NULL) && (pInteract->id != SASL_CB_LIST_END) )
    {

        switch( pInteract->id )
        {
        case SASL_CB_GETREALM:
                pInteract->defresult = pDef->pszRealm;
                break;
        case SASL_CB_AUTHNAME:
                pInteract->defresult = pDef->pszAuthName;
                break;
        case SASL_CB_PASS:
                pInteract->defresult = pDef->pszPass;
                break;
        case SASL_CB_USER:
                pInteract->defresult = pDef->pszUser;
                break;
        default:
                break;
        }

        pInteract->result = (pInteract->defresult) ? pInteract->defresult : "";
        pInteract->len    = (ULONG)strlen( (LPCSTR)pInteract->result );

        pInteract++;
    }

    return LDAP_SUCCESS;
}

static
DWORD
VMCAPEMToX509Crl(
    PSTR        pCrl,
    X509_CRL**  ppX509Crl
    );

static
DWORD
VMCAGetCrlAuthKeyIdHexString(
    X509_CRL*   pCrl,
    PSTR*       ppszAid
    );

static
DWORD
VMCAKeyIdToHexString(
    ASN1_OCTET_STRING*  pIn,
    PSTR*               ppszOut
    );

static
DWORD
VMCABytesToHexString(
    PUCHAR  pData,
    DWORD   length,
    PSTR    *pszHexString
    );

static
DWORD
VMCAGenerateCACNForLdap(
    X509* pCertificate,
    PSTR* ppszCACN
    );

static
DWORD
VMCACopyQueryResultAttributeString(
    PVMCA_LDAP_CONTEXT pContext,
    LDAPMessage* pSearchResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
    );

static
DWORD
VMCAUpdateAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR   pszObjectDN,
    PSTR    pszAttribute,
    PSTR    pszValue,
    BOOL    bAdd
    );

static
DWORD
VMCACheckAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR    pszObjectDN,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    );

static
DWORD
VMCAGetX509Name(
    X509_NAME*  pCertName,
    DWORD       dwFlags,
    PSTR*       ppszSubjectName
    );

DWORD
VmCASASLSRPBind(
     LDAP**     ppLd,
     PCSTR      pszURI,
     PCSTR      pszUPN,
     PCSTR      pszPass
     )
{
    DWORD       dwError = 0;
    PSTR        pszLowerCaseUPN = NULL;
    LDAP*       pLd = NULL;
    const int   ldapVer = LDAP_VERSION3;
    VMCA_SASL_INTERACTIVE_DEFAULT srpDefault = {0};

    if ( ppLd == NULL || pszURI == NULL || pszUPN == NULL || pszPass == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAStringToLower( (PSTR)pszUPN, &pszLowerCaseUPN );
    BAIL_ON_ERROR(dwError);

    srpDefault.pszAuthName = pszLowerCaseUPN;
    srpDefault.pszPass     = pszPass;

    dwError = ldap_initialize( &pLd, pszURI);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_sasl_interactive_bind_s( pLd,
                                            NULL,
                                            "SRP",
                                            NULL,
                                            NULL,
                                            LDAP_SASL_QUIET,
                                            _VMCASASLSRPInteraction,
                                            &srpDefault);
    BAIL_ON_ERROR(dwError);

    *ppLd = pLd;

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszLowerCaseUPN);

    return dwError;

error:

    if ( pLd )
    {
        ldap_unbind_ext_s( pLd, NULL, NULL);
    }
    goto cleanup;
}


DWORD
VMCALdapConnect(
    PSTR   pszHostName,
    DWORD  dwPort,
    PSTR   pszUsername,
    PSTR   pszPassword,
    PVMCA_LDAP_CONTEXT* ppLotus
    )
{

    DWORD dwError = 0;
    PVMCA_LDAP_CONTEXT pContext = NULL;
    PSTR pszUrl = NULL;
    BerValue ldapBindPwd = {0};

//LDAP_SUCCESS is defined as Zero in the Standard
// Which plays well with our BAIL_ON_ERROR macro

    DWORD dwVersion = LDAP_VERSION3;
    DWORD dwReturns = 20;


    if(dwPort == 0) {
        // Let us use the default LDAP_PORT, 389
        dwPort = LDAP_PORT;
    }

    dwError = VMCAAllocateMemory(sizeof(*pContext), (PVOID*)&pContext);
    BAIL_ON_ERROR(dwError);

    if (VMCAIsIPV6AddrFormat(pszHostName))
    {
        dwError = VMCAAllocateStringPrintfA(
                    &pszUrl,
                    "ldap://[%s]:%d",
                    pszHostName,
                    dwPort);
    }
    else
    {
        dwError = VMCAAllocateStringPrintfA(
                    &pszUrl,
                    "ldap://%s:%d",
                    pszHostName,
                    dwPort);
    }
    BAIL_ON_ERROR(dwError);

    if(IsNullOrEmptyString(pszPassword))
    {   // no credentials, do anonymous bind.
        dwError =  ldap_initialize(&pContext->pConnection, pszUrl);
        BAIL_ON_ERROR(dwError);

        if (pContext->pConnection == NULL) {
            ldap_get_option(pContext->pConnection, LDAP_OPT_ERROR_NUMBER, &dwError);
            //dwError = ld_errno; //LdapGetLastError();
            BAIL_ON_ERROR(dwError);
        }

        dwError = ldap_set_option(pContext->pConnection,
                                  LDAP_OPT_PROTOCOL_VERSION,
                                  (void*)&dwVersion);
        BAIL_ON_ERROR(dwError);

        dwError = ldap_set_option(pContext->pConnection,
                                  LDAP_OPT_SIZELIMIT,
                                  (void*)& dwReturns);
        BAIL_ON_ERROR(dwError);

        dwError = ldap_sasl_bind_s(
                    pContext->pConnection,
                    pszUsername,
                    LDAP_SASL_SIMPLE,
                    &ldapBindPwd,  // no credentials
                    NULL,
                    NULL,
                    NULL);
    }
    else
    {
        dwError = VmCASASLSRPBind(
                    &pContext->pConnection,
                    pszUrl,
                    pszUsername,
                    pszPassword);
    }

#ifdef LDAP_ERROR_MESSAGE
    if(dwError != 0) {
        printf("Error :%s\n",ldap_err2string(dwError));
    }
#endif
    BAIL_ON_ERROR(dwError);

    *ppLotus = pContext;

cleanup:
    VMCA_SAFE_FREE_STRINGA(pszUrl);
    return dwError;

error:
    if ((dwError != 0) && pContext)
    {
        VMCALdapClose(pContext);
    }
    goto cleanup;
}


VOID
VMCALdapClose(
    PVMCA_LDAP_CONTEXT pContext
    )
{
    if (pContext != NULL)
    {
        if (pContext->pConnection != NULL)
        {
            ldap_unbind_ext(pContext->pConnection, NULL, NULL);
        }
        VMCAFreeMemory(pContext);
    }
}


DWORD
VMCACheckLdapConnection(
    PSTR pszHostName,
    DWORD dwPort
    )
{
    DWORD dwError = 0;
    PVMCA_LDAP_CONTEXT pContext = NULL;

    dwError = VMCALdapConnect(pszHostName, dwPort, NULL, NULL, &pContext);
    BAIL_ON_ERROR(dwError);

error:

	if (pContext)
	{
		VMCALdapClose(pContext);
	}

    return dwError;
}


DWORD
VMCAGetDefaultDomainName(
    PSTR pszHostName,
    DWORD dwPort,
    PSTR* ppDomainName)
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PVMCA_LDAP_CONTEXT pLotus = NULL;

    if (!pszHostName)
    {
    	dwError = ERROR_INVALID_PARAMETER;
    	BAIL_ON_ERROR(dwError);
    }

	if (strcasecmp(pszHostName, "localhost") == 0)
	{
		pszHostName = "127.0.0.1";
	}

    dwError = VMCALdapConnect(pszHostName, dwPort, NULL, NULL, &pLotus);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAGetDefaultDomainName2(pLotus, ppDomainName);
    BAIL_ON_ERROR(dwError);

error :

    if (pLotus)
    {
        VMCALdapClose(pLotus);
    }

    return dwError;
}

DWORD
VMCAGetDefaultDomainName2(
    PVMCA_LDAP_CONTEXT pConnection,
    PSTR* ppDomainName
    )
{
	DWORD dwError = 0;
	PCHAR pszDomainNameAttr = "rootdomainnamingcontext";
	PSTR pszDomainName = NULL;

	if (ppDomainName == NULL)
	{
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_ERROR(dwError);
	}

	dwError = VMCAGetDSERootAttribute(
					pConnection,
					pszDomainNameAttr,
					&pszDomainName);
	BAIL_ON_ERROR(dwError);

	*ppDomainName = pszDomainName;

cleanup:

	return dwError;

error :

	if (ppDomainName)
	{
		*ppDomainName = NULL;
	}

	goto cleanup;
}

DWORD
VMCAGetDSERootAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PSTR  pszAttribute,
    PSTR* ppszAttrValue
    )
{
    DWORD dwError = 0; // LDAP_SUCCESS
    PSTR pAttribute = NULL;
    BerValue** ppValue = NULL;
    BerElement* pBer = NULL;
    LDAPMessage* pSearchResult = NULL;
    LDAPMessage* pResults = NULL;
    PCHAR pszFilter = "(objectClass=*)";
    PCHAR ppszAttrs[] = { pszAttribute, NULL };

    dwError = ldap_search_ext_s(
                  pContext->pConnection,     // Session handle
                  "",              // DN to start search
                  LDAP_SCOPE_BASE, // Scope
                  pszFilter,       // Filter
                  ppszAttrs,       // Retrieve list of attributes
                  0,               // Get both attributes and values
                  NULL,
                  NULL,
                  NULL,
                  0,
                  &pSearchResult); // [out] Search results
    BAIL_ON_ERROR(dwError);

    if (ldap_count_entries(pContext->pConnection, pSearchResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    // There should be only one result for this type
    pResults = ldap_first_entry(pContext->pConnection, pSearchResult);
    if (pResults == NULL) {
        ldap_get_option(pContext->pConnection, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_ERROR(dwError);
    }

    pAttribute = ldap_first_attribute(pContext->pConnection,pResults,&pBer);
    if(pAttribute == NULL) {
        ldap_get_option(pContext->pConnection, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_ERROR(dwError);
    }

    ppValue = ldap_get_values_len(pContext->pConnection, pResults, pszAttribute);
    if(ppValue == NULL) {
        ldap_get_option(pContext->pConnection, LDAP_OPT_ERROR_NUMBER, &dwError);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringA(ppValue[0]->bv_val, ppszAttrValue);
    BAIL_ON_ERROR(dwError);

error :

    if ( ppValue != NULL) {
        ldap_value_free_len(ppValue);
    }

    if(pAttribute != NULL) {
        ldap_memfree(pAttribute);
    }

    if(pBer != NULL) {
        ber_free(pBer,0);
    }

    if(pSearchResult != NULL) {
        ldap_msgfree(pSearchResult);
    }

    return dwError;
}

DWORD
VMCAGetDSEServerName(
    PVMCA_LDAP_CONTEXT pContext,
    PSTR* ppServerName
    )
{
    DWORD dwError = 0;
    PCHAR ServerNameAttr = "servername";

    PSTR pszServerName = NULL;

    if (ppServerName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAGetDSERootAttribute(
    				pContext,
    				ServerNameAttr,
    				&pszServerName);
    BAIL_ON_ERROR(dwError);

    *ppServerName = pszServerName;

cleanup:

    return dwError;

error :

	if (ppServerName)
	{
		*ppServerName = NULL;
	}

    goto cleanup;
}

static
DWORD
VMCAGetCrlAuthKeyIdHexString(
    X509_CRL*   pCrl,
    PSTR*       ppszAid
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszAid = NULL;
    AUTHORITY_KEYID *pId = NULL;

    if (!pCrl && !ppszAid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (X509_CRL_get_ext_by_NID(pCrl, NID_authority_key_identifier, -1) == -1)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    pId = (AUTHORITY_KEYID*)X509_CRL_get_ext_d2i(pCrl,
        NID_authority_key_identifier, NULL, NULL);
    if (!pId)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAKeyIdToHexString(pId->keyid, &pszAid);
    BAIL_ON_ERROR(dwError);

    *ppszAid = pszAid;

cleanup:
    if (pId)
    {
        AUTHORITY_KEYID_free(pId);
    }
    return dwError;
error:
    if (ppszAid)
    {
        *ppszAid = NULL;
    }
    VMCA_SAFE_FREE_MEMORY(pszAid);

    goto cleanup;
}

static
DWORD
VMCAPEMToX509Crl(
    PSTR        pCrl,
    X509_CRL**  ppX509Crl
    )
{
    DWORD   dwError = 0;
    BIO*    pBioMem = NULL;
    X509_CRL *pX509Crl = NULL;

    if (!pCrl || !ppX509Crl)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pBioMem = BIO_new_mem_buf((PVOID) pCrl, -1);
    if ( pBioMem == NULL)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_ERROR(dwError);
    }

    pX509Crl  = PEM_read_bio_X509_CRL(pBioMem, NULL, NULL, NULL);
    if (pX509Crl  == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    *ppX509Crl = pX509Crl;

cleanup :

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error :

    *ppX509Crl = NULL;

    if (pX509Crl != NULL)
    {
       X509_CRL_free(pX509Crl);
    }

    goto cleanup;
}

DWORD
VMCAUpdateCrlCAAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PSTR pszConfigurationDN,
    PSTR pszCrl
    )
{
    DWORD   dwError = 0;
    PSTR pszCADN = NULL;
    PSTR pszCrlAuthorityKeyId = NULL;
    PSTR pszCAContainerDN = NULL;
    X509_CRL* pCrl = NULL;
    ATTR_SEARCH_RESULT attrSearchResult = ATTR_NOT_FOUND;
    X509_NAME* pIssuer = NULL;
    PSTR pszCAIssuerDN = NULL;
    PSTR pszFoundCADN = NULL;

    dwError = VMCAPEMToX509Crl(pszCrl, &pCrl);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                    &pszCAContainerDN,
                    "CN=%s,%s",
                    CA_CONTAINER_NAME,
                    pszConfigurationDN);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAGetCrlAuthKeyIdHexString(pCrl, &pszCrlAuthorityKeyId);
    if (dwError == ERROR_SUCCESS)
    {
        if (!IsNullOrEmptyString(pszCrlAuthorityKeyId))
        {
            dwError = VMCAAllocateStringPrintfA(
                    &pszCADN,
                    "CN=%s,%s",
                    pszCrlAuthorityKeyId,
                    pszCAContainerDN);
            BAIL_ON_ERROR(dwError);
        }
    }

    if (!pszCADN)
    {
        pIssuer = X509_CRL_get_issuer(pCrl); // Don't free
        dwError = VMCAGetX509Name(pIssuer, XN_FLAG_COMPAT, &pszCAIssuerDN);
        BAIL_ON_ERROR(dwError);

        if (pszCAIssuerDN == NULL)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_ERROR(dwError);
        }
    }

    dwError = VMCACheckCAObject(
                    pContext,
                    pszCAContainerDN,
                    pszCADN,
                    pszCAIssuerDN,
                    &pszFoundCADN
                    );
    if (dwError == ERROR_INVALID_STATE && pszCAIssuerDN)
    {
        VMCA_LOG_ERROR("More than one CA found with given issuer DN: %s",
            pszCAIssuerDN);
    }
    BAIL_ON_ERROR(dwError);

    if (!pszFoundCADN)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACheckAttribute(
            pContext,
            pszFoundCADN,
            ATTR_CRL,
            pszCrl,
            &attrSearchResult
            );
    BAIL_ON_ERROR(dwError);

    if (attrSearchResult != ATTR_MATCH)
    {
        dwError = VMCAUpdateAttribute(pContext, pszFoundCADN,
                ATTR_CRL, pszCrl,
                (attrSearchResult == ATTR_NOT_FOUND));
        BAIL_ON_ERROR(dwError);
    }

cleanup:

    VMCA_SAFE_FREE_MEMORY(pszFoundCADN);
    VMCA_SAFE_FREE_MEMORY(pszCAIssuerDN);
    VMCA_SAFE_FREE_STRINGA(pszCADN);
    VMCA_SAFE_FREE_STRINGA(pszCrlAuthorityKeyId);
    VMCA_SAFE_FREE_STRINGA(pszCAContainerDN);
    if (pCrl)
    {
        X509_CRL_free(pCrl);
    }
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VMCABytesToHexString(
    PUCHAR  pData,
    DWORD   length,
    PSTR    *pszHexString
    )
{
    DWORD dwError = ERROR_SUCCESS;
    char* pszOut = NULL;
    DWORD i = 0;

    dwError = VMCAAllocateMemory(length * 2 + 1, (PVOID*)&pszOut);
    BAIL_ON_ERROR(dwError);

    for (; i < length; ++i)
    {
        sprintf(pszOut + i * 2, "%02X", pData[i]);
    }
    pszOut[length * 2] = '\0';

    *pszHexString = pszOut;

error:
    return dwError;
}

static
DWORD
VMCAKeyIdToHexString(
    ASN1_OCTET_STRING*  pIn,
    PSTR*               ppszOut
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszOut = NULL;

    if (!pIn || !ppszOut)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCABytesToHexString((PUCHAR)(pIn->data), pIn->length, &pszOut);
    BAIL_ON_ERROR(dwError);

    *ppszOut = pszOut;

error:
    return dwError;
}

static
DWORD
VMCAGenerateCACNForLdap(
    X509* pCertificate,
    PSTR* ppszCACN
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    int     length = 0;
    unsigned char*  pEncodedKey = NULL;
    unsigned char*  pKey = NULL;
    unsigned char   md[SHA_DIGEST_LENGTH];
    EVP_PKEY*       pPubKey = NULL;
    PSTR            pszCACN = NULL;
    ASN1_OCTET_STRING* pSid = NULL;

    if (pCertificate == NULL || ppszCACN == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pSid = (ASN1_OCTET_STRING*)X509_get_ext_d2i(pCertificate,
        NID_subject_key_identifier, NULL, NULL);
    if (pSid)
    {
        dwError = VMCAKeyIdToHexString(pSid, &pszCACN);
        BAIL_ON_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszCACN))
    {
        pPubKey = X509_get_pubkey(pCertificate);
        length = i2d_PUBKEY(pPubKey, NULL);
        dwError = VMCAAllocateMemory(length, (PVOID*)&pEncodedKey);
        BAIL_ON_ERROR(dwError);

        pKey = pEncodedKey;
        length = i2d_PUBKEY(pPubKey, &pKey);
        SHA1(pEncodedKey, length, md);

        dwError = VMCABytesToHexString((PUCHAR)md, SHA_DIGEST_LENGTH, &pszCACN);
        BAIL_ON_ERROR(dwError);
    }

    *ppszCACN = pszCACN;

cleanup:

    VMCA_SAFE_FREE_MEMORY(pEncodedKey);
    if (pPubKey)
    {
        EVP_PKEY_free(pPubKey);
    }
    if (pSid)
    {
        ASN1_OCTET_STRING_free(pSid);
    }

    return dwError;

error:
    if (*ppszCACN)
    {
        *ppszCACN = NULL;
    }
    VMCA_SAFE_FREE_MEMORY(pszCACN);

    goto cleanup;
}

static
DWORD
VMCAGetX509Name(
    X509_NAME*  pCertName,
    DWORD       dwFlags,
    PSTR*       ppszSubjectName
    )
{
    DWORD   dwError = 0;
    int     length = 0;
    BIO*    pBioMem = NULL;
    PSTR    pszSubjectName = NULL;

    pBioMem = BIO_new(BIO_s_mem());
    if (!pBioMem)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_ERROR(dwError);
    }

    X509_NAME_print_ex(pBioMem, pCertName, 0, dwFlags);

    length = BIO_pending(pBioMem);

    if (length <= 0)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory((DWORD)(length + 1), (PVOID*)&pszSubjectName);
    BAIL_ON_ERROR(dwError);

    if (BIO_read(pBioMem, pszSubjectName, length) != length)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    *ppszSubjectName = pszSubjectName;

cleanup:

    if (pBioMem)
    {
        BIO_free(pBioMem);
    }

    return dwError;

error:

    if (ppszSubjectName)
    {
        *ppszSubjectName = NULL;
    }

    VMCA_SAFE_FREE_MEMORY(pszSubjectName);

    goto cleanup;
}

DWORD
VMCAUpdatePkiCAAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PSTR    pszConfigurationDN,
    X509*   pCertificate
    )
{
    DWORD   dwError = 0;
    PSTR pszCertificate   = NULL;
    PSTR pszCAContainerDN = NULL;
    PSTR pszCADN = NULL;
    PSTR pszCACN = NULL;
    PSTR pszCAIssuerDN = NULL;
    PSTR pszFoundCADN = NULL;
    X509_NAME *pCertName = NULL;
    ATTR_SEARCH_RESULT attrSearchResult = ATTR_NOT_FOUND;

    if (!pCertificate ||
        !pContext ||
        IsNullOrEmptyString(pszConfigurationDN)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR (dwError);
    }

    dwError = VMCACertToPEM(
                            pCertificate,
                            &pszCertificate
                           );
    BAIL_ON_ERROR (dwError);

    pCertName = X509_get_subject_name(pCertificate);
    if ( pCertName == NULL) // Don't free
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAGetX509Name(pCertName, XN_FLAG_COMPAT, &pszCAIssuerDN);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAGenerateCACNForLdap(pCertificate, &pszCACN);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                    &pszCAContainerDN,
                    "CN=%s,%s",
                    CA_CONTAINER_NAME,
                    pszConfigurationDN);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                    &pszCADN,
                    "CN=%s,%s",
                    pszCACN,
                    pszCAContainerDN);
    BAIL_ON_ERROR(dwError);

    dwError = VMCACheckCAObject(
                    pContext,
                    NULL,
                    pszCADN,
                    NULL,
                    &pszFoundCADN
                    );
    BAIL_ON_ERROR(dwError);

    if (IsNullOrEmptyString(pszFoundCADN))
    {
        dwError = VMCACreateCAObject(
                    pContext,
                    pszCACN,
                    pszCADN
                    );
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACheckAttribute(
                    pContext,
                    pszCADN,
                    ATTR_CA_CERTIFICATE,
                    pszCertificate,
                    &attrSearchResult);
    BAIL_ON_ERROR(dwError);

    if (attrSearchResult != ATTR_MATCH)
    {
        dwError = VMCAUpdateAttribute(pContext, pszCADN,
                ATTR_CA_CERTIFICATE, pszCertificate,
                (attrSearchResult == ATTR_NOT_FOUND));
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACheckAttribute(
                    pContext,
                    pszCADN,
                    ATTR_CA_CERTIFICATE_DN,
                    pszCAIssuerDN,
                    &attrSearchResult);
    BAIL_ON_ERROR(dwError);

    if (attrSearchResult != ATTR_MATCH)
    {
        dwError = VMCAUpdateAttribute(pContext, pszCADN,
                ATTR_CA_CERTIFICATE_DN, pszCAIssuerDN,
                (attrSearchResult == ATTR_NOT_FOUND));
        BAIL_ON_ERROR(dwError);
    }

cleanup:

    VMCA_SAFE_FREE_STRINGA(pszCertificate);
    VMCA_SAFE_FREE_STRINGA(pszFoundCADN);
    VMCA_SAFE_FREE_STRINGA(pszCAContainerDN);
    VMCA_SAFE_FREE_STRINGA(pszCACN);
    VMCA_SAFE_FREE_STRINGA(pszCADN);
    VMCA_SAFE_FREE_STRINGA(pszCAIssuerDN);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VMCALdapFindObject(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR    pszBaseDN,
    ber_int_t scope,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    PSTR     *ppszObjectDN
    )
{
    DWORD dwError = 0;
    DWORD dwNumEntries = 0;
    LDAPMessage* pResult = NULL;
    LDAPMessage* pEntry = NULL;
    PSTR pszFilter = NULL;
    PSTR pszObjectDN = NULL;

    if (pszAttribute && pszValue)
    {
        VMCAAllocateStringPrintfA(&pszFilter, "(%s=%s)", pszAttribute, pszValue);
    }

    dwError = ldap_search_ext_s(
                  pContext->pConnection,
                  (PSTR)pszBaseDN,
                  scope,
                  pszFilter,
                  NULL,      /* attributes      */
                  TRUE,
                  NULL,      /* server controls */
                  NULL,      /* client controls */
                  NULL,      /* timeout         */
                  0,
                  &pResult);
    BAIL_ON_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pContext->pConnection, pResult);
    if (dwNumEntries > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }
    else if (dwNumEntries == 0)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_ERROR(dwError);
    }
    else
    {
        pEntry = ldap_first_entry(pContext->pConnection, pResult);
        if (!pEntry)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_ERROR(dwError);
        }

        pszObjectDN = ldap_get_dn(pContext->pConnection, pEntry);
        if (IsNullOrEmptyString(pszObjectDN))
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_ERROR(dwError);
        }

        *ppszObjectDN = pszObjectDN;
    }

cleanup:

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMCA_SAFE_FREE_STRINGA(pszFilter);

    return dwError;

error :
    VMCA_SAFE_FREE_STRINGA(pszObjectDN);

    if (dwError == LDAP_NO_SUCH_OBJECT)
    {
        dwError = 0;
    }

    goto cleanup;
}

static
DWORD
VMCACheckCAObject(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR    pszCAContainerDN,
    PCSTR    pszCADN,
    PCSTR    pszCAIssuerDN,
    PSTR     *ppszObjectDN
    )
{
    DWORD dwError = 0;
    PSTR  pszObjectDN = NULL;

    if (!pszCADN && !pszCAIssuerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pszCADN)
    {
        dwError = VMCALdapFindObject(pContext, pszCADN, LDAP_SCOPE_BASE,
                NULL, NULL, &pszObjectDN);
        BAIL_ON_ERROR(dwError);
    }
    else
    {
        dwError = VMCALdapFindObject(pContext, pszCAContainerDN,
                LDAP_SCOPE_ONELEVEL, ATTR_NAME_CA_CERTIFICATE_DN,
                pszCAIssuerDN, &pszObjectDN);
        BAIL_ON_ERROR(dwError);
    }

    *ppszObjectDN = pszObjectDN;

cleanup:

    return dwError;

error :

    if (ppszObjectDN)
    {
        *ppszObjectDN = NULL;
    }

    VMCA_SAFE_FREE_STRINGA(pszObjectDN);
    goto cleanup;
}

static
DWORD
VMCACreateCAObject(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR   pszCACN,
    PCSTR   pszCADN
    )
{
    DWORD dwError = 0;
    PSTR  ppszObjectClassValues[] =
                {
                    "vmwCertificationAuthority",
                    "pkiCA",
                    "top",
                    NULL
                };
    char*   modv_cn[] = { (PSTR)pszCACN, NULL };
    LDAPMod mod_object = {0};
    LDAPMod mod_cn = {0};
    LDAPMod *mods[] = { &mod_object, &mod_cn, NULL };

    mod_cn.mod_op = LDAP_MOD_ADD;
    mod_cn.mod_type = ATTR_CN;
    mod_cn.mod_values = modv_cn;

    mod_object.mod_op     = LDAP_MOD_ADD;
    mod_object.mod_type   = ATTR_OBJECTCLASS;
    mod_object.mod_values = ppszObjectClassValues;

    dwError = ldap_add_ext_s(
                  pContext->pConnection,
                  (PSTR)pszCADN,
                  mods,
                  NULL,
                  NULL
                  );
    BAIL_ON_ERROR(dwError);

cleanup:

    return dwError;

error :

    goto cleanup;
}

static
DWORD
VMCACheckAttribute(
    PVMCA_LDAP_CONTEXT    pContext,
    PCSTR    pszObjectDN,
    PCSTR    pszAttribute,
    PCSTR    pszValue,
    ATTR_SEARCH_RESULT* pAttrStatus
    )
{
    DWORD dwError = 0;
    PCHAR pszFilter = "(objectClass=*)";
    PSTR  pszRetrievedValue = NULL;
    DWORD dwNumEntries = 0;
    LDAPMessage* pSearchResult = NULL;
    PCHAR ppszAttr[] = { (PSTR)pszAttribute, NULL };

    dwError = ldap_search_ext_s(
                  pContext->pConnection,
                  (PSTR)pszObjectDN,
                  LDAP_SCOPE_BASE,
                  pszFilter,
                  ppszAttr,      /* attributes      */
                  FALSE,
                  NULL,      /* server controls */
                  NULL,      /* client controls */
                  NULL,      /* timeout         */
                  0,
                  &pSearchResult);
    BAIL_ON_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pContext->pConnection, pSearchResult);
    if (dwNumEntries == 0)
    {
        // Caller should make sure that the ObjectDN passed in does exist
        // by calling DirCliLdapCheckCAObject first.
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }
    else if (dwNumEntries != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACopyQueryResultAttributeString(pContext, pSearchResult,
        pszAttribute, TRUE, &pszRetrievedValue);
    BAIL_ON_ERROR(dwError);

    if (!pszRetrievedValue)
    {
        *pAttrStatus = ATTR_NOT_FOUND;
    }
    else if (VMCAStringCompareA(pszValue, pszRetrievedValue, FALSE))
    {
        *pAttrStatus = ATTR_DIFFER;
    }
    else
    {
        *pAttrStatus = ATTR_MATCH;
    }

cleanup:

	if (pSearchResult)
	{
		ldap_msgfree(pSearchResult);
	}
	VMCA_SAFE_FREE_STRINGA(pszRetrievedValue);

    return dwError;

error :

	*pAttrStatus = ATTR_NOT_FOUND;

	if (dwError == LDAP_NO_SUCH_OBJECT)
	{
		dwError = 0;
	}

    goto cleanup;
}

static
DWORD
VMCAUpdateAttribute(
    PVMCA_LDAP_CONTEXT pContext,
    PCSTR   pszObjectDN,
    PSTR    pszAttribute,
    PSTR    pszValue,
    BOOL    bAdd
)
{
    DWORD dwError = 0;
    LDAPMod mod_cert = {0};
    LDAPMod *mods[] = { &mod_cert, NULL};
    struct berval bercert = { 0 };
    struct berval *bervals[] = {&bercert, NULL};

    bercert.bv_len = (ULONG) strlen(pszValue);
    bercert.bv_val = pszValue;

    mod_cert.mod_op = (bAdd ? LDAP_MOD_ADD : LDAP_MOD_REPLACE)
                        | LDAP_MOD_BVALUES;
    mod_cert.mod_type = pszAttribute;
    mod_cert.mod_vals.modv_bvals = bervals;

    dwError = ldap_modify_ext_s(
                  pContext->pConnection,
                  pszObjectDN,
                  mods,
                  NULL,
                  NULL);
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
VMCACopyQueryResultAttributeString(
    PVMCA_LDAP_CONTEXT pContext,
    LDAPMessage* pSearchResult,
    PCSTR        pszAttribute,
    BOOL         bOptional,
    PSTR*        ppszOut
)
{
    DWORD   dwError = 0;
    struct berval** ppValues = NULL;
    PSTR   pszOut = NULL;

    ppValues = ldap_get_values_len(
                                pContext->pConnection,
                                pSearchResult,
                                pszAttribute);
    if (ppValues && ppValues[0])
    {
        dwError = VMCAAllocateMemory(
                        (DWORD)(sizeof(CHAR) * ppValues[0]->bv_len + 1),
                        (PVOID*)&pszOut);
        BAIL_ON_ERROR(dwError);
        memcpy(
            (PVOID) pszOut,
            (PVOID) ppValues[0]->bv_val,
            (size_t) ppValues[0]->bv_len);
    }
    else if (!bOptional)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_ERROR(dwError);
    }

    *ppszOut = pszOut;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }
    return dwError;

error:

    if (ppszOut)
    {
        *ppszOut = NULL;
    }
    VMCA_SAFE_FREE_MEMORY(pszOut);
    goto cleanup;
}

/*
 *  Quick and dirty function to verify format - colons and digits/a-f/A-F only
 */
BOOLEAN
VMCAIsIPV6AddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN     bIsIPV6 = pszAddr ? TRUE : FALSE;
    size_t      iSize = 0;
    size_t      iCnt = 0;
    size_t      iColonCnt = 0;

    if ( pszAddr != NULL )
    {
        iSize = VMCAStringLenA(pszAddr);
        for (iCnt=0; bIsIPV6 && iCnt < iSize; iCnt++)
        {
            if ( pszAddr[iCnt] == ':' )
            {
                iColonCnt++;
            }
            else if ( VMCA_ASCII_DIGIT( pszAddr[iCnt] )
                      ||
                      VMCA_ASCII_aTof( pszAddr[iCnt] )
                      ||
                      VMCA_ASCII_AToF( pszAddr[iCnt] )
                    )
            {
            }
            else
            {
                bIsIPV6 = FALSE;
            }
        }

        // should not count on iColonCnt == 7
        if ( iColonCnt < 2 )
        {
            bIsIPV6 = FALSE;
        }
    }

    return bIsIPV6;
}

DWORD
VMCALdapGetMemberships(
    PVMCA_LDAP_CONTEXT pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszAttrMemberOf = ATTR_MEMBEROF; // memberOf
    PSTR  ppszAttrs[] = { pszAttrMemberOf, NULL};
    DWORD dwCount = 0;
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;
    DWORD i = 0;
    LDAP *pLd = NULL;

    if (pConnection == NULL ||
        pConnection->pConnection == NULL ||
        IsNullOrEmptyString(pszUPNName) ||
        pppszMemberships == NULL ||
        pdwMemberships == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pLd = pConnection->pConnection;

    dwError = VMCAAllocateStringPrintfA(&pszFilter, "(%s=%s)", ATTR_KRB_UPN, pszUPNName); // userPrincipalName
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    (PSTR*)ppszAttrs,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    -1,
                    &pResult);
    BAIL_ON_VMCA_ERROR(dwError);

    dwCount = ldap_count_entries(pLd, pResult);
    if (dwCount == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMCA_ERROR(dwError);
    }
    else if (dwCount > 1)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, pszAttrMemberOf);
    if (!ppValues)
    {
        dwMemberships = 0;
    }
    else
    {
        dwMemberships = ldap_count_values_len(ppValues);
    }

    if (dwMemberships)
    {
        dwError = VMCAAllocateMemory(dwMemberships * sizeof(PSTR), (PVOID*)&ppszMemberships);
        BAIL_ON_VMCA_ERROR(dwError);

        for (i = 0; ppValues[i] != NULL; i++)
        {
            PCSTR pszMemberOf = ppValues[i]->bv_val;

            dwError = VMCAAllocateStringA(pszMemberOf, &ppszMemberships[i]);
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *pppszMemberships = ppszMemberships;
    *pdwMemberships = dwMemberships;

cleanup:

    if(ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMCA_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:
    if (ppszMemberships != NULL && dwMemberships > 0)
    {
        for (i = 0; i < dwMemberships; i++)
        {
            VMCA_SAFE_FREE_STRINGA(ppszMemberships[i]);
        }
        VMCA_SAFE_FREE_MEMORY(ppszMemberships);
    }
    goto cleanup;
}
