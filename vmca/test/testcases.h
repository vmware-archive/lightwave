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

#ifndef __VMCA_UTIL_H__
#define __VMCA_UTIL_H__
#include <string>
#include <vmcatypes.h>
#include <vmca.h>
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/thread_clock.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/program_options.hpp>
#include <time.h> //asctime
#include <iostream>
#include <cmath>
//#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>
// #include <boost/test/detail/log_level.hpp>
// #include <boost/test/auto_unit_test.hpp>
// #include <boost/test/framework.hpp>
// #include <boost/test/results_reporter.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <fstream>
#include <cassert>
#include <vector>
using namespace boost::unit_test;
using boost::property_tree::ptree;
std::string GetServerName();


namespace po = boost::program_options;

// #define VMCA_SAFE_FREE_STRINGA(PTR)    \
//     do {                          \
//         if ((PTR)) {              \
//             VMCAFreeStringA(PTR); \
//             (PTR) = NULL;         \
//         }                         \
//     } while(0)

#define VMCA_TEST_CHECK_ERROR_CODE(dwError)                               \
  do {                                                                    \
  if ((dwError) != 0 ) {                                                  \
        BOOST_TEST_MESSAGE(__FUNCTION__);                                 \
        BOOST_TEST_MESSAGE(  "Error" << (dwError)));                      \
    }                                                                     \
  }while(0)

namespace VMCAUtil {
struct VMCATestSettings
{
    std::ofstream out;
    static std::string ServerName;

    // static std::string
    // GetServerName()
    // {
    //  //return VMCATestSettings::ServerName;
    // }

    VMCATestSettings()
    {
    // try {
    //     pt:ptree pt;
    //     read_ini("testsetting.ini",pt);
    //     ServerName = pt.get<std::string>("VMCAServer.ServerName","localhost");
    // } catch (std::exception& e) {
    //     // TODO : Log the Error
    //     std::cout << e.what() << std::endl;
    // }

     // time_t rawtime = 0;
     // struct tm * timeinfo = NULL;
     // char buf[24] = {0,};

     // time ( &rawtime );
     // timeinfo = localtime (&rawtime );
     // sprintf(buf,"%d-%d-%d-%d-%d-%d",
     //     (timeinfo->tm_year - 100) + 2000,
     //     timeinfo->tm_mon,
     //     timeinfo->tm_mday,
     //     timeinfo->tm_hour,
     //     timeinfo->tm_min,
     //     timeinfo->tm_sec);

     // std::string outputFileName("test_results-");
     // outputFileName.append(buf);
     // outputFileName.append(".xml");
     // std::cout << outputFileName << std::endl;
     // out.open(outputFileName.c_str());
     // assert(out.is_open());
     // boost::unit_test::unit_test_log.set_stream(out);
     // boost::unit_test::unit_test_log.set_threshold_level( log_warnings ); // log_level.hpp
     // boost::unit_test::unit_test_log.set_format( XML );
    }
    ~VMCATestSettings()
    {

     //   out.close();
    }

};


class VMCACSRData
{
public :
    std::string cfgName;
    std::string cfgCountry;
    std::string cfgOrganization;
    std::string cfgOrgUnit;
    std::string cfgState;
    std::string cfgLocality;
    std::string cfgIPAddress;
    std::string cfgEmail;
    std::string cfgHostName;
    std::string cmpCSR;

    VMCACSRData();

};



class VMCAData
{
private :
    PVMCA_PKCS_10_REQ_DATA reqData;
    PVMCA_KEY PrivateKey;
    PVMCA_KEY PublicKey;
    PVMCA_CERTIFICATE SelfCertificate; // Self Signed Cert
    PVMCA_CERTIFICATE RootCACertificate; // Root CA Certificate
    PVMCA_CERTIFICATE GenCertificate; // Certificate Returned by VMCA


public :
    PVMCA_CSR Csr;
    VMCACSRData Config;
    std::vector<std::string> revokedCerts;

    VMCAData();
    ~VMCAData();
    PVMCA_PKCS_10_REQ_DATA GetDataReq();
    void SetDataReq(PVMCA_PKCS_10_REQ_DATA data);
    PVMCA_KEY GetPrivatKey();
    void SetPrivateKey(PVMCA_KEY key);
    PVMCA_KEY GetPublicKey();
    void SetPublicKey(PVMCA_KEY key);
    PVMCA_CERTIFICATE GetCertificate();
    void SetCertificate(PVMCA_CERTIFICATE certificate);


    PVMCA_CSR GetCSR();
    void SetCSR(PVMCA_CSR csr);
    void AddRootCertificate(std::string serverName);
    DWORD CreateSelfSignedCert();

    DWORD CreateKeyPair(unsigned int KeyLength);
    void InitCSRFields();

    void SetSelfCertificate(PVMCA_CERTIFICATE certificate);
    PVMCA_CERTIFICATE GetSelfCertificate();

    void SetRootCertificate(PVMCA_CERTIFICATE certificate);
    PVMCA_CERTIFICATE GetRootCertificate();


    DWORD GetRootCertificateFromServer(std::string serverName);
    DWORD CreateCert(std::string serverName);
    void RevokeCertificate(std::string serverName, std::string certificate);


};

typedef boost::chrono::milliseconds microseconds;
typedef boost::chrono::nanoseconds ns;
typedef boost::chrono::milliseconds ms;
typedef boost::chrono::system_clock::time_point TimePoint;

class AutoProfile{
    private :
        std::string testID;
        std::string functionName;
        TimePoint start;
        TimePoint end;


    public :
        AutoProfile(std::string FunctionName, std::string testID)
        {
            this->testID = testID;
            this->functionName= FunctionName;
            BOOST_TEST_MESSAGE( "\nEnter :" << FunctionName <<"\n");
            start = boost::chrono::system_clock::now();

        }
        ~AutoProfile()
        {
            end = boost::chrono::system_clock::now();
            ns duration  = end - start;
            BOOST_TEST_MESSAGE( "\nTime Taken : " << boost::chrono::duration_cast<ms>(duration).count()  << " ms : " << this->functionName <<"\n");

        }

};


};
#endif //__VMCA_UTIL_H__

