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



#include "includes.h"

VOID
VmKdcdStateSet(
    VMKDC_SERVER_STATE   state)
{
    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    gVmkdcGlobals.vmkdcdState = state;
    pthread_cond_signal(&gVmkdcGlobals.stateCond);
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdc: VmKdcdStateSet(%d)", state);
}

VMKDC_SERVER_STATE
VmKdcdState(
    VOID
    )
{
    VMKDC_SERVER_STATE rtnState;

    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    rtnState = gVmkdcGlobals.vmkdcdState;
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);

    return rtnState;
}

#ifdef _WIN32
/*
 * Get the filename of the master key stash file, i.e.,
 * %PROGRAMDATA%\VMware\CIS\cfg\%COMPONENT%\principal-masterkey.stash
 * which usually expands to 
 * C:\ProgramData\VMware\CIS\cfg\vmkdc\principal-masterkey.stash
 */
DWORD
VmKdcGetMasterKeyStashFile(_TCHAR *lpMasterKeyStashFile)
{
    DWORD   dwError              = 0;
#ifdef WIN2008
    const   _TCHAR vmkdcMasterKeyStashFile[]    = _T("\\VMware\\CIS\\cfg\\vmkdcd\\principal-masterkey.stash");
#else
    const   _TCHAR vmkdcMasterKeyStashFile[]    = _T("\\Application Data\\VMware\\CIS\\cfg\\vmkdcd\\principal-masterkey.stash");
#endif
    size_t  vmkdcMasterKeyStashFileLen          = VmKdcStringLenA(vmkdcMasterKeyStashFile);
    size_t  vmkdcSchemaFilePrefixLen        = 0 ;

#ifdef WIN2008
    dwError =  GetEnvironmentVariable(
        _T("PROGRAMDATA"),          // __in_opt   LPCTSTR lpName,
        lpMasterKeyStashFile,       // __out_opt  LPTSTR lpBuffer,
        MAX_PATH                    // __in       DWORD nSize
    );
#else
    dwError =  GetEnvironmentVariable(
        _T("ALLUSERSPROFILE"),      // __in_opt   LPCTSTR lpName,
        lpMasterKeyStashFile,       // __out_opt  LPTSTR lpBuffer,
        MAX_PATH                    // __in       DWORD nSize
    );
#endif
    BAIL_ON_VMKDC_ERROR(0 == dwError);
    dwError = ERROR_SUCCESS;

    vmkdcSchemaFilePrefixLen = VmKdcStringLenA(lpMasterKeyStashFile);

    if ( vmkdcSchemaFilePrefixLen + vmkdcMasterKeyStashFileLen < MAX_PATH )
    {
        dwError = VmKdcStringCatA(lpMasterKeyStashFile, MAX_PATH, vmkdcMasterKeyStashFile);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else // path too long
    {
        dwError = ERROR_BUFFER_OVERFLOW;    // The file name is too long.
                                            // In WinError.h, this error message maps to
                                            // ERROR_BUFFER_OVERFLOW. Not very
                                            // straight forward, though.
        BAIL_ON_VMKDC_ERROR(dwError);
    }
error:
    return dwError;
}
#endif
