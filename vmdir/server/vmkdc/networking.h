typedef enum _VMKDC_SERVICE_PORT_TYPE
{
    VMKDC_SERVICE_PORT_UDP = 1,
    VMKDC_SERVICE_PORT_TCP,
} VMKDC_SERVICE_PORT_TYPE;


DWORD
VmKdcSrvOpenServicePort(
    PVMKDC_GLOBALS pGlobals,
    VMKDC_SERVICE_PORT_TYPE portType);

DWORD
VmKdcSrvServicePortListen(
    PVMKDC_GLOBALS pGlobals);

DWORD
VmKdcInitConnAcceptThread(
    PVMKDC_GLOBALS pGlobals);

DWORD
VmKdcSendTcp(
    int sock,
    unsigned char *msg,
    int msgLen);

void
VmKdcSrvCloseSocketAcceptFd(
    VOID);

void
VmKdcSrvIncrementThreadCount(
    PVMKDC_GLOBALS pGlobals);

void
VmKdcSrvDecrementThreadCount(
    PVMKDC_GLOBALS pGlobals);

void
VmKdcSrvGetThreadCount(
    PVMKDC_GLOBALS pGlobals,
    PDWORD pWorkerThreadCount);
