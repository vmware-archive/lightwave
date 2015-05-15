// TBD: Adam Put this in the right place (registry?)
#define VMKDC_TICKET_MAX_LIFETIME (8 * 60 * 60)

DWORD
VmKdcProcessKdcReq(
    PVMKDC_CONTEXT pContext,
    PVMKDC_DATA *ppKrbMsg);
