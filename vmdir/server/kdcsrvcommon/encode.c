#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "pacops.h"

int write_enc_pac(char *file, void *buf, int len)
{
    FILE *fp = fopen(file, "wb");
    if (!fp)
    {
        return errno;
    }

    fwrite(buf, 1, len, fp);
    fclose(fp);
    return 0;
}

int read_enc_pac(char *file, void **buf, int *len)
{
    FILE *fp = fopen(file, "rb");
    size_t slen = 0;
    void *retbuf = NULL;

    if (!fp)
    {
        return errno;
    }

    fseek(fp, 0, SEEK_END);
    slen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    retbuf = calloc(slen, sizeof(unsigned char));
    if (!retbuf)
    {
        return errno;
    }
    fread(retbuf, 1, slen, fp);
    fclose(fp);
    *buf = retbuf;
    *len = (int) slen;
    return 0;
}

int main(void)
{
    KERB_VALIDATION_INFO mypac = {0};
    KERB_VALIDATION_INFO *decode_pac = NULL;
    unsigned char *encbuf = NULL;
    long enclen = 0;
    long sts = 0;
    void *encpac_buf = NULL;
    int encpac_len = 0;
    MES_header mes_header = {0};

    mes_header.Version = 1;    /* Must be version 1 */
    mes_header.Endianness = 0x10; /* LE  = 0x10, BE = 0x00 */
    mes_header.CommonHeaderLength = 8; /* Header must be 8 */
    mes_header.Filler1 = 0xcccccccc;   /* Filler must be 0xcccccccc */
    mes_header.ObjectBufferLength = 443; /* this is made up */
    mes_header.Referent = 0x1000; /* Don't know what this is */

    write_enc_pac("mesheader.dat", (void *) &mes_header, sizeof(mes_header));

    printf("MES_header size=%d\n", sizeof(MES_header));
    mypac.UserId = 31415;
    sts = VmKdcEncodeAuthzInfo(&mypac, &enclen, (void **) &encbuf);
    if (sts)
    {
        printf("ERROR: pac_encoder failed <0x%x> \n", sts);
        return 1;
    }
    printf("pac_encoder: len=%d\n", enclen);

    write_enc_pac("vmwpac.dat", encbuf, enclen);

    sts =  read_enc_pac("PAC_LOGON_INFO.dat", &encpac_buf, &encpac_len);
    if (sts)
    {
        printf("ERROR: failed to read encoded pac data from file\n");
        return 1;
    }

    sts = VmKdcDecodeAuthzInfo(encpac_len, encpac_buf, &decode_pac);
    if (sts)
    {
        printf("ERROR: pac_decode failed <0x%x> \n", sts);
        return 1;
    }
    printf("decoded pac: %d\n", decode_pac->UserId);

    return 0;
}
