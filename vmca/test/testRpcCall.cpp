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
#include <string>
#include <vmca_h.h>
#include <vmcaclient.h>

using namespace std;

int main(int argc, char **argv)
{
	DWORD dwError = 0;
	std::string Name ="Anu";
	std::string Organization = "vmware";
	std::string State ="Washington";
	std::string Country = "US";
	std::string Email = "aengineer@vmware.com";
	std::string ipAddress = "127.0.0.1";
	PVMCA_PKCS_10_REQ_DATA pCertReqData = NULL;
	dwError = VMCAAllocatePKCS10Data(&pCertReqData);
	dwError = VMCAInitPKCS10DataAnsi(
    Name.c_str(),
    Organization.c_str(),
    State.c_str(),
    Country.c_str(),
    Email.c_str(),
    ipAddress.c_str(),
    0, /* Time to Live, expiration in seconds 0 == 1 YEAR*/
    pCertReqData);
	VMCAFreePKCS10Data(pCertReqData); 
}

