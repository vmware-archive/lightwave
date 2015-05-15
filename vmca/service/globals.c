#include "includes.h"

VMCA_SERVER_GLOBALS gVMCAServerGlobals =
{
    // NOTE: order of fields MUST stay in sync with struct definition...
    VMCA_SF_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
    VMCA_SF_INIT(.mutexCRL, PTHREAD_MUTEX_INITIALIZER),
    VMCA_SF_INIT(.dwCurrentCRLNumber, 0),
    VMCA_SF_INIT(.svcMutex, PTHREAD_RWLOCK_INITIALIZER),
    VMCA_SF_INIT(.fVMCALog, NULL),
    VMCA_SF_INIT(.pRPCServerThread, NULL),
    VMCA_SF_INIT(.vmcadState, VMCAD_STARTUP),
    VMCA_SF_INIT(.pCA, NULL),
    VMCA_SF_INIT(.dwFuncLevel, VMCA_FUNC_LEVEL_INITIAL),
    VMCA_SF_INIT(.pDirSyncParams, NULL),
    VMCA_SF_INIT(.pDirSyncThr, NULL),
    VMCA_SF_INIT(.gpEventLog, NULL)
};



