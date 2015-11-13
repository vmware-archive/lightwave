#ifndef _WIN32
#include <config.h>

#include <execinfo.h>
#include <vmdirsys.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <uuid/uuid.h>

#include <sasl/sasl.h>
#include <sasl/saslutil.h>

#include <openssl/sha.h>
#include <openssl/rand.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <vmsuperlogging.h>
#include <vmdirserver.h>
#include <ldaphead.h>
#include <schema.h>
#include <vmacl.h>

#include <backend.h>
#include <middlelayer.h>

#else

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#if !defined(HAVE_DCERPC_WIN32)
#include <rpc.h>
#endif
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <tchar.h>

#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <sasl/sasl.h>
#include <sasl/saslutil.h>

#include <openssl/sha.h>
#include <openssl/rand.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <vmdirserver.h>
#include <ldaphead.h>
#include <schema.h>
#include <vmacl.h>

#include <backend.h>
#include <middlelayer.h>
#include "banned.h"

#endif
#include <csrp/srp.h>


