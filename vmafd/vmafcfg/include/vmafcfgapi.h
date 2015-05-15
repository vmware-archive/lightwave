//
// Config
//

#define VMAF_MAX_CONFIG_VALUE_BYTE_LENGTH (2048)

typedef DWORD (*PFN_VMAF_CFG_OPEN_CONNECTION)(
                    PVMAF_CFG_CONNECTION* ppConnection
                    );

typedef DWORD (*PFN_VMAF_CFG_OPEN_ROOT_KEY)(
					PVMAF_CFG_CONNECTION pConnection,
					PCSTR                pszKeyName,
				    DWORD                dwOptions,
				    DWORD                dwAccess,
					PVMAF_CFG_KEY*       ppKey
					);

typedef DWORD (*PFN_VMAF_CFG_OPEN_KEY)(
                    PVMAF_CFG_CONNECTION pConnection,
                    PVMAF_CFG_KEY        pKey,
                    PCSTR                pszSubKey,
                    DWORD                dwOptions,
                    DWORD                dwAccess,
                    PVMAF_CFG_KEY*       ppKey
                    );

typedef DWORD (*PFN_VMAF_CFG_CREATE_KEY)(
                    PVMAF_CFG_CONNECTION pConnection,
                    PVMAF_CFG_KEY        pKey,
                    PCSTR                pszSubKey,
                    DWORD                dwOptions,
                    DWORD                dwAccess,
                    PVMAF_CFG_KEY*       ppKey
                    );

typedef DWORD (*PFN_VMAF_CFG_READ_STRING_VALUE)(
                    PVMAF_CFG_KEY        pKey,
                    PCSTR                pszSubkey,
                    PCSTR                pszName,
                    PSTR*                ppszValue
                    );

typedef DWORD (*PFN_VMAF_CFG_READ_DWORD_VALUE)(
                    PVMAF_CFG_KEY        pKey,
                    PCSTR                pszSubkey,
                    PCSTR                pszName,
                    PDWORD               pdwValue
                    );

typedef DWORD (*PFN_VMAF_CFG_SET_VALUE)(
					PVMAF_CFG_KEY        pKey,
					PCSTR                pszValue,
					DWORD                dwType,
					PBYTE                pValue,
					DWORD                dwSize
					);

typedef VOID (*PFN_VMAF_CFG_CLOSE_KEY)(
                    PVMAF_CFG_KEY pKey
                    );

typedef VOID (*PFN_VMAF_CFG_CLOSE_CONNECTION)(
                    PVMAF_CFG_CONNECTION pConnection
                    );

typedef struct _VMAF_CFG_PACKAGE
{
    PFN_VMAF_CFG_OPEN_CONNECTION   pfnOpenConnection;
    PFN_VMAF_CFG_OPEN_ROOT_KEY     pfnOpenRootKey;
    PFN_VMAF_CFG_OPEN_KEY          pfnOpenKey;
    PFN_VMAF_CFG_CREATE_KEY        pfnCreateKey;
    PFN_VMAF_CFG_READ_STRING_VALUE pfnReadStringValue;
    PFN_VMAF_CFG_READ_DWORD_VALUE  pfnReadDWORDValue;
    PFN_VMAF_CFG_SET_VALUE         pfnSetValue;
    PFN_VMAF_CFG_CLOSE_KEY         pfnCloseKey;
    PFN_VMAF_CFG_CLOSE_CONNECTION  pfnCloseConnection;

} VMAF_CFG_PACKAGE, *PVMAF_CFG_PACKAGE;

