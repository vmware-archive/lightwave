#ifndef _WIN32
#include <config.h>

#include <vmdirsys.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <db.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>
#include <indexcfg.h>
#include <schema.h>
#include <vmdirserver.h>
#include <backend.h>

#include <bdbstore.h>

#include "structs.h"
#include "bdb.h"
#include "externs.h"

#else

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "banned.h"

#endif
