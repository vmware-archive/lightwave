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

#ifdef WIN32
#define PLUG_API __declspec(dllexport)
#else
#define PLUG_API extern
#endif

#ifdef WIN32
extern PFSRPGETFUN gpfGetSrpSecret;
#endif

#define SASL_AUXPROP_PLUG_INIT( x )                            \
extern sasl_auxprop_init_t x##_auxprop_plug_init;              \
PLUG_API int sasl_auxprop_plug_init(const sasl_utils_t *utils, \
                           int maxversion, int *out_version,   \
                           sasl_auxprop_plug_t **plug,         \
                           const char *plugname)               \
{                                                              \
        return x##_auxprop_plug_init(utils, maxversion, out_version, \
                                     plug, plugname);                \
}

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    BOOL result = TRUE;
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            gpfGetSrpSecret = (PFSRPGETFUN)GetProcAddress(
                GetModuleHandle(NULL),
                "VmDirSRPGetIdentityData"
                );
            if (!gpfGetSrpSecret)
            {
                result = FALSE;
            }
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return result;
}
#endif

SASL_AUXPROP_PLUG_INIT( lwraftdb )
