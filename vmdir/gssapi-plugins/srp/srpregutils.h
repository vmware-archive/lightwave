#ifdef _WIN32
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareAfdService\\Parameters"
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "SYSTEM\\CurrentControlSet\\services\\VMWareDirectoryService"
#else
#define VMAFD_CONFIG_PARAMETER_KEY_PATH "Services\\vmafd\\Parameters"
#define VMDIR_CONFIG_PARAMETER_KEY_PATH "Services\\vmdir"
#endif

#define VMAFD_REG_KEY_DOMAIN_STATE "DomainState"
#define VMDIR_REG_KEY_MACHINE_ACCT "dcAccountDN"
#define VMDIR_REG_KEY_MACHINE_PWD  "dcAccountPassword"
#define VMDIR_REG_KEY_DC_NAME      "DCName"


typedef struct _VMDIR_CONFIG_CONNECTION_HANDLE
{
    HANDLE hConnection;
    HKEY hKey;
} VMDIR_CONFIG_CONNECTION_HANDLE, *PVMDIR_CONFIG_CONNECTION_HANDLE;

DWORD
VMCISLIBAccountDnToUpn(
    PSTR dn,
    PSTR *retUpn);

DWORD
VmDirRegConfigHandleOpen(
    PVMDIR_CONFIG_CONNECTION_HANDLE *ppCfgHandle);

VOID
VmDirRegConfigHandleClose(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle
    );

DWORD
VmDirRegConfigGetValue(
    PVMDIR_CONFIG_CONNECTION_HANDLE pCfgHandle,
    PCSTR  pszSubKey,
    PCSTR  pszKeyName,
    DWORD  valueType,
    PBYTE  pRetValue,
    PDWORD pRetValueLen
    );
