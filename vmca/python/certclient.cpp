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



#include <certclient.h>
#include <vmca.h>
#include <vmca_error.h>

void THROW_IF_NEEDED(unsigned int dwError)
{
    if(dwError){
      std::cout << "Error Code : " << dwError << std::endl;
      throw vmca_exception(dwError);
    }
}

PSTR
VMCAErrorCodeToName(int code)
{
    int i = 0;
    VMCA_ERROR_CODE_NAME_MAP VMCA_ERROR_Table[] =
                                 VMCA_ERROR_TABLE_INITIALIZER;

    for (i=0; i<sizeof(VMCA_ERROR_Table)/sizeof(VMCA_ERROR_Table[0]); i++)
    {
        if ( code == VMCA_ERROR_Table[i].code)
        {
            return (PSTR) VMCA_ERROR_Table[i].name;
        }
    }

    return (PSTR) UNKNOWN_STRING;
}

//
// Client Functions
//
client::client(const std::string& CertServerName) : ServerName(CertServerName)
{

}

// If this function throws the user has a bad object that
// failed to authenticate with VMCA/KRB.

VOID client::Login( const std::string& UserName,
               const std::string& Password,
               const std::string& DomainName)
{
    DWORD dwError = 0;

#if 0
    /*
     * Temporarily disable login because machine account
     * credentials are used for SRP authentication.
     */
    dwError = VMCALoginUser(
        (PSTR) DomainName.c_str(),
        (PSTR) UserName.c_str(),
        (PSTR) Password.c_str());
#endif

    THROW_IF_NEEDED(dwError);
}

VOID client::Logout()
{
    DWORD dwError = 0;

#if 0
    /*
     * Temporarily disable logout because machine account
     * credentials are used for SRP authentication.
     */
    dwError = VMCALogout();
#endif

    THROW_IF_NEEDED(dwError);
}

std::string client::GetVersion()
{
    PSTR pVersion = NULL;
    DWORD dwError = 0;
    std::string result;
    dwError = VMCAGetServerVersionA(ServerName.c_str(), &pVersion);
    BAIL_ON_ERROR(dwError);
    result.assign (pVersion);
error :
    if (pVersion != NULL) {
        VMCAFreeKey(pVersion);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

bool client::AddRootCertificate(const std::string& Certificate, const std::string& PrivateKey)
{
    DWORD dwError = 0;
    dwError = VMCAAddRootCertificateA(ServerName.c_str(),
                                     (PVMCA_CERTIFICATE) Certificate.c_str(),
                                     NULL,
                                     (PVMCA_KEY) PrivateKey.c_str());

    BAIL_ON_ERROR(dwError);
error :

    THROW_IF_NEEDED(dwError);
    return true;
}

REQUEST client::GetRequest()
{
    REQUEST Req;
    return Req;
}


certificate client::GetRootCACert()
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;

    dwError =  VMCAGetRootCACertificateA(ServerName.c_str(), &pCertificate);
    BAIL_ON_ERROR(dwError);
    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;

}

vmcacontext client::OpenEnumHandle(const std::string& statusFilter)
{
    DWORD dwError = 0;
    PVOID pContext = NULL;
    VMCA_CERTIFICATE_STATUS dwStatus = VMCA_CERTIFICATE_ALL;
    vmcacontext ctx;

    // initialize enum
    ctx.pContext = NULL;
    ctx.pclient  = NULL;
    ctx.currIndex = 0;
    ctx.enumStatus = VMCA_ENUM_ERROR;

    if ( std::strcmp(statusFilter.c_str(), "active") == 0) {
        dwStatus = VMCA_CERTIFICATE_ACTIVE;
    }
    else if ( std::strcmp(statusFilter.c_str(), "revoked") == 0) {
        dwStatus = VMCA_CERTIFICATE_REVOKED;
    }
    else if ( std::strcmp(statusFilter.c_str(), "expired") == 0) {
        dwStatus = VMCA_CERTIFICATE_EXPIRED;
    }
    else if ( std::strcmp(statusFilter.c_str(), "all") == 0) {
        dwStatus = VMCA_CERTIFICATE_ALL;
    }
    else {
        goto error;
    }

    dwError = VMCAOpenEnumContextA(
            ServerName.c_str(),
            dwStatus,
            &pContext
    );
    BAIL_ON_ERROR(dwError);

    ctx.pContext = pContext;
    ctx.pclient = this;

    return ctx;

error :
    if (pContext){
       VMCACloseEnumContext(pContext);
    }

    THROW_IF_NEEDED(dwError);
    return ctx;
}

certificate *client::GetNextCertificate(vmcacontext& ctx)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate *result = NULL;

    dwError = VMCAGetNextCertificate(
            ctx.pContext,
            &pCertificate,
            &ctx.currIndex,
            &ctx.enumStatus
        );

    BAIL_ON_ERROR(dwError);

    result = new certificate();
    result->certString.assign(pCertificate);

    VMCAFreeCertificate(pCertificate);
    pCertificate = NULL;

    return result;

error :
    if ( pCertificate) {
       VMCAFreeCertificate(pCertificate);
    }

    if (dwError == VMCA_ENUM_END)
    {
        dwError = 0; // This is the Standard Success Code
    }

    THROW_IF_NEEDED(dwError);
    return NULL;
}

VOID client::CloseEnumHandle(vmcacontext& ctx)
{
    VMCACloseEnumContext(ctx.pContext);
    ctx.pContext = NULL;
}

//
// Certificate Functions
//

std::string certificate::print()
{
    DWORD dwError = 0;
    PSTR pCertString = NULL;
    std::string result;

    dwError =  VMCAGetCertificateAsStringA(
                   (PVMCA_CERTIFICATE)certString.c_str(),
                   &pCertString);
    BAIL_ON_ERROR(dwError);
    result.assign(pCertString);

error :
    if(pCertString) {
        // I know that this is not a KEY, But I just want to free a simple
        // string and VMCAFreeKey is just a free String internally.
        // TODO : Expose VMCA_SAFE_FREE_STRINGA from VMCA.h
        VMCAFreeKey(pCertString);
    }
    THROW_IF_NEEDED(dwError);
    return result;

}

bool certificate::isCACert()
{
    if ( VMCAValidateCACertificate((PSTR)certString.c_str()) == 0 ) {
        return true;
    }
    return false;
}

bool client::Revoke(const certificate& cert)
{
    DWORD dwError = 0;

    dwError = VMCARevokeCertificateA(ServerName.c_str(),
                                     (PVMCA_CERTIFICATE)cert.certString.c_str());
    BAIL_ON_ERROR(dwError);
error :
    THROW_IF_NEEDED(dwError);
    return true;

}

//
// Request Functions
//

request::request()
{
    this->KeyUsage = 0;
}

certificate client::GetCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;
    dwError =  VMCAGetSignedCertificateFromCSRA(
                    ServerName.c_str(),
                    req.GetCSR(keys).c_str(),
                    NotBefore,
                    NotAfter,
                    &pCertificate);

    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;
}


certificate client::GetCertificateFromCSR(const std::string& CSR, time_t NotBefore, time_t NotAfter)
{

    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;

    if (CSR.length() <= 0 )
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    dwError =  VMCAGetSignedCertificateFromCSRA(
                    ServerName.c_str(),
                    CSR.c_str(),
                    NotBefore,
                    NotAfter,
                    &pCertificate);
    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);
error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;

}


certificate client::GetSelfSignedCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    PVMCA_PKCS_10_REQ_DATAA data = NULL;
    certificate result;

    dwError = VMCAAllocatePKCS10DataA(&data);
    BAIL_ON_ERROR(dwError);

    dwError = req.InitPKCS10(data);
    BAIL_ON_ERROR(dwError);

    dwError = VMCACreateSelfSignedCertificateA(data,
              (PSTR) keys.privatekey.c_str(),
              NULL,
              NotBefore,
              NotAfter,
              &pCertificate);

    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    if (data != NULL) {
        VMCAFreePKCS10DataA(data);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}


KEYPAIR request::CreateKeyPair(unsigned int KeyLength) const
{
    PSTR pPrivateKey = NULL;
    PSTR pPublicKey = NULL;
    KEYPAIR result;

    DWORD dwError = VMCACreatePrivateKey(NULL, KeyLength, &pPrivateKey, &pPublicKey);
    BAIL_ON_ERROR(dwError);

    result.privatekey.assign(pPrivateKey);
    result.publickey.assign(pPublicKey);
error:
    if (pPrivateKey != NULL) {
        VMCAFreeKey(pPrivateKey);
    }
    if ( pPublicKey != NULL) {
        VMCAFreeKey(pPublicKey);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}


DWORD
request::InitPKCS10(PVMCA_PKCS_10_REQ_DATAA data) const
{

    DWORD dwError = 0;

    if (Name.length() >0 ){
        dwError = VMCASetCertValueA(VMCA_OID_CN, data,(PSTR) Name.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (Country.length() >0 ){
        dwError = VMCASetCertValueA(VMCA_OID_COUNTRY, data,(PSTR) Country.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if(Locality.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_LOCALITY, data,(PSTR) Locality.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (State.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_STATE, data,(PSTR) State.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (Organization.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_ORGANIZATION, data,(PSTR) Organization.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (OrgUnit.length() > 0 ) {
        dwError = VMCASetCertValueA(VMCA_OID_ORG_UNIT, data, (PSTR) OrgUnit.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if(DNSName.length() > 0 ) {
        dwError = VMCASetCertValueA(VMCA_OID_DNS, data, (PSTR) DNSName.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (Uri.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_URI, data, (PSTR) Uri.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if (Email.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_EMAIL, data, (PSTR) Email.c_str());
        BAIL_ON_ERROR(dwError);
    }

    if(IPAddress.length() > 0) {
        dwError = VMCASetCertValueA(VMCA_OID_IPADDRESS, data, (PSTR) IPAddress.c_str());
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCASetKeyUsageConstraintsA(data, KeyUsage);
    BAIL_ON_ERROR(dwError);
error :
    return dwError;

}

std::string request::GetCSR(const KEYPAIR& keypair) const
{

    DWORD dwError = 0;
    PVMCA_CSR pCSR = NULL;
    PVMCA_PKCS_10_REQ_DATAA data = NULL;
    std::string result;
    dwError = VMCAAllocatePKCS10DataA(&data);
    BAIL_ON_ERROR(dwError);

    dwError = InitPKCS10(data);
    BAIL_ON_ERROR(dwError);

    dwError =  VMCACreateSigningRequestA(data, (PSTR) keypair.privatekey.c_str(),
                                         NULL, &pCSR);
    BAIL_ON_ERROR(dwError);
    result.assign(pCSR);

error :

    if ( data != NULL) {
        VMCAFreePKCS10DataA(data);
    }

    if(pCSR != NULL) {
        VMCAFreeCSR(pCSR);
    }

    THROW_IF_NEEDED(dwError);
    return result;
}

VMCACRL client::GetCRL(const std::string& newCRL)
{
    VMCACRL result;
    DWORD dwError = 0;
    dwError = VMCAGetCRLA((PSTR)ServerName.c_str(),
                            NULL, (PSTR) newCRL.c_str());
    BAIL_ON_ERROR(dwError);
    result.filepath.assign(newCRL);

error :
    THROW_IF_NEEDED(dwError);
    return result;
}

time_t
vmcacrl::GetNextUpdate()
{
    time_t result;
    DWORD crlNum;
    time_t tmNextUpdate;
    time_t tmLastUpdate;
    DWORD dwError = 0;
    dwError = VMCAGetCRLInfo2(
                (PSTR)this->filepath.c_str(),
                &tmLastUpdate,
                &tmNextUpdate,
                &crlNum);
    BAIL_ON_ERROR(dwError);

    result = tmNextUpdate;

error :
    THROW_IF_NEEDED(dwError);
    return result;

}

time_t
vmcacrl::GetLastUpdate()
{
    time_t result;
    DWORD crlNum;
    time_t tmNextUpdate;
    time_t tmLastUpdate;
    DWORD dwError = 0;
    dwError = VMCAGetCRLInfo2(
                (PSTR)this->filepath.c_str(),
                &tmLastUpdate,
                &tmNextUpdate,
                &crlNum);
    BAIL_ON_ERROR(dwError);
    result = tmLastUpdate;
error :
    THROW_IF_NEEDED(dwError);
    return result;
}

DWORD
vmcacrl::GetCrlNumber()
{
    DWORD result;
    DWORD crlNum;
    time_t tmNextUpdate;
    time_t tmLastUpdate;
    DWORD dwError = 0;
    dwError = VMCAGetCRLInfo2(
                (PSTR)this->filepath.c_str(),
                &tmLastUpdate,
                &tmNextUpdate,
                &crlNum);
    BAIL_ON_ERROR(dwError);

    result = crlNum;
error :
    THROW_IF_NEEDED(dwError);
    return result;
}

//
// Client Functions
//
VMCAClient::VMCAClient(
    const std::string& username,
    const std::string& domain,
    const std::string& password,
    const std::string& netAddr
    )
{
    _pServerContext = new vmca_server_context(username, domain, password, netAddr);
}

VMCAClient::~VMCAClient()
{
    if (_pServerContext != NULL)
    {
        delete _pServerContext;
        _pServerContext = NULL;
    }
}

std::string VMCAClient::GetVersion()
{
    PSTR pVersion = NULL;
    DWORD dwError = 0;
    std::string result;
    dwError = VMCAGetServerVersionHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    &pVersion);
    BAIL_ON_ERROR(dwError);
    result.assign (pVersion);
error :
    if (pVersion != NULL) {
        VMCAFreeKey(pVersion);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

bool VMCAClient::AddRootCertificate(const std::string& Certificate, const std::string& PrivateKey)
{
    DWORD dwError = 0;
    dwError = VMCAAddRootCertificateHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    (PVMCA_CERTIFICATE) Certificate.c_str(),
                    NULL,
                    (PVMCA_KEY) PrivateKey.c_str());

    BAIL_ON_ERROR(dwError);
error :

    THROW_IF_NEEDED(dwError);
    return true;
}

REQUEST VMCAClient::GetRequest()
{
    REQUEST Req;
    return Req;
}


certificate VMCAClient::GetRootCACert()
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;

    dwError =  VMCAGetRootCACertificateHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    &pCertificate);
    BAIL_ON_ERROR(dwError);
    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;

}

vmcacontext2 VMCAClient::OpenEnumHandle(const std::string& statusFilter)
{
    DWORD dwError = 0;
    PVOID pContext = NULL;
    VMCA_CERTIFICATE_STATUS dwStatus = VMCA_CERTIFICATE_ALL;
    vmcacontext2 ctx;

    // initialize enum
    ctx.pContext = NULL;
    ctx.pclient  = NULL;
    ctx.currIndex = 0;
    ctx.enumStatus = VMCA_ENUM_ERROR;

    if ( std::strcmp(statusFilter.c_str(), "active") == 0) {
        dwStatus = VMCA_CERTIFICATE_ACTIVE;
    }
    else if ( std::strcmp(statusFilter.c_str(), "revoked") == 0) {
        dwStatus = VMCA_CERTIFICATE_REVOKED;
    }
    else if ( std::strcmp(statusFilter.c_str(), "expired") == 0) {
        dwStatus = VMCA_CERTIFICATE_EXPIRED;
    }
    else if ( std::strcmp(statusFilter.c_str(), "all") == 0) {
        dwStatus = VMCA_CERTIFICATE_ALL;
    }
    else {
        goto error;
    }

    dwError = VMCAOpenEnumContextHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    dwStatus,
                    &pContext
    );
    BAIL_ON_ERROR(dwError);

    ctx.pContext = pContext;
    ctx.pclient = this;

    return ctx;

error :
    if (pContext){
       VMCACloseEnumContext(pContext);
    }

    THROW_IF_NEEDED(dwError);
    return ctx;
}

certificate *VMCAClient::GetNextCertificate(vmcacontext2& ctx)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate *result = NULL;

    dwError = VMCAGetNextCertificate(
            ctx.pContext,
            &pCertificate,
            &ctx.currIndex,
            &ctx.enumStatus
        );

    BAIL_ON_ERROR(dwError);

    result = new certificate();
    result->certString.assign(pCertificate);

    VMCAFreeCertificate(pCertificate);
    pCertificate = NULL;

    return result;

error :
    if ( pCertificate) {
       VMCAFreeCertificate(pCertificate);
    }

    if (dwError == VMCA_ENUM_END)
    {
        dwError = 0; // This is the Standard Success Code
    }

    THROW_IF_NEEDED(dwError);
    return NULL;
}

VOID VMCAClient::CloseEnumHandle(vmcacontext2& ctx)
{
    VMCACloseEnumContext(ctx.pContext);
    ctx.pContext = NULL;
}

bool VMCAClient::Revoke(const certificate& cert)
{
    DWORD dwError = 0;

    dwError = VMCARevokeCertificateHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    (PVMCA_CERTIFICATE)cert.certString.c_str());
    BAIL_ON_ERROR(dwError);
error :
    THROW_IF_NEEDED(dwError);
    return true;

}

certificate VMCAClient::GetCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;
    dwError =  VMCAGetSignedCertificateFromCSRHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    req.GetCSR(keys).c_str(),
                    NotBefore,
                    NotAfter,
                    &pCertificate);

    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;
}


certificate VMCAClient::GetCertificateFromCSR(const std::string& CSR, time_t NotBefore, time_t NotAfter)
{

    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    certificate result;

    if (CSR.length() <= 0 )
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    dwError =  VMCAGetSignedCertificateFromCSRHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    CSR.c_str(),
                    NotBefore,
                    NotAfter,
                    &pCertificate);
    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);
error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    THROW_IF_NEEDED(dwError);
    return result;

}


certificate VMCAClient::GetSelfSignedCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter)
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    PVMCA_PKCS_10_REQ_DATAA data = NULL;
    certificate result;

    dwError = VMCAAllocatePKCS10DataA(&data);
    BAIL_ON_ERROR(dwError);

    dwError = req.InitPKCS10(data);
    BAIL_ON_ERROR(dwError);

    dwError = VMCACreateSelfSignedCertificateA(data,
              (PSTR) keys.privatekey.c_str(),
              NULL,
              NotBefore,
              NotAfter,
              &pCertificate);

    BAIL_ON_ERROR(dwError);

    result.certString.assign(pCertificate);

error :
    if(pCertificate != NULL) {
        VMCAFreeCertificate(pCertificate);
    }

    if (data != NULL) {
        VMCAFreePKCS10DataA(data);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

VMCACRL VMCAClient::GetCRL(const std::string& newCRL)
{
    VMCACRL result;
    DWORD dwError = 0;
    dwError = VMCAGetCRLHA(
                    _pServerContext->getContext(),
                    _pServerContext->getNetworkAddress().c_str(),
                    NULL,
                    (PSTR) newCRL.c_str());
    BAIL_ON_ERROR(dwError);
    result.filepath.assign(newCRL);

error :
    THROW_IF_NEEDED(dwError);
    return result;
}

vmca_server_context::vmca_server_context(
    const std::string& username,
    const std::string& domain,
    const std::string& password,
    const std::string& server_address
    ) : _username(username),
        _domain(domain),
        _password(password),
        _server_address(server_address),
        _pServerContext(NULL)
{
    PVMCA_SERVER_CONTEXT pContext = NULL;

    DWORD dwError = VMCAOpenServerA(
                        _server_address.c_str(),
                        username.c_str(),
                        domain.c_str(),
                        password.c_str(),
                        0,
                        NULL,
                        &pContext);
    THROW_IF_NEEDED(dwError);

    _pServerContext = pContext;
}

vmca_server_context::~vmca_server_context(
    VOID)
{
    if (_pServerContext != NULL)
    {
        VMCACloseServer(_pServerContext);
        _pServerContext = NULL;
    }
}

PVMCA_SERVER_CONTEXT
vmca_server_context::getContext(
    VOID
    )
{
    return _pServerContext;
}

std::string
vmca_server_context::getNetworkAddress(
    VOID
    )
{
    return _server_address;
}

