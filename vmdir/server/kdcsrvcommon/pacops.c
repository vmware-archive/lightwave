#include <includes.h>

unsigned32
VmKdcEncodeAuthzInfo(
    VMDIR_AUTHZ_INFO *pac,
    long *bufsiz,
    void **buf)
{
    idl_es_handle_t es_handle = NULL;
    error_status_t sts = 0;
    error_status_t nosts = 0;
    long ret_bufsiz = 0;
    void *retbuf = NULL;
    void *encbuf = NULL;

    idl_es_encode_dyn_buffer(
        (idl_byte**) (void*) &encbuf,
        (idl_ulong_int*) &ret_bufsiz,
        &es_handle,
        &sts);
    if (sts) goto error;

    idl_es_set_attrs(es_handle, IDL_ES_MIDL_COMPAT, &sts);
    if (sts) goto error;

    VmKdcNdrEncodeAuthzInfo(es_handle, pac);

    retbuf = calloc(ret_bufsiz, sizeof(unsigned char));
    if (!retbuf)
    {
        return rpc_s_no_memory;
    }
    memcpy(retbuf, encbuf, ret_bufsiz);

    *bufsiz = ret_bufsiz;
    *buf = retbuf;

    rpc_sm_client_free(encbuf, &nosts);

error:
    if (es_handle)
    {
        idl_es_handle_free(&es_handle, &nosts);
    }

    return sts;
}

unsigned32
VmKdcDecodeAuthzInfo(
    long bufsiz,
    void *buf,
    VMDIR_AUTHZ_INFO **pac
    )
{
    idl_es_handle_t es_handle = NULL;
    error_status_t sts = 0;
    error_status_t nosts = 0;
    MES_header mes_header = {0};

    idl_es_decode_buffer(
        (unsigned char *) buf,
        bufsiz,
        &es_handle,
        &sts);
    if (sts) goto error;

    idl_es_set_attrs(es_handle, IDL_ES_MIDL_COMPAT, &sts);
    if (sts) goto error;

    /* TBD: validate mes_header here*/
    memcpy(&mes_header, buf, sizeof(mes_header));

    VmKdcNdrDecodeAuthzInfo(es_handle, pac);

error:
    if (es_handle)
    {
        idl_es_handle_free(&es_handle, &nosts);
    }

    return sts;
}
