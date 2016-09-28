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
#include <iostream>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmca_error.h>
#include "testcases.h"
#include "vmcaOpenSsl.h"

using namespace std;
using namespace boost::unit_test;
using namespace VMCAUtil;


BOOST_AUTO_TEST_SUITE(LINUX_OS)



BOOST_AUTO_TEST_CASE(OpenSSLGetSubjectNameFromRootCA)
{

    AutoProfile Perf("OpenSSLGetSubjectNameFromRootCA","1.1.2");
    VMCAData Data;
    DWORD dwError = 0;
    PSTR pSubjectName = NULL;

    BOOST_CHECKPOINT("Call : GetRootCertificate");
    dwError = Data.GetRootCertificateFromServer(GetServerName());
    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetRootCertificate() != NULL));
    dwError = VMCAWriteCertificateToFile("RootCA-Test.cert",Data.GetRootCertificate());
    BOOST_REQUIRE(dwError == 0);
    dwError = SSLGetSubjectNameFromCertificate("RootCA-Test.cert",&pSubjectName);
    BOOST_REQUIRE( (dwError == 0 ) &&
                    (pSubjectName != NULL));
    free(pSubjectName);
}

BOOST_AUTO_TEST_CASE (OpenSSLGetSubjectNameFromCert)
{

    AutoProfile Perf("OpenSSLGetSubjectNameFromCert","1.1.2");
    std::string verifyString("/C=US/ST=California/L=Palo Alto/O=VMware/OU=VMware Engineering/CN=openssl");

    VMCAData Data;
    DWORD dwError = 0;
    PSTR pSubjectName = NULL;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.Config.cfgName="openssl";
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    dwError = Data.CreateKeyPair(1024);

    BOOST_CHECKPOINT("Call : CreateCert");
    dwError = Data.CreateCert(GetServerName());
    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetCertificate() != NULL));

    dwError = VMCAWriteCertificateToFile("SSLCert-Test.cert",Data.GetCertificate());
    BOOST_REQUIRE(dwError == 0);

    dwError = SSLGetSubjectNameFromCertificate("SSLCert-Test.cert",&pSubjectName);
    BOOST_REQUIRE( (dwError == 0 ) &&
                    (pSubjectName != NULL));
    BOOST_REQUIRE_MESSAGE(verifyString.compare(pSubjectName) == 0, "Found : " << pSubjectName);
    free(pSubjectName);
}

BOOST_AUTO_TEST_CASE(OpenSSLExternalCSRtoVMCACert)
{

    AutoProfile Perf("OpenSSLExternalCSRtoVMCACert","1.1.2");
    PSTR CSR;
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCert = NULL;
    PSTR pszCertString = NULL;
    time_t now = 0;
    time_t expire = 0;
    time(&now);
    expire = now + ( 365 * 24 * 60 * 60 * 10);

    SSLCreateCSR(&CSR);

    dwError = VMCAGetSignedCertificateFromCSR(
                        GetServerName().c_str(),
                        CSR,
                        now,
                        expire,
                        &pCert);

    BOOST_REQUIRE( (dwError == 0) &&
                   (pCert != NULL) );

    dwError = VMCAGetCertificateAsString(
                  (PSTR) pCert,
                  &pszCertString);
    BOOST_REQUIRE( (dwError == 0) &&
                    (pszCertString != NULL));

    //std::cout << pszCertString << std::endl;

    delete(CSR);
    VMCAFreeCertificate(pCert);
    VMCAFreeStringA(pszCertString);
}

BOOST_AUTO_TEST_SUITE_END()

