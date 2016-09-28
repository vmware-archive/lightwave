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
#include <vmcatypes.h>
#include <vmca.h>
#include <testcases.h>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

namespace VMCAUtil {

VMCAData::VMCAData()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    this->reqData = NULL;
    this->PrivateKey = NULL;
    this->PublicKey = NULL;
    this->Csr = NULL;
    this->SelfCertificate = NULL;
    this->RootCACertificate = NULL;
    this->GenCertificate = NULL;

    VMCAAllocatePKCS10Data(&this->reqData);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
}

VMCAData::~VMCAData()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if (this->reqData) {
        VMCAFreePKCS10Data(this->reqData);
    }

    if(this->PrivateKey) {
        VMCAFreeKey(this->PrivateKey);
    }

    if (this->PublicKey) {
        VMCAFreeKey(this->PublicKey);
    }

    if(this->SelfCertificate) {
        VMCAFreeCertificate(this->SelfCertificate);
    }

    if(this->RootCACertificate) {
        VMCAFreeCertificate(this->RootCACertificate);
    }

    if(this->GenCertificate){
        VMCAFreeCertificate(this->GenCertificate);
    }

    if(this->Csr) {
        VMCAFreeCSR(this->Csr);
    }
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}

PVMCA_PKCS_10_REQ_DATA VMCAData::GetDataReq()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
    return this->reqData;
}

void VMCAData::SetDataReq(PVMCA_PKCS_10_REQ_DATA pData)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if (this->reqData != NULL) {
        VMCAFreePKCS10Data(this->reqData);
        this->reqData = NULL;
    }
    this->reqData = pData;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}

PVMCA_KEY VMCAData::GetPrivatKey()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

    return this->PrivateKey;
}
void VMCAData::SetPrivateKey(PVMCA_KEY pKey)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if (this->PrivateKey != NULL)
    {
        VMCAFreeKey(this->PrivateKey);
        this->PrivateKey = NULL;
    }
    this->PrivateKey = pKey;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);


}
PVMCA_KEY VMCAData::GetPublicKey()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

    return this->PublicKey;
}
void VMCAData::SetPublicKey(PVMCA_KEY pKey)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if (this->PublicKey != NULL)
    {
        VMCAFreeKey(this->PublicKey);
        this->PublicKey = NULL;
    }
    this->PublicKey = pKey;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);


}

PVMCA_CERTIFICATE VMCAData::GetSelfCertificate()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
    return this->SelfCertificate;
}
void VMCAData::SetSelfCertificate(PVMCA_CERTIFICATE pCertificate)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if(this->SelfCertificate != NULL)
    {
        VMCAFreeCertificate(this->SelfCertificate);
        this->SelfCertificate = NULL;
    }
    this->SelfCertificate = pCertificate;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}




PVMCA_CERTIFICATE VMCAData::GetRootCertificate()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

    return this->RootCACertificate;
}
void VMCAData::SetRootCertificate(PVMCA_CERTIFICATE pCertificate)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if(this->RootCACertificate != NULL)
    {
        VMCAFreeCertificate(this->RootCACertificate);
        this->RootCACertificate = NULL;
    }
    this->RootCACertificate = pCertificate;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}


PVMCA_CERTIFICATE VMCAData::GetCertificate()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

    return this->GenCertificate;
}
void VMCAData::SetCertificate(PVMCA_CERTIFICATE pCertificate)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if(this->GenCertificate != NULL)
    {
        VMCAFreeCertificate(this->GenCertificate);
        this->GenCertificate = NULL;
    }
    this->GenCertificate = pCertificate;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}


PVMCA_CSR VMCAData::GetCSR()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

    return this->Csr;
}
void VMCAData::SetCSR(PVMCA_CSR pCsr)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    if(this->Csr != NULL)
    {
        VMCAFreeCSR(this->Csr);
        this->Csr = NULL;
    }
    this->Csr = pCsr;
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}

VMCACSRData::VMCACSRData()
 {
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    po::options_description config("");
    po::variables_map argsMap;
    config.add_options()
    ("Country",         po::value<std::string>(&this->cfgCountry),"\tdefault: Country = US ")
    ("Name",            po::value<std::string>(&this->cfgName),"\tdefault: Name = VMCA_DO_NOT_USE ")
    ("Organization",    po::value<std::string>(&this->cfgOrganization), "\tdefault: Organization = VMware")
    ("OrgUnit",         po::value<std::string>(&this->cfgOrgUnit), "\tdefault:. OrgUnit = VMware Engineering")
    ("State",           po::value<std::string>(&this->cfgState),"\tdefault: State = California")
    ("Locality",        po::value<std::string>(&this->cfgLocality),"\tdefault: Locality = Palo Alto")
    ("IPAddress",       po::value<std::string>(&this->cfgIPAddress),"\tdefault: IPAddress = 127.0.0.1")
    ("Email",           po::value<std::string>(&this->cfgEmail),"\tdefault: Email = vmca@vmware.com")
    ("Hostname",        po::value<std::string>(&this->cfgHostName),"\tdefault: Hostname = machine.vmware.com");

    std::ifstream ifs("VMCAUnitTest.cfg");

    if(!ifs) {
        std::cout << "Failed to open config file: " << "VMCAUnitTest.cfg" << "\n"
                  << "Exiting ...";
        exit(-1);
    } else {
        BOOST_CHECKPOINT( "Using config file :  VMCAUnitTest.cfg" );
        store(parse_config_file(ifs, config), argsMap);
        notify(argsMap);
    }
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}


void
VMCAData::InitCSRFields()
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;

    dwError = VMCASetCertValue(VMCA_OID_CN,this->GetDataReq(),
                                (PSTR)this->Config.cfgName.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgName.compare(this->GetDataReq()->pszName) == 0));

    dwError = VMCASetCertValue(VMCA_OID_COUNTRY,this->GetDataReq(),
                                (PSTR) this->Config.cfgCountry.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgCountry.compare(this->GetDataReq()->pszCountry) == 0));

    dwError = VMCASetCertValue(VMCA_OID_LOCALITY,this->GetDataReq(),
                                (PSTR) this->Config.cfgLocality.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgLocality.compare(this->GetDataReq()->pszLocality) == 0));

    dwError = VMCASetCertValue(VMCA_OID_STATE,this->GetDataReq(),
                                (PSTR) this->Config.cfgState.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgState.compare(this->GetDataReq()->pszState) == 0));

    dwError = VMCASetCertValue(VMCA_OID_ORGANIZATION,this->GetDataReq(),
                                (PSTR)this->Config.cfgOrganization.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgOrganization.compare(
                        this->GetDataReq()->pszOrganization) == 0));

    dwError = VMCASetCertValue(VMCA_OID_ORG_UNIT,this->GetDataReq(),
                        (PSTR) this->Config.cfgOrgUnit.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgOrgUnit.compare(this->GetDataReq()->pszOU) == 0));


    dwError = VMCASetCertValue(VMCA_OID_DNS,this->GetDataReq(),
                        (PSTR) this->Config.cfgIPAddress.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgIPAddress.compare(this->GetDataReq()->pszDNSName) == 0));

    dwError = VMCASetCertValue(VMCA_OID_URI,this->GetDataReq(),
                        (PSTR) this->Config.cfgHostName.c_str());
    BOOST_REQUIRE((dwError == 0) &&
        (this->Config.cfgHostName.compare(this->GetDataReq()->pszURIName) == 0));

    // dwError = VMCASetKeyUsageConstraints(this->GetDataReq(),
    //     0);
    // // printf("Key Usage Constraints Are : %d\n", this->GetDataReq()->dwKeyUsageConstraints);
    // BOOST_REQUIRE( (dwError == 0) &&
    //     ((this->GetDataReq()->dwKeyUsageConstraints == 0)));


    //dwError = VMCACreateSigningRequest(this->GetDataReq(),this->GetPrivatKey(),NULL, &this->Csr);
    //BOOST_REQUIRE(dwError == 0);

    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}

DWORD
VMCAData::CreateKeyPair(unsigned int KeyLength)
{

    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    dwError = VMCACreatePrivateKey(NULL, KeyLength, &this->PrivateKey, &this->PublicKey);
    BOOST_REQUIRE( (dwError == 0) &&
                    (this->PrivateKey != NULL) &&
                    (this->PublicKey != NULL));
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
    return dwError;

}


DWORD
VMCAData::CreateSelfSignedCert()
{

    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    time_t now = 0;
    time_t expire = 0;
    time(&now);
    expire = now + ( 365 * 24 * 60 * 60);

    dwError = VMCACreateSelfSignedCertificate(
        this->GetDataReq(),
        this->PrivateKey,
        NULL,
        now,
        expire,
        &this->SelfCertificate
    );
   BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
   return dwError;
}


DWORD
VMCAData::CreateCert(std::string serverName)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    time_t now = 0;
    time_t expire = 0;
    time(&now);
    expire = now + ( 365 * 24 * 60 * 60 * 10);

    dwError =  VMCAGetSignedCertificate((PSTR) serverName.c_str(),
        this->GetDataReq(),
        this->PrivateKey,
        NULL,
        now,
        expire,
        &this->GenCertificate
    );


    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
    return dwError;

}

DWORD
VMCAData::GetRootCertificateFromServer(std::string serverName)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    dwError = VMCAGetRootCACertificate((PCSTR)serverName.c_str(), &this->RootCACertificate);
    if (dwError != 0 ) {
        BOOST_TEST_MESSAGE( __FUNCTION__ << "Error : " << dwError);
    }

    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
    return dwError;

}

void
VMCAData::AddRootCertificate(std::string serverName)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    dwError = VMCAAddRootCertificate((PCSTR)serverName.c_str(),
    this->SelfCertificate,
    NULL,
    this->PrivateKey);

    if (dwError != 0 ) {
        BOOST_TEST_MESSAGE( __FUNCTION__ << "Error :" << dwError);
    }
    BOOST_REQUIRE( dwError == 0);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);

}


void
VMCAData::RevokeCertificate(std::string serverName, std::string certificate)
{
    BOOST_CHECKPOINT("Entered : " << __FUNCTION__);
    DWORD dwError = 0;
    dwError =  VMCARevokeCertificate(serverName.c_str(), (PSTR)certificate.c_str());

    if (dwError != 0 ) {
        BOOST_TEST_MESSAGE( __FUNCTION__ << "Error :" << dwError);
    }

    std::string temp(certificate);
    this->revokedCerts.push_back(temp);

    BOOST_REQUIRE( dwError == 0);
    BOOST_CHECKPOINT("Exited : " << __FUNCTION__);
}

}; //NameSpace


