#ifndef _WIN32
#include <config.h>

#include <vmdirsys.h>

#include <sys/stat.h>
#include <fcntl.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <lmdb.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>
#include <indexcfg.h>
#include <schema.h>
#include <vmdirserver.h>
#include <backend.h>

#include <mdbstore.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif /* GUID_DEFINED */

#ifndef UUID_DEFINED
#define UUID_DEFINED
typedef GUID UUID;
#ifndef uuid_t
#define uuid_t UUID
#endif
#endif


/*
 * All headers under Linux
 */



#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <vmdir.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>
#include <indexcfg.h>
#include <schema.h>
#include <vmdirserver.h>
#include <backend.h>

#include <lmdb.h>

#include <mdbstore.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#endif
