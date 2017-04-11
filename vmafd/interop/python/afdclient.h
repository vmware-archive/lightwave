
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



#ifndef __VMAFD_PY_CLIENT_H__
#define __VMAFD_PY_CLIENT_H__

#include <boost/python.hpp>
#include <boost/python/return_opaque_pointer.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/return_value_policy.hpp>

namespace bpl = boost::python;

typedef char* PSTR;

#include <string>
#include <time.h>
#include <vmafdclient.h>
#include <vecsclient.h>
#include <cdcclient.h>
#include <exception>
#include <iostream>
#include <Python.h>
#include <string>

PSTR VMAFDErrorCodeToName(int code);

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

typedef struct vafdcontext {
    PVOID pContext;
    int   currIndex;
    unsigned int enumStatus;
    struct client *pclient;
} VMAFDCONTEXT;

typedef struct opaque_ *opaque;

struct client {
    client(std::string ServerName);
    client();

    std::string GetStatus();

    std::string GetDomainName();
    void SetDomainName(std::string domainName);

    std::string GetDomainState();

    std::string GetLDU();
    void SetLDU(std::string LDUName);

    void SetRHTTPProxyPort(unsigned int port);

    void SetDCPort(unsigned int port);

    std::string GetCMLocation();

    std::string GetLSLocation();

    std::string GetDCName();
    void SetDCName(std::string MachineName);

    std::string GetPNID();
    void SetPNID(std::string PNID);

    std::string GetCAPath();
    void SetCAPath(std::string path);

    void JoinDomain(std::string DomainName, std::string OU,
        std::string UserName, std::string Password);

    std::string GetMachineName();

    std::string GetMachinePassword();

    std::string GetMachineCert();

    std::string GetMachinePrivateKey();

    std::string GetSiteGUID();

    std::string GetSiteName();

    std::string GetMachineID();

    void SetMachineID(std::string id);

    void SetMachineCert(
             std::string CertificateFile,
             std::string PrivateKeyFile);

    void SetMachineCertWithString(
             std::string PEMEncodedCertificate,
             std::string PEMEncodedPrivateKey);

    opaque CreateCertStore(
             std::string StoreName,
             std::string Password);

    opaque OpenCertStore(
             std::string StoreName,
             std::string Password);

    void CloseCertStore(
             opaque Store);

    void AddCert(
             opaque Store,
             int iEntryType,
             std::string Alias,
             std::string Cert,
             std::string PrivateKey,
             std::string Password,
             bool bAutoRefresh);

    void AddTrustedRoot(
             std::string PEMEncodedCertificate);

    std::string GetCertByAlias(
             opaque Store,
             std::string Alias);

    std::string GetPrivateKeyByAlias(
             opaque Store,
             std::string Password,
             std::string Alias);

    int GetEntryCount(
             opaque Store);

    opaque BeginEnumAliases(
             opaque Store,
             int EntryCount);

    bpl::list EnumAliases(
             opaque EnumContext);

    void EndEnumAliases(
             opaque EnumContext);

    void DeleteCert(
             opaque Store,
             std::string Alias);

    void DeleteCertStore(
             std::string StoreName);

    //cdc python bindings
    void EnableClientAffinity();

    void DisableClientAffinity();

    std::string GetAffinitizedDC(
             std::string DomainName,
             bool bForceRefresh);

    bpl::list EnumDCEntries();

    std::string GetCdcState();

    // members
    std::string ServerName;
};

class vmafd_exception : public std::exception
{
    unsigned int ErrorCode;
    std::string Message;

    public:
        vmafd_exception(unsigned int err)
        {
            this->ErrorCode = err;
            this->Message.assign(VMAFDErrorCodeToName(err));
         //   std::cout << "Message :" << this->Message <<std::endl;
        }

        const char *what() const throw()
        {
            return this->Message.c_str();
        }
        ~vmafd_exception() throw()
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
};

#endif //__VMAFD_CLIENT_H__
