#include "includes.h"

static DWORD
server_acquire_creds(
    PSTR service_name,
    gss_OID_desc *mech,
    gss_cred_id_t *server_creds
    )
{
    DWORD dwError               = 0;
    gss_buffer_desc name_buf    = GSS_C_EMPTY_BUFFER;
    gss_name_t server_name      = GSS_C_NO_NAME;
    OM_uint32 min               = 0;
    OM_uint32 maj               = 0;

    gss_OID_desc mech_oid_array[1];
    gss_OID_set_desc desired_mech   = {0};

    if (mech)
    {
        desired_mech.count = 1;
        desired_mech.elements = mech_oid_array;
        desired_mech.elements[0] = *mech;
    }

    if (IsNullOrEmptyString(service_name))
    {
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    name_buf.value = service_name;
    name_buf.length = strlen(name_buf.value) + 1;
    maj = gss_import_name(&min, &name_buf,
                GSS_C_NT_HOSTBASED_SERVICE, &server_name);

    //TODO: insert show error
    maj = gss_acquire_cred(
                &min,
                server_name,
                0,
                &desired_mech,
                GSS_C_ACCEPT,
                server_creds,
                NULL,
                NULL
                );
    //TODO: insert show error
    (void) gss_release_name(&min, &server_name);

cleanup:
    return (DWORD) maj;

error:
    if (maj)
    {
        maj = min ? min : maj;
    }
    if (server_name)
    {
        gss_release_name(&min, &server_name);
    }
    goto cleanup;
}

uint32_t
make_negotiate_token(
    gss_buffer_desc *pBuffer,
    char **ppszNegotiate
    )
{
    uint32_t dwError = 0;
    char *pszEncodedData = NULL;
    char *pszNegotiate = NULL;
    //int len = 0;

    if (pBuffer)
    {
        dwError = base64_encode(
                      pBuffer->value,
                      pBuffer->length,
                      &pszEncodedData);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError =  VMCAAllocateStringPrintfA(
                   &pszNegotiate,
                   "Negotiate %s",
                   pszEncodedData ? pszEncodedData : "");
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszNegotiate = pszNegotiate;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszEncodedData);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pszNegotiate);
    goto cleanup;
}

DWORD
VMCARESTMakeKrbAccessToken(
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    PVMCA_ACCESS_TOKEN pAccessToken = NULL;
    if (!ppAccessToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                            sizeof(VMCA_ACCESS_TOKEN),
                            (PVOID*)&pAccessToken
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    pAccessToken->tokenType = VMCA_AUTHORIZATION_TOKEN_TYPE_KRB;

    dwError = VMCAAllocateMemory(
                            sizeof(int),
                            (PVOID*) &pAccessToken->bKrbTicketValid
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    *pAccessToken->bKrbTicketValid = 1;

    *ppAccessToken = pAccessToken;
cleanup:

    return dwError;

error:
    if (pAccessToken)
    {
        VMCA_SAFE_FREE_MEMORY(pAccessToken->bKrbTicketValid);
        VMCA_SAFE_FREE_MEMORY(pAccessToken);
    }
    goto cleanup;
}

DWORD
VMCARESTVerifyKrbAuth(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    PSTR pszNegotiate = NULL;
    PSTR pszDecode = NULL;
    PSTR pszUser = NULL;
    char *pszToken = NULL;
    int nLength = 0;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc display_name = GSS_C_EMPTY_BUFFER;
    gss_ctx_id_t gss_context = GSS_C_NO_CONTEXT;
    gss_name_t client_name = GSS_C_NO_NAME;
    static gss_OID_desc gss_spnego_mech_oid_desc =
                                 {6, (void *)"\x2b\x06\x01\x05\x05\x02"};
    static gss_OID gss_spnego_mech_oid = &gss_spnego_mech_oid_desc;
    gss_cred_id_t server_creds;

    pszNegotiate = pAuthorization->pszAuthorizationToken;
    if ( IsNullOrEmptyString(pszNegotiate) )
    {
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if (!strcmp(pszNegotiate,"testing"))
    // TODO: REMOVE
    // TODO: DO NOT CHECK IN
    {// Kerberos backdoor for testing
        dwError = VMCARESTMakeKrbAccessToken(ppAccessToken);
        BAIL_ON_VMREST_ERROR(dwError);
        goto cleanup;
    }
    dwError = base64_decode(
                        pszNegotiate,
                        &pszDecode,
                        &nLength
                        );
    BAIL_ON_VMREST_ERROR(dwError);

    dwError = server_acquire_creds(
                            "HTTP",
                            &gss_spnego_mech_oid_desc,
                            &server_creds
                            );
    BAIL_ON_VMREST_ERROR(dwError);

    input_token.length = nLength;
    input_token.value = pszDecode;

    major_status = gss_accept_sec_context(
                                    &minor_status,
                                    &gss_context,
                                    server_creds,
                                    &input_token,
                                    GSS_C_NO_CHANNEL_BINDINGS,
                                    &client_name,
                                    &gss_spnego_mech_oid,
                                    &output_token,
                                    NULL,
                                    NULL,
                                    NULL
                                    );

    if (GSS_ERROR(major_status) )
    {
        //TODO: insert show error
        dwError = EACCES;
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if (output_token.length)
    {
        dwError = make_negotiate_token(&output_token, &pszToken);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    if (major_status == GSS_S_CONTINUE_NEEDED)
    {
        OM_uint32 min2;
        gss_buffer_desc mech_msg = GSS_C_EMPTY_BUFFER;
        gss_buffer_desc gss_msg = GSS_C_EMPTY_BUFFER;
        gss_buffer_desc minor_msg = GSS_C_EMPTY_BUFFER;
        OM_uint32 msg_ctx = 0;
        PSTR pszError = NULL;

        gss_oid_to_str(&min2, gss_spnego_mech_oid, &mech_msg);
        gss_display_status(&min2, major_status, GSS_C_GSS_CODE, gss_spnego_mech_oid, &msg_ctx, &gss_msg);
        gss_display_status(&min2, minor_status, GSS_C_MECH_CODE, gss_spnego_mech_oid, &msg_ctx, &minor_msg);

        VMCAAllocateStringPrintfA(&pszError, "gss_rc[%d:%*s] mech[%*s] minor[%u:%*s]",
            major_status, (int)gss_msg.length,
            (const char *)(gss_msg.value?gss_msg.value:""),
            (int)mech_msg.length,
            (const char *)(mech_msg.value?mech_msg.value:""),
            minor_status, (int)minor_msg.length,
            (const char *)(minor_msg.value?minor_msg.value:""));

        gss_release_buffer(&min2, &mech_msg);
        gss_release_buffer(&min2, &gss_msg);
        gss_release_buffer(&min2, &minor_msg);
    }
    if (major_status == GSS_S_COMPLETE)
    {
        gss_display_name(&minor_status, client_name, &display_name, NULL);

        dwError = VMCAAllocateStringA(display_name.value, &pszUser);
        BAIL_ON_VMREST_ERROR(dwError);
    }

    dwError = VMCARESTMakeKrbAccessToken(ppAccessToken);
    BAIL_ON_VMREST_ERROR(dwError);

cleanup:
    if (pszUser)
    {
        VMCA_SAFE_FREE_MEMORY(pszUser);
    }
    return dwError;

error:
    goto cleanup;
}

VOID
VMCARESTFreeKrb(
    PVMCA_ACCESS_TOKEN pAccessToken
    )
{
    if (pAccessToken)
    {
        VMCA_SAFE_FREE_MEMORY(pAccessToken->bKrbTicketValid);
        VMCA_SAFE_FREE_MEMORY(pAccessToken);
    }
}
