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
// #include <iostream>
// #include <string>
// #include <vmca_h.h>
// #include <vmcaclient.h>
// #define BOOST_TEST_DYN_LINK
// #include <boost/test/unit_test.hpp>
// #include <boost/test/unit_test_suite.hpp>
// #define BOOST_TEST_MODULE VMCA test


#include <iostream>
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE VMCA Test
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <vmcaclient.h>
#include <vmcacommon.h>



using namespace std;



DWORD
VMCACreateCA(
    PSTR pszCACertificate,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    X509_CA** pCA
);
// VMCACreate CA allocates a Certificate Authority based on
// the Certificate and Private Key that is passed on by the user.
//
// Arguments :
//      pszCACertificate : The CA's Root Certificate
//      pszPrivateKey    : The Private Key for the Certificate
//      pszPassPhrase    : Pass Phrase that protects the Private Key
//      pCA              : returns a pointer to the allocated CA
// Returns :
//      Error Code

DWORD
VMCASignedRequestPrivate(
    X509_CA* pCA,
    PSTR pszPKCS10Request,
    PSTR *ppszCertificate,
    time_t tmNotBefore,
    time_t tmNotAfter
);
// VMCASignedRequestPrivate takes and CSR and signs the request
//
//Arguments :
//      pCA : The CA class that can sign the request
//      pszPKCS19Request : The Request that needs to be signed
//      ppszCertificate : Points to a PEM encoded Signed Cert
//      tmNotBefore : A Valid Time String that indicates when the Certificate is Valid From
//      tmNotAfter : The End of certificates validity
// Returns :
//  Error Code



DWORD
VMCASelfSignedCertificatePrivate(
    PVMCA_PKCS_10_REQ_DATA pCertRequest,
    PSTR pszPrivateKey,
    PSTR pszPassPhrase,
    //int  bCreateSelfSignedRootCA,
    PSTR* ppszCertificate
);


VOID
GetCSR(PVMCA_CSR* pCSR, std::string CnName)
{
    DWORD dwError = 0;
    std::string Name =CnName;
    std::string Organization = "vmware";
    std::string OU = "Engineering";
    std::string State ="Washington";
    std::string Country = "US";
    std::string Email = "aengineer@vmware.com";
    std::string ipAddress = "127.0.0.1";

    PVMCA_PKCS_10_REQ_DATA pCertReq = NULL;
    dwError = VMCAAllocatePKCS10Data(&pCertReq);
    BOOST_REQUIRE(pCertReq != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

     dwError = VMCAInitPKCS10DataAnsi(
        Name.c_str(),
        Organization.c_str(),
        OU.c_str(),
        State.c_str(),
        Country.c_str(),
        Email.c_str(),
        ipAddress.c_str(),
        //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
        pCertReq);


    PVMCA_KEY publicKey = NULL;
    PVMCA_KEY privateKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);



    dwError =  VMCACreateSigningRequest(
    pCertReq,
    privateKey,
    NULL,
    pCSR);

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);


    BOOST_REQUIRE(dwError == 0);
    VMCAFreeKey(publicKey);
    VMCAFreePKCS10Data(pCertReq);
}



VOID
GetCACertificate(PVMCA_CERTIFICATE* pCert, PVMCA_KEY* pPrivateKey)
{
    DWORD dwError = 0;
    std::string Name ="VMCA";
    std::string Organization = "vmware";
    std::string OU = "Engineering";
    std::string State ="Washington";
    std::string Country = "US";
    std::string Email = "vmca@vmware.com";
    std::string ipAddress = "127.0.0.1";

    PVMCA_PKCS_10_REQ_DATA pRootCertReqData = NULL;
    dwError = VMCAAllocatePKCS10Data(&pRootCertReqData);
    BOOST_REQUIRE(pRootCertReqData != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

     dwError = VMCAInitPKCS10DataAnsi(
        Name.c_str(),
        Organization.c_str(),
        OU.c_str(),
        State.c_str(),
        Country.c_str(),
        Email.c_str(),
        ipAddress.c_str(),
        //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
        pRootCertReqData);


    PVMCA_KEY publicKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, pPrivateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(pPrivateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);

/*
    dwError =  VMCASelfSignedCertificatePrivate(
            pRootCertReqData,
            *pPrivateKey,
            NULL,
            //1,
            pCert);
*/

    BOOST_REQUIRE(dwError == 0);
    VMCAFreeKey(publicKey);
    VMCAFreePKCS10Data(pRootCertReqData);
}

BOOST_AUTO_TEST_SUITE(Client)

BOOST_AUTO_TEST_CASE( TestAllocatePKCS10 )
{
    DWORD dwError = 0;
    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    printf("Executing %s", __FUNCTION__);

    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    VMCAFreePKCS10Data(pCertReqData);
}

BOOST_AUTO_TEST_CASE( TestAllocatePKCS10_WITH_NULL )
{
    DWORD dwError = 0;
    printf("Executing %s \n", __FUNCTION__);

    dwError = VMCAAllocatePKCS10Data(NULL);
    BOOST_REQUIRE(dwError != 0);
}

BOOST_AUTO_TEST_CASE( TestAllocatePKCS10_AND_INIT_FIELDS )
{
    DWORD dwError = 0;
    std::string Name ="Anu";
    std::string Organization = "vmware";
    std::string OU = "Engineering";
    std::string State ="Washington";
    std::string Country = "US";
    std::string Email = "aengineer@vmware.com";
    std::string ipAddress = "127.0.0.1";
    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    printf("Executing %s\n", __FUNCTION__);

    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

    dwError = VMCAInitPKCS10DataAnsi(
    Name.c_str(),
    Organization.c_str(),
    OU.c_str(),
    State.c_str(),
    Country.c_str(),
    Email.c_str(),
    ipAddress.c_str(),
    //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);

    BOOST_REQUIRE(Name.compare(pCertReqData->pszName) == 0 );
    BOOST_REQUIRE(Organization.compare(pCertReqData->pszOrganization) == 0 );
    BOOST_REQUIRE(OU.compare(pCertReqData->pszOU) == 0 );
    BOOST_REQUIRE(State.compare(pCertReqData->pszState) == 0 );
    BOOST_REQUIRE(Country.compare(pCertReqData->pszCountry) == 0 );
    BOOST_REQUIRE(Email.compare(pCertReqData->pszEmail) == 0 );
    BOOST_REQUIRE(ipAddress.compare(pCertReqData->pszIPAddress) == 0 );
    VMCAFreePKCS10Data(pCertReqData);
}


BOOST_AUTO_TEST_CASE( TestAllocatePKCS10_AND_INIT_FIELDS_WITH_NULLS )
{
    DWORD dwError = 0;
    PSTR Name = NULL;
    PSTR Organization = NULL;
    PSTR OU = NULL;
    PSTR State = NULL;
    PSTR Country = NULL;
    PSTR Email = NULL;
    PSTR  ipAddress = NULL;
    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    printf("Executing %s\n", __FUNCTION__);

    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

    dwError = VMCAInitPKCS10DataAnsi(
    Name,
    Organization,
    OU,
    State,
    Country,
    Email,
    ipAddress,
    //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);

    BOOST_REQUIRE(dwError == 0);

    PVMCA_KEY privateKey = NULL;
    PVMCA_KEY publicKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);

    PVMCA_CSR  pAllocatedCSR = NULL;

    dwError =  VMCACreateSigningRequest(
    pCertReqData,
    privateKey,
    NULL,
    &pAllocatedCSR);

    //
    // Name and Country must be Set, Verify that call fails
    //
    BOOST_REQUIRE(dwError != 0);

    VMCAFreePKCS10Data(pCertReqData);
    VMCAFreeKey(privateKey);
    VMCAFreeKey(publicKey);
    VMCAFreeCSR(pAllocatedCSR);
}

BOOST_AUTO_TEST_CASE( TestAllocatePKCS10_AND_INIT_FIELDS_WITH_NULLS_OTHER_THAN_COUNTRY_AND_NAME )
{
    DWORD dwError = 0;
    PSTR Name = "Anue";
    PSTR Organization = NULL;
    PSTR OU = NULL;
    PSTR State = NULL;
    PSTR Country = "US";
    PSTR Email = NULL;
    PSTR  ipAddress = NULL;
    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    printf("Executing %s\n", __FUNCTION__);

    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

    dwError = VMCAInitPKCS10DataAnsi(
    Name,
    Organization,
    OU,
    State,
    Country,
    Email,
    ipAddress,
    //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);

    BOOST_REQUIRE(dwError == 0);

    PVMCA_KEY privateKey = NULL;
    PVMCA_KEY publicKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);

    PVMCA_CSR  pAllocatedCSR = NULL;

    dwError =  VMCACreateSigningRequest(
    pCertReqData,
    privateKey,
    NULL,
    &pAllocatedCSR);

    //
    // Name and Country must is Set, Verify that call succeeded
    //
    BOOST_REQUIRE(dwError == 0);

    VMCAFreePKCS10Data(pCertReqData);
    VMCAFreeKey(privateKey);
    VMCAFreeKey(publicKey);
    VMCAFreeCSR(pAllocatedCSR);
}


BOOST_AUTO_TEST_CASE( TestCreateCSR )
{
    DWORD dwError = 0;
    std::string Name ="Anu";
    std::string Organization = "vmware";
    std::string OU = "Engineering";
    std::string State ="Washington";
    std::string Country = "US";
    std::string Email = "aengineer@vmware.com";
    std::string ipAddress = "127.0.0.1";

    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    printf("Executing %s\n", __FUNCTION__);

    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

    dwError = VMCAInitPKCS10DataAnsi(
    Name.c_str(),
    Organization.c_str(),
    OU.c_str(),
    State.c_str(),
    Country.c_str(),
    Email.c_str(),
    ipAddress.c_str(),
    //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);

    PVMCA_KEY privateKey = NULL;
    PVMCA_KEY publicKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);

    PVMCA_CSR  pAllocatedCSR = NULL;
    dwError =  VMCACreateSigningRequest(
    pCertReqData,
    privateKey,
    NULL,
    &pAllocatedCSR);

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);

     // std::cout << "\nBegin : Test Certificate Signing Request Generation \n"
     //          << pAllocatedCSR << std::endl
     //          << "End : Test Test Certificate Signing Request Generation \n";


    VMCAFreePKCS10Data(pCertReqData);
    VMCAFreeKey(privateKey);
    VMCAFreeKey(publicKey);
    VMCAFreeCSR(pAllocatedCSR);

}





BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Common)
BOOST_AUTO_TEST_CASE( TestPrivateKeyGen )
{
    DWORD dwError = 0;
    PVMCA_KEY privateKey = NULL;
    PVMCA_KEY publicKey = NULL;
    printf("Executing %s\n", __FUNCTION__);

    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );
    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);
    // std::cout << "\nBegin : Test Private Key Generation \n"
    //           << privateKey << std::endl
    //           << publicKey << std::endl
    //           << "End : Test Private Key Generation \n";
    VMCAFreeKey(privateKey);
    VMCAFreeKey(publicKey);

}


BOOST_AUTO_TEST_CASE( TestCreateCertificate )
{

// This test does the following things
//
// 1) Create a CSR Request DATA Object and init it with appropriate values
// 2) Create a Private Certificate
// 3) Create a CSR
// 4) Send a CA with Self Signed Certificate
// 5) Use that CA to sign the CSR and Get a Valid Certificate
// 6) Print out the Certificate
//

    DWORD dwError = 0;
    std::string Name ="Anu";
    std::string Organization = "vmware";
    std::string OU = "Engineering";
    std::string State ="Washington";
    std::string Country = "US";
    std::string Email = "aengineer@vmware.com";
    std::string ipAddress = "127.0.0.1";
    printf("Executing %s\n", __FUNCTION__);


    PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
    dwError = VMCAAllocatePKCS10Data(&pCertReqData);
    BOOST_REQUIRE(pCertReqData != NULL);
    BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

    dwError = VMCAInitPKCS10DataAnsi(
    Name.c_str(),
    Organization.c_str(),
    OU.c_str(),
    State.c_str(),
    Country.c_str(),
    Email.c_str(),
    ipAddress.c_str(),
    //0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);

    PVMCA_KEY privateKey = NULL;
    PVMCA_KEY publicKey = NULL;
    dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);
    BOOST_REQUIRE(publicKey != NULL);

    PVMCA_CSR  pAllocatedCSR = NULL;
    dwError =  VMCACreateSigningRequest(
    pCertReqData,
    privateKey,
    NULL,
    &pAllocatedCSR);

    BOOST_REQUIRE(dwError == 0);
    BOOST_REQUIRE(privateKey != NULL);

     // std::cout << "\nBegin : Test Certificate Signing Request Generation \n"
     //          << pAllocatedCSR << std::endl
     //          << "End : Test Test Certificate Signing Request Generation \n";



    VMCAFreePKCS10Data(pCertReqData);
    VMCAFreeKey(privateKey);
    VMCAFreeKey(publicKey);
    VMCAFreeCSR(pAllocatedCSR);

}





BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(Server)

// BOOST_AUTO_TEST_CASE( TestCreateSelfSignedCert )
// {
//     DWORD dwError = 0;
//     std::string CA_Name ="God";
//     std::string CA_Organization = "Heaven";
//     std::string CA_OU = "Engineering";
//     std::string CA_State ="Washington";
//     std::string CA_Country = "US";
//     std::string CA_Email = "god@Heaven.com";
//     std::string CA_ipAddress = "127.0.0.1";

//     std::string Name ="Anu";
//     std::string Organization = "vmware";
//     std::string OU = "Engineering";
//     std::string State ="Washington";
//     std::string Country = "US";
//     std::string Email = "aengineer@vmware.com";
//     std::string ipAddress = "127.0.0.1";


//     PVMCA_PKCS_10_REQ_DATA pRootCertReqData = NULL;
//     dwError = VMCAAllocatePKCS10Data(&pRootCertReqData);
//     BOOST_REQUIRE(pRootCertReqData != NULL);
//     BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

//     dwError = VMCAInitPKCS10DataAnsi(
//         CA_Name.c_str(),
//         CA_Organization.c_str(),
//         CA_OU.c_str(),
//         CA_State.c_str(),
//         CA_Country.c_str(),
//         CA_Email.c_str(),
//         CA_ipAddress.c_str(),
//         0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
//         pRootCertReqData);

//     PVMCA_KEY privateKey = NULL;
//     PVMCA_KEY publicKey = NULL;
//     dwError = VMCACreatePrivateKey( NULL, 1024, &privateKey, &publicKey );

//     BOOST_REQUIRE(dwError == 0);
//     BOOST_REQUIRE(privateKey != NULL);
//     BOOST_REQUIRE(publicKey != NULL);

//     PVMCA_CERTIFICATE pCARootCertificate = NULL;
//     dwError = VMCASelfSignedCertificatePrivate(
//                 pRootCertReqData,
//                 privateKey,
//                 NULL,
//                 TRUE,
//                 &pCARootCertificate);

//     BOOST_REQUIRE(dwError == 0);
//     BOOST_REQUIRE(pCARootCertificate != NULL);

//     std::cout << "\nBegin : Test Create Self Certificate \n"
//               << pCARootCertificate << std::endl
//               << "End : Test Create Self Certificate  \n";


//     X509_CA *pCA = NULL;
//     // dwError = VMCACreateCA(
//     //     pCARootCertificate,
//     //     privateKey,
//     //     NULL,
//     //     &pCA
//     // );

//     //BOOST_REQUIRE( pCA != NULL );

//     PVMCA_PKCS_10_REQ_DATA pPersonalCertReqData = NULL;
//     dwError = VMCAAllocatePKCS10Data(&pPersonalCertReqData);
//     BOOST_REQUIRE(pPersonalCertReqData != NULL);
//     BOOST_TEST_CHECKPOINT( "Calling into VMCAInitPKCS10DataAnsi");

//     dwError = VMCAInitPKCS10DataAnsi(
//         Name.c_str(),
//         Organization.c_str(),
//         OU.c_str(),
//         State.c_str(),
//         Country.c_str(),
//         Email.c_str(),
//         ipAddress.c_str(),
//         0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
//         pPersonalCertReqData);

//     PVMCA_KEY PersonalPrivateKey = NULL;
//     PVMCA_KEY PersonalPublicKey = NULL;
//     dwError = VMCACreatePrivateKey( NULL, 1024, &PersonalPrivateKey, &PersonalPublicKey );

//     BOOST_REQUIRE(dwError == 0);
//     BOOST_REQUIRE(PersonalPrivateKey != NULL);
//     BOOST_REQUIRE(PersonalPublicKey != NULL);


//     PVMCA_CSR  pAllocatedCSR = NULL;
//     dwError =  VMCACreateSigningRequest(
//     pPersonalCertReqData,
//     PersonalPrivateKey,
//     NULL,
//     &pAllocatedCSR);

//     BOOST_REQUIRE(dwError == 0);


//     PVMCA_CERTIFICATE pSignedCertificate = NULL;
//     time_t now;
//     time_t expire;
//     time(&now);
//     expire = now + (60 * 60 * 24* 365);

//     X509_Time stime(now);
//     X509_Time etime(expire);



//     std::string startTime(stime.readable_string());
//     std::string endTime(etime.readable_string());
//     std::cout << "Start Time : " << startTime << std::endl
//               << "End Time : "  << endTime    << std::endl;


//  //     dwError = VMCASignedRequestPrivate(
//  //                        pCARootCertificate,
//  //                        privateKey,
//  //                        NULL,
//  //                        pAllocatedCSR,
//  //                        &pSignedCertificate,
//  //                        now,
//  //                        expire);

//  // BOOST_REQUIRE(dwError == 0);
//  std::cout << pSignedCertificate <<std::endl;


// }

BOOST_AUTO_TEST_CASE( TestAddCertificate )
{

    DWORD dwError = 0;
    PVMCA_CERTIFICATE cert;
    PVMCA_KEY key;
    printf("Executing %s\n", __FUNCTION__);

    GetCACertificate(&cert, &key);
    //X509_CA* pCA;

    dwError =  VMCAAddRootCertificate(
                        "127.0.0.1",
                        cert,
                        NULL,
                        key
                        );

    printf("Error is : %d", dwError);
    BOOST_REQUIRE(dwError == 0);

}

BOOST_AUTO_TEST_CASE( TestSignedCert )
{

    PVMCA_CSR csr;
    DWORD cerlen = 0;
    PVMCA_CERTIFICATE cert = NULL;
    GetCSR(&csr, "cn=anue,cn=Users,dc=vmware,dc=com");
    PVMCA_CERTIFICATE_CONTAINER containers = NULL;

    DWORD dwError = 0;

    printf("Executing %s\n", __FUNCTION__);


    // TODO: fix this

    // dwError = VMCAGetSignedCertificate(
    //"localhost",
    //csr,
    //0, // valid from
    //0, // duration in seconds
    //&cerlen,
    //&containers );

    printf("Certificate : \n %s\n", containers->pCert);
    BOOST_REQUIRE(dwError==0);

}


BOOST_AUTO_TEST_CASE( TestGetRootCACertificate )
{

    DWORD cerlen = 0;
    PVMCA_CERTIFICATE cert = NULL;
    PVMCA_CERTIFICATE_CONTAINER containers = NULL;

    DWORD dwError = 0;
    printf("Executing %s\n", __FUNCTION__);

    // TODO: reimplement this

    //DWORD dwError =  VMCAGetRootCACertificate(
    //"localhost",
    //&cerlen,
    //&containers);

    printf(" Root CA Certificate : \n %s\n", containers->pCert);
    BOOST_REQUIRE(dwError==0);

}

BOOST_AUTO_TEST_CASE( TestRevokeCert)
{

    PVMCA_CSR csr;
    DWORD cerlen = 0;
    PVMCA_CERTIFICATE cert = NULL;
    GetCSR(&csr, "cn=RevokeTest,cn=Users,dc=vmware,dc=com");
    PVMCA_CERTIFICATE_CONTAINER containers = NULL;

    DWORD dwError = 0;
    printf("Executing %s\n", "TestRevokeCert");

    // TODO: fix this

    //DWORD dwError = VMCAGetSignedCertificate(
    //"localhost",
    //csr,
    //0, // valid from
    //0, // duration in seconds
    //&cerlen,
    //&containers );
    //
    printf("Certificate : \n %s\n", containers->pCert);
    dwError = VMCARevokeCertificate("localhost",(PSTR)containers->pCert);

    BOOST_REQUIRE(dwError==0);
    VMCAFreeCertificate((PSTR)containers->pCert);

}




BOOST_AUTO_TEST_SUITE_END()

