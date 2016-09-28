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
//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE VMCA Unit Test
//#include <boost/test/unit_test_suite.hpp>
//#include <boost/test/unit_test.hpp>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmca_error.h>
#include "testcases.h"
#include <dirent.h>


using namespace std;
using namespace boost::unit_test;
using namespace VMCAUtil;
void
VMCASetBit(unsigned long *flag, int bit)
{
    *flag |= 1<< bit;
}



//
// Please don't remove these unless you understand
// How Boost Unit Test works
//

BOOST_GLOBAL_FIXTURE(VMCATestSettings);

std::string GetServerName()
{
 try {
        pt:ptree pt;
        read_ini("testsettings.ini",pt);
        std::string ServerName =
        pt.get<std::string>("VMCAServer.ServerName", "localhost");
        return ServerName;
    } catch (std::exception& e) {
        // TODO : Log the Error
        std::cout << e.what() << std::endl;
        return "localhost";
    };
}

//
// Test under Test Code 1.1
//
BOOST_AUTO_TEST_SUITE(BVT)


BOOST_AUTO_TEST_CASE(BVT_CreateKeyPair)
/* This Test Cases Reads the Root CA Certificate
from the Running VMCA Server.
*/
{
    AutoProfile Perf("BVT_CreateKeyPair","1.1.2");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    dwError = Data.CreateKeyPair(1024);

    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetPrivatKey() != NULL) &&
                    (Data.GetPublicKey() != NULL) );


}


BOOST_AUTO_TEST_CASE(BVT_SetRootCACertificate)
/* This function  tries to create a Public
and Private Key */
{
    AutoProfile Perf("BVT_SetRootCACertificate","1.3");
    VMCAData Data;
    DWORD dwError = 0;
    DWORD dwMask = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;


    BOOST_CHECKPOINT("Call : CreateKeyPair");
    dwError = Data.CreateKeyPair(1024);

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.Config.cfgName = "CA,OU=LDU-TEST,dc=vsphere,dc=local";
    Data.Config.cfgCountry = "US";
    VMCASetBit((unsigned long int *)&dwMask, VMCA_KEY_CERT_SIGN);
    VMCASetBit((unsigned long int *)&dwMask, VMCA_KEY_CRL_SIGN);


    dwError = VMCASetKeyUsageConstraints(Data.GetDataReq(), dwMask );
    BOOST_REQUIRE(dwError == 0);

    Data.InitCSRFields();

    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetPrivatKey() != NULL) &&
                    (Data.GetPublicKey() != NULL) );

    time_t now;
    time_t expire;
    time(&now);
    expire = now + (365 * 24 * 60 * 60 * 10);

    dwError =  VMCACreateSelfSignedCertificate(
                   Data.GetDataReq(),
                   Data.GetPrivatKey(),
                   NULL,
                   now,
                   expire,
                   &pCertificate);
    printf("Error Code is : %d\n", dwError);
    BOOST_REQUIRE( (dwError == 0) &&
                   (pCertificate != NULL));

//
// Step 6:
// Now that we have the Certificate and the Private Key,
// we should push these artifacts to the Server
//
    dwError = VMCAAddRootCertificate(
                  GetServerName().c_str(),
                  pCertificate,
                  NULL,
                  Data.GetPrivatKey() );
    printf("Error Code is : %d\n", dwError);

    BOOST_REQUIRE(  (dwError == 0) ||
                    (dwError == VMCA_ROOT_CA_ALREADY_EXISTS));

    VMCAFreeCertificate(pCertificate);

}

BOOST_AUTO_TEST_CASE(BVT_GetRootCACertificate)
/* This Test Cases Reads the Root CA Certificate
from the Running VMCA Server.
*/
{
    AutoProfile Perf("BVT_GetRootCACertificate","1.1.2");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : GetRootCertificate");
    dwError = Data.GetRootCertificateFromServer(GetServerName());
    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetRootCertificate() != NULL));
}

BOOST_AUTO_TEST_CASE(SLOW_CreateDifferentSizeKeys)
/* This function  tries to create a Public
and Private Key */
{
    AutoProfile Perf("SLOW_CreateDifferentSizeKeys","1.3");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    dwError = Data.CreateKeyPair(1024);
    for(int keylen = 1024; keylen <= 4*1024 ; keylen+=1024) {

        Data.SetPublicKey(NULL);
        Data.SetPrivateKey(NULL);
        dwError = Data.CreateKeyPair(keylen);

        BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetPrivatKey() != NULL) &&
                    (Data.GetPublicKey() != NULL));
        BOOST_TEST_MESSAGE("Key Size Done :" << keylen);
    }

}


BOOST_AUTO_TEST_CASE(BVT_CreateCSR)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("BVT_CreateCSR","1.1");
    VMCAData Data;
    DWORD dwError = 0;
    PVMCA_CSR csr = NULL;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    dwError = Data.CreateKeyPair(2048);
    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetPrivatKey() != NULL) &&
                    (Data.GetPublicKey() != NULL));


    dwError = VMCACreateSigningRequest(Data.GetDataReq(),Data.GetPrivatKey(),NULL, &csr);
    BOOST_REQUIRE(  (dwError == 0) &&
                    (csr != NULL));

    VMCAFreeCSR(csr);
}

BOOST_AUTO_TEST_CASE(BVT_CreateSelfSignedCertificate)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("BVT_CreateSelfSignedCertificate","1.1");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    Data.CreateKeyPair(1024);

    BOOST_CHECKPOINT("Call : CreateSelfSignedCert");
    dwError = Data.CreateSelfSignedCert();
    BOOST_REQUIRE( (dwError == 0) &&
                (Data.GetSelfCertificate() != NULL));

}


BOOST_AUTO_TEST_CASE(NEG_InvalidCSR_Country_Missing)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("NEG_InvalidCSR_Country_Missing","1.2");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();
    Data.Config.cfgCountry="";


    BOOST_CHECKPOINT("Call : CreateCert should fail");
    dwError = Data.CreateCert(GetServerName());

    BOOST_REQUIRE( (dwError != 0) &&
        Data.GetCertificate() == NULL);
}

BOOST_AUTO_TEST_CASE(NEG_InvalidCSR_Name_Missing)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("NEG_InvalidCSR_Country_Missing","1.2");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.Config.cfgName="";
    Data.InitCSRFields();


    BOOST_CHECKPOINT("Call : CreateCert should fail");
    dwError = Data.CreateCert(GetServerName());

    BOOST_REQUIRE( (dwError != 0) &&
        Data.GetCertificate() == NULL);

}


BOOST_AUTO_TEST_CASE(BVT_VMCAGetSignedCertificate)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("BVT_VMCAGetSignedCertificate","1.1.3");
    VMCAData Data;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    dwError = Data.CreateKeyPair(1024);

    BOOST_CHECKPOINT("Call : CreateCert");
    dwError = Data.CreateCert(GetServerName());
    BOOST_REQUIRE_MESSAGE( (dwError == 0) &&
                    (Data.GetCertificate() != NULL), "Error Code : " << dwError);

}

BOOST_AUTO_TEST_CASE(BVT_Generate1000Certificate)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("BVT_Generate1000Certificate","1.1.3");
    VMCAData Data;
    DWORD dwError = 0;


    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    Data.CreateKeyPair(1024);

    for( int x = 0; x <1000; x++) {
        Data.SetCertificate(NULL);
        BOOST_CHECKPOINT("Call : CreateCert");
        dwError = Data.CreateCert(GetServerName());
        BOOST_REQUIRE_MESSAGE( (dwError == 0) &&
                    (Data.GetCertificate() != NULL), "Error Code : " << dwError);
    }
}


BOOST_AUTO_TEST_CASE(BVT_Enum100Certs_ALL)
{

AutoProfile Perf("BVT_Enum100Certs","1.1.3");

DWORD dwError = 0;
PVOID pContext = NULL;
PVMCA_CERTIFICATE pCertificate = NULL;
int CurrentIndex = 0;
VMCA_ENUM_CERT_RETURN_CODE Status;

dwError = VMCAOpenEnumContext(  GetServerName().c_str(),
                                VMCA_CERTIFICATE_ALL,
                                &pContext);

BOOST_REQUIRE(( dwError == 0) &&
               (pContext != NULL));

while ( CurrentIndex < 100 ) {
    dwError = VMCAGetNextCertificate(
        pContext,
        &pCertificate,
        &CurrentIndex,
        &Status
    );

    BOOST_REQUIRE(( dwError == 0) &&
                   (pCertificate != NULL) &&
                   (Status == VMCA_ENUM_SUCCESS));

    VMCAFreeCertificate(pCertificate);
    }

    dwError = VMCACloseEnumContext(pContext);
    BOOST_REQUIRE( dwError == 0);

}

BOOST_AUTO_TEST_CASE(BVT_EnumAndRevoke100Certs)
{

AutoProfile Perf("BVT_EnumAndRevoke100Certs","1.1.3");

DWORD dwError = 0;
PVOID pContext = NULL;
PVMCA_CERTIFICATE pCertificate = NULL;
int CurrentIndex = 0;
VMCA_ENUM_CERT_RETURN_CODE Status;

dwError = VMCAOpenEnumContext(  GetServerName().c_str(),
                                VMCA_CERTIFICATE_ACTIVE,
                                &pContext);

BOOST_REQUIRE(( dwError == 0) &&
               (pContext != NULL));

while ( CurrentIndex < 100 ) {
    dwError = VMCAGetNextCertificate(
        pContext,
        &pCertificate,
        &CurrentIndex,
        &Status
    );
    std::cout << "Error Value : "
              << dwError << std::endl;
    BOOST_REQUIRE(( dwError == 0) &&
                   (pCertificate != NULL) &&
                   ((Status == VMCA_ENUM_SUCCESS) || (Status == VMCA_ENUM_END)));
    dwError = VMCARevokeCertificate(GetServerName().c_str(), pCertificate);
    if( dwError == VMCA_CERT_IO_FAILURE) {
    std::cout << "Revoke Error Value : " << dwError << std::endl;

    std::cout
                << std::endl << std::endl
                << pCertificate
                << std::endl << std::endl;
    }

    VMCAFreeCertificate(pCertificate);
    }

    dwError = VMCACloseEnumContext(pContext);

    std::cout << "CloseEnum Error Value : " << dwError << std::endl;
    BOOST_REQUIRE( dwError == 0);

}

BOOST_AUTO_TEST_CASE(BVT_Enum100Certs_ACTIVE)
{

AutoProfile Perf("BVT_Enum100Certs","1.1.3");

DWORD dwError = 0;
PVOID pContext = NULL;
PVMCA_CERTIFICATE pCertificate = NULL;
int CurrentIndex = 0;
VMCA_ENUM_CERT_RETURN_CODE Status;

dwError = VMCAOpenEnumContext(  GetServerName().c_str(),
                                VMCA_CERTIFICATE_ACTIVE,
                                &pContext);

BOOST_REQUIRE(( dwError == 0) &&
               (pContext != NULL));

while ( CurrentIndex < 100 ) {
    dwError = VMCAGetNextCertificate(
        pContext,
        &pCertificate,
        &CurrentIndex,
        &Status
    );

    BOOST_REQUIRE(( dwError == 0) &&
                   (pCertificate != NULL) &&
                   (Status == VMCA_ENUM_SUCCESS));

    VMCAFreeCertificate(pCertificate);
    }

    dwError = VMCACloseEnumContext(pContext);
    BOOST_REQUIRE( dwError == 0);

}



BOOST_AUTO_TEST_CASE(CreateACertwithKEY_CRL_SIGN)
{
    AutoProfile("CreateACertwithKEY_CRL_SIGN","1.1.4");
    VMCAData Data;
    PSTR pCertString = NULL;
    DWORD dwError = 0;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call : CreateKeyPair");
    Data.CreateKeyPair(1024);

    dwError = VMCASetKeyUsageConstraints(Data.GetDataReq(), VMCA_KEY_CRL_SIGN);
     BOOST_REQUIRE_MESSAGE( (dwError == 0), "Erorr : " << dwError);

    BOOST_CHECKPOINT("Call : CreateCert");
    dwError = Data.CreateCert(GetServerName());
    BOOST_REQUIRE( dwError == 0);

    dwError = VMCAGetCertificateAsString(Data.GetCertificate(), &pCertString);
    BOOST_REQUIRE( dwError == 0);

    dwError = VMCAWriteCertificateToFile("KEY_CRL_SIGN.crt",Data.GetCertificate());
    BOOST_REQUIRE( dwError == 0);

    std::cout << pCertString << std::endl;

}


BOOST_AUTO_TEST_CASE(testGetCertificateCount)
{
    AutoProfile("testGetCertificateCount","1.1.4");
    DWORD dwError = 0;
    DWORD dwCount = 0;
    BOOST_CHECKPOINT("Call : VMCAGetCertificateCount");

    dwError = VMCAGetCertificateCount(GetServerName().c_str(),
                                            VMCA_CERTIFICATE_ALL, &dwCount);

    BOOST_REQUIRE( dwError == 0);
    std::cout <<  "All Certificate Count = " << dwCount << std::endl;

}

BOOST_AUTO_TEST_CASE(testReadLocalCerts)
{
    AutoProfile("testReadLocalCerts","1.1.4");
    DWORD dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEntry = NULL;
    PVMCA_CERTIFICATE pszCert = NULL;
    PSTR pCertString = NULL;
    pDir = opendir("./build/certool");
    char fullFileName[1024] = { 0 };
    if(pDir == NULL) {
        dwError = errno;
    }

    BOOST_CHECKPOINT("Call : VMCAReadCertificateFromFilePrivate");

    while((pEntry = readdir(pDir)) != NULL) {
        if ( pEntry->d_type == DT_REG) {
            if ( strncmp("testcert", pEntry->d_name, strlen("testcert")) == 0) {

                sprintf(fullFileName, "%s/%s","./build/certool",pEntry->d_name);
                printf("%s\n", fullFileName);

                dwError = VMCAReadCertificateFromFile(
                    fullFileName,
                    &pszCert
                    );

                if (dwError != 0) {
                    printf("Read From File : Error Code is : %d", dwError);
                }

                dwError = VMCAGetCertificateAsString(
                                    (PSTR)pszCert,&pCertString );
                if (dwError != 0) {
                    printf("Covert to String :Error Code is : %d", dwError);
                }


                BOOST_REQUIRE(dwError == 0);
                VMCAFreeCertificate(pszCert);
                VMCAFreeCertificate(pCertString);
                }
        }
    }
}
BOOST_AUTO_TEST_SUITE_END()
