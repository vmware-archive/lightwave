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
#include <iostream>
#include <fstream>
#include <time.h>
#include <vmca_error.h>
#include <vmcacommon.h>

#ifdef _WIN32
#include <windows.h>
#endif




//
// This file contains mostly utility functions needed by the CertTool
// This file also does lot of OS specific stuff and this file is designed
// to be full of OS Specific Stuff with #ifdefs
int
GetSleepTime(int secondsToSleep)
{
#if _WIN32
    return secondsToSleep * 1000; // Windows Sleeps in MilliSeconds
#else
    return secondsToSleep; // Posix Sleeps in Seconds.
#endif
}

DWORD
GetMachineNameInternal(int type, PSTR *ppMachineName)
//
// 1 = ComputerNameDnsFullyQualified
// 3 = ComputerNameNetBIOS
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724301(v=vs.85).aspx
//
{
    const int NAMEBUF = 1024;
    DWORD dwError = 0;
#ifdef _WIN32
    wchar_t CompName[NAMEBUF] = {0};
#else
    char CompName[NAMEBUF] = {0};
#endif

    DWORD dwSize = sizeof(CompName);

#ifdef _WIN32
    if (!GetComputerNameExW((COMPUTER_NAME_FORMAT)type,CompName,&dwSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMCA_ERROR(dwError);
    }
#else
    struct hostent* h = NULL;
    dwError = gethostname(CompName,dwSize);
    BAIL_ON_VMCA_ERROR(dwError);
    if (type == FQDN)
    {
        h = gethostbyname(CompName);
        if ( h == NULL) 
        {
            dwError = -1;
            BAIL_ON_VMCA_ERROR(dwError);
        }
        strncpy(CompName, h->h_name, NAMEBUF);
    }
#endif
error :
    if ((type == FQDN) &&
        (dwError > 0) &&
        CompName[0] != '\0')
    {
        // Clear the Error we will just return the host Name as the
        // FQDN.
        dwError = 0;
    }

#ifdef _WIN32
    dwError = VMCAAllocateStringAFromW (CompName, ppMachineName);
#else
    dwError = VMCAAllocateStringA(CompName,ppMachineName);
#endif

    return dwError;
}


DWORD
GetMachineName(PSTR *ppMachineName)
{
    return GetMachineNameInternal(NETBIOSNAME, ppMachineName);
}


DWORD
GetFQDN(PSTR *ppFQDN)
{
    return GetMachineNameInternal(FQDN, ppFQDN);
}
