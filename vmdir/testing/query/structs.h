
typedef struct __VMDIR_QUERY_ARGS
{
    PSTR pszHostname;

    PSTR pszBindDN;
    PSTR pszPassword;

    PSTR pszFilter;
    PSTR pszBaseDN;

} VMDIR_QUERY_ARGS, *PVMDIR_QUERY_ARGS;
