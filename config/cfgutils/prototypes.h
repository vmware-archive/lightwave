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



// certificate.c

DWORD
VmwDeployMakeRootCACert(
    PCSTR pszHostname,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PSTR* ppszCert
    );

DWORD
VmwDeployGetRootCACert(
    PCSTR pszServername,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PSTR* ppszCert
    );

DWORD
VmwDeployCreateMachineSSLCert(
    PCSTR pszServername,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszSubjectName,
    PCSTR pszSubjectAltName,
    PSTR* ppszPrivateKey,
    PSTR* ppszCert
    );

DWORD
VmwDeployAddTrustedRoot(
    PCSTR pszServername,
    PCSTR pszCACert
    );

DWORD
VmwDeployGetDomainDN(
    PCSTR pszDomain,
    PSTR* ppszDomainDN
    );

// dns.c

DWORD
VmwDeploySetForwarders(
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszForwarders
    );

// logging.c

VOID
VmwDeployLogMessage(
    VMW_DEPLOY_LOG_LEVEL logLevel,
    PCSTR                pszFormat,
    ...
    );

// service.c

DWORD
VmwDeployStartService(
    PCSTR pszName
    );

DWORD
VmwDeployStopService(
    PCSTR pszName
    );

DWORD
VmwDeployRestartService(
    PCSTR pszName
    );

// sysutils.c

BOOLEAN
VmwDeployIsIPAddress(
    PCSTR pszIPAddr
    );

BOOLEAN
VmwDeployIsIPV4Address(
    PCSTR pszIPAddr
    );

BOOLEAN
VmwDeployIsIPV6Address(
    PCSTR pszIPAddr
    );

DWORD
VmwDeployWriteToFile(
    PCSTR pszContent,
    PCSTR pszDirPath,
    PCSTR pszFilename
    );

// validate.c

DWORD
VmwDeployValidateHostname(
    PCSTR pszHostname
    );

DWORD
VmwDeployValidateDNSForwarders(
    PCSTR pszForwarders
    );

