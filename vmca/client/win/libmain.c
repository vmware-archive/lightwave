/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : libmain.c
 *
 * Abstract :
 *
 *            VMware Certificate Service
 *
 *            Client API
 *
 *            Library Entry Points
 *
 */

#include "includes.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    // Perform actions based on the reason for calling.
    switch( ul_reason_for_call )
    {
        case DLL_PROCESS_ATTACH:
            (DWORD)VMCACommonInit();
            break;

        case DLL_PROCESS_DETACH:
            VMCACommonShutdown();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

