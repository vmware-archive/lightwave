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
#include "certool.h"
#include <vmca.h>
#include <vmcacommon.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <vmca_error.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#ifndef __NO_AFD__
#include <vmafdclient.h>
#endif

#if _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#define snprintf _snprintf
#define sleep Sleep
#endif // _MSC_VER

std::string FriendlyName;

const size_t CERTOOL_PRIVATE_KEY_LENGTH = 2048;

//
// This file is written as a sample code for the end user.
// Hence this file will be overly verbose.
// Please bear with the example style of commenting.
//

static DWORD
_HandleOpenServer(
    PVMCA_SERVER_CONTEXT *ppServer
    )
{
    DWORD dwError = 0;
    PVMCA_SERVER_CONTEXT hServer = NULL;

    dwError = VMCAOpenServerA(
                  (PCSTR) argServerName.c_str(),
                  (PCSTR) argSrpUpn.c_str(),
                  NULL,
                  (PCSTR) argSrpPwd.c_str(),
                  0, //flags
                  NULL, // Reserved
                  &hServer);
    BAIL_ON_ERROR(dwError);

    *ppServer = hServer;

error:
    return dwError;
}

static
DWORD
UpdateReqData(PVMCA_PKCS_10_REQ_DATAA pCertReqData)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( cfgName.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_CN,
                  pCertReqData, (PSTR)cfgName.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if (cfgDomainComponent.length() > 0)
    {
        dwError = VMCASetCertValueA(
                                    VMCA_OID_DC,
                                    pCertReqData,
                                    (PSTR)cfgDomainComponent.c_str()
                                   );
        BAIL_ON_ERROR(dwError);
    }

    if(cfgCountry.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_COUNTRY,
                  pCertReqData, (PSTR)cfgCountry.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgLocality.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_LOCALITY,
                  pCertReqData, (PSTR)cfgLocality.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if (cfgState.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_STATE,
                  pCertReqData, (PSTR)cfgState.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgOrganization.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_ORGANIZATION,
                  pCertReqData,(PSTR) cfgOrganization.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgOrgUnit.length()>0) {
      dwError = VMCASetCertValueA( VMCA_OID_ORG_UNIT,
                  pCertReqData, (PSTR)cfgOrgUnit.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgHostName.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_DNS,
                  pCertReqData, (PSTR)cfgHostName.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgEmail.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_EMAIL,
                pCertReqData, (PSTR)cfgEmail.c_str());
      BAIL_ON_ERROR(dwError);
    }

    if(cfgIPAddress.length() > 0) {
      dwError = VMCASetCertValueA( VMCA_OID_IPADDRESS,
          pCertReqData, (PSTR)cfgIPAddress.c_str());
      BAIL_ON_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
HandleInitCSR()

//
// This function generates a Root CA CSR , A private Key and Public Key for
// CA Certificates use, In order to use this CSR the generated file must
// taken to a CA that can sign the certificate and return it to us.
//
// Here are the steps in the initCSR creation process
//  1) We need to create a Public/Private Key Pair
//  2) Create a Certificate Signing Request Object -- which is PVMCA_PKCS_10_REQ_DATAA
//  3) Populate all fields that we are intersted in
//  4) Sign the CSR with our Private Key -- Please note: Private key never leaves our machine
//  5) Write the Private Key, Public Key and CSR to  user specified location
//
{

    DWORD dwError = 0;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CSR pCSR = NULL;
    PVMCA_CERTIFICATE pCertificate = NULL;

    std::ofstream oPrivatefile;
    std::ofstream oPublicfile;
    std::ofstream oCSRfile;

//
// Step 1: Create a Public / Private Key Pair
// The Key Length is set to 1024, Password is set to NULL
//
    dwError = VMCACreatePrivateKey( NULL, CERTOOL_PRIVATE_KEY_LENGTH, &pPrivateKey, &pPublicKey );
    BAIL_ON_ERROR(dwError);
//
// Step 2: Create a Certificate Signing Request Object
//
    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    BAIL_ON_ERROR(dwError);

    dwError = UpdateReqData(pCertReqData);
    BAIL_ON_ERROR(dwError);

//
// Step 4: Create a Signing Request CSR ( PKCS10)
//
    dwError =  VMCACreateSigningRequestA(
                   pCertReqData,
                   pPrivateKey,
                   NULL,
                   &pCSR);
    BAIL_ON_ERROR(dwError);
//
// Step 5 :
// write the private key to a location specified by the user
// This for illustration purpose only, you can also use
// VMCAWritePrivateKeyToFile which will handle things like Password on the
// Private Key.
//
    oPrivatefile.open (argPrivateKey.c_str());
    oPrivatefile << pPrivateKey;
    oPrivatefile.close();

    oPublicfile.open (argPublicKey.c_str());
    oPublicfile << pPublicKey;
    oPrivatefile.close();

    oCSRfile.open(argCsr.c_str());
    oCSRfile << pCSR;
    oCSRfile.close();

error:
    VMCAFreeKey(pPrivateKey);
    VMCAFreeKey(pPublicKey);
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeCSR(pCSR);
    VMCAFreeCertificate(pCertificate);

    return dwError;
}

VOID
VMCAGetGMTime( time_t tm, time_t *pUtcTime)
{
#ifndef _WIN32
    struct tm ptm = {0};
    gmtime_r(&tm, &ptm);
    *pUtcTime = timegm(&ptm);
#else
    struct tm* tptr = NULL;
    tptr = gmtime( &tm );
    *pUtcTime = _mkgmtime(tptr);
#endif
}

DWORD
HandleCreateSelfSignedCA()
//
// This function generates a Root CA Certificate and uploads that
// to VMCA Server
// In order to generate a Self Signed Certificate, we need to create
// a bunch of artifacts, whichSelf when combined will produce a Certificate.
//
//  1) We need to create a Public/Private Key Pair
//  2) Create a Certificate Signing Request Object -- which is PVMCA_PKCS_10_REQ_DATAA
//  3) Populate all fields that we are intersted in
//  4) Sign the CSR with our Private Key -- Please note: Private key never leaves our machine
//  5) Create a Local CA Certificate
//  6) Push that Certificate to the Server, so that CA can be operational
//
//
{
    #define HOST_NAME_MAX 64
    DWORD dwError = 0;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CSR pCSR = NULL;
    PVMCA_CERTIFICATE pCertificate = NULL;
    PSTR pszDefaultDomainName = NULL;
    PCSTR pszServer = "localhost";
    char hostName [HOST_NAME_MAX] = {0,};
    unsigned long dwMask = 0;
    time_t now = 0;
    time_t expire = 0;
    time_t UtcNow = 0;
    time_t UtcExpire = 0;
    PVMCA_SERVER_CONTEXT hServer = NULL;

#ifndef __NO_AFD__
    PVECS_STORE pStore = NULL;
#endif

//
// Step 1: Create a Public / Private Key Pair
// The Key Length is set to 1024, Password is set to NULL
//
    dwError = VMCACreatePrivateKey( NULL, CERTOOL_PRIVATE_KEY_LENGTH,
                                    &pPrivateKey, &pPublicKey);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to create key pair\n", dwError);
    }
    BAIL_ON_ERROR(dwError);
//
// Step 2: Create a Certificate Signing Request Object
//
    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to allocate CSR\n", dwError);
    }
    BAIL_ON_ERROR(dwError);
//
// Step 3: Populate all Fields -- There are two ways to do this
// One technique is illustrated here, and another one will be done
// as a comment for the user to understand different ways of doing it
//
// all cfgVariables are globals declared in certool.cpp
// where the values where parsed from a config file
//
#ifdef _WIN32
{
    WORD wVersionRequested = 0;
    WSADATA wsaData = {0};

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    dwError = WSAStartup(wVersionRequested, &wsaData);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to start winsock library\n", dwError);
    }
    BAIL_ON_ERROR(dwError);
}
#endif


    dwError = gethostname(hostName, HOST_NAME_MAX);
    if ( dwError != 0) {
      printf("gethostname failed, please check if host name is more \
        than 64 chars.\n");
    }
    BAIL_ON_ERROR(dwError);

#ifdef _WIN32
    WSACleanup();
#endif

    dwError = VmAfdGetDomainNameA(
                                  pszServer,
                                  &pszDefaultDomainName
                                 );
    if (dwError != 0)
    {
       printf("Warning: %d, Failed to get domain name\n", dwError);
       dwError = 0;
    }

    // Current in the cloud VM we don't wait for VMDIR
    // This would be fixed when we have one first boot.
    // Don't fail , this avoids the failure due the race condition.

    dwError = VMCAInitPKCS10DataWithDCA(
                              cfgName.c_str(),
                              pszDefaultDomainName,
                              hostName,
                              cfgOrgUnit.c_str(),
                              cfgState.c_str(),
                              cfgCountry.c_str(),
                              cfgEmail.c_str(),
                              cfgIPAddress.c_str(),
                              pCertReqData);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to create CSR\n", dwError);
    }
    BAIL_ON_ERROR(dwError);


    VMCASetBit(&dwMask, VMCA_KEY_CERT_SIGN);
    VMCASetBit(&dwMask, VMCA_KEY_CRL_SIGN);


    VMCASetKeyUsageConstraintsA(
        pCertReqData,
        dwMask );

//
// Step 5 :
// Since we are creating a Self Signed Certificate
// Here we are not making a Call to the Server.
// We generate a self signed Root CA Certificate and
// then push that certificate to the Server.
//
// Let us make the certificate valid for 10 years
// 365 days * 24 hours * 60 minutes * 60 seconds * 10
//

    time(&now);
    now = now - (argPredates * 60);
    expire = now + VMCA_DEFAULT_CA_CERT_VALIDITY;

    VMCAGetGMTime(now, &UtcNow);
    VMCAGetGMTime(expire, &UtcExpire);

    dwError =  VMCACreateSelfSignedCertificateA(
                 pCertReqData,
                 pPrivateKey,
                 NULL,
                 UtcNow,
                 UtcExpire,
                 &pCertificate);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to create root certificate\n", dwError);
    }
    BAIL_ON_ERROR(dwError);
//
// Step 6:
// Now that we have the Certificate and the Private Key,
// we should push these artifacts to the Server
//
    dwError = _HandleOpenServer(&hServer);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to open connection to server\n", dwError);
    }
    BAIL_ON_ERROR(dwError);

    /* RPC client call */
    dwError = VMCAAddRootCertificateHA(
                  hServer,
                  argServerName.c_str(),
                  pCertificate,
                  NULL,
                  pPrivateKey);
    if (dwError != 0)
    {
       printf("Error: %d, Failed to add root certificate\n", dwError);
    }
    BAIL_ON_ERROR(dwError);

//
// At this point the CA Server has a Root Certificate provisioned that
// allows the CA to start issuing Certs. VC might want to insert this
// Root CA Cert to the ESX and VC's Cert Store.
//

//
// Add VMCA Root Certificate to VECS Trusted Roots
// on the local host
#ifndef __NO_AFD__

    dwError = VecsCreateCertStoreA(
                NULL,
                TRUSTED_ROOTS_STORE_NAME,
                NULL,
                &pStore);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = VecsOpenCertStoreA(
                    NULL,
                    TRUSTED_ROOTS_STORE_NAME,
                    NULL,
                    &pStore);
    }
    if (dwError != 0)
    {
       printf("Error: %d, Failed to open certificate store\n", dwError);
    }
    BAIL_ON_ERROR(dwError);

    dwError =  VecsAddEntryA(
                   pStore,
                   CERT_ENTRY_TYPE_TRUSTED_CERT,
                   NULL,
                   pCertificate,
                   NULL, /* private key */
                   NULL, /* password */
                   1);   /* auto refresh */
    if (dwError != 0)
    {
       printf("Warning : %d, Adding Trusted Root to VECS Store Failed.\n", dwError);
       dwError = 0;
    }

#endif

error:
#ifndef __NO_AFD__
    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }
#endif
    VMCAFreeKey(pPrivateKey);
    VMCAFreeKey(pPublicKey);
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeCSR(pCSR);
    VMCAFreeCertificate(pCertificate);
    if (pszDefaultDomainName)
    {
        VMCAFreeStringA(pszDefaultDomainName);
    }
    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    return dwError;
}

DWORD
HandleRootCACertificate()
//
// HandleRoot CA Certificate reads a CA Cert  and a Private Key
// from a file, and uploads that to the VMware CA Server. This is
// another way of Initing the CA Server.
// The Steps involved here are
//   1) Read the Cert from the File
//   2) Read the Private Key from the File
//   3) Send Both these artifacts to the Server
{

    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    PVMCA_KEY pKey = NULL;
    PVMCA_SERVER_CONTEXT hServer = NULL;
//
// Step 1 : Read the Certificate
//
    dwError =  VMCAReadCertificateChainFromFile(
                                (LPSTR)argCert.c_str(),
                                &pCertificate
                                );
    BAIL_ON_ERROR(dwError);

//
// Step 2: Read the Private key from the File
// PLEASE NOTE : The support for password is not in the
// Command Tool, hence not supported here.
//

    dwError = VMCAReadPrivateKeyFromFile(
                  (LPSTR)argPrivateKey.c_str(),
                  NULL,
                  &pKey);
    BAIL_ON_ERROR(dwError);

//
//Step 3:
// Now that we have the Certificate and the Private Key,
// we should push these artifacts to the Server
//
    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    /* RPC client call */
    dwError = VMCAAddRootCertificateHA(
                  hServer,
                  argServerName.c_str(),
                  pCertificate,
                  NULL,
                  pKey);
    BAIL_ON_ERROR(dwError);



error :
    VMCAFreeKey(pKey);
    VMCAFreeCertificate(pCertificate);
    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    return dwError;
}

DWORD
HandleGenCert()
// HandleGenCert creates a CSR sends it to the Server
// and gets a singed certificate back.
// Here are the Steps in this algorithm
// 1) Create a PKCS10 request object
// 2) Populate it with values
// 3) Reads the  Private Key
// 4) Create a CSR
// 5) Send it to VMware certificate server to get signature
// 6) Write the certificate back to the file.
{
    DWORD dwError = 0;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_KEY pKey = NULL;
    PVMCA_CSR pCSR = NULL;
    DWORD dwCertLength = 0;
    PVMCA_CERTIFICATE pCertificate = NULL;
    unsigned long dwMask = 0;
    time_t startFrom;
    PVMCA_SERVER_CONTEXT hServer = NULL;
    time_t timeUtcNow = 0, timeUtcExpire = 0;

    //
    // Step 1: Create a Certificate Signing Request Object
    //
    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    BAIL_ON_ERROR(dwError);

    dwError = UpdateReqData(pCertReqData);
    BAIL_ON_ERROR(dwError);

    /* Current behaviour for VMCA Clients*/
    VMCASetBit(&dwMask, VMCA_DIGITAL_SIGNATURE);
    VMCASetBit(&dwMask, VMCA_NON_REPUDIATION);
    VMCASetBit(&dwMask, VMCA_KEY_ENCIPHERMENT);

    dwError = VMCASetKeyUsageConstraintsA(
        pCertReqData,
        dwMask  );
    BAIL_ON_ERROR(dwError);

    //
    // Step 3 : Read the Private key from the File
    // PLEASE NOTE : The support for password is not in the
    // Command Tool, hence not supported here yet.
    //
    dwError = VMCAReadPrivateKeyFromFile(
                  (LPSTR)argPrivateKey.c_str(),
                  NULL,
                  &pKey);
    BAIL_ON_ERROR(dwError);

  //
  // Step 4: Create a Signing Request CSR ( PKCS10)
  //
    dwError =  VMCACreateSigningRequestA(
                   pCertReqData,
                   pKey,
                   NULL,
                   &pCSR);
    BAIL_ON_ERROR(dwError);

  //
  // Let us make the certificate valid for 6 months
  // if it is not a valid time
  // 180 days * 24 hours * 60 minutes * 60 seconds
  //
    if ( now == 0 )
    {
      time(&now);
    }
    startFrom = now - VMCA_CERT_EXPIRY_START_LAG;
      /* The certificate is predated by 10
       * minutes to avoid time-sync issues
       */
    expire = startFrom + VMCA_DEFAULT_CERT_VALIDITY;

    VMCAGetGMTime(startFrom, &timeUtcNow);
    VMCAGetGMTime(expire, &timeUtcExpire);

  //
  // Step 5: Send the CSR to server to get it signed
  //
    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    /* RPC client call */
    dwError =  VMCAGetSignedCertificateFromCSRHA(
                   hServer,
                   argServerName.c_str(),
                   pCSR,
                   timeUtcNow,
                   timeUtcExpire,
                   &pCertificate);
    BAIL_ON_ERROR(dwError);

  //
  // Step 6 : Write the Certificate to a file
  // In the VC / ESX world you will write this to
  // the certificate store, that will also have the
  // VMCA Root Certificate that will allow you to verify
  // the trust.
  //

  dwError =  VMCAWriteCertificateChainToFile(
                       (LPSTR) argCert.c_str(),
                       pCertificate);

    BAIL_ON_ERROR(dwError);


error :
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeKey(pKey);
    VMCAFreeCSR(pCSR);
    VMCAFreeCertificate(pCertificate);
    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    return dwError;
}

DWORD
HandleGenKey()
//
// This function generates a Private / Public Key pair and
// write it down to a file.
//
// Here are the steps
//  1) Create Key Pair
//  2) Write to files
//
{
    DWORD dwError = 0;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
//
// Step 1: Create a Public / Private Key Pair
// The Key Length is set to 1024, Password is set to NULL
//
    dwError = VMCACreatePrivateKey( NULL, CERTOOL_PRIVATE_KEY_LENGTH, &pPrivateKey, &pPublicKey );
    BAIL_ON_ERROR(dwError);
//
// Step 2.1 Write Private key to a file
//
    dwError =  VMCAWritePrivateKeyToFile(
                   (LPSTR) argPrivateKey.c_str(),
                   (LPSTR) pPrivateKey,
                   NULL,
                   NULL);
    BAIL_ON_ERROR(dwError);

//
// Step 2.2 Write Public Key to File
//
    dwError = VMCAWritePublicKeyToFile(
                  (LPSTR) argPublicKey.c_str(),
                  (LPSTR) pPublicKey
              );
    BAIL_ON_ERROR(dwError);

error:
    VMCAFreeKey(pPublicKey);
    VMCAFreeKey(pPrivateKey);

    return dwError;
}


DWORD
HandleRevokeCert()
// This function calls into VMCA and revokes a given certificate
// Here is the Algorithm to do that
//  1) Read the Certificate from a file
//  2) Send it across to Server asking it to be revoked
//
{
    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCert = NULL;
    PVMCA_SERVER_CONTEXT hServer = NULL;
//
// Step 1: Read the Certificate from File
//
    dwError = VMCAReadCertificateFromFile(
                  (LPSTR)argCert.c_str(),
                  &pCert);
    BAIL_ON_ERROR(dwError);
//
// Step 2: Call into Server to revoke the Certificate
//
    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    dwError = VMCARevokeCertA(
                  hServer,
                  (PCSTR) argServerName.c_str(),
                  (PSTR) pCert,
                  VMCA_CRL_REASON_UNSPECIFIED
                  );
    BAIL_ON_ERROR (dwError);

error :
    VMCAFreeCertificate(pCert);
    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    return dwError;
}




DWORD
HandleViewCert()
// This function demonstrates the use of
// GetCertificate as a String.
// 1) Read the Certificate
// 2) Ask VMCA to convert it into a printable String
//
{

    DWORD dwError = 0;
    PSTR pszCertString = NULL;
    PVMCA_CERTIFICATE pCert = NULL;
//
//Step 1: read the Certificate
//
    dwError = VMCAReadCertificateFromFile(
                  (LPSTR) argCert.c_str(),
                  &pCert);
    BAIL_ON_ERROR(dwError);
//
// Step 2 : Ask VMCA to convert to a printable string
//
    dwError = VMCAGetCertificateAsStringA(
                  (LPSTR) pCert,
                  &pszCertString);
    BAIL_ON_ERROR(dwError);
//
// Print it
//
    std::cout << pszCertString << std::endl;

error :
    if(pszCertString) {
        VMCA_SAFE_FREE_STRINGA(pszCertString);
    }
    VMCAFreeCertificate(pCert);

    return dwError;
}

DWORD
HandleGetRootCA()
// This function allows the users to retrive the
// Root CA Certificate that is being used for
// Signing all certificates. This certificate
// has to be trusted by all clients and generally
// this certificate will be inserted into the certificate
// store of ESX or VC.
// Algo :
// 1) Call into VMCA server and ask for Root CA Certitificate
// 2) In this case we are just printing it out to screen, but we might
// consider just inserting this to a certificate store.

{

    DWORD dwError = 0;
    DWORD dwCertLength = 0;
    PVMCA_CERTIFICATE pCert = NULL;
    PSTR pszCertString = NULL;
    PVMCA_SERVER_CONTEXT hServer = NULL;
//
// Step 1: Get the Root CA Certificate from the Server
//
    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    /* RPC client call */
    dwError = VMCAGetRootCACertificateHA(
                  hServer,
                  (PCSTR) argServerName.c_str(),
                  &pCert);
    BAIL_ON_ERROR(dwError);

    if (argCert.length() > 0)
    {

      dwError = VMCAWriteCertificateChainToFile((PSTR)argCert.c_str(),pCert);
      BAIL_ON_ERROR(dwError);
    }
//
// Convert and Print the Root CA Certificate
//
    dwError = VMCAGetCertificateAsStringA(
                  (LPSTR) pCert,
                  &pszCertString);
    BAIL_ON_ERROR(dwError);

    std::cout << pszCertString << std::endl;
    if (argCert.length() > 0) {
      std::cout << "Certificate written to file : " << argCert <<std::endl;
    }

error:
    VMCAFreeCertificate(pCert);
    if (pszCertString)
    {
        VMCA_SAFE_FREE_STRINGA(pszCertString);
    }

    if (hServer)
    {
        VMCACloseServer(hServer);
    }
    return dwError;
}


DWORD
HandleVersionRequest()
//
// This function returns a Server Version String
//
{
    DWORD dwError = 0;
    PSTR pServerVersion = NULL;
    PVMCA_SERVER_CONTEXT hServer = NULL;

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);
//
// Call into VMCA Server to get Version Number
//
    dwError =  VMCAGetServerVersionHA(
                    hServer,
                   (PCSTR) argServerName.c_str(),
                   &pServerVersion
               );
    BAIL_ON_ERROR(dwError);
//
// Print
//
    std::cout << pServerVersion << std::endl;

error:
    VMCA_SAFE_FREE_STRINGA(pServerVersion);

    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    return dwError;
}

DWORD
HandleEnumCerts()
//
// This function enumerate certificates
// 1) open enum context list
// 2) Print the certficate until done
//
{
    DWORD dwError = 0;
    DWORD dwCertLength = 0;
    PVOID pContext = NULL;
    PVMCA_CERTIFICATE pCertificate = NULL;
    PSTR pszCertString = NULL;
    int currIndex = 0;
    VMCA_ENUM_CERT_RETURN_CODE enumStatus = VMCA_ENUM_ERROR;
    VMCA_CERTIFICATE_STATUS dwStatus = VMCA_CERTIFICATE_ALL;
    PVMCA_SERVER_CONTEXT hServer = NULL;

    if ( std::strcmp(argFilter.c_str(), "active") == 0) {
        dwStatus = VMCA_CERTIFICATE_ACTIVE;
    }
    else if ( std::strcmp(argFilter.c_str(), "revoked") == 0) {
        dwStatus = VMCA_CERTIFICATE_REVOKED;
    }
    else if ( std::strcmp(argFilter.c_str(), "expired") == 0) {
        dwStatus = VMCA_CERTIFICATE_EXPIRED;
        std::cout << "This flag is not currently supported" << std::endl;
        dwError  = ERROR_NOT_SUPPORTED;
        BAIL_ON_ERROR (dwError);
    }
    else if ( std::strcmp(argFilter.c_str(), "all") == 0) {
        dwStatus = VMCA_CERTIFICATE_ALL;
    }
    else {
        goto error;
    }

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAOpenEnumContextHA(
                  hServer,
                  (PSTR)argServerName.c_str(),
                  dwStatus,
                  &pContext
              );
    BAIL_ON_ERROR(dwError);

    do {
        dwError = VMCAGetNextCertificate(
                      pContext,
                      &pCertificate,
                      &currIndex,
                      &enumStatus
                  );
        if (enumStatus == VMCA_ENUM_END)
        {
            break;
        }

        BAIL_ON_ERROR(dwError);
        std::cout << "Cert \n" << pCertificate << "\n";

        std::cerr << currIndex << std::endl;
        dwError = VMCAGetCertificateAsStringA(
                      (LPSTR) pCertificate,
                      &pszCertString);
        BAIL_ON_ERROR(dwError);

        std::cout << pszCertString << std::endl;

        VMCAFreeStringA(pszCertString);
        VMCAFreeCertificate(pCertificate);
        pszCertString = NULL;
        pCertificate = NULL;
    } while( enumStatus != VMCA_ENUM_END);


error :
    if (pContext) {
        VMCACloseEnumContext(pContext);
    }
    if (hServer) {
      VMCACloseServer(hServer);
    }
    if ( pCertificate) {
        VMCAFreeCertificate(pCertificate);
    }

    return dwError;
}

DWORD
HandleGetDC()
{
    DWORD dwError = 0;
    PSTR pDomainName = NULL;
    dwError = VMCAGetDefaultDomainName(
                  (PSTR)(argServerName.length() > 0 ? argServerName.c_str(): NULL),
                  argPort,
                  &pDomainName);
    BAIL_ON_ERROR(dwError);
    std::cout << "Default Domain Name : " << pDomainName << std::endl;

error :
    VMCA_SAFE_FREE_STRINGA(pDomainName);
    return dwError;
}


#define BUF_SIZE 1024
DWORD
HandleGenCISCert()
{
    DWORD dwError = 0;
    char CommonName [BUF_SIZE] = { 0 };
    PSTR pszDomainName = NULL;
    PSTR pszOrgUnit    = NULL;
    PSTR pszMachineID  = NULL;
    PCSTR format = "%s@%s";
    int ret = 0;
    PSTR pszCert = NULL;
    PSTR pszPrivateKey = NULL;
    PCSTR pszLocalHost = "localhost";
    std::string vecsAlias(cfgName);
#ifndef __NO_AFD__
    PVECS_STORE pStore = NULL;
#endif

    dwError = VmAfdGetMachineIDA(
                                 pszLocalHost,
                                 &pszMachineID
                                );
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocateStringPrintfA(
                                        &pszOrgUnit,
                                        "mID-%s",
                                        pszMachineID
                                       );
    BAIL_ON_ERROR (dwError);

    cfgOrgUnit.assign(pszOrgUnit);

    if(argFQDN.length() >0)
    {
        cfgHostName.assign(argFQDN);
    }

    // CIS Cert is a Standard Form of Certs that we want to issue
    // for CIS Components in VMWare.
    // The Format is as follows
    // CN =<name>, OU = mID-<machineID>, DC=<DomainComponents>
    dwError = VmAfdGetDomainNameA(pszLocalHost, &pszDomainName);
    BAIL_ON_ERROR(dwError);

    cfgDomainComponent.assign(pszDomainName);
    if(cfgCountry.length() == 0 ) {
        cfgCountry.assign("US"); //default to US
    }

    dwError = HandleGenCert();
    BAIL_ON_ERROR(dwError);

    // Add Cert to VECS Store
    // By definition a VECS store has to be on a Local Machine.
    // However Certool might run on machines where there is
    // No VECS, so we ignore failure, and continue.

    dwError = VMCAReadCertificateFromFile((PSTR)argCert.c_str(), &pszCert);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAReadPrivateKeyFromFile((PSTR)argPrivateKey.c_str(),
        NULL,  &pszPrivateKey);
    BAIL_ON_ERROR(dwError);

#ifndef __NO_AFD__
    dwError = VecsCreateCertStoreA(
                        NULL,
                        vecsAlias.c_str(),
                        NULL,
                        &pStore);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = VecsOpenCertStoreA(
                        NULL,
                        vecsAlias.c_str(),
                        NULL,
                        &pStore);
    }
    BAIL_ON_ERROR(dwError);

    dwError =  VecsAddEntryA(
                        pStore,
                        CERT_ENTRY_TYPE_PRIVATE_KEY,
                        vecsAlias.c_str(),
                        pszCert,
                        pszPrivateKey,
                        NULL, /* password */
                        1);   /* auto refresh */
    if (dwError != 0)
    {
        printf("Warning : %d, Adding cert to VECS Store Failed.", dwError);
        dwError = 0;
    }

#endif

error :
    if(pszDomainName)
    {
        VmAfdFreeString(pszDomainName);
    }

    VMCA_SAFE_FREE_STRINGA(pszMachineID);
    VMCA_SAFE_FREE_STRINGA(pszOrgUnit);

#ifndef __NO_AFD__
    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }
#endif
    if( pszCert != NULL) {
        VMCA_SAFE_FREE_STRINGA(pszCert);
    }

    if (pszPrivateKey != NULL) {
        VMCA_SAFE_FREE_STRINGA(pszPrivateKey);
    }

    return dwError;
}


DWORD
HandleWaitVMCA()
{
    DWORD dwError = 0;
    int secondsToSleep = 0;
    time_t  endTime, currentTime;
    PVMCA_CERTIFICATE pCert = NULL;
    PVMCA_SERVER_CONTEXT hServer = NULL;

    if ( argWait <= 0 )
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    time(&currentTime);

    endTime = currentTime + ( argWait * 60 );
    while ( currentTime < endTime ) {

        // Call into VMCA Server to get Version Number
        dwError = _HandleOpenServer(&hServer);

        if (dwError == ERROR_SUCCESS)
        {
            dwError = VMCAGetRootCACertificateHA(
                hServer,
                (PCSTR) argServerName.c_str(),
                &pCert
                );

            VMCACloseServer(hServer);

            if ( dwError == 0 )
            {
                // means server is up and working
                break;
            }
        }
        sleep(GetSleepTime(10)); // Sleep for 10 seconds
        printf(".");
        fflush(stdout);
        time(&currentTime);
    }

    if ((currentTime >= endTime) &&
        (dwError != 0))
    {
        dwError = VMCA_ERROR_TIME_OUT;
        BAIL_ON_ERROR(dwError);
    }

error:
    if (pCert != NULL ) {
        VMCAFreeCertificate(pCert);
    }

    return dwError;

}

DWORD
HandleWaitVMDIR()
{
  DWORD dwError = 0;
  int secondsToSleep = 0;
  time_t  endTime, currentTime;

  if ( argWait <= 0 )
  {
    dwError = VMCA_ARGUMENT_ERROR;
    BAIL_ON_ERROR(dwError);
  }

  time(&currentTime);

  endTime = currentTime + ( argWait * 60 );
  while ( currentTime < endTime ) {

// Call into VMDIR Server to make sure that it is up and running.

    dwError =  VMCACheckLdapConnection(
                   (PSTR) argServerName.c_str(),argPort);
    if ( dwError == 0 )
    {
      // means server is up and working
      break;
    }
    sleep(GetSleepTime(10)); // Sleep for 10 seconds
    printf(".");
    fflush(stdout);
    time(&currentTime);
  }

  if ((currentTime >= endTime) &&
      (dwError != 0))
  {
    dwError = VMCA_ERROR_TIME_OUT;
    BAIL_ON_ERROR(dwError);
  }

error:
  return dwError;
}


DWORD
HandleInitVMCA()
{

  DWORD dwError = 0;
  int secondsToSleep = 0;
  time_t  endTime, currentTime;
  PSTR pVersionString = NULL;
  PVMCA_SERVER_CONTEXT hServer = NULL;

  if ( argWait <= 0 )
  {
    dwError = VMCA_ARGUMENT_ERROR;
    BAIL_ON_ERROR(dwError);
  }

  time(&currentTime);

  endTime = currentTime + ( argWait * 60 );
  while ( currentTime < endTime ) {

 // Call into VMCA Server to get Version Number

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAGetServerVersionHA(
                  hServer,
                  (PCSTR) argServerName.c_str(),
                  &pVersionString
              );

    VMCACloseServer(hServer);
    hServer = NULL;

    if ( dwError == 0 )
    {
      // means server is up and working
      break;
    }
    sleep(GetSleepTime(10)); // Sleep for 10 seconds
    printf(".");
    fflush(stdout);
    time(&currentTime);
  }

  if ((currentTime >= endTime) &&
      (dwError != 0))
  {
    dwError = VMCA_ERROR_TIME_OUT;
    BAIL_ON_ERROR(dwError);
  }

error:
  if (pVersionString != NULL ) {
    VMCAFreeStringA(pVersionString);
  }

  return dwError;

}




DWORD
HandleStatusCert()
{

    DWORD dwError = 0;
    PVMCA_CERTIFICATE pCert = NULL;
    DWORD dwStatus = VMCA_CERTIFICATE_ALL;
    PVMCA_SERVER_CONTEXT hServer = NULL;
//
// Step 1: Read the Certificate from File
//
    dwError = VMCAReadCertificateFromFile(
                  (LPSTR)argCert.c_str(),
                  &pCert);
    BAIL_ON_ERROR(dwError);
//
// Step 2: Call into Server to verify  the Status of the Certificate
//

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAVerifyCertificateHA(
                   hServer,
                  (PCSTR) argServerName.c_str(),
                  (PSTR) pCert,
                  &dwStatus);
    BAIL_ON_ERROR(dwError);

    switch (dwStatus)
    {
        case VMCA_CERTIFICATE_ACTIVE:
          std::cout << "Certificate : ACTIVE" << std::endl;
          break;

        case VMCA_CERTIFICATE_REVOKED:
          std::cout << "Certificate : REVOKED" << std::endl;
          break;

        case VMCA_CERTIFICATE_EXPIRED:
          std::cout << "Certificate : EXPIRED" << std::endl;
          break;

        default:
          std::cout << "Certificate : INVALID_STATE" << std::endl;
    }

error :
    if (hServer)
    {
        VMCACloseServer(hServer);
    }

    VMCAFreeCertificate(pCert);
    return dwError;
}

DWORD
HandleVecsEnum()
{
    DWORD dwError = 0;

    dwError = VMCA_NOT_IMPLEMENTED;

    return dwError;
}

DWORD
HandleGetCRL()
{
  DWORD dwError = 0;
  PVMCA_SERVER_CONTEXT hServer = NULL;

  dwError = _HandleOpenServer(&hServer);
  BAIL_ON_ERROR(dwError);

  dwError = VMCAGetCRLHA(
    hServer,
    (PCSTR)argServerName.c_str(),
    (PSTR) argCurrCrl.c_str(),
    (PSTR) argCrl.c_str()
    );

error:
  if (hServer)
  {
      VMCACloseServer(hServer);
  }
  return dwError;
}


DWORD
HandleGenCRL()
{
  DWORD dwError = 0;
  PVMCA_SERVER_CONTEXT hServer = NULL;

  dwError = _HandleOpenServer(&hServer);
  BAIL_ON_ERROR(dwError);

  dwError = VMCAReGenCRLHA(
    hServer,
    (PCSTR)argServerName.c_str()
    );

error:
  if (hServer)
  {
      VMCACloseServer(hServer);
  }
  return dwError;
}


DWORD
HandleCRLInfo()
{
  DWORD dwError = 0;
  time_t dwLastUpdate = 0;
  time_t dwNextUpdate = 0;
  DWORD dwCrlNumber = 0;
  time_t t = 0;

  dwError =  VMCAGetCRLInfo2(
              (PSTR) argCrl.c_str(),
              &dwLastUpdate,
              &dwNextUpdate,
              &dwCrlNumber);
  BAIL_ON_ERROR(dwError);
  t = (time_t) dwLastUpdate;
  std::cout  << "CRL Name    : " << argCrl << std::endl;
  std::cout  << "Last Update : "  <<  asctime(gmtime(&t));
  t = (time_t) dwNextUpdate;
  std::cout  << "Next Update : "  <<  asctime(gmtime(&t));
  std::cout  << "CRL Number  : " << dwCrlNumber << std::endl;


error :
  return dwError;
}

DWORD
HandlePrintError()
{

    DWORD dwError = 0;
    PSTR pszShortErrMessage = NULL;
    PSTR pszLongMessage = NULL;
    dwError  = VMCAGetShortError(argErr, &pszShortErrMessage);
    BAIL_ON_ERROR(dwError);
    std::cout << "Short Error : " << pszShortErrMessage << std::endl;

    dwError = VMCAGetErrorString(argErr, &pszLongMessage);
    BAIL_ON_ERROR(dwError);
    std::cout << pszLongMessage << std::endl;

error :
    VMCA_SAFE_FREE_STRINGA(pszShortErrMessage);
    VMCA_SAFE_FREE_STRINGA(pszLongMessage);
    return dwError;
}


DWORD
HandlePrintCRL()
{
  DWORD dwError = 0;
  PSTR pszCRLString = NULL;
  dwError = VMCAPrintCRL((PSTR)argCrl.c_str(), &pszCRLString);
  BAIL_ON_ERROR(dwError);

  std::cout << "CRL Info : " << std::endl << pszCRLString <<std::endl;

error :
  VMCA_SAFE_FREE_STRINGA(pszCRLString);
  return dwError;
}


DWORD
HandlePublishRoots()
{
  DWORD dwError = 0;
  PVMCA_SERVER_CONTEXT hServer = NULL;

  dwError = _HandleOpenServer(&hServer);
  BAIL_ON_ERROR(dwError);

  dwError = VMCAPublishRootCertsHA(hServer, (LPSTR) argServerName.c_str());
  BAIL_ON_ERROR(dwError);

error:
  if (hServer)
  {
      VMCACloseServer(hServer);
  }
  return dwError;
}

DWORD
HandleUpdateSchema()
{
    DWORD dwError = 0;
    //TODO: Implement schema update. Placeholder for upgrade tests
    return dwError;
}

DWORD
HandleGenSelfCert()
{
    DWORD dwError = 0;

    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPublicKey = NULL;
    PVMCA_PKCS_10_REQ_DATAA pCertReqData = NULL;
    PVMCA_CERTIFICATE pCertificate = NULL;
    unsigned long dwMask = 0;
    time_t timeNow = 0, timeExpire = 0;
    time_t timeUtcNow = 0, timeUtcExpire = 0;

    dwError = VMCACreatePrivateKey(
                    NULL,
                    CERTOOL_PRIVATE_KEY_LENGTH,
                    &pPrivateKey,
                    &pPublicKey
                    );
    BAIL_ON_ERROR(dwError);

    dwError = VMCAAllocatePKCS10DataA(&pCertReqData);
    BAIL_ON_ERROR(dwError);

    dwError = VMCAInitPKCS10DataWithDCA(
                    cfgName.c_str(),
                    cfgDomainComponent.c_str(),
                    cfgOrganization.c_str(),
                    cfgOrgUnit.c_str(),
                    cfgState.c_str(),
                    cfgCountry.c_str(),
                    cfgEmail.c_str(),
                    cfgIPAddress.c_str(),
                    pCertReqData
                    );
    BAIL_ON_ERROR(dwError);

    if(cfgLocality.length() > 0)
    {
        dwError = VMCASetCertValueA(
                        VMCA_OID_LOCALITY,
                        pCertReqData,
                        (PSTR)cfgLocality.c_str()
                        );
        BAIL_ON_ERROR(dwError);
    }

    if(cfgHostName.length() > 0)
    {
        dwError = VMCASetCertValueA(
                        VMCA_OID_DNS,
                        pCertReqData,
                        (PSTR)cfgHostName.c_str()
                        );
        BAIL_ON_ERROR(dwError);
    }

    VMCASetBit(&dwMask, VMCA_KEY_CERT_SIGN);
    VMCASetBit(&dwMask, VMCA_KEY_CRL_SIGN);

    VMCASetKeyUsageConstraintsA(
                    pCertReqData,
                    dwMask
                    );

    time(&timeNow);
    timeNow -= (argPredates * 60);
    timeExpire = timeNow + VMCA_DEFAULT_CA_CERT_VALIDITY;

    VMCAGetGMTime(timeNow, &timeUtcNow);
    VMCAGetGMTime(timeExpire, &timeUtcExpire);

    dwError =  VMCACreateSelfSignedCertificateA(
                     pCertReqData,
                     pPrivateKey,
                     NULL,
                     timeUtcNow,
                     timeUtcExpire,
                     &pCertificate
                     );
    BAIL_ON_ERROR(dwError);

    dwError =  VMCAWriteCertificateToFile(
                    (LPSTR) argOutCert.c_str(),
                    pCertificate
                    );
    BAIL_ON_ERROR(dwError);

    dwError =  VMCAWritePrivateKeyToFile(
                    (LPSTR) argOutPrivateKey.c_str(),
                    pPrivateKey,
                    NULL,
                    NULL
                    );
    BAIL_ON_ERROR(dwError);

error:
    VMCAFreeKey(pPrivateKey);
    VMCAFreeKey(pPublicKey);
    VMCAFreePKCS10DataA(pCertReqData);
    VMCAFreeCertificate(pCertificate);

    return dwError;
}

DWORD
HandleGenCSRFromCert()
{
    DWORD dwError = 0;

    EVP_PKEY *pPrivateKey = NULL;
    X509_REQ *pCertReq = NULL;
    X509 *pCertificate = NULL;

    dwError = VMCAReadX509FromFile(
                        (LPSTR) argCert.c_str(),
                        &pCertificate
                        );
    BAIL_ON_ERROR (dwError);

    dwError = VMCAReadPKEYFromFile(
                        (LPSTR) argPrivateKey.c_str(),
                        NULL,
                        &pPrivateKey
                        );
    BAIL_ON_ERROR (dwError);

    dwError = VMCAGetCSRFromCert(
                          pCertificate,
                          pPrivateKey,
                          &pCertReq
                          );
    BAIL_ON_ERROR (dwError);

    dwError = VMCACSRToFile(
                       pCertReq,
                       (LPSTR) argCsr.c_str()
                       );
    BAIL_ON_ERROR (dwError);

cleanup:
    X509_free(pCertificate);
    X509_REQ_free(pCertReq);
    EVP_PKEY_free(pPrivateKey);
    return dwError;

error:
    goto cleanup;

}

DWORD
HandleSetServerOption()
{
    DWORD   dwError = 0;
    unsigned int    dwOption = 0;
    PVMCA_SERVER_CONTEXT    hServer = NULL;

    if (std::strcmp(argOption.c_str(), VMCA_OPTION_MULTIPLE_SAN) == 0) {
        dwOption = VMCA_SERVER_OPT_ALLOW_MULTIPLE_SAN;
    }
    else
    {
        std::cerr << "Unrecognizable server option: " << argOption << std::endl;
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    // Call into VMCA Server to set server option
    dwError =  VMCASetServerOptionHA(
            hServer, (PCSTR)argServerName.c_str(), dwOption);
    BAIL_ON_ERROR(dwError);

error:
    if (hServer)
    {
        VMCACloseServer(hServer);
    }
    return dwError;
}

DWORD
HandleUnsetServerOption()
{
    DWORD   dwError = 0;
    unsigned int    dwOption = 0;
    PVMCA_SERVER_CONTEXT    hServer = NULL;

    if (std::strcmp(argOption.c_str(), VMCA_OPTION_MULTIPLE_SAN) == 0) {
        dwOption = VMCA_SERVER_OPT_ALLOW_MULTIPLE_SAN;
    }
    else
    {
        std::cerr << "Unrecognizable server option: " << argOption << std::endl;
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
    }

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    // Call into VMCA Server to unset server option
    dwError =  VMCAUnsetServerOptionHA(
            hServer, (PCSTR)argServerName.c_str(), dwOption);
    BAIL_ON_ERROR(dwError);

error:
    if (hServer)
    {
        VMCACloseServer(hServer);
    }
    return dwError;
}

DWORD
HandleGetServerOption()
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    unsigned int    dwOption = 0;
    PVMCA_SERVER_CONTEXT    hServer = NULL;

    dwError = _HandleOpenServer(&hServer);
    BAIL_ON_ERROR(dwError);

    // Call into VMCA Server to get server option
    dwError =  VMCAGetServerOptionHA(
            hServer, (PCSTR)argServerName.c_str(), &dwOption);
    BAIL_ON_ERROR(dwError);

    // Print enabled options
    for (i = 1; i < VMCA_SERVER_OPT_COUNT; i <<= 1)
    {
        if (dwOption & i)
        {
            switch (i)
            {
            case VMCA_SERVER_OPT_ALLOW_MULTIPLE_SAN:
                std::cout << VMCA_OPTION_MULTIPLE_SAN << std::endl;
                break;

            default:
                break;
            }
        }
    }

error:
    if (hServer)
    {
        VMCACloseServer(hServer);
    }
    return dwError;
}
