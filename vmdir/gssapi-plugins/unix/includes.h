#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <lw/types.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>
#else
#include <Windows.h>
#define snprintf _snprintf
#endif

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>
#include <vmdirerrors.h>
#include <vmdirclient.h>

#include "unixregutils.h"
#include "unixreg.h"
