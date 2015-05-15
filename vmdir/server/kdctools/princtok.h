#if 0
typedef struct _KEY_VALUE
{
    int keyType;
    int kvno;
    int keyLen;
    unsigned char *keyValue;
} KEY_VALUE, *PKEY_VALUE;
#endif

struct _KEY_ENTRY
{
    char *princName;
    int numKeys;
    PVMKDC_KEY *keys;
//    PKEY_VALUE *keys;
};

typedef struct _KEY_ENTRY KEY_ENTRY, *PKEY_ENTRY;

void tokenizeLine(char *line, PKEY_ENTRY *ppRetKeyEntry);
void keyEntryFree(PKEY_ENTRY pKeyEntry);
