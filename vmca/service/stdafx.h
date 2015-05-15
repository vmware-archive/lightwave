#ifndef _STDAFX_H_
#define _STDAFX_H_

#ifndef _WIN32

#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>
#include <compat/dcerpc.h>
#include <ldap.h>
#include <vmca_h.h>
#else
#include <windows.h>
#include <tchar.h>
#include <direct.h> // for _mkdir
#if defined(_DEBUG)
#include "../x64/Debug/vmca_h.h"
#else
#include "../x64/Release/vmca_h.h"
#endif
#endif

#include <dce/dcethread.h>
#include <dce/rpc.h>
#include <vmcasys.h>
#include <vmcacommon.h>
#include <vmcadb.h>
#include <vmca_error.h>
#include "defines.h"
#include "errormap.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"


#endif // _STDAFX_H_
