
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



#ifndef __VMCA_CERT_CLIENT_H__
#define __VMCA_CERT_CLIENT_H__

#ifdef _WIN32
#include <windows.h>
#endif

typedef char* PSTR;
#include <string>
#include <time.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <exception>
#include <iostream>
#include <string>
#include <stdio.h>
#include <boost/format.hpp>


PSTR VMCAErrorCodeToName(int code);


struct client;

#define BAIL_ON_ERROR(dwError) do { if (dwError) goto error; } while(0)
void THROW_IF_NEEDED(unsigned int dwError);


//#define ERROR_INVALID_CLIENT 1001

typedef struct keypair {
    std::string privatekey;
    std::string publickey;
} KEYPAIR;

typedef struct certificate {
    std::string certString;
    std::string print();
    bool isCACert();
 } CERTIFICATE;

typedef struct request {
    request();
    KEYPAIR CreateKeyPair(unsigned int KeyLength) const;
    std::string GetCSR(const KEYPAIR& keypair) const;
    DWORD InitPKCS10(PVMCA_PKCS_10_REQ_DATAA data) const;
    std::string Name;
    std::string Country;
    std::string Locality;
    std::string State;
    std::string Organization;
    std::string OrgUnit;
    std::string DNSName;
    std::string Uri;
    std::string Email;
    std::string IPAddress;
    unsigned int KeyUsage;
} REQUEST, *PREQUEST;

typedef struct vmcacontext {
    PVOID pContext;
    int   currIndex;
    VMCA_ENUM_CERT_RETURN_CODE enumStatus;
    struct client *pclient;
} VMCACONTEXT;

typedef struct vmcacontext2 {
    PVOID pContext;
    int   currIndex;
    VMCA_ENUM_CERT_RETURN_CODE enumStatus;
    struct VMCAClient *pclient;
} VMCACONTEXT2;

typedef struct vmcacrl {
    std::string filepath;
    time_t GetNextUpdate();
    time_t GetLastUpdate();
    DWORD GetCrlNumber();
}VMCACRL;

struct client {
    client(const std::string& CertServerName);
    VOID Login(const std::string& UserName,
                const std::string& Password, const std::string& DomainName);
    REQUEST GetRequest();
    std::string GetVersion();
    bool AddRootCertificate(const std::string& Certificate, const std::string& PrivateKey);
    certificate GetRootCACert();

    //enum APIs
    vmcacontext OpenEnumHandle(const std::string& statusFilter);
    certificate *GetNextCertificate(vmcacontext& ctx);
    VOID CloseEnumHandle(vmcacontext& ctx);

    certificate GetCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter);
    certificate GetSelfSignedCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter);
    certificate GetCertificateFromCSR(const std::string& CSR, time_t NotBefore, time_t NotAfter);
    bool Revoke(const certificate& cert);
    VMCACRL GetCRL(const std::string& newCrl);
    VOID Logout();

    // members
    std::string ServerName;

};

class vmca_server_context
{
public:

    vmca_server_context(
        const std::string& username,
        const std::string& domain,
        const std::string& password,
        const std::string& server_address
        );

    ~vmca_server_context();

    PVMCA_SERVER_CONTEXT getContext();
    std::string getNetworkAddress();

protected:

    vmca_server_context(vmca_server_context& other);
    vmca_server_context& operator=(const vmca_server_context& other);

private:

    PVMCA_SERVER_CONTEXT _pServerContext;

    std::string _username;
    std::string _domain;
    std::string _password;
    std::string _server_address;
};

struct VMCAClient {
    VMCAClient(
        const std::string& username,
        const std::string& domain,
        const std::string& password,
        const std::string& netAddr
        );
    ~VMCAClient();

    REQUEST GetRequest();
    std::string GetVersion();
    bool AddRootCertificate(const std::string& Certificate, const std::string& PrivateKey);
    certificate GetRootCACert();

    //enum APIs
    vmcacontext2 OpenEnumHandle(const std::string& statusFilter);
    certificate *GetNextCertificate(vmcacontext2& ctx);
    VOID CloseEnumHandle(vmcacontext2& ctx);

    certificate GetCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter);
    certificate GetSelfSignedCertificate(const REQUEST& req, const KEYPAIR& keys, time_t NotBefore, time_t NotAfter);
    certificate GetCertificateFromCSR(const std::string& CSR, time_t NotBefore, time_t NotAfter);
    bool Revoke(const certificate& cert);
    VMCACRL GetCRL(const std::string& newCrl);

    // members
    vmca_server_context* _pServerContext;
};


class vmca_exception : public std::exception
{
        unsigned int ErrorCode;
        std::string Message;
        std::string ExceptionText;
    public:
        vmca_exception(unsigned int err)
        {
            this->ErrorCode = err;
            this->Message.assign(VMCAErrorCodeToName(err));
            // Format it like IOError, "[Error 87] UNKNOWN"
            boost::format et("[Error %d] %s");
            et % this->ErrorCode % this->Message;
            this->ExceptionText.assign(et.str());
            std::cout << "Message :" << this->Message <<std::endl;
        }

        const char *what() const throw()
        {
            return this->Message.c_str();
        }
        ~vmca_exception() throw()
        {

        }

        std::string getMessage()
        {
            return this->Message;
        }

        unsigned int getErrorCode()
        {
            return this->ErrorCode;
        }

        std::string print()
        {
            return this->ExceptionText;
        }
};


#endif //__VMCA_CERT_CLIENT_H__



