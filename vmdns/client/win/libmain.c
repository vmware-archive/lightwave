/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : libmain.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Client API
 *
 *            Library Entry Points
 *
 */

#include "includes.h"

#ifdef _WIN32

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif