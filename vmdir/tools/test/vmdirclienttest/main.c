#include "includes.h"

/* Example of why you don't call public APIs internally */
ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

#define SIZE_256    256

#if 0
static void _PrintKrbKey(PBYTE pMasterKey, DWORD dwLen)
{
    DWORD i=0;
    printf("\nkey size =  %d\n",dwLen);
    for (i=0; i<dwLen; i++)
    {
        printf("%02x", pMasterKey[i]);
        if ((i+1) % 32 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}
#endif

#if 0
void TestVmDirGetKrbMasterKey()
{
    char        pszDomainName[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwSize = 0;

    printf( "  Kerberos realm name:");
    scanf("%s", pszDomainName);

    dwError = VmDirGetKrbMasterKey(
        pszDomainName,
        &pLocalByte,
        &dwSize
        );

    printf("TestRpcVmDirGetKrbMasterKey returns %d\n",dwError);

    if (dwError == 0 )
    {
        _PrintKrbKey(pLocalByte, dwSize);
    }

    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}
#endif

#if 0
void TestVmDirGetKrbUPNKey()
{
    char        pszUpnName[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwSize = 0;

    printf( "  UserPrincipalName:");
    scanf("%s", pszUpnName);

    dwError = VmDirGetKrbUPNKey(
        pszUpnName,
        &pLocalByte,
        &dwSize
        );
    printf("TestRpcVmDirGetKrbUPNKey returns %d\n",dwError);

    if (dwError == 0)
    {
        _PrintKrbKey(pLocalByte, dwSize);
    }

    printf("\n");
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}
#endif

void
TestVmDirForceResetPassword(
    VOID
    )
{
    char        pszDN[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwByteSize = 0;

    printf( "  Account DN: ");
    scanf("%s", pszDN);

    dwError = VmDirForceResetPassword( pszDN, &pLocalByte, &dwByteSize );

    printf("TestRpcVmDirForceResetPassword returns %d\n",dwError);

    if (dwError == 0)
    {
        DWORD dwCnt = 0;
        printf("Pasword reset to:");
        for (dwCnt=0; dwCnt<dwByteSize; dwCnt++)
        {
            printf("%c", pLocalByte[dwCnt]);
        }
        printf("\n");
    }

    printf("\n");
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}

void
TestVmDirSetLogParameters(
    VOID
    )
{
    DWORD       dwError = 0;
    char        pszLogLevel[SIZE_256] = {0};
    int         iMask = 0;

    printf( "  Log level (ERROR|WARNING|INFO|VERBOSE|DEBUG):");
    scanf("%s", pszLogLevel);
    printf( "  Log mask :");
    scanf("%d", &iMask);

    dwError = VmDirSetLogLevel(pszLogLevel);
    if (dwError)
    {
        printf( "VmDirSetLogLevel failed (%u)", dwError);
    }

    dwError = VmDirSetLogMask(iMask);
    if (dwError)
    {
        printf( "VmDirSetLogMask failed (%u)", dwError);
    }
}

void TestVmDirCreateUser()
{
    char        pszUserName[SIZE_256] = {0};
    char        pszPassword[SIZE_256] = {0};
    char        pszUPNName[SIZE_256] = {0};
    DWORD       dwError = 0;

    printf( "  User account:");
    scanf("%s", pszUserName);
    printf( "  User password:");
    scanf("%s", pszPassword);
    printf( "  UPN name:");
    scanf("%s", pszUPNName);

    dwError = VmDirCreateUser(
        pszUserName,
        pszPassword,
        pszUPNName,
        FALSE
        );
    printf("TestRpcVmDirCreateUser returns %d\n",dwError);
}

void TestVmDirCreateUserEx()
{
    DWORD  dwError = 0;
    char   szAdmin[] = "Administrator";
    char   szDomain[] = "vsphere.local";
    char   szServer[SIZE_256] = {0};
    char   szAdminPassword[SIZE_256] = {0};
    char   szUserName[SIZE_256] = {0};
    char   szAccount[SIZE_256] = {0};
    char   szFirstname[SIZE_256] = {0};
    char   szLastname[SIZE_256] = {0};
    char   szUPNName[SIZE_256] = {0};
    char   szPassword[SIZE_256] = {0};
    PVMDIR_SERVER_CONTEXT pVmDirCtx = NULL;
    VMDIR_USER_CREATE_PARAMS_A createParams = {0};

    printf("DC:");
    scanf("%s", szServer);
    printf("Password (%s@%s):", szAdmin, szDomain);
    scanf("%s", szAdminPassword);
    printf("First name:");
    scanf("%s", szFirstname);
    printf("Last name:");
    scanf("%s", szLastname);
    printf("Account name:");
    scanf("%s", szAccount);
    printf("Common name:");
    scanf("%s", szUserName);
    printf("Password:");
    scanf("%s", szPassword);

    dwError = VmDirOpenServerA(
                   IsNullOrEmptyString(szServer) ? "localhost" : szServer,
                   szAdmin,
                   szDomain,
                   szAdminPassword,
                   0,
                   NULL,
                   &pVmDirCtx);
    BAIL_ON_VMDIR_ERROR(dwError);  

    createParams.pszName = &szUserName[0];
    createParams.pszAccount = &szAccount[0];
    createParams.pszFirstname = &szFirstname[0];
    createParams.pszLastname = &szLastname[0];
    createParams.pszUPN      = szUPNName;
    createParams.pszPassword = &szPassword[0];

    dwError = VmDirCreateUserA(
                   pVmDirCtx,
                   &createParams);
    BAIL_ON_VMDIR_ERROR(dwError);  

error:

    printf("TestRpcVmDirCreateUserEx returns %d\n",dwError);

    if (pVmDirCtx)
    {
        VmDirCloseServer(pVmDirCtx);
    }
}

void
TestVmDirDBFileTransfer()
{
#define VMDIR_DB_READ_BLOCK_SIZE     10000000
#define VMDIR_MDB_DATA_FILE_NAME "data.mdb"

    DWORD       dwError = 0;
    char        pszServerName[VMDIR_MAX_HOSTNAME_LEN];
#if 0
    PCSTR       pszServerEndpoint = NULL;
#endif
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    FILE *      pFileHandle = NULL;
    UINT32      writeSize = 0;
    UINT32      dwCount = 0;
    FILE *      pFile = NULL;
    char        dbLocalFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PBYTE       pReadBuffer = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    char        dbRemoteFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};

#ifndef _WIN32
    const char  *dbHomeDir = VMDIR_DB_DIR;
    const char   fileSeperator = '/';
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    const char   fileSeperator = '\\';

    dwError = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( dwError );
#endif

    printf("Enter partner hostname: ");
    scanf("%s", pszServerName);

    printf( "TestVmDirDBFileTransfer: Connecting to the replication partner (%s) ...\n", pszServerName );

    dwError = VmDirOpenServerA(
                  pszServerName,
                  NULL,
                  NULL,
                  NULL,
                  0,
                  NULL,
                  &hServer);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
            "TestVmDirDBFileTransfer: VmDirOpenServerA() call failed with error: %d, host name = %s",
            dwError, pszServerName);

    printf( "TestVmDirDBFileTransfer: Setting vmdir state to VMDIRD_READ_ONLY \n" );

    dwError = VmDirSetState( hServer, VMDIRD_STATE_READ_ONLY );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "TestVmDirDBFileTransfer: VmDirSetState() call failed with error: %d", dwError  );

    dwError = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator,
                                  VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
            "TestVmDirDBFileTransfer: VmDirStringPrintFA() call failed with error: %d", dwError );

    dwError = VmDirStringPrintFA( dbLocalFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%s", dbRemoteFilename, ".partner" );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
            "TestVmDirDBFileTransfer: VmDirStringPrintFA() call failed with error: %d", dwError );

    // Open local file
    if ((pFile = fopen(dbLocalFilename, "wb")) == NULL)
    {
        dwError = errno;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
            "TestVmDirDBFileTransfer: fopen() call failed, DB file: %s, error: %s.", dbLocalFilename, strerror(errno) );
    }

    printf( "TestVmDirDBFileTransfer: Opening the REMOTE DB file ... : %s\n", dbRemoteFilename );

    dwError = VmDirOpenDBFile( hServer, dbRemoteFilename, &pFileHandle );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (;;)
    {
        printf( "TestVmDirDBFileTransfer: Reading the REMOTE DB file (%p) ..., buffer size (%d)\n",
                pFileHandle, VMDIR_DB_READ_BLOCK_SIZE );

        dwCount = VMDIR_DB_READ_BLOCK_SIZE;

        dwError = VmDirReadDBFile( hServer, pFileHandle, &dwCount, &pReadBuffer );

        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "TestVmDirDBFileTransfer: VmDirReadDBFile() call failed, error: %d", dwError );

        printf( "TestVmDirDBFileTransfer: Writing the LOCAL DB file ... : %s, buf size (%d)\n",
                dbLocalFilename, dwCount );

        writeSize = (UINT32)fwrite(pReadBuffer, 1, dwCount, pFile);

        VMDIR_SAFE_FREE_MEMORY(pReadBuffer);

        if(writeSize < dwCount)
        {
            dwError = -1;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                    "TestVmDirDBFileTransfer: fwrite() call failed, recvSize: %d, writeSize: %d.",
                    dwCount, writeSize );
        }
        if (dwCount < VMDIR_DB_READ_BLOCK_SIZE)
        {
            printf( "DONE copying the file %s \n", dbLocalFilename );
            break;
        }
    }

    printf( "TestVmDirDBFileTransfer: Closing the REMOTE DB file (%p) ...\n", pFileHandle );

    dwError = VmDirCloseDBFile( hServer, pFileHandle );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hServer)
    {
        printf( "TestVmDirDBFileTransfer: Setting vmdir state to VMDIRD_NORMAL \n" );

        dwError = VmDirSetState( hServer, VMDIRD_STATE_NORMAL );
        VmDirCloseServer(hServer);
    }
    if (pFile != NULL)
    {
        fclose(pFile);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(pReadBuffer);

    return;

error:
    printf( "%s\n", pszLocalErrorMsg ? pszLocalErrorMsg : "Hmmm ... no local error message."  );
    goto cleanup;
}

void
TestVmDirReplNow()
{

    DWORD       dwError = 0;
    char        pszServerName[VMDIR_MAX_HOSTNAME_LEN];
    PSTR        pszLocalErrorMsg = NULL;

    printf("Enter partner hostname: ");
    scanf("%s", pszServerName);

    dwError = VmDirReplNow( pszServerName );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "TestVmDirReplNow: VmDirReplNow() call failed with error: %d", dwError  );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return;

error:
    printf( "%s\n", pszLocalErrorMsg ? pszLocalErrorMsg : "Hmmm ... no local error message."  );
    goto cleanup;
}

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, TCHAR *targv[])
#endif
{
    while (1)
    {
        int choice = -1;

        printf( "\n\n==================\n");
        printf( "Please select:\n");
        printf( "0. exit\n");
        printf( "1. TestVmDirSASLClient\n");
#if 0
        printf( "2. TestVmDirGetKrbMasterKey\n");
        printf( "3. TestVmDirGetKrbUPNKey\n");
#endif
        printf( "4. TestVmDirCreateUser\n");
        printf( "5. TestVmDirDBFileTransfer\n");
        printf( "6. TestVmDirReplNow\n");
        printf( "7. TestVmDirForceResetPassword\n");
        printf( "8. TestVmDirSetLogParameters\n");
        printf( "9. TestVmDirCreateUserEx\n");
        printf( "==================\n\n");
        scanf("%d", &choice);

        if (!choice)
        {
            goto cleanup;
        }

        switch (choice)
        {
          case 1:
              TestVmDirSASLClient();
              break;

#if 0
          case 2:
              TestVmDirGetKrbMasterKey();
              break;

          case 3:
              TestVmDirGetKrbUPNKey();
              break;
#endif

          case 4:
              TestVmDirCreateUser();
              break;

          case 5:
              TestVmDirDBFileTransfer();
              break;

          case 6:
              TestVmDirReplNow();
              break;

          case 7:
              TestVmDirForceResetPassword();
              break;

          case 8:
              TestVmDirSetLogParameters();
              break;

          case 9:
              TestVmDirCreateUserEx();
              break;

          default:
              goto cleanup;
        }
    }

cleanup:

    return 0;

}
