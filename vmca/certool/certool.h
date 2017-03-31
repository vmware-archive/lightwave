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
#ifndef __CERT_TOOL_H__
#define __CERT_TOOL_H__

#include <boost/program_options.hpp>
#include <string>
#include <includes.h>

namespace po = boost::program_options;

//
// Globals
//
extern std::string cfgName;
extern std::string cfgDomainComponent;
extern std::string cfgCountry;
extern std::string cfgOrganization;
extern std::string cfgOrgUnit;
extern std::string cfgState;
extern std::string cfgLocality;
extern std::string cfgIPAddress;
extern std::string cfgEmail;
extern std::string cfgHostName;

extern std::string argServerName;
extern std::string argSrpUpn;
extern std::string argSrpPwd;
extern std::string argCert;
extern std::string argPrivateKey;
extern std::string argPublicKey;
extern std::string argConfig;
extern std::string argCsr;
extern std::string argHelpModule;
extern std::string argFilter;
extern std::string argPassword;
extern std::string argUserName;
extern std::string argDomainName;
extern int argPort;
extern int argWait;
extern int argErr;
extern std::string argFQDN;
extern std::string argIP;
extern bool argPKCS12;
extern std::string argOutPrivateKey;
extern std::string argOutCert;
extern std::string argOption;
extern int argPredates;

extern bool argStorePrivate;
extern bool argStoreTrusted;
extern bool argStoreRevoked;
extern bool argStoreAll;


extern std::string argCrl;
extern std::string argCurrCrl;

extern time_t now;
extern time_t expire;

#define VMCA_TIME_SECS_PER_MINUTE           ( 60)
#define VMCA_TIME_SECS_PER_HOUR             ( 60 * VMCA_TIME_SECS_PER_MINUTE)
#define VMCA_TIME_SECS_PER_DAY              ( 24 * VMCA_TIME_SECS_PER_HOUR)
#define VMCA_TIME_SECS_PER_WEEK             (  7 * VMCA_TIME_SECS_PER_DAY)
#define VMCA_TIME_SECS_PER_YEAR             (365 * VMCA_TIME_SECS_PER_DAY)

#define VMCA_DEFAULT_CA_CERT_VALIDITY       (10 * VMCA_TIME_SECS_PER_YEAR)
#define VMCA_CA_CERT_EXPIRY_START_LAG       ( 3 * VMCA_TIME_SECS_PER_DAY)
#define VMCA_CERT_EXPIRY_START_LAG          (10 * VMCA_TIME_SECS_PER_MINUTE)
#define VMCA_DEFAULT_CERT_VALIDITY          (10 * VMCA_TIME_SECS_PER_YEAR)

#define VMCA_MAX_PREDATE_PERMITED           (VMCA_TIME_SECS_PER_WEEK/VMCA_TIME_SECS_PER_MINUTE)
#define VMCA_DEFAULT_CA_CERT_START_PREDATE  (VMCA_CA_CERT_EXPIRY_START_LAG/VMCA_TIME_SECS_PER_MINUTE)

#define VMCA_OPTION_MULTIPLE_SAN            "multiplesan"

enum VMCA_FILE_ENCODING
{
    VMCA_FILE_ENCODING_UTF8,
    VMCA_FILE_ENCODING_UTF16LE,
    VMCA_FILE_ENCODING_UTF16BE,
    VMCA_FILE_ENCODING_UTF7
};

DWORD
HandleInitCSR();

DWORD
HandleCreateSelfSignedCA();

DWORD
HandleRootCACertificate();

DWORD
HandleGenCert();

DWORD
HandleGenKey();

DWORD
HandleRevokeCert();

DWORD
HandleViewCert();

DWORD
HandleGetRootCA();

DWORD
HandleVersionRequest();

DWORD
HandleEnumCerts();

DWORD
HandleGetDC();

DWORD
HandleGenCISCert();

DWORD
HandleWaitVMCA();

DWORD
HandleWaitVMDIR();

DWORD
HandleStatusCert();

DWORD
HandleInitVMCA();

DWORD
HandleLogin();

DWORD
HandleLogout();

DWORD
HandleVecsEnum();

DWORD
HandleGetCRL();

DWORD
HandleGenCRL();

DWORD
HandleCRLInfo();

DWORD
HandlePrintError();

DWORD
HandlePrintCRL();

DWORD
HandlePublishRoots();

DWORD
HandleUpdateSchema();

DWORD
HandleSetRootCA();

DWORD
HandleGenSelfCert();

DWORD
HandleGenCSRFromCert();

DWORD
HandleSetServerOption();

DWORD
HandleUnsetServerOption();

DWORD
HandleGetServerOption();
//Utility Functions

#define FQDN 1
#define NETBIOSNAME 3

int
GetSleepTime(int secondToSleep);

DWORD
GetMachineName(PSTR *ppMachineName);

DWORD
GetFQDN(PSTR *ppFQDN);

#endif //__CERT_TOOL_H__


