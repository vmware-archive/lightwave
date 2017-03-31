// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define SECURITY_WIN32	1
#include "targetver.h"
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntsecapi.h>
#include <stdio.h>
#include <tchar.h>
#include <sspi.h>
#include <dsgetdc.h>
#include <lm.h>
#include <winldap.h>
#include <stdlib.h>
#include <idmcommon.h>

#ifdef HAVE_LWMEM_H
#include <lwmem.h>
#endif

#include <vmstssys.h>

#include "defines.h"
