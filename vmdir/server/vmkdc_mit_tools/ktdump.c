#include "includes.h"
int slap_debug = 0;
int ldap_syslog = 0;
VMKDC_GLOBALS gVmkdcGlobals = {0};

void
ktPrintData(PVMKDC_MIT_KEYTAB_FILE pKt)
{
    int i;
    time_t cur_time;
    int keyLen = 0;
    unsigned char *keyData = NULL;

    printf("Entry Len:\t %d\n", pKt->entrySize);
    printf("Principal Type:\t %d\n", pKt->princType);
    printf("Principal:\t ");
    for (i=0; i<pKt->nameComponentsLen; i++)
    {
        printf("%s", pKt->nameComponents[i]);
        if ((i+1) < pKt->nameComponentsLen)
        {
            printf("/");
        }
    }
    if (pKt->realm)
    {
        printf("@%s", pKt->realm);
    }
    printf("\n");

    cur_time = (time_t) pKt->timeStamp;
    printf("Time Stamp:\t %.*s (%d)\n", 24, ctime(&cur_time), pKt->timeStamp);
    printf("Key VNO:\t %d\n", pKt->key->kvno);
    printf("Key type:\t %d\n", pKt->key->type);
    keyLen = VMKDC_GET_LEN_DATA(pKt->key->data);
    keyData = VMKDC_GET_PTR_DATA(pKt->key->data);
    printf("Key len:\t %d\n", keyLen);
    printf("Key:\t\t ");
    for (i=0; i<keyLen; i++)
    {
        printf("%02x", keyData[i]);
    }
    printf("\n");
}


int main(int argc, char *argv[])
{
    DWORD dwError = 0;
    PVMKDC_KEYTAB_HANDLE pKeyTab = NULL;
    PVMKDC_KEYTAB_HANDLE pWriteKeyTab = NULL;
    PVMKDC_MIT_KEYTAB_FILE ktData = NULL;
    BOOLEAN bWriteKeyTable = FALSE;

    if (argc==1)
    {
        printf("usage: %s stashfile\n", argv[0]);
        return 0;
    }

    dwError = VmKdcInitKrb5(&gVmkdcGlobals.pKrb5Ctx);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcParseKeyTabOpen(
                  argv[1],
                  "r",
                  &pKeyTab);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (argc>2)
    {
        dwError = VmKdcParseKeyTabOpen(
                      argv[2],
                      "a",
                      &pWriteKeyTab);
        BAIL_ON_VMKDC_ERROR(dwError);
        bWriteKeyTable = TRUE;
    }

    do
    {
        dwError = VmKdcParseKeyTabRead(pKeyTab, &ktData);

        if (dwError == 0)
        {
            if (bWriteKeyTable)
            {
                VmKdcParseKeyTabWrite(pWriteKeyTab, ktData);
            }
            ktPrintData(ktData);
        }
    } while (dwError == 0);
error:
    VmKdcParseKeyTabFreeEntry(ktData);
    VmKdcParseKeyTabClose(pKeyTab);
    VmKdcParseKeyTabClose(pWriteKeyTab);
    VmKdcDestroyKrb5(gVmkdcGlobals.pKrb5Ctx);
    return 0;
}
