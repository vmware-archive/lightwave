// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define usPort 4444
#define SECURITY_WIN32
#define SEC_SUCCESS(Status) ((Status) >= 0)
#define BAIL_ON_SECURITY_ERROR(dwError)  if ((dwError) < 0) goto error;

#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <vmidmgr.h>
#include "Sspiexample.h"
#include "structs.h"
#include "externs.h"
#include "prototypes.h"


// TODO: reference additional headers your program requires here

#define BAIL_ON_ERROR(dwError) if (dwError) goto error;
