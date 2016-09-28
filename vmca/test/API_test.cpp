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


BOOST_AUTO_TEST_SUITE(API_TEST)

BOOST_AUTO_TEST_CASE(NEG_CreateCSR_WITH_JUNK_KEY)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("NEG_CreateCSR_WITH_JUNK_KEY","1.1");
    VMCAData Data;
    DWORD dwError = 0;
    PVMCA_CSR csr = NULL;
    PVMCA_KEY key = "Junkjunk";

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();


    dwError = VMCACreateSigningRequest(Data.GetDataReq(),key,NULL, &csr);
    BOOST_REQUIRE(  (dwError  != 0) &&
                    (csr == NULL));

}

BOOST_AUTO_TEST_CASE(NEG_CreateCSR_WITH_INVALID_POINTER)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("NEG_CreateCSR_WITH_INVALID_POINTER","1.1");
    VMCAData Data;
    DWORD dwError = 0;
    PVMCA_CSR csr = NULL;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    dwError = Data.CreateKeyPair(1024);
    BOOST_REQUIRE( (dwError == 0) &&
                    (Data.GetPrivatKey() != NULL) &&
                    (Data.GetPublicKey() != NULL));



    dwError = VMCACreateSigningRequest(Data.GetDataReq(),Data.GetPrivatKey(),NULL, NULL);
    BOOST_REQUIRE(  (dwError  != 0) &&
                    (csr == NULL));

    dwError = VMCACreateSigningRequest(NULL,Data.GetPrivatKey(),NULL, &csr);
    BOOST_REQUIRE(  (dwError  != 0) &&
                    (csr == NULL));


}


BOOST_AUTO_TEST_CASE(NEG_CreatePrivate_Key)
/* This test case reads VMCAUnitTest.Cfg and creates a
Self Signed Certificate.
*/
{
    AutoProfile Perf("NEG_CreatePrivate_Key","1.1.3");
    VMCAData Data;
    DWORD dwError = 0;
    PVMCA_KEY pPrivateKey = NULL;
    PVMCA_KEY pPubliCkey = NULL;

    BOOST_CHECKPOINT("Call : InitCSRField");
    Data.InitCSRFields();

    BOOST_CHECKPOINT("Call  Key == 1: VMCACreatePrivateKey");

    // Will fail since we don't create Keys with less than 1024 Length
    dwError =  VMCACreatePrivateKey(NULL,1, &pPrivateKey, &pPubliCkey);
    BOOST_REQUIRE(dwError != 0);

     BOOST_CHECKPOINT("Call  Key == 17 KB : VMCACreatePrivateKey");

    // Will fail since we don't create Keys with greater than 16K Length
    dwError =  VMCACreatePrivateKey(NULL,17*1024, &pPrivateKey, &pPubliCkey);
    BOOST_REQUIRE(dwError != 0);


    BOOST_CHECKPOINT("Call  PrivateKey == NULL : VMCACreatePrivateKey");

    // Will fail since Private Key is NULL
    dwError =  VMCACreatePrivateKey(NULL,1, NULL, &pPubliCkey);
    BOOST_REQUIRE(dwError != 0);


    BOOST_CHECKPOINT("Call  Public Key == NULL : VMCACreatePrivateKey");

    // Will fail since Public  Key is NULL
    dwError =  VMCACreatePrivateKey(NULL,1, &pPrivateKey, NULL);
    BOOST_REQUIRE(dwError != 0);


}


BOOST_AUTO_TEST_CASE(NEG_AddRootCertificate_Key)
{

AutoProfile Perf("NEG_AddRootCertificate_Key","1.1.3");
DWORD dwError = 0;
char *Non_CA_Cert = "\
-----BEGIN CERTIFICATE-----\
MIICqjCCAhOgAwIBAgIRAO635LBkSG37dOHfkEgVqjkwDQYJKoZIhvcNAQELBQAw\
TzELMAkGA1UEBhMCVVMxDzANBgNVBAoTBlZNd2FyZTEbMBkGA1UECxMSVk13YXJl\
IEVuZ2luZWVyaW5nMRIwEAYDVQQDEwlDZXJUb29sQ0EwHhcNMTIwOTEzMjIyNzIx\
WhcNMTMwOTEzMjIyNzIxWjBRMQswCQYDVQQGEwJVUzEPMA0GA1UEChMGVk13YXJl\
MRswGQYDVQQLExJWTXdhcmUgRW5naW5lZXJpbmcxFDASBgNVBAMTC3Rlc3RjZXJ0\
NDgyMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDW5iaZPnHj/RbA1vMC8TDb\
VPkQ8rFgcE5sfjZthbtcl4BSPzOVc5psOwCgOk87EMPvmowj9tJoMrR6oN5dK9BQ\
NjJz6e2k0EjD83lqUHC5vC0rGhTVcW+gP4XWGkEH6rDhyhU3MRzmlM9zzZzHe4K+\
Eubult47gXbkIuF7iHYotwIDAQABo4GDMIGAMAwGA1UdEwEB/wQCMAAwDgYDVR0P\
AQH/BAQDAgTwMB8GA1UdIwQYMBaAFC97AyA5oUTxHRK5E3qIRYZXmbqGMB0GA1Ud\
DgQWBBQqZwOjiovsW/WerVNx4qBNWxc9vzAgBgNVHREEGTAXgQ92bWNhQHZtd2Fy\
ZS5jb22HBH8AAAEwDQYJKoZIhvcNAQELBQADgYEAgBMGHkcjWtF4KGJTSLqWwKxE\
nL07odigwlFPHw/mISf10olhOTHY29e5dMTjNNcpYQUtLxho+1XYBkpknKdpG83y\
QlGBOBeLVi7SfWpN50ivpN4cfXYduzP7G0+0G2LdrqNwYJGrCRcmriXV4Qv/m7Kx\
zLMm/mTNkEYENz8URB8=\
-----END CERTIFICATE----- ";

char *RealCACert = "\
-----BEGIN CERTIFICATE-----\
MIICizCCAfSgAwIBAgIRAKffvGi8bWr/kLivsDCzP1QwDQYJKoZIhvcNAQELBQAw\
TzELMAkGA1UEBhMCVVMxDzANBgNVBAoTBlZNd2FyZTEbMBkGA1UECxMSVk13YXJl\
IEVuZ2luZWVyaW5nMRIwEAYDVQQDEwlDZXJUb29sQ0EwHhcNMTIwOTEzMjIyNjI5\
WhcNMTMwOTEzMjIyNjI5WjBPMQswCQYDVQQGEwJVUzEPMA0GA1UEChMGVk13YXJl\
MRswGQYDVQQLExJWTXdhcmUgRW5naW5lZXJpbmcxEjAQBgNVBAMTCUNlclRvb2xD\
QTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAuiyN/2YBLLM7r7v2/ka4P1Wt\
qphH5UaYzZ78eiTDHcokFxHMU3WrYtZO6ZW3F+DG+IMHuvBgNXrE4fxVtTCphPdo\
RTVsvNBNuLiz6MNiKIhqR7C8dgH8apLOISFVAWwL7GIYbMNey0xr/USDnvJ6zpQd\
uXBsvjbqhHC1Rt7/rEsCAwEAAaNnMGUwEgYDVR0TAQH/BAgwBgEB/wIBATAOBgNV\
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFC97AyA5oUTxHRK5E3qIRYZXmbqGMCAGA1Ud\
EQQZMBeBD3ZtY2FAdm13YXJlLmNvbYcEfwAAATANBgkqhkiG9w0BAQsFAAOBgQAS\
7Hqt1ZSHKPlT7c8Qjq/5R1TK8wDEFoAFW361d0V4IuyXZO/ZQCZMSoDcZS3q1Ras\
xLWx2y1E3yaypDXuye1k6a7Ko7exg4HhKyTpQSaBs1sr39oQqiVGkFdXCvfQXnzJ\
OMthKCfV1A67lcC3tIu+avKziVawXAZIJ8b92UVYlQ==\
-----END CERTIFICATE-----";

char *RealPrivateKey = "\
----BEGIN PRIVATE KEY-----\
MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBALosjf9mASyzO6+7\
9v5GuD9VraqYR+VGmM2e/Hokwx3KJBcRzFN1q2LWTumVtxfgxviDB7rwYDV6xOH8\
VbUwqYT3aEU1bLzQTbi4s+jDYiiIakewvHYB/GqSziEhVQFsC+xiGGzDXstMa/1E\
g57yes6UHblwbL426oRwtUbe/6xLAgMBAAECgYARom7219lZO+CPWX1G6GyP9aEk\
E68nszaxFccLfzG+Jh7VOPnclyMOeYc/zk8cyc1LzjmF7KC1IBv7ERt0zkp+mBoS\
hEJapE1teuI4h3KvZzdhrOL39tAiTcqBolKdZW445QGkdnCLLApeFv36HJ95WwQ2\
QhdB2+Z1Xntff4JTmQJBAOIpHwwaGKx/WYWs07NVD6EX7zoKQ3SlRHJc5BJ1fwuh\
fsycGpVjZq6rQ9FCTt6jGe13RO+WQLqUXX/eBTJyiUMCQQDSvNUty1okZ8YZQu2N\
hDTBpXBGxCSD3BDlU140c8eS799mB79hqq3rXPpnSfXiKqNuDyMyNfLKxciUdZpo\
sPxZAkEArRJ9APzjjvZyItsssxtQN56iY09Baf2jkMVHgFzMWbE/4QNBULtd9CN6\
ZaSRyM0WZWSVq6fXZzFtEBJu9bkzEwJAeFraUWYjHEmcLM85hqoryzCmF/RreldT\
2vUrBz/cikGuz8G6/peQ7qvSYu0tnbaGUhQZINMIz9/3dzpXLlVuOQJAHuSu6O8i\
udbhH9Quz5r8f97OHuKaI7VDfTaTH4RbQ5S88ofXXd0KJq9tIlj70mPzbyL9GOTv\
lt6BvToX0SPGEg==\
-----END PRIVATE KEY-----";

char *NotCAPrivateKey ="";

BOOST_CHECKPOINT("Call Add Root CA with NULL ServerName");
dwError =  VMCAAddRootCertificate(
    NULL,
    RealCACert,
    NULL,
    RealPrivateKey);

BOOST_REQUIRE(dwError != 0);

BOOST_CHECKPOINT("Call Add Root CA with NULL Cert");
dwError =  VMCAAddRootCertificate(
    GetServerName().c_str(),
    NULL,
    NULL,
    RealPrivateKey);

BOOST_REQUIRE(dwError != 0);

BOOST_CHECKPOINT("Call Add Root CA with NULL Private Key");
dwError =  VMCAAddRootCertificate(
    GetServerName().c_str(),
    RealCACert,
    NULL,
    NULL);

BOOST_REQUIRE(dwError != 0);

BOOST_CHECKPOINT("Call Add Root CA with Not a CA Cert Key");
dwError =  VMCAAddRootCertificate(
    GetServerName().c_str(),
    Non_CA_Cert,
    NULL,
    RealPrivateKey);
BOOST_REQUIRE(dwError != 0);


BOOST_CHECKPOINT("Call Add Root CA with Not a CA Cert Key");
dwError =  VMCAAddRootCertificate(
    GetServerName().c_str(),
    RealCACert,
    NULL,
    NotCAPrivateKey);
BOOST_REQUIRE(dwError != 0);

BOOST_CHECKPOINT("Call Add Root CA with Not a CA Cert Key");
dwError =  VMCAAddRootCertificate(
    GetServerName().c_str(),
    "Junk Junk",
    NULL,
    NotCAPrivateKey);
BOOST_REQUIRE(dwError != 0);

}

BOOST_AUTO_TEST_CASE(NEG_VMCACreateSelfSignedCertificate_Key)
{

AutoProfile Perf("NEG_AddRootCertificate_Key","1.1.3");
DWORD dwError = 0;

char *RealPrivateKey = "\
----BEGIN PRIVATE KEY-----\
MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBALosjf9mASyzO6+7\
9v5GuD9VraqYR+VGmM2e/Hokwx3KJBcRzFN1q2LWTumVtxfgxviDB7rwYDV6xOH8\
VbUwqYT3aEU1bLzQTbi4s+jDYiiIakewvHYB/GqSziEhVQFsC+xiGGzDXstMa/1E\
g57yes6UHblwbL426oRwtUbe/6xLAgMBAAECgYARom7219lZO+CPWX1G6GyP9aEk\
E68nszaxFccLfzG+Jh7VOPnclyMOeYc/zk8cyc1LzjmF7KC1IBv7ERt0zkp+mBoS\
hEJapE1teuI4h3KvZzdhrOL39tAiTcqBolKdZW445QGkdnCLLApeFv36HJ95WwQ2\
QhdB2+Z1Xntff4JTmQJBAOIpHwwaGKx/WYWs07NVD6EX7zoKQ3SlRHJc5BJ1fwuh\
fsycGpVjZq6rQ9FCTt6jGe13RO+WQLqUXX/eBTJyiUMCQQDSvNUty1okZ8YZQu2N\
hDTBpXBGxCSD3BDlU140c8eS799mB79hqq3rXPpnSfXiKqNuDyMyNfLKxciUdZpo\
sPxZAkEArRJ9APzjjvZyItsssxtQN56iY09Baf2jkMVHgFzMWbE/4QNBULtd9CN6\
ZaSRyM0WZWSVq6fXZzFtEBJu9bkzEwJAeFraUWYjHEmcLM85hqoryzCmF/RreldT\
2vUrBz/cikGuz8G6/peQ7qvSYu0tnbaGUhQZINMIz9/3dzpXLlVuOQJAHuSu6O8i\
udbhH9Quz5r8f97OHuKaI7VDfTaTH4RbQ5S88ofXXd0KJq9tIlj70mPzbyL9GOTv\
lt6BvToX0SPGEg==\
-----END PRIVATE KEY-----";
PVMCA_CERTIFICATE pCertificate = NULL;

VMCAData Data;
BOOST_CHECKPOINT("Call : InitCSRField");
Data.InitCSRFields();

time_t now;
time_t expire;
time (&now);
expire = now + ( 60 * 60* 24 * 365 * 10);

BOOST_CHECKPOINT("Call : VMCACreateSelfSignedCertificate with Null Request");
dwError = VMCACreateSelfSignedCertificate(
    NULL,
    RealPrivateKey,
    NULL,
    now,
    expire,
    &pCertificate);
    BOOST_REQUIRE((dwError != 0) &&
                   (pCertificate == NULL));


BOOST_CHECKPOINT("Call : VMCACreateSelfSignedCertificate with Null Private Key");
dwError = VMCACreateSelfSignedCertificate(
    Data.GetDataReq(),
    NULL,
    NULL,
    now,
    expire,
    &pCertificate);
    BOOST_REQUIRE((dwError != 0) &&
                   (pCertificate == NULL));


BOOST_CHECKPOINT("Call : VMCACreateSelfSignedCertificate with Invalid Private Key");
dwError = VMCACreateSelfSignedCertificate(
    Data.GetDataReq(),
    "Not a Private Key",
    NULL,
    now,
    expire,
    &pCertificate);
    BOOST_REQUIRE((dwError != 0) &&
                   (pCertificate == NULL));


BOOST_CHECKPOINT("Call : VMCACreateSelfSignedCertificate with NULL certificate Pointer");
dwError = VMCACreateSelfSignedCertificate(
    Data.GetDataReq(),
    RealPrivateKey,
    NULL,
    now,
    expire,
    NULL);
    BOOST_REQUIRE((dwError != 0) &&
                   (pCertificate == NULL));


BOOST_CHECKPOINT("Call : VMCACreateSelfSignedCertificate with Start Time in future than expire time");
dwError = VMCACreateSelfSignedCertificate(
    Data.GetDataReq(),
    "Not a Private Key",
    NULL,
    expire,
    now,
    &pCertificate);
    BOOST_REQUIRE((dwError != 0) &&
                   (pCertificate == NULL));

}

BOOST_AUTO_TEST_CASE(NEG_EnumTest)
{

AutoProfile Perf("NEG_EnumTest","1.1.3");
DWORD dwError = 0;
PVOID pContext = NULL;
PVMCA_CERTIFICATE pCertificate = NULL;
int CurrentIndex = 0;
VMCA_ENUM_CERT_RETURN_CODE enumStatus;

BOOST_CHECKPOINT("Call : VMCAOpenEnumContext with NULL Server");

dwError =  VMCAOpenEnumContext( NULL,
                                VMCA_CERTIFICATE_ACTIVE,
                                &pContext);
BOOST_REQUIRE(dwError != 0);

BOOST_CHECKPOINT("Call : VMCAOpenEnumContext with NULL Context");

dwError =  VMCAOpenEnumContext( GetServerName().c_str(),
                                VMCA_CERTIFICATE_ACTIVE,
                                NULL);
BOOST_REQUIRE(dwError != 0);

// Now get real Handle So we can test out rest of the Stuff.

BOOST_CHECKPOINT("Call : VMCAOpenEnumContext Should Succeed");

dwError =  VMCAOpenEnumContext( GetServerName().c_str(),
                                VMCA_CERTIFICATE_ACTIVE,
                                &pContext);
BOOST_REQUIRE(dwError == 0);

BOOST_CHECKPOINT("Call : VMCAGetNextCertificate with NULL Server");


dwError =
VMCAGetNextCertificate(
    NULL,
    &pCertificate,
    &CurrentIndex,
    &enumStatus
);
printf("1) Enum Status -> %d", enumStatus);
BOOST_REQUIRE(dwError != 0);

dwError =
VMCAGetNextCertificate(
    pContext,
    NULL,
    &CurrentIndex,
    &enumStatus
);
printf("2) Enum Status -> %d", enumStatus);
BOOST_REQUIRE(dwError != 0);


dwError =
VMCAGetNextCertificate(
    pContext,
    &pCertificate,
    NULL,
    &enumStatus
);
printf("3) Enum Status -> %d", enumStatus);
BOOST_REQUIRE(dwError != 0);


dwError =
VMCAGetNextCertificate(
    pContext,
    &pCertificate,
    &CurrentIndex,
    NULL
);
BOOST_REQUIRE(dwError != 0);


//
// Way the code is written, it just works if you pass NULL.
dwError = VMCACloseEnumContext( NULL);
BOOST_REQUIRE( dwError == 0);

dwError = VMCACloseEnumContext( pContext);
BOOST_REQUIRE( dwError == 0);

}
BOOST_AUTO_TEST_SUITE_END()
