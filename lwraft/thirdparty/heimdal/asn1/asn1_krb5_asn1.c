/* Generated from ./krb5.asn1 */
/* Do not edit */

#define  ASN1_LIB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "krb5-types.h"
#include "krb5_asn1.h"
#include "krb5_asn1-priv.h"
#include "asn1_err.h"
#include "der.h"
#include "der-private.h"
#include "asn1-template.h"
#include "parse_units.h"

int ASN1CALL
encode_NAME_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const NAME_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_NAME_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, NAME_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_NAME_TYPE(data);
return e;
}

void ASN1CALL
free_NAME_TYPE(NAME_TYPE *data)
{
}

size_t ASN1CALL
length_NAME_TYPE(const NAME_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_NAME_TYPE(const NAME_TYPE *from, NAME_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_MESSAGE_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const MESSAGE_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_MESSAGE_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, MESSAGE_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_MESSAGE_TYPE(data);
return e;
}

void ASN1CALL
free_MESSAGE_TYPE(MESSAGE_TYPE *data)
{
}

size_t ASN1CALL
length_MESSAGE_TYPE(const MESSAGE_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_MESSAGE_TYPE(const MESSAGE_TYPE *from, MESSAGE_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_PADATA_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PADATA_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PADATA_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PADATA_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PADATA_TYPE(data);
return e;
}

void ASN1CALL
free_PADATA_TYPE(PADATA_TYPE *data)
{
}

size_t ASN1CALL
length_PADATA_TYPE(const PADATA_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PADATA_TYPE(const PADATA_TYPE *from, PADATA_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_AUTHDATA_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AUTHDATA_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AUTHDATA_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AUTHDATA_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AUTHDATA_TYPE(data);
return e;
}

void ASN1CALL
free_AUTHDATA_TYPE(AUTHDATA_TYPE *data)
{
}

size_t ASN1CALL
length_AUTHDATA_TYPE(const AUTHDATA_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AUTHDATA_TYPE(const AUTHDATA_TYPE *from, AUTHDATA_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_CKSUMTYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const CKSUMTYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_CKSUMTYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, CKSUMTYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_CKSUMTYPE(data);
return e;
}

void ASN1CALL
free_CKSUMTYPE(CKSUMTYPE *data)
{
}

size_t ASN1CALL
length_CKSUMTYPE(const CKSUMTYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_CKSUMTYPE(const CKSUMTYPE *from, CKSUMTYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_ENCTYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ENCTYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ENCTYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ENCTYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ENCTYPE(data);
return e;
}

void ASN1CALL
free_ENCTYPE(ENCTYPE *data)
{
}

size_t ASN1CALL
length_ENCTYPE(const ENCTYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ENCTYPE(const ENCTYPE *from, ENCTYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_krb5uint32(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const krb5uint32 *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_unsigned(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_krb5uint32(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, krb5uint32 *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_unsigned(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_krb5uint32(data);
return e;
}

void ASN1CALL
free_krb5uint32(krb5uint32 *data)
{
}

size_t ASN1CALL
length_krb5uint32(const krb5uint32 *data)
{
size_t ret = 0;
ret += der_length_unsigned(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_krb5uint32(const krb5uint32 *from, krb5uint32 *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_krb5int32(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const krb5int32 *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_integer(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_krb5int32(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, krb5int32 *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_integer(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_krb5int32(data);
return e;
}

void ASN1CALL
free_krb5int32(krb5int32 *data)
{
}

size_t ASN1CALL
length_krb5int32(const krb5int32 *data)
{
size_t ret = 0;
ret += der_length_integer(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_krb5int32(const krb5int32 *from, krb5int32 *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_KerberosString(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KerberosString *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_general_string(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KerberosString(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KerberosString *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_GeneralString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_general_string(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KerberosString(data);
return e;
}

void ASN1CALL
free_KerberosString(KerberosString *data)
{
der_free_general_string(data);
}

size_t ASN1CALL
length_KerberosString(const KerberosString *data)
{
size_t ret = 0;
ret += der_length_general_string(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KerberosString(const KerberosString *from, KerberosString *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_general_string(from, to)) goto fail;
return 0;
fail:
free_KerberosString(to);
return ENOMEM;
}

int ASN1CALL
encode_Realm(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Realm *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_general_string(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Realm(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Realm *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_GeneralString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_general_string(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Realm(data);
return e;
}

void ASN1CALL
free_Realm(Realm *data)
{
der_free_general_string(data);
}

size_t ASN1CALL
length_Realm(const Realm *data)
{
size_t ret = 0;
ret += der_length_general_string(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Realm(const Realm *from, Realm *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_general_string(from, to)) goto fail;
return 0;
fail:
free_Realm(to);
return ENOMEM;
}

int ASN1CALL
encode_PrincipalName(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PrincipalName *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* name-string */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->name_string)->len - 1; i >= 0; --i) {
size_t name_string_tag_tag_for_oldret = ret;
ret = 0;
e = der_put_general_string(p, len, &(&(data)->name_string)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += name_string_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* name-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_NAME_TYPE(p, len, &(data)->name_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PrincipalName(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PrincipalName *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t name_type_datalen, name_type_oldlen;
Der_type name_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &name_type_type, 0, &name_type_datalen, &l);
if (e == 0 && name_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_type_oldlen = len;
if (name_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_type_datalen;
e = decode_NAME_TYPE(p, len, &(data)->name_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = name_type_oldlen - name_type_datalen;
}
{
size_t name_string_datalen, name_string_oldlen;
Der_type name_string_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &name_string_type, 1, &name_string_datalen, &l);
if (e == 0 && name_string_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_string_oldlen = len;
if (name_string_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_string_datalen;
{
size_t name_string_Tag_datalen, name_string_Tag_oldlen;
Der_type name_string_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &name_string_Tag_type, UT_Sequence, &name_string_Tag_datalen, &l);
if (e == 0 && name_string_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_string_Tag_oldlen = len;
if (name_string_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_string_Tag_datalen;
{
size_t name_string_Tag_Tag_origlen = len;
size_t name_string_Tag_Tag_oldret = ret;
size_t name_string_Tag_Tag_olen = 0;
void *name_string_Tag_Tag_tmp;
ret = 0;
(&(data)->name_string)->len = 0;
(&(data)->name_string)->val = NULL;
while(ret < name_string_Tag_Tag_origlen) {
size_t name_string_Tag_Tag_nlen = name_string_Tag_Tag_olen + sizeof(*((&(data)->name_string)->val));
if (name_string_Tag_Tag_olen > name_string_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
name_string_Tag_Tag_olen = name_string_Tag_Tag_nlen;
name_string_Tag_Tag_tmp = realloc((&(data)->name_string)->val, name_string_Tag_Tag_olen);
if (name_string_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->name_string)->val = name_string_Tag_Tag_tmp;
{
size_t name_string_Tag_Tag_s_of_datalen, name_string_Tag_Tag_s_of_oldlen;
Der_type name_string_Tag_Tag_s_of_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &name_string_Tag_Tag_s_of_type, UT_GeneralString, &name_string_Tag_Tag_s_of_datalen, &l);
if (e == 0 && name_string_Tag_Tag_s_of_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_string_Tag_Tag_s_of_oldlen = len;
if (name_string_Tag_Tag_s_of_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_string_Tag_Tag_s_of_datalen;
e = der_get_general_string(p, len, &(&(data)->name_string)->val[(&(data)->name_string)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = name_string_Tag_Tag_s_of_oldlen - name_string_Tag_Tag_s_of_datalen;
}
(&(data)->name_string)->len++;
len = name_string_Tag_Tag_origlen - ret;
}
ret += name_string_Tag_Tag_oldret;
}
len = name_string_Tag_oldlen - name_string_Tag_datalen;
}
len = name_string_oldlen - name_string_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PrincipalName(data);
return e;
}

void ASN1CALL
free_PrincipalName(PrincipalName *data)
{
free_NAME_TYPE(&(data)->name_type);
while((&(data)->name_string)->len){
der_free_general_string(&(&(data)->name_string)->val[(&(data)->name_string)->len-1]);
(&(data)->name_string)->len--;
}
free((&(data)->name_string)->val);
(&(data)->name_string)->val = NULL;
}

size_t ASN1CALL
length_PrincipalName(const PrincipalName *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_NAME_TYPE(&(data)->name_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t name_string_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->name_string)->len - 1; i >= 0; --i){
size_t name_string_tag_tag_for_oldret = ret;
ret = 0;
ret += der_length_general_string(&(&(data)->name_string)->val[i]);
ret += 1 + der_length_len (ret);
ret += name_string_tag_tag_for_oldret;
}
ret += name_string_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PrincipalName(const PrincipalName *from, PrincipalName *to)
{
memset(to, 0, sizeof(*to));
if(copy_NAME_TYPE(&(from)->name_type, &(to)->name_type)) goto fail;
if(((&(to)->name_string)->val = malloc((&(from)->name_string)->len * sizeof(*(&(to)->name_string)->val))) == NULL && (&(from)->name_string)->len != 0)
goto fail;
for((&(to)->name_string)->len = 0; (&(to)->name_string)->len < (&(from)->name_string)->len; (&(to)->name_string)->len++){
if(der_copy_general_string(&(&(from)->name_string)->val[(&(to)->name_string)->len], &(&(to)->name_string)->val[(&(to)->name_string)->len])) goto fail;
}
return 0;
fail:
free_PrincipalName(to);
return ENOMEM;
}

int ASN1CALL
encode_Principal(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Principal *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* realm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* name */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Principal(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Principal *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t name_datalen, name_oldlen;
Der_type name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &name_type, 0, &name_datalen, &l);
if (e == 0 && name_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_oldlen = len;
if (name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_datalen;
e = decode_PrincipalName(p, len, &(data)->name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = name_oldlen - name_datalen;
}
{
size_t realm_datalen, realm_oldlen;
Der_type realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &realm_type, 1, &realm_datalen, &l);
if (e == 0 && realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
realm_oldlen = len;
if (realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = realm_datalen;
e = decode_Realm(p, len, &(data)->realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = realm_oldlen - realm_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Principal(data);
return e;
}

void ASN1CALL
free_Principal(Principal *data)
{
free_PrincipalName(&(data)->name);
free_Realm(&(data)->realm);
}

size_t ASN1CALL
length_Principal(const Principal *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Principal(const Principal *from, Principal *to)
{
memset(to, 0, sizeof(*to));
if(copy_PrincipalName(&(from)->name, &(to)->name)) goto fail;
if(copy_Realm(&(from)->realm, &(to)->realm)) goto fail;
return 0;
fail:
free_Principal(to);
return ENOMEM;
}

int ASN1CALL
encode_Principals(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Principals *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_Principal(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Principals(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Principals *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_Principal(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Principals(data);
return e;
}

void ASN1CALL
free_Principals(Principals *data)
{
while((data)->len){
free_Principal(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_Principals(const Principals *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_Principal(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Principals(const Principals *from, Principals *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_Principal(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_Principals(to);
return ENOMEM;
}

int ASN1CALL
add_Principals(Principals *data, const Principal *element)
{
int ret;
void *ptr;

ptr = realloc(data->val, 
	(data->len + 1) * sizeof(data->val[0]));
if (ptr == NULL) return ENOMEM;
data->val = ptr;

ret = copy_Principal(element, &data->val[data->len]);
if (ret) return ret;
data->len++;
return 0;
}

int ASN1CALL
remove_Principals(Principals *data, unsigned int element)
{
void *ptr;

if (data->len == 0 || element >= data->len)
	return ASN1_OVERRUN;
free_Principal(&data->val[element]);
data->len--;
if (element < data->len)
	memmove(&data->val[element], &data->val[element + 1], 
		sizeof(data->val[0]) * (data->len - element));
ptr = realloc(data->val, data->len * sizeof(data->val[0]));
if (ptr != NULL || data->len == 0) data->val = ptr;
return 0;
}

int ASN1CALL
encode_HostAddress(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const HostAddress *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* address */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* addr-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->addr_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_HostAddress(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, HostAddress *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t addr_type_datalen, addr_type_oldlen;
Der_type addr_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &addr_type_type, 0, &addr_type_datalen, &l);
if (e == 0 && addr_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
addr_type_oldlen = len;
if (addr_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = addr_type_datalen;
e = decode_krb5int32(p, len, &(data)->addr_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = addr_type_oldlen - addr_type_datalen;
}
{
size_t address_datalen, address_oldlen;
Der_type address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &address_type, 1, &address_datalen, &l);
if (e == 0 && address_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
address_oldlen = len;
if (address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = address_datalen;
{
size_t address_Tag_datalen, address_Tag_oldlen;
Der_type address_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &address_Tag_type, UT_OctetString, &address_Tag_datalen, &l);
if (e == 0 && address_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
address_Tag_oldlen = len;
if (address_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = address_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = address_Tag_oldlen - address_Tag_datalen;
}
len = address_oldlen - address_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_HostAddress(data);
return e;
}

void ASN1CALL
free_HostAddress(HostAddress *data)
{
free_krb5int32(&(data)->addr_type);
der_free_octet_string(&(data)->address);
}

size_t ASN1CALL
length_HostAddress(const HostAddress *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->addr_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->address);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_HostAddress(const HostAddress *from, HostAddress *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->addr_type, &(to)->addr_type)) goto fail;
if(der_copy_octet_string(&(from)->address, &(to)->address)) goto fail;
return 0;
fail:
free_HostAddress(to);
return ENOMEM;
}

int ASN1CALL
encode_HostAddresses(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const HostAddresses *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_HostAddress(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_HostAddresses(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, HostAddresses *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_HostAddress(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_HostAddresses(data);
return e;
}

void ASN1CALL
free_HostAddresses(HostAddresses *data)
{
while((data)->len){
free_HostAddress(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_HostAddresses(const HostAddresses *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_HostAddress(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_HostAddresses(const HostAddresses *from, HostAddresses *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_HostAddress(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_HostAddresses(to);
return ENOMEM;
}

int ASN1CALL
encode_KerberosTime(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KerberosTime *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_generalized_time(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralizedTime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KerberosTime(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KerberosTime *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_GeneralizedTime, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_generalized_time(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KerberosTime(data);
return e;
}

void ASN1CALL
free_KerberosTime(KerberosTime *data)
{
}

size_t ASN1CALL
length_KerberosTime(const KerberosTime *data)
{
size_t ret = 0;
ret += der_length_generalized_time(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KerberosTime(const KerberosTime *from, KerberosTime *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_AuthorizationDataElement(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AuthorizationDataElement *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* ad-data */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->ad_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* ad-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->ad_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AuthorizationDataElement(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AuthorizationDataElement *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t ad_type_datalen, ad_type_oldlen;
Der_type ad_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ad_type_type, 0, &ad_type_datalen, &l);
if (e == 0 && ad_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ad_type_oldlen = len;
if (ad_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ad_type_datalen;
e = decode_krb5int32(p, len, &(data)->ad_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ad_type_oldlen - ad_type_datalen;
}
{
size_t ad_data_datalen, ad_data_oldlen;
Der_type ad_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ad_data_type, 1, &ad_data_datalen, &l);
if (e == 0 && ad_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ad_data_oldlen = len;
if (ad_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ad_data_datalen;
{
size_t ad_data_Tag_datalen, ad_data_Tag_oldlen;
Der_type ad_data_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &ad_data_Tag_type, UT_OctetString, &ad_data_Tag_datalen, &l);
if (e == 0 && ad_data_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ad_data_Tag_oldlen = len;
if (ad_data_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ad_data_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->ad_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ad_data_Tag_oldlen - ad_data_Tag_datalen;
}
len = ad_data_oldlen - ad_data_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AuthorizationDataElement(data);
return e;
}

void ASN1CALL
free_AuthorizationDataElement(AuthorizationDataElement *data)
{
free_krb5int32(&(data)->ad_type);
der_free_octet_string(&(data)->ad_data);
}

size_t ASN1CALL
length_AuthorizationDataElement(const AuthorizationDataElement *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->ad_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->ad_data);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AuthorizationDataElement(const AuthorizationDataElement *from, AuthorizationDataElement *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->ad_type, &(to)->ad_type)) goto fail;
if(der_copy_octet_string(&(from)->ad_data, &(to)->ad_data)) goto fail;
return 0;
fail:
free_AuthorizationDataElement(to);
return ENOMEM;
}

int ASN1CALL
encode_AuthorizationData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AuthorizationData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_AuthorizationDataElement(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AuthorizationData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AuthorizationData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_AuthorizationDataElement(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AuthorizationData(data);
return e;
}

void ASN1CALL
free_AuthorizationData(AuthorizationData *data)
{
while((data)->len){
free_AuthorizationDataElement(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_AuthorizationData(const AuthorizationData *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_AuthorizationDataElement(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AuthorizationData(const AuthorizationData *from, AuthorizationData *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_AuthorizationDataElement(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_AuthorizationData(to);
return ENOMEM;
}

int ASN1CALL
add_AuthorizationData(AuthorizationData *data, const AuthorizationDataElement *element)
{
int ret;
void *ptr;

ptr = realloc(data->val, 
	(data->len + 1) * sizeof(data->val[0]));
if (ptr == NULL) return ENOMEM;
data->val = ptr;

ret = copy_AuthorizationDataElement(element, &data->val[data->len]);
if (ret) return ret;
data->len++;
return 0;
}

int ASN1CALL
remove_AuthorizationData(AuthorizationData *data, unsigned int element)
{
void *ptr;

if (data->len == 0 || element >= data->len)
	return ASN1_OVERRUN;
free_AuthorizationDataElement(&data->val[element]);
data->len--;
if (element < data->len)
	memmove(&data->val[element], &data->val[element + 1], 
		sizeof(data->val[0]) * (data->len - element));
ptr = realloc(data->val, data->len * sizeof(data->val[0]));
if (ptr != NULL || data->len == 0) data->val = ptr;
return 0;
}

int ASN1CALL
encode_APOptions(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const APOptions *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
unsigned char c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->mutual_required) {
c |= 1<<5;
}
if((data)->use_session_key) {
c |= 1<<6;
}
if((data)->reserved) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
if (len < 1) return ASN1_OVERFLOW;
*p-- = 0;
len -= 1;
ret += 1;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_BitString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_APOptions(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, APOptions *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_BitString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
if (len < 1) return ASN1_OVERRUN;
p++; len--; ret++;
do {
if (len < 1) break;
(data)->reserved = (*p >> 7) & 1;
(data)->use_session_key = (*p >> 6) & 1;
(data)->mutual_required = (*p >> 5) & 1;
} while(0);
p += len; ret += len;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_APOptions(data);
return e;
}

void ASN1CALL
free_APOptions(APOptions *data)
{
}

size_t ASN1CALL
length_APOptions(const APOptions *data)
{
size_t ret = 0;
ret += 5;
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_APOptions(const APOptions *from, APOptions *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

unsigned APOptions2int(APOptions f)
{
unsigned r = 0;
if(f.reserved) r |= (1U << 0);
if(f.use_session_key) r |= (1U << 1);
if(f.mutual_required) r |= (1U << 2);
return r;
}

APOptions int2APOptions(unsigned n)
{
	APOptions flags;

	memset(&flags, 0, sizeof(flags));

	flags.reserved = (n >> 0) & 1;
	flags.use_session_key = (n >> 1) & 1;
	flags.mutual_required = (n >> 2) & 1;
	return flags;
}

static struct units APOptions_units[] = {
	{"mutual-required",	1U << 2},
	{"use-session-key",	1U << 1},
	{"reserved",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_APOptions_units(void){
return APOptions_units;
}

int ASN1CALL
encode_TicketFlags(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TicketFlags *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
unsigned char c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->enc_pa_rep) {
c |= 1<<0;
}
if((data)->anonymous) {
c |= 1<<1;
}
if((data)->ok_as_delegate) {
c |= 1<<2;
}
if((data)->transited_policy_checked) {
c |= 1<<3;
}
if((data)->hw_authent) {
c |= 1<<4;
}
if((data)->pre_authent) {
c |= 1<<5;
}
if((data)->initial) {
c |= 1<<6;
}
if((data)->renewable) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->invalid) {
c |= 1<<0;
}
if((data)->postdated) {
c |= 1<<1;
}
if((data)->may_postdate) {
c |= 1<<2;
}
if((data)->proxy) {
c |= 1<<3;
}
if((data)->proxiable) {
c |= 1<<4;
}
if((data)->forwarded) {
c |= 1<<5;
}
if((data)->forwardable) {
c |= 1<<6;
}
if((data)->reserved) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
if (len < 1) return ASN1_OVERFLOW;
*p-- = 0;
len -= 1;
ret += 1;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_BitString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TicketFlags(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TicketFlags *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_BitString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
if (len < 1) return ASN1_OVERRUN;
p++; len--; ret++;
do {
if (len < 1) break;
(data)->reserved = (*p >> 7) & 1;
(data)->forwardable = (*p >> 6) & 1;
(data)->forwarded = (*p >> 5) & 1;
(data)->proxiable = (*p >> 4) & 1;
(data)->proxy = (*p >> 3) & 1;
(data)->may_postdate = (*p >> 2) & 1;
(data)->postdated = (*p >> 1) & 1;
(data)->invalid = (*p >> 0) & 1;
p++; len--; ret++;
if (len < 1) break;
(data)->renewable = (*p >> 7) & 1;
(data)->initial = (*p >> 6) & 1;
(data)->pre_authent = (*p >> 5) & 1;
(data)->hw_authent = (*p >> 4) & 1;
(data)->transited_policy_checked = (*p >> 3) & 1;
(data)->ok_as_delegate = (*p >> 2) & 1;
(data)->anonymous = (*p >> 1) & 1;
(data)->enc_pa_rep = (*p >> 0) & 1;
} while(0);
p += len; ret += len;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TicketFlags(data);
return e;
}

void ASN1CALL
free_TicketFlags(TicketFlags *data)
{
}

size_t ASN1CALL
length_TicketFlags(const TicketFlags *data)
{
size_t ret = 0;
ret += 5;
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TicketFlags(const TicketFlags *from, TicketFlags *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

unsigned TicketFlags2int(TicketFlags f)
{
unsigned r = 0;
if(f.reserved) r |= (1U << 0);
if(f.forwardable) r |= (1U << 1);
if(f.forwarded) r |= (1U << 2);
if(f.proxiable) r |= (1U << 3);
if(f.proxy) r |= (1U << 4);
if(f.may_postdate) r |= (1U << 5);
if(f.postdated) r |= (1U << 6);
if(f.invalid) r |= (1U << 7);
if(f.renewable) r |= (1U << 8);
if(f.initial) r |= (1U << 9);
if(f.pre_authent) r |= (1U << 10);
if(f.hw_authent) r |= (1U << 11);
if(f.transited_policy_checked) r |= (1U << 12);
if(f.ok_as_delegate) r |= (1U << 13);
if(f.anonymous) r |= (1U << 14);
if(f.enc_pa_rep) r |= (1U << 15);
return r;
}

TicketFlags int2TicketFlags(unsigned n)
{
	TicketFlags flags;

	memset(&flags, 0, sizeof(flags));

	flags.reserved = (n >> 0) & 1;
	flags.forwardable = (n >> 1) & 1;
	flags.forwarded = (n >> 2) & 1;
	flags.proxiable = (n >> 3) & 1;
	flags.proxy = (n >> 4) & 1;
	flags.may_postdate = (n >> 5) & 1;
	flags.postdated = (n >> 6) & 1;
	flags.invalid = (n >> 7) & 1;
	flags.renewable = (n >> 8) & 1;
	flags.initial = (n >> 9) & 1;
	flags.pre_authent = (n >> 10) & 1;
	flags.hw_authent = (n >> 11) & 1;
	flags.transited_policy_checked = (n >> 12) & 1;
	flags.ok_as_delegate = (n >> 13) & 1;
	flags.anonymous = (n >> 14) & 1;
	flags.enc_pa_rep = (n >> 15) & 1;
	return flags;
}

static struct units TicketFlags_units[] = {
	{"enc-pa-rep",	1U << 15},
	{"anonymous",	1U << 14},
	{"ok-as-delegate",	1U << 13},
	{"transited-policy-checked",	1U << 12},
	{"hw-authent",	1U << 11},
	{"pre-authent",	1U << 10},
	{"initial",	1U << 9},
	{"renewable",	1U << 8},
	{"invalid",	1U << 7},
	{"postdated",	1U << 6},
	{"may-postdate",	1U << 5},
	{"proxy",	1U << 4},
	{"proxiable",	1U << 3},
	{"forwarded",	1U << 2},
	{"forwardable",	1U << 1},
	{"reserved",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_TicketFlags_units(void){
return TicketFlags_units;
}

int ASN1CALL
encode_KDCOptions(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KDCOptions *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
unsigned char c = 0;
if((data)->validate) {
c |= 1<<0;
}
if((data)->renew) {
c |= 1<<1;
}
if((data)->enc_tkt_in_skey) {
c |= 1<<3;
}
if((data)->renewable_ok) {
c |= 1<<4;
}
if((data)->disable_transited_check) {
c |= 1<<5;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->constrained_delegation) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->canonicalize) {
c |= 1<<0;
}
if((data)->request_anonymous) {
c |= 1<<1;
}
if((data)->renewable) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->postdated) {
c |= 1<<1;
}
if((data)->allow_postdate) {
c |= 1<<2;
}
if((data)->proxy) {
c |= 1<<3;
}
if((data)->proxiable) {
c |= 1<<4;
}
if((data)->forwarded) {
c |= 1<<5;
}
if((data)->forwardable) {
c |= 1<<6;
}
if((data)->reserved) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
if (len < 1) return ASN1_OVERFLOW;
*p-- = 0;
len -= 1;
ret += 1;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_BitString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KDCOptions(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KDCOptions *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_BitString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
if (len < 1) return ASN1_OVERRUN;
p++; len--; ret++;
do {
if (len < 1) break;
(data)->reserved = (*p >> 7) & 1;
(data)->forwardable = (*p >> 6) & 1;
(data)->forwarded = (*p >> 5) & 1;
(data)->proxiable = (*p >> 4) & 1;
(data)->proxy = (*p >> 3) & 1;
(data)->allow_postdate = (*p >> 2) & 1;
(data)->postdated = (*p >> 1) & 1;
p++; len--; ret++;
if (len < 1) break;
(data)->renewable = (*p >> 7) & 1;
(data)->request_anonymous = (*p >> 1) & 1;
(data)->canonicalize = (*p >> 0) & 1;
p++; len--; ret++;
if (len < 1) break;
(data)->constrained_delegation = (*p >> 7) & 1;
p++; len--; ret++;
if (len < 1) break;
(data)->disable_transited_check = (*p >> 5) & 1;
(data)->renewable_ok = (*p >> 4) & 1;
(data)->enc_tkt_in_skey = (*p >> 3) & 1;
(data)->renew = (*p >> 1) & 1;
(data)->validate = (*p >> 0) & 1;
} while(0);
p += len; ret += len;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KDCOptions(data);
return e;
}

void ASN1CALL
free_KDCOptions(KDCOptions *data)
{
}

size_t ASN1CALL
length_KDCOptions(const KDCOptions *data)
{
size_t ret = 0;
ret += 5;
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KDCOptions(const KDCOptions *from, KDCOptions *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

unsigned KDCOptions2int(KDCOptions f)
{
unsigned r = 0;
if(f.reserved) r |= (1U << 0);
if(f.forwardable) r |= (1U << 1);
if(f.forwarded) r |= (1U << 2);
if(f.proxiable) r |= (1U << 3);
if(f.proxy) r |= (1U << 4);
if(f.allow_postdate) r |= (1U << 5);
if(f.postdated) r |= (1U << 6);
if(f.renewable) r |= (1U << 8);
if(f.request_anonymous) r |= (1U << 14);
if(f.canonicalize) r |= (1U << 15);
if(f.constrained_delegation) r |= (1U << 16);
if(f.disable_transited_check) r |= (1U << 26);
if(f.renewable_ok) r |= (1U << 27);
if(f.enc_tkt_in_skey) r |= (1U << 28);
if(f.renew) r |= (1U << 30);
if(f.validate) r |= (1U << 31);
return r;
}

KDCOptions int2KDCOptions(unsigned n)
{
	KDCOptions flags;

	memset(&flags, 0, sizeof(flags));

	flags.reserved = (n >> 0) & 1;
	flags.forwardable = (n >> 1) & 1;
	flags.forwarded = (n >> 2) & 1;
	flags.proxiable = (n >> 3) & 1;
	flags.proxy = (n >> 4) & 1;
	flags.allow_postdate = (n >> 5) & 1;
	flags.postdated = (n >> 6) & 1;
	flags.renewable = (n >> 8) & 1;
	flags.request_anonymous = (n >> 14) & 1;
	flags.canonicalize = (n >> 15) & 1;
	flags.constrained_delegation = (n >> 16) & 1;
	flags.disable_transited_check = (n >> 26) & 1;
	flags.renewable_ok = (n >> 27) & 1;
	flags.enc_tkt_in_skey = (n >> 28) & 1;
	flags.renew = (n >> 30) & 1;
	flags.validate = (n >> 31) & 1;
	return flags;
}

static struct units KDCOptions_units[] = {
	{"validate",	1U << 31},
	{"renew",	1U << 30},
	{"enc-tkt-in-skey",	1U << 28},
	{"renewable-ok",	1U << 27},
	{"disable-transited-check",	1U << 26},
	{"constrained-delegation",	1U << 16},
	{"canonicalize",	1U << 15},
	{"request-anonymous",	1U << 14},
	{"renewable",	1U << 8},
	{"postdated",	1U << 6},
	{"allow-postdate",	1U << 5},
	{"proxy",	1U << 4},
	{"proxiable",	1U << 3},
	{"forwarded",	1U << 2},
	{"forwardable",	1U << 1},
	{"reserved",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_KDCOptions_units(void){
return KDCOptions_units;
}

int ASN1CALL
encode_LR_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const LR_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_LR_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, LR_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_LR_TYPE(data);
return e;
}

void ASN1CALL
free_LR_TYPE(LR_TYPE *data)
{
}

size_t ASN1CALL
length_LR_TYPE(const LR_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_LR_TYPE(const LR_TYPE *from, LR_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_LastReq(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const LastReq *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
/* lr-value */
{
size_t Top_tag_S_Of_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(&(data)->val[i])->lr_value, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_S_Of_tag_oldret;
}
/* lr-type */
{
size_t Top_tag_S_Of_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_LR_TYPE(p, len, &(&(data)->val[i])->lr_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_S_Of_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_LastReq(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, LastReq *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
{
size_t Top_Tag_s_of_datalen, Top_Tag_s_of_oldlen;
Der_type Top_Tag_s_of_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_s_of_type, UT_Sequence, &Top_Tag_s_of_datalen, &l);
if (e == 0 && Top_Tag_s_of_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_s_of_oldlen = len;
if (Top_Tag_s_of_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_s_of_datalen;
{
size_t lr_type_datalen, lr_type_oldlen;
Der_type lr_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &lr_type_type, 0, &lr_type_datalen, &l);
if (e == 0 && lr_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
lr_type_oldlen = len;
if (lr_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = lr_type_datalen;
e = decode_LR_TYPE(p, len, &(&(data)->val[(data)->len])->lr_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = lr_type_oldlen - lr_type_datalen;
}
{
size_t lr_value_datalen, lr_value_oldlen;
Der_type lr_value_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &lr_value_type, 1, &lr_value_datalen, &l);
if (e == 0 && lr_value_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
lr_value_oldlen = len;
if (lr_value_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = lr_value_datalen;
e = decode_KerberosTime(p, len, &(&(data)->val[(data)->len])->lr_value, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = lr_value_oldlen - lr_value_datalen;
}
len = Top_Tag_s_of_oldlen - Top_Tag_s_of_datalen;
}
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_LastReq(data);
return e;
}

void ASN1CALL
free_LastReq(LastReq *data)
{
while((data)->len){
free_LR_TYPE(&(&(data)->val[(data)->len-1])->lr_type);
free_KerberosTime(&(&(data)->val[(data)->len-1])->lr_value);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_LastReq(const LastReq *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
{
size_t Top_tag_S_Of_tag_oldret = ret;
ret = 0;
ret += length_LR_TYPE(&(&(data)->val[i])->lr_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_S_Of_tag_oldret;
}
{
size_t Top_tag_S_Of_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(&(data)->val[i])->lr_value);
ret += 1 + der_length_len (ret);
ret += Top_tag_S_Of_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_LastReq(const LastReq *from, LastReq *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_LR_TYPE(&(&(from)->val[(to)->len])->lr_type, &(&(to)->val[(to)->len])->lr_type)) goto fail;
if(copy_KerberosTime(&(&(from)->val[(to)->len])->lr_value, &(&(to)->val[(to)->len])->lr_value)) goto fail;
}
return 0;
fail:
free_LastReq(to);
return ENOMEM;
}

int ASN1CALL
encode_EncryptedData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncryptedData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* cipher */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->cipher, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* kvno */
if((data)->kvno) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5uint32(p, len, (data)->kvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(data)->etype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncryptedData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncryptedData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t etype_datalen, etype_oldlen;
Der_type etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &etype_type, 0, &etype_datalen, &l);
if (e == 0 && etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_oldlen = len;
if (etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_datalen;
e = decode_ENCTYPE(p, len, &(data)->etype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = etype_oldlen - etype_datalen;
}
{
size_t kvno_datalen, kvno_oldlen;
Der_type kvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &kvno_type, 1, &kvno_datalen, &l);
if (e == 0 && kvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->kvno = NULL;
} else {
(data)->kvno = calloc(1, sizeof(*(data)->kvno));
if ((data)->kvno == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
kvno_oldlen = len;
if (kvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = kvno_datalen;
e = decode_krb5uint32(p, len, (data)->kvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = kvno_oldlen - kvno_datalen;
}
}
{
size_t cipher_datalen, cipher_oldlen;
Der_type cipher_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cipher_type, 2, &cipher_datalen, &l);
if (e == 0 && cipher_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cipher_oldlen = len;
if (cipher_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cipher_datalen;
{
size_t cipher_Tag_datalen, cipher_Tag_oldlen;
Der_type cipher_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &cipher_Tag_type, UT_OctetString, &cipher_Tag_datalen, &l);
if (e == 0 && cipher_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cipher_Tag_oldlen = len;
if (cipher_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cipher_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->cipher, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cipher_Tag_oldlen - cipher_Tag_datalen;
}
len = cipher_oldlen - cipher_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncryptedData(data);
return e;
}

void ASN1CALL
free_EncryptedData(EncryptedData *data)
{
free_ENCTYPE(&(data)->etype);
if((data)->kvno) {
free_krb5uint32((data)->kvno);
free((data)->kvno);
(data)->kvno = NULL;
}
der_free_octet_string(&(data)->cipher);
}

size_t ASN1CALL
length_EncryptedData(const EncryptedData *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(data)->etype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->kvno){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5uint32((data)->kvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->cipher);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncryptedData(const EncryptedData *from, EncryptedData *to)
{
memset(to, 0, sizeof(*to));
if(copy_ENCTYPE(&(from)->etype, &(to)->etype)) goto fail;
if((from)->kvno) {
(to)->kvno = malloc(sizeof(*(to)->kvno));
if((to)->kvno == NULL) goto fail;
if(copy_krb5uint32((from)->kvno, (to)->kvno)) goto fail;
}else
(to)->kvno = NULL;
if(der_copy_octet_string(&(from)->cipher, &(to)->cipher)) goto fail;
return 0;
fail:
free_EncryptedData(to);
return ENOMEM;
}

int ASN1CALL
encode_EncryptionKey(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncryptionKey *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* keyvalue */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->keyvalue, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* keytype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->keytype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncryptionKey(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncryptionKey *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t keytype_datalen, keytype_oldlen;
Der_type keytype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &keytype_type, 0, &keytype_datalen, &l);
if (e == 0 && keytype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
keytype_oldlen = len;
if (keytype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = keytype_datalen;
e = decode_krb5int32(p, len, &(data)->keytype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = keytype_oldlen - keytype_datalen;
}
{
size_t keyvalue_datalen, keyvalue_oldlen;
Der_type keyvalue_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &keyvalue_type, 1, &keyvalue_datalen, &l);
if (e == 0 && keyvalue_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
keyvalue_oldlen = len;
if (keyvalue_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = keyvalue_datalen;
{
size_t keyvalue_Tag_datalen, keyvalue_Tag_oldlen;
Der_type keyvalue_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &keyvalue_Tag_type, UT_OctetString, &keyvalue_Tag_datalen, &l);
if (e == 0 && keyvalue_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
keyvalue_Tag_oldlen = len;
if (keyvalue_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = keyvalue_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->keyvalue, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = keyvalue_Tag_oldlen - keyvalue_Tag_datalen;
}
len = keyvalue_oldlen - keyvalue_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncryptionKey(data);
return e;
}

void ASN1CALL
free_EncryptionKey(EncryptionKey *data)
{
free_krb5int32(&(data)->keytype);
der_free_octet_string(&(data)->keyvalue);
}

size_t ASN1CALL
length_EncryptionKey(const EncryptionKey *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->keytype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->keyvalue);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncryptionKey(const EncryptionKey *from, EncryptionKey *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->keytype, &(to)->keytype)) goto fail;
if(der_copy_octet_string(&(from)->keyvalue, &(to)->keyvalue)) goto fail;
return 0;
fail:
free_EncryptionKey(to);
return ENOMEM;
}

int ASN1CALL
encode_TransitedEncoding(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TransitedEncoding *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* contents */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->contents, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* tr-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->tr_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TransitedEncoding(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TransitedEncoding *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t tr_type_datalen, tr_type_oldlen;
Der_type tr_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &tr_type_type, 0, &tr_type_datalen, &l);
if (e == 0 && tr_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
tr_type_oldlen = len;
if (tr_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = tr_type_datalen;
e = decode_krb5int32(p, len, &(data)->tr_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = tr_type_oldlen - tr_type_datalen;
}
{
size_t contents_datalen, contents_oldlen;
Der_type contents_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &contents_type, 1, &contents_datalen, &l);
if (e == 0 && contents_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
contents_oldlen = len;
if (contents_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = contents_datalen;
{
size_t contents_Tag_datalen, contents_Tag_oldlen;
Der_type contents_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &contents_Tag_type, UT_OctetString, &contents_Tag_datalen, &l);
if (e == 0 && contents_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
contents_Tag_oldlen = len;
if (contents_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = contents_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->contents, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = contents_Tag_oldlen - contents_Tag_datalen;
}
len = contents_oldlen - contents_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TransitedEncoding(data);
return e;
}

void ASN1CALL
free_TransitedEncoding(TransitedEncoding *data)
{
free_krb5int32(&(data)->tr_type);
der_free_octet_string(&(data)->contents);
}

size_t ASN1CALL
length_TransitedEncoding(const TransitedEncoding *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->tr_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->contents);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TransitedEncoding(const TransitedEncoding *from, TransitedEncoding *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->tr_type, &(to)->tr_type)) goto fail;
if(der_copy_octet_string(&(from)->contents, &(to)->contents)) goto fail;
return 0;
fail:
free_TransitedEncoding(to);
return ENOMEM;
}

int ASN1CALL
encode_Ticket(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Ticket *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-part */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_part, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* sname */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* realm */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* tkt-vno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->tkt_vno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Ticket(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Ticket *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 1, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t tkt_vno_datalen, tkt_vno_oldlen;
Der_type tkt_vno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &tkt_vno_type, 0, &tkt_vno_datalen, &l);
if (e == 0 && tkt_vno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
tkt_vno_oldlen = len;
if (tkt_vno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = tkt_vno_datalen;
e = decode_krb5int32(p, len, &(data)->tkt_vno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = tkt_vno_oldlen - tkt_vno_datalen;
}
{
size_t realm_datalen, realm_oldlen;
Der_type realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &realm_type, 1, &realm_datalen, &l);
if (e == 0 && realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
realm_oldlen = len;
if (realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = realm_datalen;
e = decode_Realm(p, len, &(data)->realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = realm_oldlen - realm_datalen;
}
{
size_t sname_datalen, sname_oldlen;
Der_type sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sname_type, 2, &sname_datalen, &l);
if (e == 0 && sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sname_oldlen = len;
if (sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sname_datalen;
e = decode_PrincipalName(p, len, &(data)->sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sname_oldlen - sname_datalen;
}
{
size_t enc_part_datalen, enc_part_oldlen;
Der_type enc_part_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_part_type, 3, &enc_part_datalen, &l);
if (e == 0 && enc_part_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_part_oldlen = len;
if (enc_part_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_part_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_part, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_part_oldlen - enc_part_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Ticket(data);
return e;
}

void ASN1CALL
free_Ticket(Ticket *data)
{
free_krb5int32(&(data)->tkt_vno);
free_Realm(&(data)->realm);
free_PrincipalName(&(data)->sname);
free_EncryptedData(&(data)->enc_part);
}

size_t ASN1CALL
length_Ticket(const Ticket *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->tkt_vno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_part);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Ticket(const Ticket *from, Ticket *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->tkt_vno, &(to)->tkt_vno)) goto fail;
if(copy_Realm(&(from)->realm, &(to)->realm)) goto fail;
if(copy_PrincipalName(&(from)->sname, &(to)->sname)) goto fail;
if(copy_EncryptedData(&(from)->enc_part, &(to)->enc_part)) goto fail;
return 0;
fail:
free_Ticket(to);
return ENOMEM;
}

int ASN1CALL
encode_EncTicketPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncTicketPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* authorization-data */
if((data)->authorization_data) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_AuthorizationData(p, len, (data)->authorization_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* caddr */
if((data)->caddr) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddresses(p, len, (data)->caddr, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* renew-till */
if((data)->renew_till) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->renew_till, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* endtime */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->endtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* starttime */
if((data)->starttime) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->starttime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* authtime */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->authtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* transited */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_TransitedEncoding(p, len, &(data)->transited, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cname */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* crealm */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->crealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* key */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, &(data)->key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* flags */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_TicketFlags(p, len, &(data)->flags, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncTicketPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncTicketPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 3, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t flags_datalen, flags_oldlen;
Der_type flags_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &flags_type, 0, &flags_datalen, &l);
if (e == 0 && flags_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
flags_oldlen = len;
if (flags_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = flags_datalen;
e = decode_TicketFlags(p, len, &(data)->flags, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = flags_oldlen - flags_datalen;
}
{
size_t key_datalen, key_oldlen;
Der_type key_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &key_type, 1, &key_datalen, &l);
if (e == 0 && key_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
key_oldlen = len;
if (key_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = key_datalen;
e = decode_EncryptionKey(p, len, &(data)->key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_oldlen - key_datalen;
}
{
size_t crealm_datalen, crealm_oldlen;
Der_type crealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &crealm_type, 2, &crealm_datalen, &l);
if (e == 0 && crealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
crealm_oldlen = len;
if (crealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = crealm_datalen;
e = decode_Realm(p, len, &(data)->crealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = crealm_oldlen - crealm_datalen;
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 3, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, &(data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
{
size_t transited_datalen, transited_oldlen;
Der_type transited_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &transited_type, 4, &transited_datalen, &l);
if (e == 0 && transited_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
transited_oldlen = len;
if (transited_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = transited_datalen;
e = decode_TransitedEncoding(p, len, &(data)->transited, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = transited_oldlen - transited_datalen;
}
{
size_t authtime_datalen, authtime_oldlen;
Der_type authtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authtime_type, 5, &authtime_datalen, &l);
if (e == 0 && authtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
authtime_oldlen = len;
if (authtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authtime_datalen;
e = decode_KerberosTime(p, len, &(data)->authtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authtime_oldlen - authtime_datalen;
}
{
size_t starttime_datalen, starttime_oldlen;
Der_type starttime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &starttime_type, 6, &starttime_datalen, &l);
if (e == 0 && starttime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->starttime = NULL;
} else {
(data)->starttime = calloc(1, sizeof(*(data)->starttime));
if ((data)->starttime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
starttime_oldlen = len;
if (starttime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = starttime_datalen;
e = decode_KerberosTime(p, len, (data)->starttime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = starttime_oldlen - starttime_datalen;
}
}
{
size_t endtime_datalen, endtime_oldlen;
Der_type endtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &endtime_type, 7, &endtime_datalen, &l);
if (e == 0 && endtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
endtime_oldlen = len;
if (endtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = endtime_datalen;
e = decode_KerberosTime(p, len, &(data)->endtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = endtime_oldlen - endtime_datalen;
}
{
size_t renew_till_datalen, renew_till_oldlen;
Der_type renew_till_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &renew_till_type, 8, &renew_till_datalen, &l);
if (e == 0 && renew_till_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->renew_till = NULL;
} else {
(data)->renew_till = calloc(1, sizeof(*(data)->renew_till));
if ((data)->renew_till == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
renew_till_oldlen = len;
if (renew_till_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = renew_till_datalen;
e = decode_KerberosTime(p, len, (data)->renew_till, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = renew_till_oldlen - renew_till_datalen;
}
}
{
size_t caddr_datalen, caddr_oldlen;
Der_type caddr_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &caddr_type, 9, &caddr_datalen, &l);
if (e == 0 && caddr_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->caddr = NULL;
} else {
(data)->caddr = calloc(1, sizeof(*(data)->caddr));
if ((data)->caddr == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
caddr_oldlen = len;
if (caddr_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = caddr_datalen;
e = decode_HostAddresses(p, len, (data)->caddr, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = caddr_oldlen - caddr_datalen;
}
}
{
size_t authorization_data_datalen, authorization_data_oldlen;
Der_type authorization_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authorization_data_type, 10, &authorization_data_datalen, &l);
if (e == 0 && authorization_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->authorization_data = NULL;
} else {
(data)->authorization_data = calloc(1, sizeof(*(data)->authorization_data));
if ((data)->authorization_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
authorization_data_oldlen = len;
if (authorization_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authorization_data_datalen;
e = decode_AuthorizationData(p, len, (data)->authorization_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authorization_data_oldlen - authorization_data_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncTicketPart(data);
return e;
}

void ASN1CALL
free_EncTicketPart(EncTicketPart *data)
{
free_TicketFlags(&(data)->flags);
free_EncryptionKey(&(data)->key);
free_Realm(&(data)->crealm);
free_PrincipalName(&(data)->cname);
free_TransitedEncoding(&(data)->transited);
free_KerberosTime(&(data)->authtime);
if((data)->starttime) {
free_KerberosTime((data)->starttime);
free((data)->starttime);
(data)->starttime = NULL;
}
free_KerberosTime(&(data)->endtime);
if((data)->renew_till) {
free_KerberosTime((data)->renew_till);
free((data)->renew_till);
(data)->renew_till = NULL;
}
if((data)->caddr) {
free_HostAddresses((data)->caddr);
free((data)->caddr);
(data)->caddr = NULL;
}
if((data)->authorization_data) {
free_AuthorizationData((data)->authorization_data);
free((data)->authorization_data);
(data)->authorization_data = NULL;
}
}

size_t ASN1CALL
length_EncTicketPart(const EncTicketPart *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_TicketFlags(&(data)->flags);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey(&(data)->key);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->crealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_TransitedEncoding(&(data)->transited);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->authtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->starttime){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->starttime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->endtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->renew_till){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->renew_till);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->caddr){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_HostAddresses((data)->caddr);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->authorization_data){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_AuthorizationData((data)->authorization_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncTicketPart(const EncTicketPart *from, EncTicketPart *to)
{
memset(to, 0, sizeof(*to));
if(copy_TicketFlags(&(from)->flags, &(to)->flags)) goto fail;
if(copy_EncryptionKey(&(from)->key, &(to)->key)) goto fail;
if(copy_Realm(&(from)->crealm, &(to)->crealm)) goto fail;
if(copy_PrincipalName(&(from)->cname, &(to)->cname)) goto fail;
if(copy_TransitedEncoding(&(from)->transited, &(to)->transited)) goto fail;
if(copy_KerberosTime(&(from)->authtime, &(to)->authtime)) goto fail;
if((from)->starttime) {
(to)->starttime = malloc(sizeof(*(to)->starttime));
if((to)->starttime == NULL) goto fail;
if(copy_KerberosTime((from)->starttime, (to)->starttime)) goto fail;
}else
(to)->starttime = NULL;
if(copy_KerberosTime(&(from)->endtime, &(to)->endtime)) goto fail;
if((from)->renew_till) {
(to)->renew_till = malloc(sizeof(*(to)->renew_till));
if((to)->renew_till == NULL) goto fail;
if(copy_KerberosTime((from)->renew_till, (to)->renew_till)) goto fail;
}else
(to)->renew_till = NULL;
if((from)->caddr) {
(to)->caddr = malloc(sizeof(*(to)->caddr));
if((to)->caddr == NULL) goto fail;
if(copy_HostAddresses((from)->caddr, (to)->caddr)) goto fail;
}else
(to)->caddr = NULL;
if((from)->authorization_data) {
(to)->authorization_data = malloc(sizeof(*(to)->authorization_data));
if((to)->authorization_data == NULL) goto fail;
if(copy_AuthorizationData((from)->authorization_data, (to)->authorization_data)) goto fail;
}else
(to)->authorization_data = NULL;
return 0;
fail:
free_EncTicketPart(to);
return ENOMEM;
}

int ASN1CALL
encode_Checksum(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Checksum *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cksumtype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_CKSUMTYPE(p, len, &(data)->cksumtype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Checksum(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Checksum *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t cksumtype_datalen, cksumtype_oldlen;
Der_type cksumtype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cksumtype_type, 0, &cksumtype_datalen, &l);
if (e == 0 && cksumtype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cksumtype_oldlen = len;
if (cksumtype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cksumtype_datalen;
e = decode_CKSUMTYPE(p, len, &(data)->cksumtype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cksumtype_oldlen - cksumtype_datalen;
}
{
size_t checksum_datalen, checksum_oldlen;
Der_type checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &checksum_type, 1, &checksum_datalen, &l);
if (e == 0 && checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
checksum_oldlen = len;
if (checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = checksum_datalen;
{
size_t checksum_Tag_datalen, checksum_Tag_oldlen;
Der_type checksum_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &checksum_Tag_type, UT_OctetString, &checksum_Tag_datalen, &l);
if (e == 0 && checksum_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
checksum_Tag_oldlen = len;
if (checksum_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = checksum_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = checksum_Tag_oldlen - checksum_Tag_datalen;
}
len = checksum_oldlen - checksum_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Checksum(data);
return e;
}

void ASN1CALL
free_Checksum(Checksum *data)
{
free_CKSUMTYPE(&(data)->cksumtype);
der_free_octet_string(&(data)->checksum);
}

size_t ASN1CALL
length_Checksum(const Checksum *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_CKSUMTYPE(&(data)->cksumtype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->checksum);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Checksum(const Checksum *from, Checksum *to)
{
memset(to, 0, sizeof(*to));
if(copy_CKSUMTYPE(&(from)->cksumtype, &(to)->cksumtype)) goto fail;
if(der_copy_octet_string(&(from)->checksum, &(to)->checksum)) goto fail;
return 0;
fail:
free_Checksum(to);
return ENOMEM;
}

int ASN1CALL
encode_Authenticator(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Authenticator *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* authorization-data */
if((data)->authorization_data) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_AuthorizationData(p, len, (data)->authorization_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* seq-number */
if((data)->seq_number) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5uint32(p, len, (data)->seq_number, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* subkey */
if((data)->subkey) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, (data)->subkey, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ctime */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->ctime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cusec */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->cusec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cksum */
if((data)->cksum) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, (data)->cksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cname */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* crealm */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->crealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* authenticator-vno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->authenticator_vno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_Authenticator(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Authenticator *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 2, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t authenticator_vno_datalen, authenticator_vno_oldlen;
Der_type authenticator_vno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authenticator_vno_type, 0, &authenticator_vno_datalen, &l);
if (e == 0 && authenticator_vno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
authenticator_vno_oldlen = len;
if (authenticator_vno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authenticator_vno_datalen;
e = decode_krb5int32(p, len, &(data)->authenticator_vno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authenticator_vno_oldlen - authenticator_vno_datalen;
}
{
size_t crealm_datalen, crealm_oldlen;
Der_type crealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &crealm_type, 1, &crealm_datalen, &l);
if (e == 0 && crealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
crealm_oldlen = len;
if (crealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = crealm_datalen;
e = decode_Realm(p, len, &(data)->crealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = crealm_oldlen - crealm_datalen;
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 2, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, &(data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
{
size_t cksum_datalen, cksum_oldlen;
Der_type cksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cksum_type, 3, &cksum_datalen, &l);
if (e == 0 && cksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->cksum = NULL;
} else {
(data)->cksum = calloc(1, sizeof(*(data)->cksum));
if ((data)->cksum == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
cksum_oldlen = len;
if (cksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cksum_datalen;
e = decode_Checksum(p, len, (data)->cksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cksum_oldlen - cksum_datalen;
}
}
{
size_t cusec_datalen, cusec_oldlen;
Der_type cusec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cusec_type, 4, &cusec_datalen, &l);
if (e == 0 && cusec_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cusec_oldlen = len;
if (cusec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cusec_datalen;
e = decode_krb5int32(p, len, &(data)->cusec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cusec_oldlen - cusec_datalen;
}
{
size_t ctime_datalen, ctime_oldlen;
Der_type ctime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ctime_type, 5, &ctime_datalen, &l);
if (e == 0 && ctime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ctime_oldlen = len;
if (ctime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ctime_datalen;
e = decode_KerberosTime(p, len, &(data)->ctime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ctime_oldlen - ctime_datalen;
}
{
size_t subkey_datalen, subkey_oldlen;
Der_type subkey_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &subkey_type, 6, &subkey_datalen, &l);
if (e == 0 && subkey_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->subkey = NULL;
} else {
(data)->subkey = calloc(1, sizeof(*(data)->subkey));
if ((data)->subkey == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
subkey_oldlen = len;
if (subkey_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = subkey_datalen;
e = decode_EncryptionKey(p, len, (data)->subkey, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = subkey_oldlen - subkey_datalen;
}
}
{
size_t seq_number_datalen, seq_number_oldlen;
Der_type seq_number_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &seq_number_type, 7, &seq_number_datalen, &l);
if (e == 0 && seq_number_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->seq_number = NULL;
} else {
(data)->seq_number = calloc(1, sizeof(*(data)->seq_number));
if ((data)->seq_number == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
seq_number_oldlen = len;
if (seq_number_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = seq_number_datalen;
e = decode_krb5uint32(p, len, (data)->seq_number, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = seq_number_oldlen - seq_number_datalen;
}
}
{
size_t authorization_data_datalen, authorization_data_oldlen;
Der_type authorization_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authorization_data_type, 8, &authorization_data_datalen, &l);
if (e == 0 && authorization_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->authorization_data = NULL;
} else {
(data)->authorization_data = calloc(1, sizeof(*(data)->authorization_data));
if ((data)->authorization_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
authorization_data_oldlen = len;
if (authorization_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authorization_data_datalen;
e = decode_AuthorizationData(p, len, (data)->authorization_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authorization_data_oldlen - authorization_data_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_Authenticator(data);
return e;
}

void ASN1CALL
free_Authenticator(Authenticator *data)
{
free_krb5int32(&(data)->authenticator_vno);
free_Realm(&(data)->crealm);
free_PrincipalName(&(data)->cname);
if((data)->cksum) {
free_Checksum((data)->cksum);
free((data)->cksum);
(data)->cksum = NULL;
}
free_krb5int32(&(data)->cusec);
free_KerberosTime(&(data)->ctime);
if((data)->subkey) {
free_EncryptionKey((data)->subkey);
free((data)->subkey);
(data)->subkey = NULL;
}
if((data)->seq_number) {
free_krb5uint32((data)->seq_number);
free((data)->seq_number);
(data)->seq_number = NULL;
}
if((data)->authorization_data) {
free_AuthorizationData((data)->authorization_data);
free((data)->authorization_data);
(data)->authorization_data = NULL;
}
}

size_t ASN1CALL
length_Authenticator(const Authenticator *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->authenticator_vno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->crealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->cksum){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Checksum((data)->cksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->cusec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->ctime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->subkey){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey((data)->subkey);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->seq_number){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5uint32((data)->seq_number);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->authorization_data){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_AuthorizationData((data)->authorization_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Authenticator(const Authenticator *from, Authenticator *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->authenticator_vno, &(to)->authenticator_vno)) goto fail;
if(copy_Realm(&(from)->crealm, &(to)->crealm)) goto fail;
if(copy_PrincipalName(&(from)->cname, &(to)->cname)) goto fail;
if((from)->cksum) {
(to)->cksum = malloc(sizeof(*(to)->cksum));
if((to)->cksum == NULL) goto fail;
if(copy_Checksum((from)->cksum, (to)->cksum)) goto fail;
}else
(to)->cksum = NULL;
if(copy_krb5int32(&(from)->cusec, &(to)->cusec)) goto fail;
if(copy_KerberosTime(&(from)->ctime, &(to)->ctime)) goto fail;
if((from)->subkey) {
(to)->subkey = malloc(sizeof(*(to)->subkey));
if((to)->subkey == NULL) goto fail;
if(copy_EncryptionKey((from)->subkey, (to)->subkey)) goto fail;
}else
(to)->subkey = NULL;
if((from)->seq_number) {
(to)->seq_number = malloc(sizeof(*(to)->seq_number));
if((to)->seq_number == NULL) goto fail;
if(copy_krb5uint32((from)->seq_number, (to)->seq_number)) goto fail;
}else
(to)->seq_number = NULL;
if((from)->authorization_data) {
(to)->authorization_data = malloc(sizeof(*(to)->authorization_data));
if((to)->authorization_data == NULL) goto fail;
if(copy_AuthorizationData((from)->authorization_data, (to)->authorization_data)) goto fail;
}else
(to)->authorization_data = NULL;
return 0;
fail:
free_Authenticator(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_DATA(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_DATA *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* padata-value */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->padata_value, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* padata-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PADATA_TYPE(p, len, &(data)->padata_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_DATA(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_DATA *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t padata_type_datalen, padata_type_oldlen;
Der_type padata_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_type_type, 1, &padata_type_datalen, &l);
if (e == 0 && padata_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_type_oldlen = len;
if (padata_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_type_datalen;
e = decode_PADATA_TYPE(p, len, &(data)->padata_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = padata_type_oldlen - padata_type_datalen;
}
{
size_t padata_value_datalen, padata_value_oldlen;
Der_type padata_value_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_value_type, 2, &padata_value_datalen, &l);
if (e == 0 && padata_value_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_value_oldlen = len;
if (padata_value_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_value_datalen;
{
size_t padata_value_Tag_datalen, padata_value_Tag_oldlen;
Der_type padata_value_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &padata_value_Tag_type, UT_OctetString, &padata_value_Tag_datalen, &l);
if (e == 0 && padata_value_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_value_Tag_oldlen = len;
if (padata_value_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_value_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->padata_value, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = padata_value_Tag_oldlen - padata_value_Tag_datalen;
}
len = padata_value_oldlen - padata_value_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_DATA(data);
return e;
}

void ASN1CALL
free_PA_DATA(PA_DATA *data)
{
free_PADATA_TYPE(&(data)->padata_type);
der_free_octet_string(&(data)->padata_value);
}

size_t ASN1CALL
length_PA_DATA(const PA_DATA *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PADATA_TYPE(&(data)->padata_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->padata_value);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_DATA(const PA_DATA *from, PA_DATA *to)
{
memset(to, 0, sizeof(*to));
if(copy_PADATA_TYPE(&(from)->padata_type, &(to)->padata_type)) goto fail;
if(der_copy_octet_string(&(from)->padata_value, &(to)->padata_value)) goto fail;
return 0;
fail:
free_PA_DATA(to);
return ENOMEM;
}

int ASN1CALL
encode_ETYPE_INFO_ENTRY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ETYPE_INFO_ENTRY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* salttype */
if((data)->salttype) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->salttype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* salt */
if((data)->salt) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, (data)->salt, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(data)->etype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ETYPE_INFO_ENTRY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ETYPE_INFO_ENTRY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t etype_datalen, etype_oldlen;
Der_type etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &etype_type, 0, &etype_datalen, &l);
if (e == 0 && etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_oldlen = len;
if (etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_datalen;
e = decode_ENCTYPE(p, len, &(data)->etype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = etype_oldlen - etype_datalen;
}
{
size_t salt_datalen, salt_oldlen;
Der_type salt_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &salt_type, 1, &salt_datalen, &l);
if (e == 0 && salt_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->salt = NULL;
} else {
(data)->salt = calloc(1, sizeof(*(data)->salt));
if ((data)->salt == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
salt_oldlen = len;
if (salt_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = salt_datalen;
{
size_t salt_Tag_datalen, salt_Tag_oldlen;
Der_type salt_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &salt_Tag_type, UT_OctetString, &salt_Tag_datalen, &l);
if (e == 0 && salt_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
salt_Tag_oldlen = len;
if (salt_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = salt_Tag_datalen;
e = der_get_octet_string(p, len, (data)->salt, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = salt_Tag_oldlen - salt_Tag_datalen;
}
len = salt_oldlen - salt_datalen;
}
}
{
size_t salttype_datalen, salttype_oldlen;
Der_type salttype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &salttype_type, 2, &salttype_datalen, &l);
if (e == 0 && salttype_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->salttype = NULL;
} else {
(data)->salttype = calloc(1, sizeof(*(data)->salttype));
if ((data)->salttype == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
salttype_oldlen = len;
if (salttype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = salttype_datalen;
e = decode_krb5int32(p, len, (data)->salttype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = salttype_oldlen - salttype_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ETYPE_INFO_ENTRY(data);
return e;
}

void ASN1CALL
free_ETYPE_INFO_ENTRY(ETYPE_INFO_ENTRY *data)
{
free_ENCTYPE(&(data)->etype);
if((data)->salt) {
der_free_octet_string((data)->salt);
free((data)->salt);
(data)->salt = NULL;
}
if((data)->salttype) {
free_krb5int32((data)->salttype);
free((data)->salttype);
(data)->salttype = NULL;
}
}

size_t ASN1CALL
length_ETYPE_INFO_ENTRY(const ETYPE_INFO_ENTRY *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(data)->etype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->salt){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string((data)->salt);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->salttype){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->salttype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ETYPE_INFO_ENTRY(const ETYPE_INFO_ENTRY *from, ETYPE_INFO_ENTRY *to)
{
memset(to, 0, sizeof(*to));
if(copy_ENCTYPE(&(from)->etype, &(to)->etype)) goto fail;
if((from)->salt) {
(to)->salt = malloc(sizeof(*(to)->salt));
if((to)->salt == NULL) goto fail;
if(der_copy_octet_string((from)->salt, (to)->salt)) goto fail;
}else
(to)->salt = NULL;
if((from)->salttype) {
(to)->salttype = malloc(sizeof(*(to)->salttype));
if((to)->salttype == NULL) goto fail;
if(copy_krb5int32((from)->salttype, (to)->salttype)) goto fail;
}else
(to)->salttype = NULL;
return 0;
fail:
free_ETYPE_INFO_ENTRY(to);
return ENOMEM;
}

int ASN1CALL
encode_ETYPE_INFO(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ETYPE_INFO *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_ETYPE_INFO_ENTRY(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ETYPE_INFO(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ETYPE_INFO *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_ETYPE_INFO_ENTRY(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ETYPE_INFO(data);
return e;
}

void ASN1CALL
free_ETYPE_INFO(ETYPE_INFO *data)
{
while((data)->len){
free_ETYPE_INFO_ENTRY(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_ETYPE_INFO(const ETYPE_INFO *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_ETYPE_INFO_ENTRY(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ETYPE_INFO(const ETYPE_INFO *from, ETYPE_INFO *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_ETYPE_INFO_ENTRY(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_ETYPE_INFO(to);
return ENOMEM;
}

int ASN1CALL
add_ETYPE_INFO(ETYPE_INFO *data, const ETYPE_INFO_ENTRY *element)
{
int ret;
void *ptr;

ptr = realloc(data->val, 
	(data->len + 1) * sizeof(data->val[0]));
if (ptr == NULL) return ENOMEM;
data->val = ptr;

ret = copy_ETYPE_INFO_ENTRY(element, &data->val[data->len]);
if (ret) return ret;
data->len++;
return 0;
}

int ASN1CALL
remove_ETYPE_INFO(ETYPE_INFO *data, unsigned int element)
{
void *ptr;

if (data->len == 0 || element >= data->len)
	return ASN1_OVERRUN;
free_ETYPE_INFO_ENTRY(&data->val[element]);
data->len--;
if (element < data->len)
	memmove(&data->val[element], &data->val[element + 1], 
		sizeof(data->val[0]) * (data->len - element));
ptr = realloc(data->val, data->len * sizeof(data->val[0]));
if (ptr != NULL || data->len == 0) data->val = ptr;
return 0;
}

int ASN1CALL
encode_ETYPE_INFO2_ENTRY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ETYPE_INFO2_ENTRY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* s2kparams */
if((data)->s2kparams) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, (data)->s2kparams, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* salt */
if((data)->salt) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosString(p, len, (data)->salt, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(data)->etype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ETYPE_INFO2_ENTRY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ETYPE_INFO2_ENTRY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t etype_datalen, etype_oldlen;
Der_type etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &etype_type, 0, &etype_datalen, &l);
if (e == 0 && etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_oldlen = len;
if (etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_datalen;
e = decode_ENCTYPE(p, len, &(data)->etype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = etype_oldlen - etype_datalen;
}
{
size_t salt_datalen, salt_oldlen;
Der_type salt_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &salt_type, 1, &salt_datalen, &l);
if (e == 0 && salt_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->salt = NULL;
} else {
(data)->salt = calloc(1, sizeof(*(data)->salt));
if ((data)->salt == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
salt_oldlen = len;
if (salt_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = salt_datalen;
e = decode_KerberosString(p, len, (data)->salt, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = salt_oldlen - salt_datalen;
}
}
{
size_t s2kparams_datalen, s2kparams_oldlen;
Der_type s2kparams_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &s2kparams_type, 2, &s2kparams_datalen, &l);
if (e == 0 && s2kparams_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->s2kparams = NULL;
} else {
(data)->s2kparams = calloc(1, sizeof(*(data)->s2kparams));
if ((data)->s2kparams == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
s2kparams_oldlen = len;
if (s2kparams_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = s2kparams_datalen;
{
size_t s2kparams_Tag_datalen, s2kparams_Tag_oldlen;
Der_type s2kparams_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &s2kparams_Tag_type, UT_OctetString, &s2kparams_Tag_datalen, &l);
if (e == 0 && s2kparams_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
s2kparams_Tag_oldlen = len;
if (s2kparams_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = s2kparams_Tag_datalen;
e = der_get_octet_string(p, len, (data)->s2kparams, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = s2kparams_Tag_oldlen - s2kparams_Tag_datalen;
}
len = s2kparams_oldlen - s2kparams_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ETYPE_INFO2_ENTRY(data);
return e;
}

void ASN1CALL
free_ETYPE_INFO2_ENTRY(ETYPE_INFO2_ENTRY *data)
{
free_ENCTYPE(&(data)->etype);
if((data)->salt) {
free_KerberosString((data)->salt);
free((data)->salt);
(data)->salt = NULL;
}
if((data)->s2kparams) {
der_free_octet_string((data)->s2kparams);
free((data)->s2kparams);
(data)->s2kparams = NULL;
}
}

size_t ASN1CALL
length_ETYPE_INFO2_ENTRY(const ETYPE_INFO2_ENTRY *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(data)->etype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->salt){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosString((data)->salt);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->s2kparams){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string((data)->s2kparams);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ETYPE_INFO2_ENTRY(const ETYPE_INFO2_ENTRY *from, ETYPE_INFO2_ENTRY *to)
{
memset(to, 0, sizeof(*to));
if(copy_ENCTYPE(&(from)->etype, &(to)->etype)) goto fail;
if((from)->salt) {
(to)->salt = malloc(sizeof(*(to)->salt));
if((to)->salt == NULL) goto fail;
if(copy_KerberosString((from)->salt, (to)->salt)) goto fail;
}else
(to)->salt = NULL;
if((from)->s2kparams) {
(to)->s2kparams = malloc(sizeof(*(to)->s2kparams));
if((to)->s2kparams == NULL) goto fail;
if(der_copy_octet_string((from)->s2kparams, (to)->s2kparams)) goto fail;
}else
(to)->s2kparams = NULL;
return 0;
fail:
free_ETYPE_INFO2_ENTRY(to);
return ENOMEM;
}

int ASN1CALL
encode_ETYPE_INFO2(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ETYPE_INFO2 *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_ETYPE_INFO2_ENTRY(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ETYPE_INFO2(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ETYPE_INFO2 *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_ETYPE_INFO2_ENTRY(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
if ((data)->len < 1) {
e = ASN1_MIN_CONSTRAINT; goto fail;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ETYPE_INFO2(data);
return e;
}

void ASN1CALL
free_ETYPE_INFO2(ETYPE_INFO2 *data)
{
while((data)->len){
free_ETYPE_INFO2_ENTRY(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_ETYPE_INFO2(const ETYPE_INFO2 *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_ETYPE_INFO2_ENTRY(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ETYPE_INFO2(const ETYPE_INFO2 *from, ETYPE_INFO2 *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_ETYPE_INFO2_ENTRY(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_ETYPE_INFO2(to);
return ENOMEM;
}

int ASN1CALL
add_ETYPE_INFO2(ETYPE_INFO2 *data, const ETYPE_INFO2_ENTRY *element)
{
int ret;
void *ptr;

ptr = realloc(data->val, 
	(data->len + 1) * sizeof(data->val[0]));
if (ptr == NULL) return ENOMEM;
data->val = ptr;

ret = copy_ETYPE_INFO2_ENTRY(element, &data->val[data->len]);
if (ret) return ret;
data->len++;
return 0;
}

int ASN1CALL
remove_ETYPE_INFO2(ETYPE_INFO2 *data, unsigned int element)
{
void *ptr;

if (data->len == 0 || element >= data->len)
	return ASN1_OVERRUN;
free_ETYPE_INFO2_ENTRY(&data->val[element]);
data->len--;
if (element < data->len)
	memmove(&data->val[element], &data->val[element + 1], 
		sizeof(data->val[0]) * (data->len - element));
ptr = realloc(data->val, data->len * sizeof(data->val[0]));
if (ptr != NULL || data->len == 0) data->val = ptr;
return 0;
}

int ASN1CALL
encode_METHOD_DATA(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const METHOD_DATA *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_PA_DATA(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_METHOD_DATA(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, METHOD_DATA *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_PA_DATA(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_METHOD_DATA(data);
return e;
}

void ASN1CALL
free_METHOD_DATA(METHOD_DATA *data)
{
while((data)->len){
free_PA_DATA(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_METHOD_DATA(const METHOD_DATA *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_PA_DATA(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_METHOD_DATA(const METHOD_DATA *from, METHOD_DATA *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_PA_DATA(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_METHOD_DATA(to);
return ENOMEM;
}

int ASN1CALL
add_METHOD_DATA(METHOD_DATA *data, const PA_DATA *element)
{
int ret;
void *ptr;

ptr = realloc(data->val, 
	(data->len + 1) * sizeof(data->val[0]));
if (ptr == NULL) return ENOMEM;
data->val = ptr;

ret = copy_PA_DATA(element, &data->val[data->len]);
if (ret) return ret;
data->len++;
return 0;
}

int ASN1CALL
remove_METHOD_DATA(METHOD_DATA *data, unsigned int element)
{
void *ptr;

if (data->len == 0 || element >= data->len)
	return ASN1_OVERRUN;
free_PA_DATA(&data->val[element]);
data->len--;
if (element < data->len)
	memmove(&data->val[element], &data->val[element + 1], 
		sizeof(data->val[0]) * (data->len - element));
ptr = realloc(data->val, data->len * sizeof(data->val[0]));
if (ptr != NULL || data->len == 0) data->val = ptr;
return 0;
}

int ASN1CALL
encode_TypedData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TypedData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* data-value */
if((data)->data_value) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, (data)->data_value, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* data-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->data_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TypedData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TypedData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t data_type_datalen, data_type_oldlen;
Der_type data_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &data_type_type, 0, &data_type_datalen, &l);
if (e == 0 && data_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
data_type_oldlen = len;
if (data_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = data_type_datalen;
e = decode_krb5int32(p, len, &(data)->data_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = data_type_oldlen - data_type_datalen;
}
{
size_t data_value_datalen, data_value_oldlen;
Der_type data_value_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &data_value_type, 1, &data_value_datalen, &l);
if (e == 0 && data_value_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->data_value = NULL;
} else {
(data)->data_value = calloc(1, sizeof(*(data)->data_value));
if ((data)->data_value == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
data_value_oldlen = len;
if (data_value_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = data_value_datalen;
{
size_t data_value_Tag_datalen, data_value_Tag_oldlen;
Der_type data_value_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &data_value_Tag_type, UT_OctetString, &data_value_Tag_datalen, &l);
if (e == 0 && data_value_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
data_value_Tag_oldlen = len;
if (data_value_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = data_value_Tag_datalen;
e = der_get_octet_string(p, len, (data)->data_value, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = data_value_Tag_oldlen - data_value_Tag_datalen;
}
len = data_value_oldlen - data_value_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TypedData(data);
return e;
}

void ASN1CALL
free_TypedData(TypedData *data)
{
free_krb5int32(&(data)->data_type);
if((data)->data_value) {
der_free_octet_string((data)->data_value);
free((data)->data_value);
(data)->data_value = NULL;
}
}

size_t ASN1CALL
length_TypedData(const TypedData *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->data_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->data_value){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string((data)->data_value);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TypedData(const TypedData *from, TypedData *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->data_type, &(to)->data_type)) goto fail;
if((from)->data_value) {
(to)->data_value = malloc(sizeof(*(to)->data_value));
if((to)->data_value == NULL) goto fail;
if(der_copy_octet_string((from)->data_value, (to)->data_value)) goto fail;
}else
(to)->data_value = NULL;
return 0;
fail:
free_TypedData(to);
return ENOMEM;
}

int ASN1CALL
encode_TYPED_DATA(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TYPED_DATA *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_TypedData(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TYPED_DATA(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TYPED_DATA *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_TypedData(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
if ((data)->len < 1) {
e = ASN1_MIN_CONSTRAINT; goto fail;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TYPED_DATA(data);
return e;
}

void ASN1CALL
free_TYPED_DATA(TYPED_DATA *data)
{
while((data)->len){
free_TypedData(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_TYPED_DATA(const TYPED_DATA *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_TypedData(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TYPED_DATA(const TYPED_DATA *from, TYPED_DATA *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_TypedData(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_TYPED_DATA(to);
return ENOMEM;
}

int ASN1CALL
encode_KDC_REQ_BODY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KDC_REQ_BODY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* additional-tickets */
if((data)->additional_tickets) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)((data)->additional_tickets)->len - 1; i >= 0; --i) {
size_t additional_tickets_tag_tag_for_oldret = ret;
ret = 0;
e = encode_Ticket(p, len, &((data)->additional_tickets)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += additional_tickets_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 11, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* enc-authorization-data */
if((data)->enc_authorization_data) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, (data)->enc_authorization_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* addresses */
if((data)->addresses) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddresses(p, len, (data)->addresses, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->etype)->len - 1; i >= 0; --i) {
size_t etype_tag_tag_for_oldret = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(&(data)->etype)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += etype_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* nonce */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* rtime */
if((data)->rtime) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->rtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* till */
if((data)->till) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->till, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* from */
if((data)->from) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->from, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sname */
if((data)->sname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* realm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cname */
if((data)->cname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* kdc-options */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KDCOptions(p, len, &(data)->kdc_options, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KDC_REQ_BODY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KDC_REQ_BODY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t kdc_options_datalen, kdc_options_oldlen;
Der_type kdc_options_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &kdc_options_type, 0, &kdc_options_datalen, &l);
if (e == 0 && kdc_options_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
kdc_options_oldlen = len;
if (kdc_options_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = kdc_options_datalen;
e = decode_KDCOptions(p, len, &(data)->kdc_options, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = kdc_options_oldlen - kdc_options_datalen;
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 1, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->cname = NULL;
} else {
(data)->cname = calloc(1, sizeof(*(data)->cname));
if ((data)->cname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, (data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
}
{
size_t realm_datalen, realm_oldlen;
Der_type realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &realm_type, 2, &realm_datalen, &l);
if (e == 0 && realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
realm_oldlen = len;
if (realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = realm_datalen;
e = decode_Realm(p, len, &(data)->realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = realm_oldlen - realm_datalen;
}
{
size_t sname_datalen, sname_oldlen;
Der_type sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sname_type, 3, &sname_datalen, &l);
if (e == 0 && sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sname = NULL;
} else {
(data)->sname = calloc(1, sizeof(*(data)->sname));
if ((data)->sname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sname_oldlen = len;
if (sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sname_datalen;
e = decode_PrincipalName(p, len, (data)->sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sname_oldlen - sname_datalen;
}
}
{
size_t from_datalen, from_oldlen;
Der_type from_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &from_type, 4, &from_datalen, &l);
if (e == 0 && from_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->from = NULL;
} else {
(data)->from = calloc(1, sizeof(*(data)->from));
if ((data)->from == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
from_oldlen = len;
if (from_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = from_datalen;
e = decode_KerberosTime(p, len, (data)->from, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = from_oldlen - from_datalen;
}
}
{
size_t till_datalen, till_oldlen;
Der_type till_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &till_type, 5, &till_datalen, &l);
if (e == 0 && till_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->till = NULL;
} else {
(data)->till = calloc(1, sizeof(*(data)->till));
if ((data)->till == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
till_oldlen = len;
if (till_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = till_datalen;
e = decode_KerberosTime(p, len, (data)->till, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = till_oldlen - till_datalen;
}
}
{
size_t rtime_datalen, rtime_oldlen;
Der_type rtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &rtime_type, 6, &rtime_datalen, &l);
if (e == 0 && rtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->rtime = NULL;
} else {
(data)->rtime = calloc(1, sizeof(*(data)->rtime));
if ((data)->rtime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
rtime_oldlen = len;
if (rtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = rtime_datalen;
e = decode_KerberosTime(p, len, (data)->rtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = rtime_oldlen - rtime_datalen;
}
}
{
size_t nonce_datalen, nonce_oldlen;
Der_type nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &nonce_type, 7, &nonce_datalen, &l);
if (e == 0 && nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
nonce_oldlen = len;
if (nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = nonce_datalen;
e = decode_krb5int32(p, len, &(data)->nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = nonce_oldlen - nonce_datalen;
}
{
size_t etype_datalen, etype_oldlen;
Der_type etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &etype_type, 8, &etype_datalen, &l);
if (e == 0 && etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_oldlen = len;
if (etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_datalen;
{
size_t etype_Tag_datalen, etype_Tag_oldlen;
Der_type etype_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &etype_Tag_type, UT_Sequence, &etype_Tag_datalen, &l);
if (e == 0 && etype_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_Tag_oldlen = len;
if (etype_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_Tag_datalen;
{
size_t etype_Tag_Tag_origlen = len;
size_t etype_Tag_Tag_oldret = ret;
size_t etype_Tag_Tag_olen = 0;
void *etype_Tag_Tag_tmp;
ret = 0;
(&(data)->etype)->len = 0;
(&(data)->etype)->val = NULL;
while(ret < etype_Tag_Tag_origlen) {
size_t etype_Tag_Tag_nlen = etype_Tag_Tag_olen + sizeof(*((&(data)->etype)->val));
if (etype_Tag_Tag_olen > etype_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
etype_Tag_Tag_olen = etype_Tag_Tag_nlen;
etype_Tag_Tag_tmp = realloc((&(data)->etype)->val, etype_Tag_Tag_olen);
if (etype_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->etype)->val = etype_Tag_Tag_tmp;
e = decode_ENCTYPE(p, len, &(&(data)->etype)->val[(&(data)->etype)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->etype)->len++;
len = etype_Tag_Tag_origlen - ret;
}
ret += etype_Tag_Tag_oldret;
}
len = etype_Tag_oldlen - etype_Tag_datalen;
}
len = etype_oldlen - etype_datalen;
}
{
size_t addresses_datalen, addresses_oldlen;
Der_type addresses_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &addresses_type, 9, &addresses_datalen, &l);
if (e == 0 && addresses_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->addresses = NULL;
} else {
(data)->addresses = calloc(1, sizeof(*(data)->addresses));
if ((data)->addresses == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
addresses_oldlen = len;
if (addresses_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = addresses_datalen;
e = decode_HostAddresses(p, len, (data)->addresses, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = addresses_oldlen - addresses_datalen;
}
}
{
size_t enc_authorization_data_datalen, enc_authorization_data_oldlen;
Der_type enc_authorization_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_authorization_data_type, 10, &enc_authorization_data_datalen, &l);
if (e == 0 && enc_authorization_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->enc_authorization_data = NULL;
} else {
(data)->enc_authorization_data = calloc(1, sizeof(*(data)->enc_authorization_data));
if ((data)->enc_authorization_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
enc_authorization_data_oldlen = len;
if (enc_authorization_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_authorization_data_datalen;
e = decode_EncryptedData(p, len, (data)->enc_authorization_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_authorization_data_oldlen - enc_authorization_data_datalen;
}
}
{
size_t additional_tickets_datalen, additional_tickets_oldlen;
Der_type additional_tickets_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &additional_tickets_type, 11, &additional_tickets_datalen, &l);
if (e == 0 && additional_tickets_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->additional_tickets = NULL;
} else {
(data)->additional_tickets = calloc(1, sizeof(*(data)->additional_tickets));
if ((data)->additional_tickets == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
additional_tickets_oldlen = len;
if (additional_tickets_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = additional_tickets_datalen;
{
size_t additional_tickets_Tag_datalen, additional_tickets_Tag_oldlen;
Der_type additional_tickets_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &additional_tickets_Tag_type, UT_Sequence, &additional_tickets_Tag_datalen, &l);
if (e == 0 && additional_tickets_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
additional_tickets_Tag_oldlen = len;
if (additional_tickets_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = additional_tickets_Tag_datalen;
{
size_t additional_tickets_Tag_Tag_origlen = len;
size_t additional_tickets_Tag_Tag_oldret = ret;
size_t additional_tickets_Tag_Tag_olen = 0;
void *additional_tickets_Tag_Tag_tmp;
ret = 0;
((data)->additional_tickets)->len = 0;
((data)->additional_tickets)->val = NULL;
while(ret < additional_tickets_Tag_Tag_origlen) {
size_t additional_tickets_Tag_Tag_nlen = additional_tickets_Tag_Tag_olen + sizeof(*(((data)->additional_tickets)->val));
if (additional_tickets_Tag_Tag_olen > additional_tickets_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
additional_tickets_Tag_Tag_olen = additional_tickets_Tag_Tag_nlen;
additional_tickets_Tag_Tag_tmp = realloc(((data)->additional_tickets)->val, additional_tickets_Tag_Tag_olen);
if (additional_tickets_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
((data)->additional_tickets)->val = additional_tickets_Tag_Tag_tmp;
e = decode_Ticket(p, len, &((data)->additional_tickets)->val[((data)->additional_tickets)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
((data)->additional_tickets)->len++;
len = additional_tickets_Tag_Tag_origlen - ret;
}
ret += additional_tickets_Tag_Tag_oldret;
}
len = additional_tickets_Tag_oldlen - additional_tickets_Tag_datalen;
}
len = additional_tickets_oldlen - additional_tickets_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KDC_REQ_BODY(data);
return e;
}

void ASN1CALL
free_KDC_REQ_BODY(KDC_REQ_BODY *data)
{
free_KDCOptions(&(data)->kdc_options);
if((data)->cname) {
free_PrincipalName((data)->cname);
free((data)->cname);
(data)->cname = NULL;
}
free_Realm(&(data)->realm);
if((data)->sname) {
free_PrincipalName((data)->sname);
free((data)->sname);
(data)->sname = NULL;
}
if((data)->from) {
free_KerberosTime((data)->from);
free((data)->from);
(data)->from = NULL;
}
if((data)->till) {
free_KerberosTime((data)->till);
free((data)->till);
(data)->till = NULL;
}
if((data)->rtime) {
free_KerberosTime((data)->rtime);
free((data)->rtime);
(data)->rtime = NULL;
}
free_krb5int32(&(data)->nonce);
while((&(data)->etype)->len){
free_ENCTYPE(&(&(data)->etype)->val[(&(data)->etype)->len-1]);
(&(data)->etype)->len--;
}
free((&(data)->etype)->val);
(&(data)->etype)->val = NULL;
if((data)->addresses) {
free_HostAddresses((data)->addresses);
free((data)->addresses);
(data)->addresses = NULL;
}
if((data)->enc_authorization_data) {
free_EncryptedData((data)->enc_authorization_data);
free((data)->enc_authorization_data);
(data)->enc_authorization_data = NULL;
}
if((data)->additional_tickets) {
while(((data)->additional_tickets)->len){
free_Ticket(&((data)->additional_tickets)->val[((data)->additional_tickets)->len-1]);
((data)->additional_tickets)->len--;
}
free(((data)->additional_tickets)->val);
((data)->additional_tickets)->val = NULL;
free((data)->additional_tickets);
(data)->additional_tickets = NULL;
}
}

size_t ASN1CALL
length_KDC_REQ_BODY(const KDC_REQ_BODY *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KDCOptions(&(data)->kdc_options);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->cname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->from){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->from);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->till){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->till);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->rtime){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->rtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t etype_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->etype)->len - 1; i >= 0; --i){
size_t etype_tag_tag_for_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(&(data)->etype)->val[i]);
ret += etype_tag_tag_for_oldret;
}
ret += etype_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->addresses){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_HostAddresses((data)->addresses);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->enc_authorization_data){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData((data)->enc_authorization_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->additional_tickets){
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t additional_tickets_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = ((data)->additional_tickets)->len - 1; i >= 0; --i){
size_t additional_tickets_tag_tag_for_oldret = ret;
ret = 0;
ret += length_Ticket(&((data)->additional_tickets)->val[i]);
ret += additional_tickets_tag_tag_for_oldret;
}
ret += additional_tickets_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KDC_REQ_BODY(const KDC_REQ_BODY *from, KDC_REQ_BODY *to)
{
memset(to, 0, sizeof(*to));
if(copy_KDCOptions(&(from)->kdc_options, &(to)->kdc_options)) goto fail;
if((from)->cname) {
(to)->cname = malloc(sizeof(*(to)->cname));
if((to)->cname == NULL) goto fail;
if(copy_PrincipalName((from)->cname, (to)->cname)) goto fail;
}else
(to)->cname = NULL;
if(copy_Realm(&(from)->realm, &(to)->realm)) goto fail;
if((from)->sname) {
(to)->sname = malloc(sizeof(*(to)->sname));
if((to)->sname == NULL) goto fail;
if(copy_PrincipalName((from)->sname, (to)->sname)) goto fail;
}else
(to)->sname = NULL;
if((from)->from) {
(to)->from = malloc(sizeof(*(to)->from));
if((to)->from == NULL) goto fail;
if(copy_KerberosTime((from)->from, (to)->from)) goto fail;
}else
(to)->from = NULL;
if((from)->till) {
(to)->till = malloc(sizeof(*(to)->till));
if((to)->till == NULL) goto fail;
if(copy_KerberosTime((from)->till, (to)->till)) goto fail;
}else
(to)->till = NULL;
if((from)->rtime) {
(to)->rtime = malloc(sizeof(*(to)->rtime));
if((to)->rtime == NULL) goto fail;
if(copy_KerberosTime((from)->rtime, (to)->rtime)) goto fail;
}else
(to)->rtime = NULL;
if(copy_krb5int32(&(from)->nonce, &(to)->nonce)) goto fail;
if(((&(to)->etype)->val = malloc((&(from)->etype)->len * sizeof(*(&(to)->etype)->val))) == NULL && (&(from)->etype)->len != 0)
goto fail;
for((&(to)->etype)->len = 0; (&(to)->etype)->len < (&(from)->etype)->len; (&(to)->etype)->len++){
if(copy_ENCTYPE(&(&(from)->etype)->val[(&(to)->etype)->len], &(&(to)->etype)->val[(&(to)->etype)->len])) goto fail;
}
if((from)->addresses) {
(to)->addresses = malloc(sizeof(*(to)->addresses));
if((to)->addresses == NULL) goto fail;
if(copy_HostAddresses((from)->addresses, (to)->addresses)) goto fail;
}else
(to)->addresses = NULL;
if((from)->enc_authorization_data) {
(to)->enc_authorization_data = malloc(sizeof(*(to)->enc_authorization_data));
if((to)->enc_authorization_data == NULL) goto fail;
if(copy_EncryptedData((from)->enc_authorization_data, (to)->enc_authorization_data)) goto fail;
}else
(to)->enc_authorization_data = NULL;
if((from)->additional_tickets) {
(to)->additional_tickets = malloc(sizeof(*(to)->additional_tickets));
if((to)->additional_tickets == NULL) goto fail;
if((((to)->additional_tickets)->val = malloc(((from)->additional_tickets)->len * sizeof(*((to)->additional_tickets)->val))) == NULL && ((from)->additional_tickets)->len != 0)
goto fail;
for(((to)->additional_tickets)->len = 0; ((to)->additional_tickets)->len < ((from)->additional_tickets)->len; ((to)->additional_tickets)->len++){
if(copy_Ticket(&((from)->additional_tickets)->val[((to)->additional_tickets)->len], &((to)->additional_tickets)->val[((to)->additional_tickets)->len])) goto fail;
}
}else
(to)->additional_tickets = NULL;
return 0;
fail:
free_KDC_REQ_BODY(to);
return ENOMEM;
}

int ASN1CALL
encode_KDC_REQ(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KDC_REQ *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* req-body */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KDC_REQ_BODY(p, len, &(data)->req_body, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* padata */
if((data)->padata) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_METHOD_DATA(p, len, (data)->padata, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* pvno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KDC_REQ(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KDC_REQ *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 1, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 2, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t padata_datalen, padata_oldlen;
Der_type padata_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_type, 3, &padata_datalen, &l);
if (e == 0 && padata_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->padata = NULL;
} else {
(data)->padata = calloc(1, sizeof(*(data)->padata));
if ((data)->padata == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
padata_oldlen = len;
if (padata_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_datalen;
e = decode_METHOD_DATA(p, len, (data)->padata, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = padata_oldlen - padata_datalen;
}
}
{
size_t req_body_datalen, req_body_oldlen;
Der_type req_body_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &req_body_type, 4, &req_body_datalen, &l);
if (e == 0 && req_body_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
req_body_oldlen = len;
if (req_body_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = req_body_datalen;
e = decode_KDC_REQ_BODY(p, len, &(data)->req_body, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = req_body_oldlen - req_body_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KDC_REQ(data);
return e;
}

void ASN1CALL
free_KDC_REQ(KDC_REQ *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
if((data)->padata) {
free_METHOD_DATA((data)->padata);
free((data)->padata);
(data)->padata = NULL;
}
free_KDC_REQ_BODY(&(data)->req_body);
}

size_t ASN1CALL
length_KDC_REQ(const KDC_REQ *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->padata){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_METHOD_DATA((data)->padata);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KDC_REQ_BODY(&(data)->req_body);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KDC_REQ(const KDC_REQ *from, KDC_REQ *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if((from)->padata) {
(to)->padata = malloc(sizeof(*(to)->padata));
if((to)->padata == NULL) goto fail;
if(copy_METHOD_DATA((from)->padata, (to)->padata)) goto fail;
}else
(to)->padata = NULL;
if(copy_KDC_REQ_BODY(&(from)->req_body, &(to)->req_body)) goto fail;
return 0;
fail:
free_KDC_REQ(to);
return ENOMEM;
}

int ASN1CALL
encode_AS_REQ(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AS_REQ *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_KDC_REQ(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AS_REQ(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AS_REQ *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 10, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_KDC_REQ(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AS_REQ(data);
return e;
}

void ASN1CALL
free_AS_REQ(AS_REQ *data)
{
free_KDC_REQ(data);
}

size_t ASN1CALL
length_AS_REQ(const AS_REQ *data)
{
size_t ret = 0;
ret += length_KDC_REQ(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AS_REQ(const AS_REQ *from, AS_REQ *to)
{
memset(to, 0, sizeof(*to));
if(copy_KDC_REQ(from, to)) goto fail;
return 0;
fail:
free_AS_REQ(to);
return ENOMEM;
}

int ASN1CALL
encode_TGS_REQ(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TGS_REQ *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_KDC_REQ(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 12, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TGS_REQ(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TGS_REQ *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 12, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_KDC_REQ(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TGS_REQ(data);
return e;
}

void ASN1CALL
free_TGS_REQ(TGS_REQ *data)
{
free_KDC_REQ(data);
}

size_t ASN1CALL
length_TGS_REQ(const TGS_REQ *data)
{
size_t ret = 0;
ret += length_KDC_REQ(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TGS_REQ(const TGS_REQ *from, TGS_REQ *to)
{
memset(to, 0, sizeof(*to));
if(copy_KDC_REQ(from, to)) goto fail;
return 0;
fail:
free_TGS_REQ(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_ENC_TS_ENC(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_ENC_TS_ENC *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* pausec */
if((data)->pausec) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->pausec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* patimestamp */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->patimestamp, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_ENC_TS_ENC(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_ENC_TS_ENC *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t patimestamp_datalen, patimestamp_oldlen;
Der_type patimestamp_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &patimestamp_type, 0, &patimestamp_datalen, &l);
if (e == 0 && patimestamp_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
patimestamp_oldlen = len;
if (patimestamp_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = patimestamp_datalen;
e = decode_KerberosTime(p, len, &(data)->patimestamp, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = patimestamp_oldlen - patimestamp_datalen;
}
{
size_t pausec_datalen, pausec_oldlen;
Der_type pausec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pausec_type, 1, &pausec_datalen, &l);
if (e == 0 && pausec_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->pausec = NULL;
} else {
(data)->pausec = calloc(1, sizeof(*(data)->pausec));
if ((data)->pausec == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
pausec_oldlen = len;
if (pausec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pausec_datalen;
e = decode_krb5int32(p, len, (data)->pausec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pausec_oldlen - pausec_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_ENC_TS_ENC(data);
return e;
}

void ASN1CALL
free_PA_ENC_TS_ENC(PA_ENC_TS_ENC *data)
{
free_KerberosTime(&(data)->patimestamp);
if((data)->pausec) {
free_krb5int32((data)->pausec);
free((data)->pausec);
(data)->pausec = NULL;
}
}

size_t ASN1CALL
length_PA_ENC_TS_ENC(const PA_ENC_TS_ENC *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->patimestamp);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->pausec){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->pausec);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_ENC_TS_ENC(const PA_ENC_TS_ENC *from, PA_ENC_TS_ENC *to)
{
memset(to, 0, sizeof(*to));
if(copy_KerberosTime(&(from)->patimestamp, &(to)->patimestamp)) goto fail;
if((from)->pausec) {
(to)->pausec = malloc(sizeof(*(to)->pausec));
if((to)->pausec == NULL) goto fail;
if(copy_krb5int32((from)->pausec, (to)->pausec)) goto fail;
}else
(to)->pausec = NULL;
return 0;
fail:
free_PA_ENC_TS_ENC(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_PAC_REQUEST(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_PAC_REQUEST *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* include-pac */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_boolean(p, len, &(data)->include_pac, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Boolean, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_PAC_REQUEST(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_PAC_REQUEST *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t include_pac_datalen, include_pac_oldlen;
Der_type include_pac_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &include_pac_type, 0, &include_pac_datalen, &l);
if (e == 0 && include_pac_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
include_pac_oldlen = len;
if (include_pac_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = include_pac_datalen;
{
size_t include_pac_Tag_datalen, include_pac_Tag_oldlen;
Der_type include_pac_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &include_pac_Tag_type, UT_Boolean, &include_pac_Tag_datalen, &l);
if (e == 0 && include_pac_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
include_pac_Tag_oldlen = len;
if (include_pac_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = include_pac_Tag_datalen;
e = der_get_boolean(p, len, &(data)->include_pac, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = include_pac_Tag_oldlen - include_pac_Tag_datalen;
}
len = include_pac_oldlen - include_pac_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_PAC_REQUEST(data);
return e;
}

void ASN1CALL
free_PA_PAC_REQUEST(PA_PAC_REQUEST *data)
{
}

size_t ASN1CALL
length_PA_PAC_REQUEST(const PA_PAC_REQUEST *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += 1;
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_PAC_REQUEST(const PA_PAC_REQUEST *from, PA_PAC_REQUEST *to)
{
memset(to, 0, sizeof(*to));
*(&(to)->include_pac) = *(&(from)->include_pac);
return 0;
}

int ASN1CALL
encode_PROV_SRV_LOCATION(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PROV_SRV_LOCATION *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = der_put_general_string(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PROV_SRV_LOCATION(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PROV_SRV_LOCATION *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_GeneralString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = der_get_general_string(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PROV_SRV_LOCATION(data);
return e;
}

void ASN1CALL
free_PROV_SRV_LOCATION(PROV_SRV_LOCATION *data)
{
der_free_general_string(data);
}

size_t ASN1CALL
length_PROV_SRV_LOCATION(const PROV_SRV_LOCATION *data)
{
size_t ret = 0;
ret += der_length_general_string(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PROV_SRV_LOCATION(const PROV_SRV_LOCATION *from, PROV_SRV_LOCATION *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_general_string(from, to)) goto fail;
return 0;
fail:
free_PROV_SRV_LOCATION(to);
return ENOMEM;
}

int ASN1CALL
encode_KDC_REP(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KDC_REP *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-part */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_part, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* ticket */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Ticket(p, len, &(data)->ticket, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cname */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* crealm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->crealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* padata */
if((data)->padata) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_METHOD_DATA(p, len, (data)->padata, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* pvno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KDC_REP(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KDC_REP *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t padata_datalen, padata_oldlen;
Der_type padata_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_type, 2, &padata_datalen, &l);
if (e == 0 && padata_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->padata = NULL;
} else {
(data)->padata = calloc(1, sizeof(*(data)->padata));
if ((data)->padata == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
padata_oldlen = len;
if (padata_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_datalen;
e = decode_METHOD_DATA(p, len, (data)->padata, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = padata_oldlen - padata_datalen;
}
}
{
size_t crealm_datalen, crealm_oldlen;
Der_type crealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &crealm_type, 3, &crealm_datalen, &l);
if (e == 0 && crealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
crealm_oldlen = len;
if (crealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = crealm_datalen;
e = decode_Realm(p, len, &(data)->crealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = crealm_oldlen - crealm_datalen;
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 4, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, &(data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
{
size_t ticket_datalen, ticket_oldlen;
Der_type ticket_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ticket_type, 5, &ticket_datalen, &l);
if (e == 0 && ticket_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ticket_oldlen = len;
if (ticket_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ticket_datalen;
e = decode_Ticket(p, len, &(data)->ticket, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ticket_oldlen - ticket_datalen;
}
{
size_t enc_part_datalen, enc_part_oldlen;
Der_type enc_part_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_part_type, 6, &enc_part_datalen, &l);
if (e == 0 && enc_part_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_part_oldlen = len;
if (enc_part_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_part_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_part, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_part_oldlen - enc_part_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KDC_REP(data);
return e;
}

void ASN1CALL
free_KDC_REP(KDC_REP *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
if((data)->padata) {
free_METHOD_DATA((data)->padata);
free((data)->padata);
(data)->padata = NULL;
}
free_Realm(&(data)->crealm);
free_PrincipalName(&(data)->cname);
free_Ticket(&(data)->ticket);
free_EncryptedData(&(data)->enc_part);
}

size_t ASN1CALL
length_KDC_REP(const KDC_REP *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->padata){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_METHOD_DATA((data)->padata);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->crealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Ticket(&(data)->ticket);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_part);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KDC_REP(const KDC_REP *from, KDC_REP *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if((from)->padata) {
(to)->padata = malloc(sizeof(*(to)->padata));
if((to)->padata == NULL) goto fail;
if(copy_METHOD_DATA((from)->padata, (to)->padata)) goto fail;
}else
(to)->padata = NULL;
if(copy_Realm(&(from)->crealm, &(to)->crealm)) goto fail;
if(copy_PrincipalName(&(from)->cname, &(to)->cname)) goto fail;
if(copy_Ticket(&(from)->ticket, &(to)->ticket)) goto fail;
if(copy_EncryptedData(&(from)->enc_part, &(to)->enc_part)) goto fail;
return 0;
fail:
free_KDC_REP(to);
return ENOMEM;
}

int ASN1CALL
encode_AS_REP(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AS_REP *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_KDC_REP(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 11, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AS_REP(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AS_REP *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 11, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_KDC_REP(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AS_REP(data);
return e;
}

void ASN1CALL
free_AS_REP(AS_REP *data)
{
free_KDC_REP(data);
}

size_t ASN1CALL
length_AS_REP(const AS_REP *data)
{
size_t ret = 0;
ret += length_KDC_REP(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AS_REP(const AS_REP *from, AS_REP *to)
{
memset(to, 0, sizeof(*to));
if(copy_KDC_REP(from, to)) goto fail;
return 0;
fail:
free_AS_REP(to);
return ENOMEM;
}

int ASN1CALL
encode_TGS_REP(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const TGS_REP *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_KDC_REP(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 13, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_TGS_REP(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, TGS_REP *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 13, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_KDC_REP(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_TGS_REP(data);
return e;
}

void ASN1CALL
free_TGS_REP(TGS_REP *data)
{
free_KDC_REP(data);
}

size_t ASN1CALL
length_TGS_REP(const TGS_REP *data)
{
size_t ret = 0;
ret += length_KDC_REP(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_TGS_REP(const TGS_REP *from, TGS_REP *to)
{
memset(to, 0, sizeof(*to));
if(copy_KDC_REP(from, to)) goto fail;
return 0;
fail:
free_TGS_REP(to);
return ENOMEM;
}

int ASN1CALL
encode_EncKDCRepPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncKDCRepPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* encrypted-pa-data */
if((data)->encrypted_pa_data) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_METHOD_DATA(p, len, (data)->encrypted_pa_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 12, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* caddr */
if((data)->caddr) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddresses(p, len, (data)->caddr, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 11, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sname */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* srealm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->srealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* renew-till */
if((data)->renew_till) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->renew_till, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* endtime */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->endtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* starttime */
if((data)->starttime) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->starttime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* authtime */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->authtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* flags */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_TicketFlags(p, len, &(data)->flags, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* key-expiration */
if((data)->key_expiration) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->key_expiration, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* nonce */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* last-req */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_LastReq(p, len, &(data)->last_req, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* key */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, &(data)->key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncKDCRepPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncKDCRepPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t key_datalen, key_oldlen;
Der_type key_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &key_type, 0, &key_datalen, &l);
if (e == 0 && key_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
key_oldlen = len;
if (key_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = key_datalen;
e = decode_EncryptionKey(p, len, &(data)->key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_oldlen - key_datalen;
}
{
size_t last_req_datalen, last_req_oldlen;
Der_type last_req_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &last_req_type, 1, &last_req_datalen, &l);
if (e == 0 && last_req_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
last_req_oldlen = len;
if (last_req_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = last_req_datalen;
e = decode_LastReq(p, len, &(data)->last_req, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = last_req_oldlen - last_req_datalen;
}
{
size_t nonce_datalen, nonce_oldlen;
Der_type nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &nonce_type, 2, &nonce_datalen, &l);
if (e == 0 && nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
nonce_oldlen = len;
if (nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = nonce_datalen;
e = decode_krb5int32(p, len, &(data)->nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = nonce_oldlen - nonce_datalen;
}
{
size_t key_expiration_datalen, key_expiration_oldlen;
Der_type key_expiration_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &key_expiration_type, 3, &key_expiration_datalen, &l);
if (e == 0 && key_expiration_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->key_expiration = NULL;
} else {
(data)->key_expiration = calloc(1, sizeof(*(data)->key_expiration));
if ((data)->key_expiration == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
key_expiration_oldlen = len;
if (key_expiration_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = key_expiration_datalen;
e = decode_KerberosTime(p, len, (data)->key_expiration, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_expiration_oldlen - key_expiration_datalen;
}
}
{
size_t flags_datalen, flags_oldlen;
Der_type flags_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &flags_type, 4, &flags_datalen, &l);
if (e == 0 && flags_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
flags_oldlen = len;
if (flags_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = flags_datalen;
e = decode_TicketFlags(p, len, &(data)->flags, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = flags_oldlen - flags_datalen;
}
{
size_t authtime_datalen, authtime_oldlen;
Der_type authtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authtime_type, 5, &authtime_datalen, &l);
if (e == 0 && authtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
authtime_oldlen = len;
if (authtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authtime_datalen;
e = decode_KerberosTime(p, len, &(data)->authtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authtime_oldlen - authtime_datalen;
}
{
size_t starttime_datalen, starttime_oldlen;
Der_type starttime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &starttime_type, 6, &starttime_datalen, &l);
if (e == 0 && starttime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->starttime = NULL;
} else {
(data)->starttime = calloc(1, sizeof(*(data)->starttime));
if ((data)->starttime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
starttime_oldlen = len;
if (starttime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = starttime_datalen;
e = decode_KerberosTime(p, len, (data)->starttime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = starttime_oldlen - starttime_datalen;
}
}
{
size_t endtime_datalen, endtime_oldlen;
Der_type endtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &endtime_type, 7, &endtime_datalen, &l);
if (e == 0 && endtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
endtime_oldlen = len;
if (endtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = endtime_datalen;
e = decode_KerberosTime(p, len, &(data)->endtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = endtime_oldlen - endtime_datalen;
}
{
size_t renew_till_datalen, renew_till_oldlen;
Der_type renew_till_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &renew_till_type, 8, &renew_till_datalen, &l);
if (e == 0 && renew_till_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->renew_till = NULL;
} else {
(data)->renew_till = calloc(1, sizeof(*(data)->renew_till));
if ((data)->renew_till == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
renew_till_oldlen = len;
if (renew_till_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = renew_till_datalen;
e = decode_KerberosTime(p, len, (data)->renew_till, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = renew_till_oldlen - renew_till_datalen;
}
}
{
size_t srealm_datalen, srealm_oldlen;
Der_type srealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &srealm_type, 9, &srealm_datalen, &l);
if (e == 0 && srealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
srealm_oldlen = len;
if (srealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = srealm_datalen;
e = decode_Realm(p, len, &(data)->srealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = srealm_oldlen - srealm_datalen;
}
{
size_t sname_datalen, sname_oldlen;
Der_type sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sname_type, 10, &sname_datalen, &l);
if (e == 0 && sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sname_oldlen = len;
if (sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sname_datalen;
e = decode_PrincipalName(p, len, &(data)->sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sname_oldlen - sname_datalen;
}
{
size_t caddr_datalen, caddr_oldlen;
Der_type caddr_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &caddr_type, 11, &caddr_datalen, &l);
if (e == 0 && caddr_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->caddr = NULL;
} else {
(data)->caddr = calloc(1, sizeof(*(data)->caddr));
if ((data)->caddr == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
caddr_oldlen = len;
if (caddr_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = caddr_datalen;
e = decode_HostAddresses(p, len, (data)->caddr, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = caddr_oldlen - caddr_datalen;
}
}
{
size_t encrypted_pa_data_datalen, encrypted_pa_data_oldlen;
Der_type encrypted_pa_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &encrypted_pa_data_type, 12, &encrypted_pa_data_datalen, &l);
if (e == 0 && encrypted_pa_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->encrypted_pa_data = NULL;
} else {
(data)->encrypted_pa_data = calloc(1, sizeof(*(data)->encrypted_pa_data));
if ((data)->encrypted_pa_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
encrypted_pa_data_oldlen = len;
if (encrypted_pa_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = encrypted_pa_data_datalen;
e = decode_METHOD_DATA(p, len, (data)->encrypted_pa_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = encrypted_pa_data_oldlen - encrypted_pa_data_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncKDCRepPart(data);
return e;
}

void ASN1CALL
free_EncKDCRepPart(EncKDCRepPart *data)
{
free_EncryptionKey(&(data)->key);
free_LastReq(&(data)->last_req);
free_krb5int32(&(data)->nonce);
if((data)->key_expiration) {
free_KerberosTime((data)->key_expiration);
free((data)->key_expiration);
(data)->key_expiration = NULL;
}
free_TicketFlags(&(data)->flags);
free_KerberosTime(&(data)->authtime);
if((data)->starttime) {
free_KerberosTime((data)->starttime);
free((data)->starttime);
(data)->starttime = NULL;
}
free_KerberosTime(&(data)->endtime);
if((data)->renew_till) {
free_KerberosTime((data)->renew_till);
free((data)->renew_till);
(data)->renew_till = NULL;
}
free_Realm(&(data)->srealm);
free_PrincipalName(&(data)->sname);
if((data)->caddr) {
free_HostAddresses((data)->caddr);
free((data)->caddr);
(data)->caddr = NULL;
}
if((data)->encrypted_pa_data) {
free_METHOD_DATA((data)->encrypted_pa_data);
free((data)->encrypted_pa_data);
(data)->encrypted_pa_data = NULL;
}
}

size_t ASN1CALL
length_EncKDCRepPart(const EncKDCRepPart *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey(&(data)->key);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_LastReq(&(data)->last_req);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->key_expiration){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->key_expiration);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_TicketFlags(&(data)->flags);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->authtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->starttime){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->starttime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->endtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->renew_till){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->renew_till);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->srealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->caddr){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_HostAddresses((data)->caddr);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->encrypted_pa_data){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_METHOD_DATA((data)->encrypted_pa_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncKDCRepPart(const EncKDCRepPart *from, EncKDCRepPart *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncryptionKey(&(from)->key, &(to)->key)) goto fail;
if(copy_LastReq(&(from)->last_req, &(to)->last_req)) goto fail;
if(copy_krb5int32(&(from)->nonce, &(to)->nonce)) goto fail;
if((from)->key_expiration) {
(to)->key_expiration = malloc(sizeof(*(to)->key_expiration));
if((to)->key_expiration == NULL) goto fail;
if(copy_KerberosTime((from)->key_expiration, (to)->key_expiration)) goto fail;
}else
(to)->key_expiration = NULL;
if(copy_TicketFlags(&(from)->flags, &(to)->flags)) goto fail;
if(copy_KerberosTime(&(from)->authtime, &(to)->authtime)) goto fail;
if((from)->starttime) {
(to)->starttime = malloc(sizeof(*(to)->starttime));
if((to)->starttime == NULL) goto fail;
if(copy_KerberosTime((from)->starttime, (to)->starttime)) goto fail;
}else
(to)->starttime = NULL;
if(copy_KerberosTime(&(from)->endtime, &(to)->endtime)) goto fail;
if((from)->renew_till) {
(to)->renew_till = malloc(sizeof(*(to)->renew_till));
if((to)->renew_till == NULL) goto fail;
if(copy_KerberosTime((from)->renew_till, (to)->renew_till)) goto fail;
}else
(to)->renew_till = NULL;
if(copy_Realm(&(from)->srealm, &(to)->srealm)) goto fail;
if(copy_PrincipalName(&(from)->sname, &(to)->sname)) goto fail;
if((from)->caddr) {
(to)->caddr = malloc(sizeof(*(to)->caddr));
if((to)->caddr == NULL) goto fail;
if(copy_HostAddresses((from)->caddr, (to)->caddr)) goto fail;
}else
(to)->caddr = NULL;
if((from)->encrypted_pa_data) {
(to)->encrypted_pa_data = malloc(sizeof(*(to)->encrypted_pa_data));
if((to)->encrypted_pa_data == NULL) goto fail;
if(copy_METHOD_DATA((from)->encrypted_pa_data, (to)->encrypted_pa_data)) goto fail;
}else
(to)->encrypted_pa_data = NULL;
return 0;
fail:
free_EncKDCRepPart(to);
return ENOMEM;
}

int ASN1CALL
encode_EncASRepPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncASRepPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_EncKDCRepPart(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 25, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncASRepPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncASRepPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 25, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_EncKDCRepPart(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncASRepPart(data);
return e;
}

void ASN1CALL
free_EncASRepPart(EncASRepPart *data)
{
free_EncKDCRepPart(data);
}

size_t ASN1CALL
length_EncASRepPart(const EncASRepPart *data)
{
size_t ret = 0;
ret += length_EncKDCRepPart(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncASRepPart(const EncASRepPart *from, EncASRepPart *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncKDCRepPart(from, to)) goto fail;
return 0;
fail:
free_EncASRepPart(to);
return ENOMEM;
}

int ASN1CALL
encode_EncTGSRepPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncTGSRepPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_EncKDCRepPart(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 26, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncTGSRepPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncTGSRepPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 26, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
e = decode_EncKDCRepPart(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncTGSRepPart(data);
return e;
}

void ASN1CALL
free_EncTGSRepPart(EncTGSRepPart *data)
{
free_EncKDCRepPart(data);
}

size_t ASN1CALL
length_EncTGSRepPart(const EncTGSRepPart *data)
{
size_t ret = 0;
ret += length_EncKDCRepPart(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncTGSRepPart(const EncTGSRepPart *from, EncTGSRepPart *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncKDCRepPart(from, to)) goto fail;
return 0;
fail:
free_EncTGSRepPart(to);
return ENOMEM;
}

int ASN1CALL
encode_AP_REQ(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AP_REQ *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* authenticator */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->authenticator, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ticket */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Ticket(p, len, &(data)->ticket, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ap-options */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_APOptions(p, len, &(data)->ap_options, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 14, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AP_REQ(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AP_REQ *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 14, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t ap_options_datalen, ap_options_oldlen;
Der_type ap_options_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ap_options_type, 2, &ap_options_datalen, &l);
if (e == 0 && ap_options_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ap_options_oldlen = len;
if (ap_options_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ap_options_datalen;
e = decode_APOptions(p, len, &(data)->ap_options, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ap_options_oldlen - ap_options_datalen;
}
{
size_t ticket_datalen, ticket_oldlen;
Der_type ticket_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ticket_type, 3, &ticket_datalen, &l);
if (e == 0 && ticket_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ticket_oldlen = len;
if (ticket_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ticket_datalen;
e = decode_Ticket(p, len, &(data)->ticket, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ticket_oldlen - ticket_datalen;
}
{
size_t authenticator_datalen, authenticator_oldlen;
Der_type authenticator_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authenticator_type, 4, &authenticator_datalen, &l);
if (e == 0 && authenticator_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
authenticator_oldlen = len;
if (authenticator_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authenticator_datalen;
e = decode_EncryptedData(p, len, &(data)->authenticator, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authenticator_oldlen - authenticator_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AP_REQ(data);
return e;
}

void ASN1CALL
free_AP_REQ(AP_REQ *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
free_APOptions(&(data)->ap_options);
free_Ticket(&(data)->ticket);
free_EncryptedData(&(data)->authenticator);
}

size_t ASN1CALL
length_AP_REQ(const AP_REQ *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_APOptions(&(data)->ap_options);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Ticket(&(data)->ticket);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->authenticator);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AP_REQ(const AP_REQ *from, AP_REQ *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if(copy_APOptions(&(from)->ap_options, &(to)->ap_options)) goto fail;
if(copy_Ticket(&(from)->ticket, &(to)->ticket)) goto fail;
if(copy_EncryptedData(&(from)->authenticator, &(to)->authenticator)) goto fail;
return 0;
fail:
free_AP_REQ(to);
return ENOMEM;
}

int ASN1CALL
encode_AP_REP(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AP_REP *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-part */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_part, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 15, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AP_REP(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AP_REP *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 15, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t enc_part_datalen, enc_part_oldlen;
Der_type enc_part_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_part_type, 2, &enc_part_datalen, &l);
if (e == 0 && enc_part_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_part_oldlen = len;
if (enc_part_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_part_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_part, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_part_oldlen - enc_part_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AP_REP(data);
return e;
}

void ASN1CALL
free_AP_REP(AP_REP *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
free_EncryptedData(&(data)->enc_part);
}

size_t ASN1CALL
length_AP_REP(const AP_REP *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_part);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AP_REP(const AP_REP *from, AP_REP *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if(copy_EncryptedData(&(from)->enc_part, &(to)->enc_part)) goto fail;
return 0;
fail:
free_AP_REP(to);
return ENOMEM;
}

int ASN1CALL
encode_EncAPRepPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncAPRepPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* seq-number */
if((data)->seq_number) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5uint32(p, len, (data)->seq_number, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* subkey */
if((data)->subkey) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, (data)->subkey, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cusec */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->cusec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ctime */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->ctime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 27, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncAPRepPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncAPRepPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 27, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t ctime_datalen, ctime_oldlen;
Der_type ctime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ctime_type, 0, &ctime_datalen, &l);
if (e == 0 && ctime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ctime_oldlen = len;
if (ctime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ctime_datalen;
e = decode_KerberosTime(p, len, &(data)->ctime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ctime_oldlen - ctime_datalen;
}
{
size_t cusec_datalen, cusec_oldlen;
Der_type cusec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cusec_type, 1, &cusec_datalen, &l);
if (e == 0 && cusec_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cusec_oldlen = len;
if (cusec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cusec_datalen;
e = decode_krb5int32(p, len, &(data)->cusec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cusec_oldlen - cusec_datalen;
}
{
size_t subkey_datalen, subkey_oldlen;
Der_type subkey_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &subkey_type, 2, &subkey_datalen, &l);
if (e == 0 && subkey_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->subkey = NULL;
} else {
(data)->subkey = calloc(1, sizeof(*(data)->subkey));
if ((data)->subkey == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
subkey_oldlen = len;
if (subkey_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = subkey_datalen;
e = decode_EncryptionKey(p, len, (data)->subkey, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = subkey_oldlen - subkey_datalen;
}
}
{
size_t seq_number_datalen, seq_number_oldlen;
Der_type seq_number_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &seq_number_type, 3, &seq_number_datalen, &l);
if (e == 0 && seq_number_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->seq_number = NULL;
} else {
(data)->seq_number = calloc(1, sizeof(*(data)->seq_number));
if ((data)->seq_number == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
seq_number_oldlen = len;
if (seq_number_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = seq_number_datalen;
e = decode_krb5uint32(p, len, (data)->seq_number, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = seq_number_oldlen - seq_number_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncAPRepPart(data);
return e;
}

void ASN1CALL
free_EncAPRepPart(EncAPRepPart *data)
{
free_KerberosTime(&(data)->ctime);
free_krb5int32(&(data)->cusec);
if((data)->subkey) {
free_EncryptionKey((data)->subkey);
free((data)->subkey);
(data)->subkey = NULL;
}
if((data)->seq_number) {
free_krb5uint32((data)->seq_number);
free((data)->seq_number);
(data)->seq_number = NULL;
}
}

size_t ASN1CALL
length_EncAPRepPart(const EncAPRepPart *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->ctime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->cusec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->subkey){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey((data)->subkey);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->seq_number){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5uint32((data)->seq_number);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncAPRepPart(const EncAPRepPart *from, EncAPRepPart *to)
{
memset(to, 0, sizeof(*to));
if(copy_KerberosTime(&(from)->ctime, &(to)->ctime)) goto fail;
if(copy_krb5int32(&(from)->cusec, &(to)->cusec)) goto fail;
if((from)->subkey) {
(to)->subkey = malloc(sizeof(*(to)->subkey));
if((to)->subkey == NULL) goto fail;
if(copy_EncryptionKey((from)->subkey, (to)->subkey)) goto fail;
}else
(to)->subkey = NULL;
if((from)->seq_number) {
(to)->seq_number = malloc(sizeof(*(to)->seq_number));
if((to)->seq_number == NULL) goto fail;
if(copy_krb5uint32((from)->seq_number, (to)->seq_number)) goto fail;
}else
(to)->seq_number = NULL;
return 0;
fail:
free_EncAPRepPart(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB_SAFE_BODY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB_SAFE_BODY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* r-address */
if((data)->r_address) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->r_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* s-address */
if((data)->s_address) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->s_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* seq-number */
if((data)->seq_number) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5uint32(p, len, (data)->seq_number, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* usec */
if((data)->usec) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->usec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* timestamp */
if((data)->timestamp) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->timestamp, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* user-data */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->user_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB_SAFE_BODY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB_SAFE_BODY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t user_data_datalen, user_data_oldlen;
Der_type user_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &user_data_type, 0, &user_data_datalen, &l);
if (e == 0 && user_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
user_data_oldlen = len;
if (user_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = user_data_datalen;
{
size_t user_data_Tag_datalen, user_data_Tag_oldlen;
Der_type user_data_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &user_data_Tag_type, UT_OctetString, &user_data_Tag_datalen, &l);
if (e == 0 && user_data_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
user_data_Tag_oldlen = len;
if (user_data_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = user_data_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->user_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = user_data_Tag_oldlen - user_data_Tag_datalen;
}
len = user_data_oldlen - user_data_datalen;
}
{
size_t timestamp_datalen, timestamp_oldlen;
Der_type timestamp_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &timestamp_type, 1, &timestamp_datalen, &l);
if (e == 0 && timestamp_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->timestamp = NULL;
} else {
(data)->timestamp = calloc(1, sizeof(*(data)->timestamp));
if ((data)->timestamp == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
timestamp_oldlen = len;
if (timestamp_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = timestamp_datalen;
e = decode_KerberosTime(p, len, (data)->timestamp, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = timestamp_oldlen - timestamp_datalen;
}
}
{
size_t usec_datalen, usec_oldlen;
Der_type usec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &usec_type, 2, &usec_datalen, &l);
if (e == 0 && usec_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->usec = NULL;
} else {
(data)->usec = calloc(1, sizeof(*(data)->usec));
if ((data)->usec == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
usec_oldlen = len;
if (usec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = usec_datalen;
e = decode_krb5int32(p, len, (data)->usec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = usec_oldlen - usec_datalen;
}
}
{
size_t seq_number_datalen, seq_number_oldlen;
Der_type seq_number_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &seq_number_type, 3, &seq_number_datalen, &l);
if (e == 0 && seq_number_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->seq_number = NULL;
} else {
(data)->seq_number = calloc(1, sizeof(*(data)->seq_number));
if ((data)->seq_number == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
seq_number_oldlen = len;
if (seq_number_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = seq_number_datalen;
e = decode_krb5uint32(p, len, (data)->seq_number, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = seq_number_oldlen - seq_number_datalen;
}
}
{
size_t s_address_datalen, s_address_oldlen;
Der_type s_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &s_address_type, 4, &s_address_datalen, &l);
if (e == 0 && s_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->s_address = NULL;
} else {
(data)->s_address = calloc(1, sizeof(*(data)->s_address));
if ((data)->s_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
s_address_oldlen = len;
if (s_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = s_address_datalen;
e = decode_HostAddress(p, len, (data)->s_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = s_address_oldlen - s_address_datalen;
}
}
{
size_t r_address_datalen, r_address_oldlen;
Der_type r_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &r_address_type, 5, &r_address_datalen, &l);
if (e == 0 && r_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->r_address = NULL;
} else {
(data)->r_address = calloc(1, sizeof(*(data)->r_address));
if ((data)->r_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
r_address_oldlen = len;
if (r_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = r_address_datalen;
e = decode_HostAddress(p, len, (data)->r_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = r_address_oldlen - r_address_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB_SAFE_BODY(data);
return e;
}

void ASN1CALL
free_KRB_SAFE_BODY(KRB_SAFE_BODY *data)
{
der_free_octet_string(&(data)->user_data);
if((data)->timestamp) {
free_KerberosTime((data)->timestamp);
free((data)->timestamp);
(data)->timestamp = NULL;
}
if((data)->usec) {
free_krb5int32((data)->usec);
free((data)->usec);
(data)->usec = NULL;
}
if((data)->seq_number) {
free_krb5uint32((data)->seq_number);
free((data)->seq_number);
(data)->seq_number = NULL;
}
if((data)->s_address) {
free_HostAddress((data)->s_address);
free((data)->s_address);
(data)->s_address = NULL;
}
if((data)->r_address) {
free_HostAddress((data)->r_address);
free((data)->r_address);
(data)->r_address = NULL;
}
}

size_t ASN1CALL
length_KRB_SAFE_BODY(const KRB_SAFE_BODY *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->user_data);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->timestamp){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->timestamp);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->usec){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->usec);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->seq_number){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5uint32((data)->seq_number);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->s_address){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->s_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->r_address){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->r_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB_SAFE_BODY(const KRB_SAFE_BODY *from, KRB_SAFE_BODY *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_octet_string(&(from)->user_data, &(to)->user_data)) goto fail;
if((from)->timestamp) {
(to)->timestamp = malloc(sizeof(*(to)->timestamp));
if((to)->timestamp == NULL) goto fail;
if(copy_KerberosTime((from)->timestamp, (to)->timestamp)) goto fail;
}else
(to)->timestamp = NULL;
if((from)->usec) {
(to)->usec = malloc(sizeof(*(to)->usec));
if((to)->usec == NULL) goto fail;
if(copy_krb5int32((from)->usec, (to)->usec)) goto fail;
}else
(to)->usec = NULL;
if((from)->seq_number) {
(to)->seq_number = malloc(sizeof(*(to)->seq_number));
if((to)->seq_number == NULL) goto fail;
if(copy_krb5uint32((from)->seq_number, (to)->seq_number)) goto fail;
}else
(to)->seq_number = NULL;
if((from)->s_address) {
(to)->s_address = malloc(sizeof(*(to)->s_address));
if((to)->s_address == NULL) goto fail;
if(copy_HostAddress((from)->s_address, (to)->s_address)) goto fail;
}else
(to)->s_address = NULL;
if((from)->r_address) {
(to)->r_address = malloc(sizeof(*(to)->r_address));
if((to)->r_address == NULL) goto fail;
if(copy_HostAddress((from)->r_address, (to)->r_address)) goto fail;
}else
(to)->r_address = NULL;
return 0;
fail:
free_KRB_SAFE_BODY(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB_SAFE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB_SAFE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* cksum */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->cksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* safe-body */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KRB_SAFE_BODY(p, len, &(data)->safe_body, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 20, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB_SAFE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB_SAFE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 20, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t safe_body_datalen, safe_body_oldlen;
Der_type safe_body_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &safe_body_type, 2, &safe_body_datalen, &l);
if (e == 0 && safe_body_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
safe_body_oldlen = len;
if (safe_body_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = safe_body_datalen;
e = decode_KRB_SAFE_BODY(p, len, &(data)->safe_body, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = safe_body_oldlen - safe_body_datalen;
}
{
size_t cksum_datalen, cksum_oldlen;
Der_type cksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cksum_type, 3, &cksum_datalen, &l);
if (e == 0 && cksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cksum_oldlen = len;
if (cksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cksum_datalen;
e = decode_Checksum(p, len, &(data)->cksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cksum_oldlen - cksum_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB_SAFE(data);
return e;
}

void ASN1CALL
free_KRB_SAFE(KRB_SAFE *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
free_KRB_SAFE_BODY(&(data)->safe_body);
free_Checksum(&(data)->cksum);
}

size_t ASN1CALL
length_KRB_SAFE(const KRB_SAFE *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KRB_SAFE_BODY(&(data)->safe_body);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->cksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB_SAFE(const KRB_SAFE *from, KRB_SAFE *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if(copy_KRB_SAFE_BODY(&(from)->safe_body, &(to)->safe_body)) goto fail;
if(copy_Checksum(&(from)->cksum, &(to)->cksum)) goto fail;
return 0;
fail:
free_KRB_SAFE(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB_PRIV(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB_PRIV *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-part */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_part, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 21, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB_PRIV(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB_PRIV *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 21, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t enc_part_datalen, enc_part_oldlen;
Der_type enc_part_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_part_type, 3, &enc_part_datalen, &l);
if (e == 0 && enc_part_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_part_oldlen = len;
if (enc_part_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_part_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_part, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_part_oldlen - enc_part_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB_PRIV(data);
return e;
}

void ASN1CALL
free_KRB_PRIV(KRB_PRIV *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
free_EncryptedData(&(data)->enc_part);
}

size_t ASN1CALL
length_KRB_PRIV(const KRB_PRIV *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_part);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB_PRIV(const KRB_PRIV *from, KRB_PRIV *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if(copy_EncryptedData(&(from)->enc_part, &(to)->enc_part)) goto fail;
return 0;
fail:
free_KRB_PRIV(to);
return ENOMEM;
}

int ASN1CALL
encode_EncKrbPrivPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncKrbPrivPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* r-address */
if((data)->r_address) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->r_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* s-address */
if((data)->s_address) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->s_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* seq-number */
if((data)->seq_number) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5uint32(p, len, (data)->seq_number, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* usec */
if((data)->usec) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->usec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* timestamp */
if((data)->timestamp) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->timestamp, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* user-data */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->user_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 28, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncKrbPrivPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncKrbPrivPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 28, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t user_data_datalen, user_data_oldlen;
Der_type user_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &user_data_type, 0, &user_data_datalen, &l);
if (e == 0 && user_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
user_data_oldlen = len;
if (user_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = user_data_datalen;
{
size_t user_data_Tag_datalen, user_data_Tag_oldlen;
Der_type user_data_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &user_data_Tag_type, UT_OctetString, &user_data_Tag_datalen, &l);
if (e == 0 && user_data_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
user_data_Tag_oldlen = len;
if (user_data_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = user_data_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->user_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = user_data_Tag_oldlen - user_data_Tag_datalen;
}
len = user_data_oldlen - user_data_datalen;
}
{
size_t timestamp_datalen, timestamp_oldlen;
Der_type timestamp_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &timestamp_type, 1, &timestamp_datalen, &l);
if (e == 0 && timestamp_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->timestamp = NULL;
} else {
(data)->timestamp = calloc(1, sizeof(*(data)->timestamp));
if ((data)->timestamp == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
timestamp_oldlen = len;
if (timestamp_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = timestamp_datalen;
e = decode_KerberosTime(p, len, (data)->timestamp, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = timestamp_oldlen - timestamp_datalen;
}
}
{
size_t usec_datalen, usec_oldlen;
Der_type usec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &usec_type, 2, &usec_datalen, &l);
if (e == 0 && usec_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->usec = NULL;
} else {
(data)->usec = calloc(1, sizeof(*(data)->usec));
if ((data)->usec == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
usec_oldlen = len;
if (usec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = usec_datalen;
e = decode_krb5int32(p, len, (data)->usec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = usec_oldlen - usec_datalen;
}
}
{
size_t seq_number_datalen, seq_number_oldlen;
Der_type seq_number_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &seq_number_type, 3, &seq_number_datalen, &l);
if (e == 0 && seq_number_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->seq_number = NULL;
} else {
(data)->seq_number = calloc(1, sizeof(*(data)->seq_number));
if ((data)->seq_number == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
seq_number_oldlen = len;
if (seq_number_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = seq_number_datalen;
e = decode_krb5uint32(p, len, (data)->seq_number, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = seq_number_oldlen - seq_number_datalen;
}
}
{
size_t s_address_datalen, s_address_oldlen;
Der_type s_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &s_address_type, 4, &s_address_datalen, &l);
if (e == 0 && s_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->s_address = NULL;
} else {
(data)->s_address = calloc(1, sizeof(*(data)->s_address));
if ((data)->s_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
s_address_oldlen = len;
if (s_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = s_address_datalen;
e = decode_HostAddress(p, len, (data)->s_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = s_address_oldlen - s_address_datalen;
}
}
{
size_t r_address_datalen, r_address_oldlen;
Der_type r_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &r_address_type, 5, &r_address_datalen, &l);
if (e == 0 && r_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->r_address = NULL;
} else {
(data)->r_address = calloc(1, sizeof(*(data)->r_address));
if ((data)->r_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
r_address_oldlen = len;
if (r_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = r_address_datalen;
e = decode_HostAddress(p, len, (data)->r_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = r_address_oldlen - r_address_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncKrbPrivPart(data);
return e;
}

void ASN1CALL
free_EncKrbPrivPart(EncKrbPrivPart *data)
{
der_free_octet_string(&(data)->user_data);
if((data)->timestamp) {
free_KerberosTime((data)->timestamp);
free((data)->timestamp);
(data)->timestamp = NULL;
}
if((data)->usec) {
free_krb5int32((data)->usec);
free((data)->usec);
(data)->usec = NULL;
}
if((data)->seq_number) {
free_krb5uint32((data)->seq_number);
free((data)->seq_number);
(data)->seq_number = NULL;
}
if((data)->s_address) {
free_HostAddress((data)->s_address);
free((data)->s_address);
(data)->s_address = NULL;
}
if((data)->r_address) {
free_HostAddress((data)->r_address);
free((data)->r_address);
(data)->r_address = NULL;
}
}

size_t ASN1CALL
length_EncKrbPrivPart(const EncKrbPrivPart *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->user_data);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->timestamp){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->timestamp);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->usec){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->usec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->seq_number){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5uint32((data)->seq_number);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->s_address){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->s_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->r_address){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->r_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncKrbPrivPart(const EncKrbPrivPart *from, EncKrbPrivPart *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_octet_string(&(from)->user_data, &(to)->user_data)) goto fail;
if((from)->timestamp) {
(to)->timestamp = malloc(sizeof(*(to)->timestamp));
if((to)->timestamp == NULL) goto fail;
if(copy_KerberosTime((from)->timestamp, (to)->timestamp)) goto fail;
}else
(to)->timestamp = NULL;
if((from)->usec) {
(to)->usec = malloc(sizeof(*(to)->usec));
if((to)->usec == NULL) goto fail;
if(copy_krb5int32((from)->usec, (to)->usec)) goto fail;
}else
(to)->usec = NULL;
if((from)->seq_number) {
(to)->seq_number = malloc(sizeof(*(to)->seq_number));
if((to)->seq_number == NULL) goto fail;
if(copy_krb5uint32((from)->seq_number, (to)->seq_number)) goto fail;
}else
(to)->seq_number = NULL;
if((from)->s_address) {
(to)->s_address = malloc(sizeof(*(to)->s_address));
if((to)->s_address == NULL) goto fail;
if(copy_HostAddress((from)->s_address, (to)->s_address)) goto fail;
}else
(to)->s_address = NULL;
if((from)->r_address) {
(to)->r_address = malloc(sizeof(*(to)->r_address));
if((to)->r_address == NULL) goto fail;
if(copy_HostAddress((from)->r_address, (to)->r_address)) goto fail;
}else
(to)->r_address = NULL;
return 0;
fail:
free_EncKrbPrivPart(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB_CRED(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB_CRED *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-part */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_part, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* tickets */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->tickets)->len - 1; i >= 0; --i) {
size_t tickets_tag_tag_for_oldret = ret;
ret = 0;
e = encode_Ticket(p, len, &(&(data)->tickets)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += tickets_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 22, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB_CRED(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB_CRED *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 22, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t tickets_datalen, tickets_oldlen;
Der_type tickets_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &tickets_type, 2, &tickets_datalen, &l);
if (e == 0 && tickets_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
tickets_oldlen = len;
if (tickets_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = tickets_datalen;
{
size_t tickets_Tag_datalen, tickets_Tag_oldlen;
Der_type tickets_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &tickets_Tag_type, UT_Sequence, &tickets_Tag_datalen, &l);
if (e == 0 && tickets_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
tickets_Tag_oldlen = len;
if (tickets_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = tickets_Tag_datalen;
{
size_t tickets_Tag_Tag_origlen = len;
size_t tickets_Tag_Tag_oldret = ret;
size_t tickets_Tag_Tag_olen = 0;
void *tickets_Tag_Tag_tmp;
ret = 0;
(&(data)->tickets)->len = 0;
(&(data)->tickets)->val = NULL;
while(ret < tickets_Tag_Tag_origlen) {
size_t tickets_Tag_Tag_nlen = tickets_Tag_Tag_olen + sizeof(*((&(data)->tickets)->val));
if (tickets_Tag_Tag_olen > tickets_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
tickets_Tag_Tag_olen = tickets_Tag_Tag_nlen;
tickets_Tag_Tag_tmp = realloc((&(data)->tickets)->val, tickets_Tag_Tag_olen);
if (tickets_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->tickets)->val = tickets_Tag_Tag_tmp;
e = decode_Ticket(p, len, &(&(data)->tickets)->val[(&(data)->tickets)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->tickets)->len++;
len = tickets_Tag_Tag_origlen - ret;
}
ret += tickets_Tag_Tag_oldret;
}
len = tickets_Tag_oldlen - tickets_Tag_datalen;
}
len = tickets_oldlen - tickets_datalen;
}
{
size_t enc_part_datalen, enc_part_oldlen;
Der_type enc_part_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_part_type, 3, &enc_part_datalen, &l);
if (e == 0 && enc_part_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_part_oldlen = len;
if (enc_part_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_part_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_part, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_part_oldlen - enc_part_datalen;
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB_CRED(data);
return e;
}

void ASN1CALL
free_KRB_CRED(KRB_CRED *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
while((&(data)->tickets)->len){
free_Ticket(&(&(data)->tickets)->val[(&(data)->tickets)->len-1]);
(&(data)->tickets)->len--;
}
free((&(data)->tickets)->val);
(&(data)->tickets)->val = NULL;
free_EncryptedData(&(data)->enc_part);
}

size_t ASN1CALL
length_KRB_CRED(const KRB_CRED *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
{
size_t tickets_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->tickets)->len - 1; i >= 0; --i){
size_t tickets_tag_tag_for_oldret = ret;
ret = 0;
ret += length_Ticket(&(&(data)->tickets)->val[i]);
ret += tickets_tag_tag_for_oldret;
}
ret += tickets_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_part);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB_CRED(const KRB_CRED *from, KRB_CRED *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if(((&(to)->tickets)->val = malloc((&(from)->tickets)->len * sizeof(*(&(to)->tickets)->val))) == NULL && (&(from)->tickets)->len != 0)
goto fail;
for((&(to)->tickets)->len = 0; (&(to)->tickets)->len < (&(from)->tickets)->len; (&(to)->tickets)->len++){
if(copy_Ticket(&(&(from)->tickets)->val[(&(to)->tickets)->len], &(&(to)->tickets)->val[(&(to)->tickets)->len])) goto fail;
}
if(copy_EncryptedData(&(from)->enc_part, &(to)->enc_part)) goto fail;
return 0;
fail:
free_KRB_CRED(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbCredInfo(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbCredInfo *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* caddr */
if((data)->caddr) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddresses(p, len, (data)->caddr, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sname */
if((data)->sname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* srealm */
if((data)->srealm) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->srealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* renew-till */
if((data)->renew_till) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->renew_till, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* endtime */
if((data)->endtime) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->endtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* starttime */
if((data)->starttime) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->starttime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* authtime */
if((data)->authtime) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->authtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* flags */
if((data)->flags) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_TicketFlags(p, len, (data)->flags, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* pname */
if((data)->pname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->pname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* prealm */
if((data)->prealm) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->prealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* key */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, &(data)->key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbCredInfo(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbCredInfo *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t key_datalen, key_oldlen;
Der_type key_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &key_type, 0, &key_datalen, &l);
if (e == 0 && key_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
key_oldlen = len;
if (key_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = key_datalen;
e = decode_EncryptionKey(p, len, &(data)->key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_oldlen - key_datalen;
}
{
size_t prealm_datalen, prealm_oldlen;
Der_type prealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &prealm_type, 1, &prealm_datalen, &l);
if (e == 0 && prealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->prealm = NULL;
} else {
(data)->prealm = calloc(1, sizeof(*(data)->prealm));
if ((data)->prealm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
prealm_oldlen = len;
if (prealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = prealm_datalen;
e = decode_Realm(p, len, (data)->prealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = prealm_oldlen - prealm_datalen;
}
}
{
size_t pname_datalen, pname_oldlen;
Der_type pname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pname_type, 2, &pname_datalen, &l);
if (e == 0 && pname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->pname = NULL;
} else {
(data)->pname = calloc(1, sizeof(*(data)->pname));
if ((data)->pname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
pname_oldlen = len;
if (pname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pname_datalen;
e = decode_PrincipalName(p, len, (data)->pname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pname_oldlen - pname_datalen;
}
}
{
size_t flags_datalen, flags_oldlen;
Der_type flags_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &flags_type, 3, &flags_datalen, &l);
if (e == 0 && flags_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->flags = NULL;
} else {
(data)->flags = calloc(1, sizeof(*(data)->flags));
if ((data)->flags == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
flags_oldlen = len;
if (flags_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = flags_datalen;
e = decode_TicketFlags(p, len, (data)->flags, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = flags_oldlen - flags_datalen;
}
}
{
size_t authtime_datalen, authtime_oldlen;
Der_type authtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authtime_type, 4, &authtime_datalen, &l);
if (e == 0 && authtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->authtime = NULL;
} else {
(data)->authtime = calloc(1, sizeof(*(data)->authtime));
if ((data)->authtime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
authtime_oldlen = len;
if (authtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authtime_datalen;
e = decode_KerberosTime(p, len, (data)->authtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authtime_oldlen - authtime_datalen;
}
}
{
size_t starttime_datalen, starttime_oldlen;
Der_type starttime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &starttime_type, 5, &starttime_datalen, &l);
if (e == 0 && starttime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->starttime = NULL;
} else {
(data)->starttime = calloc(1, sizeof(*(data)->starttime));
if ((data)->starttime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
starttime_oldlen = len;
if (starttime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = starttime_datalen;
e = decode_KerberosTime(p, len, (data)->starttime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = starttime_oldlen - starttime_datalen;
}
}
{
size_t endtime_datalen, endtime_oldlen;
Der_type endtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &endtime_type, 6, &endtime_datalen, &l);
if (e == 0 && endtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->endtime = NULL;
} else {
(data)->endtime = calloc(1, sizeof(*(data)->endtime));
if ((data)->endtime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
endtime_oldlen = len;
if (endtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = endtime_datalen;
e = decode_KerberosTime(p, len, (data)->endtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = endtime_oldlen - endtime_datalen;
}
}
{
size_t renew_till_datalen, renew_till_oldlen;
Der_type renew_till_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &renew_till_type, 7, &renew_till_datalen, &l);
if (e == 0 && renew_till_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->renew_till = NULL;
} else {
(data)->renew_till = calloc(1, sizeof(*(data)->renew_till));
if ((data)->renew_till == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
renew_till_oldlen = len;
if (renew_till_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = renew_till_datalen;
e = decode_KerberosTime(p, len, (data)->renew_till, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = renew_till_oldlen - renew_till_datalen;
}
}
{
size_t srealm_datalen, srealm_oldlen;
Der_type srealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &srealm_type, 8, &srealm_datalen, &l);
if (e == 0 && srealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->srealm = NULL;
} else {
(data)->srealm = calloc(1, sizeof(*(data)->srealm));
if ((data)->srealm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
srealm_oldlen = len;
if (srealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = srealm_datalen;
e = decode_Realm(p, len, (data)->srealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = srealm_oldlen - srealm_datalen;
}
}
{
size_t sname_datalen, sname_oldlen;
Der_type sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sname_type, 9, &sname_datalen, &l);
if (e == 0 && sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sname = NULL;
} else {
(data)->sname = calloc(1, sizeof(*(data)->sname));
if ((data)->sname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sname_oldlen = len;
if (sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sname_datalen;
e = decode_PrincipalName(p, len, (data)->sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sname_oldlen - sname_datalen;
}
}
{
size_t caddr_datalen, caddr_oldlen;
Der_type caddr_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &caddr_type, 10, &caddr_datalen, &l);
if (e == 0 && caddr_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->caddr = NULL;
} else {
(data)->caddr = calloc(1, sizeof(*(data)->caddr));
if ((data)->caddr == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
caddr_oldlen = len;
if (caddr_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = caddr_datalen;
e = decode_HostAddresses(p, len, (data)->caddr, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = caddr_oldlen - caddr_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbCredInfo(data);
return e;
}

void ASN1CALL
free_KrbCredInfo(KrbCredInfo *data)
{
free_EncryptionKey(&(data)->key);
if((data)->prealm) {
free_Realm((data)->prealm);
free((data)->prealm);
(data)->prealm = NULL;
}
if((data)->pname) {
free_PrincipalName((data)->pname);
free((data)->pname);
(data)->pname = NULL;
}
if((data)->flags) {
free_TicketFlags((data)->flags);
free((data)->flags);
(data)->flags = NULL;
}
if((data)->authtime) {
free_KerberosTime((data)->authtime);
free((data)->authtime);
(data)->authtime = NULL;
}
if((data)->starttime) {
free_KerberosTime((data)->starttime);
free((data)->starttime);
(data)->starttime = NULL;
}
if((data)->endtime) {
free_KerberosTime((data)->endtime);
free((data)->endtime);
(data)->endtime = NULL;
}
if((data)->renew_till) {
free_KerberosTime((data)->renew_till);
free((data)->renew_till);
(data)->renew_till = NULL;
}
if((data)->srealm) {
free_Realm((data)->srealm);
free((data)->srealm);
(data)->srealm = NULL;
}
if((data)->sname) {
free_PrincipalName((data)->sname);
free((data)->sname);
(data)->sname = NULL;
}
if((data)->caddr) {
free_HostAddresses((data)->caddr);
free((data)->caddr);
(data)->caddr = NULL;
}
}

size_t ASN1CALL
length_KrbCredInfo(const KrbCredInfo *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey(&(data)->key);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->prealm){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->prealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->pname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->pname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->flags){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_TicketFlags((data)->flags);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->authtime){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->authtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->starttime){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->starttime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->endtime){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->endtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->renew_till){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->renew_till);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->srealm){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->srealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->caddr){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_HostAddresses((data)->caddr);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbCredInfo(const KrbCredInfo *from, KrbCredInfo *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncryptionKey(&(from)->key, &(to)->key)) goto fail;
if((from)->prealm) {
(to)->prealm = malloc(sizeof(*(to)->prealm));
if((to)->prealm == NULL) goto fail;
if(copy_Realm((from)->prealm, (to)->prealm)) goto fail;
}else
(to)->prealm = NULL;
if((from)->pname) {
(to)->pname = malloc(sizeof(*(to)->pname));
if((to)->pname == NULL) goto fail;
if(copy_PrincipalName((from)->pname, (to)->pname)) goto fail;
}else
(to)->pname = NULL;
if((from)->flags) {
(to)->flags = malloc(sizeof(*(to)->flags));
if((to)->flags == NULL) goto fail;
if(copy_TicketFlags((from)->flags, (to)->flags)) goto fail;
}else
(to)->flags = NULL;
if((from)->authtime) {
(to)->authtime = malloc(sizeof(*(to)->authtime));
if((to)->authtime == NULL) goto fail;
if(copy_KerberosTime((from)->authtime, (to)->authtime)) goto fail;
}else
(to)->authtime = NULL;
if((from)->starttime) {
(to)->starttime = malloc(sizeof(*(to)->starttime));
if((to)->starttime == NULL) goto fail;
if(copy_KerberosTime((from)->starttime, (to)->starttime)) goto fail;
}else
(to)->starttime = NULL;
if((from)->endtime) {
(to)->endtime = malloc(sizeof(*(to)->endtime));
if((to)->endtime == NULL) goto fail;
if(copy_KerberosTime((from)->endtime, (to)->endtime)) goto fail;
}else
(to)->endtime = NULL;
if((from)->renew_till) {
(to)->renew_till = malloc(sizeof(*(to)->renew_till));
if((to)->renew_till == NULL) goto fail;
if(copy_KerberosTime((from)->renew_till, (to)->renew_till)) goto fail;
}else
(to)->renew_till = NULL;
if((from)->srealm) {
(to)->srealm = malloc(sizeof(*(to)->srealm));
if((to)->srealm == NULL) goto fail;
if(copy_Realm((from)->srealm, (to)->srealm)) goto fail;
}else
(to)->srealm = NULL;
if((from)->sname) {
(to)->sname = malloc(sizeof(*(to)->sname));
if((to)->sname == NULL) goto fail;
if(copy_PrincipalName((from)->sname, (to)->sname)) goto fail;
}else
(to)->sname = NULL;
if((from)->caddr) {
(to)->caddr = malloc(sizeof(*(to)->caddr));
if((to)->caddr == NULL) goto fail;
if(copy_HostAddresses((from)->caddr, (to)->caddr)) goto fail;
}else
(to)->caddr = NULL;
return 0;
fail:
free_KrbCredInfo(to);
return ENOMEM;
}

int ASN1CALL
encode_EncKrbCredPart(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncKrbCredPart *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* r-address */
if((data)->r_address) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->r_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* s-address */
if((data)->s_address) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_HostAddress(p, len, (data)->s_address, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* usec */
if((data)->usec) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->usec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* timestamp */
if((data)->timestamp) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->timestamp, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* nonce */
if((data)->nonce) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ticket-info */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->ticket_info)->len - 1; i >= 0; --i) {
size_t ticket_info_tag_tag_for_oldret = ret;
ret = 0;
e = encode_KrbCredInfo(p, len, &(&(data)->ticket_info)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += ticket_info_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 29, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EncKrbCredPart(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncKrbCredPart *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 29, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t ticket_info_datalen, ticket_info_oldlen;
Der_type ticket_info_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ticket_info_type, 0, &ticket_info_datalen, &l);
if (e == 0 && ticket_info_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ticket_info_oldlen = len;
if (ticket_info_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ticket_info_datalen;
{
size_t ticket_info_Tag_datalen, ticket_info_Tag_oldlen;
Der_type ticket_info_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &ticket_info_Tag_type, UT_Sequence, &ticket_info_Tag_datalen, &l);
if (e == 0 && ticket_info_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ticket_info_Tag_oldlen = len;
if (ticket_info_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ticket_info_Tag_datalen;
{
size_t ticket_info_Tag_Tag_origlen = len;
size_t ticket_info_Tag_Tag_oldret = ret;
size_t ticket_info_Tag_Tag_olen = 0;
void *ticket_info_Tag_Tag_tmp;
ret = 0;
(&(data)->ticket_info)->len = 0;
(&(data)->ticket_info)->val = NULL;
while(ret < ticket_info_Tag_Tag_origlen) {
size_t ticket_info_Tag_Tag_nlen = ticket_info_Tag_Tag_olen + sizeof(*((&(data)->ticket_info)->val));
if (ticket_info_Tag_Tag_olen > ticket_info_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
ticket_info_Tag_Tag_olen = ticket_info_Tag_Tag_nlen;
ticket_info_Tag_Tag_tmp = realloc((&(data)->ticket_info)->val, ticket_info_Tag_Tag_olen);
if (ticket_info_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->ticket_info)->val = ticket_info_Tag_Tag_tmp;
e = decode_KrbCredInfo(p, len, &(&(data)->ticket_info)->val[(&(data)->ticket_info)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->ticket_info)->len++;
len = ticket_info_Tag_Tag_origlen - ret;
}
ret += ticket_info_Tag_Tag_oldret;
}
len = ticket_info_Tag_oldlen - ticket_info_Tag_datalen;
}
len = ticket_info_oldlen - ticket_info_datalen;
}
{
size_t nonce_datalen, nonce_oldlen;
Der_type nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &nonce_type, 1, &nonce_datalen, &l);
if (e == 0 && nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->nonce = NULL;
} else {
(data)->nonce = calloc(1, sizeof(*(data)->nonce));
if ((data)->nonce == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
nonce_oldlen = len;
if (nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = nonce_datalen;
e = decode_krb5int32(p, len, (data)->nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = nonce_oldlen - nonce_datalen;
}
}
{
size_t timestamp_datalen, timestamp_oldlen;
Der_type timestamp_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &timestamp_type, 2, &timestamp_datalen, &l);
if (e == 0 && timestamp_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->timestamp = NULL;
} else {
(data)->timestamp = calloc(1, sizeof(*(data)->timestamp));
if ((data)->timestamp == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
timestamp_oldlen = len;
if (timestamp_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = timestamp_datalen;
e = decode_KerberosTime(p, len, (data)->timestamp, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = timestamp_oldlen - timestamp_datalen;
}
}
{
size_t usec_datalen, usec_oldlen;
Der_type usec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &usec_type, 3, &usec_datalen, &l);
if (e == 0 && usec_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->usec = NULL;
} else {
(data)->usec = calloc(1, sizeof(*(data)->usec));
if ((data)->usec == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
usec_oldlen = len;
if (usec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = usec_datalen;
e = decode_krb5int32(p, len, (data)->usec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = usec_oldlen - usec_datalen;
}
}
{
size_t s_address_datalen, s_address_oldlen;
Der_type s_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &s_address_type, 4, &s_address_datalen, &l);
if (e == 0 && s_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->s_address = NULL;
} else {
(data)->s_address = calloc(1, sizeof(*(data)->s_address));
if ((data)->s_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
s_address_oldlen = len;
if (s_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = s_address_datalen;
e = decode_HostAddress(p, len, (data)->s_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = s_address_oldlen - s_address_datalen;
}
}
{
size_t r_address_datalen, r_address_oldlen;
Der_type r_address_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &r_address_type, 5, &r_address_datalen, &l);
if (e == 0 && r_address_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->r_address = NULL;
} else {
(data)->r_address = calloc(1, sizeof(*(data)->r_address));
if ((data)->r_address == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
r_address_oldlen = len;
if (r_address_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = r_address_datalen;
e = decode_HostAddress(p, len, (data)->r_address, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = r_address_oldlen - r_address_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EncKrbCredPart(data);
return e;
}

void ASN1CALL
free_EncKrbCredPart(EncKrbCredPart *data)
{
while((&(data)->ticket_info)->len){
free_KrbCredInfo(&(&(data)->ticket_info)->val[(&(data)->ticket_info)->len-1]);
(&(data)->ticket_info)->len--;
}
free((&(data)->ticket_info)->val);
(&(data)->ticket_info)->val = NULL;
if((data)->nonce) {
free_krb5int32((data)->nonce);
free((data)->nonce);
(data)->nonce = NULL;
}
if((data)->timestamp) {
free_KerberosTime((data)->timestamp);
free((data)->timestamp);
(data)->timestamp = NULL;
}
if((data)->usec) {
free_krb5int32((data)->usec);
free((data)->usec);
(data)->usec = NULL;
}
if((data)->s_address) {
free_HostAddress((data)->s_address);
free((data)->s_address);
(data)->s_address = NULL;
}
if((data)->r_address) {
free_HostAddress((data)->r_address);
free((data)->r_address);
(data)->r_address = NULL;
}
}

size_t ASN1CALL
length_EncKrbCredPart(const EncKrbCredPart *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
{
size_t ticket_info_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->ticket_info)->len - 1; i >= 0; --i){
size_t ticket_info_tag_tag_for_oldret = ret;
ret = 0;
ret += length_KrbCredInfo(&(&(data)->ticket_info)->val[i]);
ret += ticket_info_tag_tag_for_oldret;
}
ret += ticket_info_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->nonce){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->timestamp){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->timestamp);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->usec){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->usec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->s_address){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->s_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->r_address){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_HostAddress((data)->r_address);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EncKrbCredPart(const EncKrbCredPart *from, EncKrbCredPart *to)
{
memset(to, 0, sizeof(*to));
if(((&(to)->ticket_info)->val = malloc((&(from)->ticket_info)->len * sizeof(*(&(to)->ticket_info)->val))) == NULL && (&(from)->ticket_info)->len != 0)
goto fail;
for((&(to)->ticket_info)->len = 0; (&(to)->ticket_info)->len < (&(from)->ticket_info)->len; (&(to)->ticket_info)->len++){
if(copy_KrbCredInfo(&(&(from)->ticket_info)->val[(&(to)->ticket_info)->len], &(&(to)->ticket_info)->val[(&(to)->ticket_info)->len])) goto fail;
}
if((from)->nonce) {
(to)->nonce = malloc(sizeof(*(to)->nonce));
if((to)->nonce == NULL) goto fail;
if(copy_krb5int32((from)->nonce, (to)->nonce)) goto fail;
}else
(to)->nonce = NULL;
if((from)->timestamp) {
(to)->timestamp = malloc(sizeof(*(to)->timestamp));
if((to)->timestamp == NULL) goto fail;
if(copy_KerberosTime((from)->timestamp, (to)->timestamp)) goto fail;
}else
(to)->timestamp = NULL;
if((from)->usec) {
(to)->usec = malloc(sizeof(*(to)->usec));
if((to)->usec == NULL) goto fail;
if(copy_krb5int32((from)->usec, (to)->usec)) goto fail;
}else
(to)->usec = NULL;
if((from)->s_address) {
(to)->s_address = malloc(sizeof(*(to)->s_address));
if((to)->s_address == NULL) goto fail;
if(copy_HostAddress((from)->s_address, (to)->s_address)) goto fail;
}else
(to)->s_address = NULL;
if((from)->r_address) {
(to)->r_address = malloc(sizeof(*(to)->r_address));
if((to)->r_address == NULL) goto fail;
if(copy_HostAddress((from)->r_address, (to)->r_address)) goto fail;
}else
(to)->r_address = NULL;
return 0;
fail:
free_EncKrbCredPart(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB_ERROR(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB_ERROR *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* e-data */
if((data)->e_data) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, (data)->e_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 12, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* e-text */
if((data)->e_text) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->e_text, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 11, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* sname */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 10, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* realm */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cname */
if((data)->cname) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* crealm */
if((data)->crealm) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->crealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* error-code */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->error_code, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* susec */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->susec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* stime */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->stime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* cusec */
if((data)->cusec) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, (data)->cusec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* ctime */
if((data)->ctime) {
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->ctime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* msg-type */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
/* pvno */
{
size_t Top_tag_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->pvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_APPL, CONS, 30, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB_ERROR(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB_ERROR *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_APPL, &Top_type, 30, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_datalen, Top_Tag_oldlen;
Der_type Top_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_Tag_type, UT_Sequence, &Top_Tag_datalen, &l);
if (e == 0 && Top_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_Tag_oldlen = len;
if (Top_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_Tag_datalen;
{
size_t pvno_datalen, pvno_oldlen;
Der_type pvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &pvno_type, 0, &pvno_datalen, &l);
if (e == 0 && pvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
pvno_oldlen = len;
if (pvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = pvno_datalen;
e = decode_krb5int32(p, len, &(data)->pvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = pvno_oldlen - pvno_datalen;
}
{
size_t msg_type_datalen, msg_type_oldlen;
Der_type msg_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &msg_type_type, 1, &msg_type_datalen, &l);
if (e == 0 && msg_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
msg_type_oldlen = len;
if (msg_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = msg_type_datalen;
e = decode_MESSAGE_TYPE(p, len, &(data)->msg_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = msg_type_oldlen - msg_type_datalen;
}
{
size_t ctime_datalen, ctime_oldlen;
Der_type ctime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ctime_type, 2, &ctime_datalen, &l);
if (e == 0 && ctime_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->ctime = NULL;
} else {
(data)->ctime = calloc(1, sizeof(*(data)->ctime));
if ((data)->ctime == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
ctime_oldlen = len;
if (ctime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ctime_datalen;
e = decode_KerberosTime(p, len, (data)->ctime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ctime_oldlen - ctime_datalen;
}
}
{
size_t cusec_datalen, cusec_oldlen;
Der_type cusec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cusec_type, 3, &cusec_datalen, &l);
if (e == 0 && cusec_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->cusec = NULL;
} else {
(data)->cusec = calloc(1, sizeof(*(data)->cusec));
if ((data)->cusec == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
cusec_oldlen = len;
if (cusec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cusec_datalen;
e = decode_krb5int32(p, len, (data)->cusec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cusec_oldlen - cusec_datalen;
}
}
{
size_t stime_datalen, stime_oldlen;
Der_type stime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &stime_type, 4, &stime_datalen, &l);
if (e == 0 && stime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
stime_oldlen = len;
if (stime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = stime_datalen;
e = decode_KerberosTime(p, len, &(data)->stime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = stime_oldlen - stime_datalen;
}
{
size_t susec_datalen, susec_oldlen;
Der_type susec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &susec_type, 5, &susec_datalen, &l);
if (e == 0 && susec_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
susec_oldlen = len;
if (susec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = susec_datalen;
e = decode_krb5int32(p, len, &(data)->susec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = susec_oldlen - susec_datalen;
}
{
size_t error_code_datalen, error_code_oldlen;
Der_type error_code_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &error_code_type, 6, &error_code_datalen, &l);
if (e == 0 && error_code_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
error_code_oldlen = len;
if (error_code_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = error_code_datalen;
e = decode_krb5int32(p, len, &(data)->error_code, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = error_code_oldlen - error_code_datalen;
}
{
size_t crealm_datalen, crealm_oldlen;
Der_type crealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &crealm_type, 7, &crealm_datalen, &l);
if (e == 0 && crealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->crealm = NULL;
} else {
(data)->crealm = calloc(1, sizeof(*(data)->crealm));
if ((data)->crealm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
crealm_oldlen = len;
if (crealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = crealm_datalen;
e = decode_Realm(p, len, (data)->crealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = crealm_oldlen - crealm_datalen;
}
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 8, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->cname = NULL;
} else {
(data)->cname = calloc(1, sizeof(*(data)->cname));
if ((data)->cname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, (data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
}
{
size_t realm_datalen, realm_oldlen;
Der_type realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &realm_type, 9, &realm_datalen, &l);
if (e == 0 && realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
realm_oldlen = len;
if (realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = realm_datalen;
e = decode_Realm(p, len, &(data)->realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = realm_oldlen - realm_datalen;
}
{
size_t sname_datalen, sname_oldlen;
Der_type sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sname_type, 10, &sname_datalen, &l);
if (e == 0 && sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sname_oldlen = len;
if (sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sname_datalen;
e = decode_PrincipalName(p, len, &(data)->sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sname_oldlen - sname_datalen;
}
{
size_t e_text_datalen, e_text_oldlen;
Der_type e_text_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &e_text_type, 11, &e_text_datalen, &l);
if (e == 0 && e_text_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->e_text = NULL;
} else {
(data)->e_text = calloc(1, sizeof(*(data)->e_text));
if ((data)->e_text == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
e_text_oldlen = len;
if (e_text_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = e_text_datalen;
{
size_t e_text_Tag_datalen, e_text_Tag_oldlen;
Der_type e_text_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &e_text_Tag_type, UT_GeneralString, &e_text_Tag_datalen, &l);
if (e == 0 && e_text_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
e_text_Tag_oldlen = len;
if (e_text_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = e_text_Tag_datalen;
e = der_get_general_string(p, len, (data)->e_text, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = e_text_Tag_oldlen - e_text_Tag_datalen;
}
len = e_text_oldlen - e_text_datalen;
}
}
{
size_t e_data_datalen, e_data_oldlen;
Der_type e_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &e_data_type, 12, &e_data_datalen, &l);
if (e == 0 && e_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->e_data = NULL;
} else {
(data)->e_data = calloc(1, sizeof(*(data)->e_data));
if ((data)->e_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
e_data_oldlen = len;
if (e_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = e_data_datalen;
{
size_t e_data_Tag_datalen, e_data_Tag_oldlen;
Der_type e_data_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &e_data_Tag_type, UT_OctetString, &e_data_Tag_datalen, &l);
if (e == 0 && e_data_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
e_data_Tag_oldlen = len;
if (e_data_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = e_data_Tag_datalen;
e = der_get_octet_string(p, len, (data)->e_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = e_data_Tag_oldlen - e_data_Tag_datalen;
}
len = e_data_oldlen - e_data_datalen;
}
}
len = Top_Tag_oldlen - Top_Tag_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB_ERROR(data);
return e;
}

void ASN1CALL
free_KRB_ERROR(KRB_ERROR *data)
{
free_krb5int32(&(data)->pvno);
free_MESSAGE_TYPE(&(data)->msg_type);
if((data)->ctime) {
free_KerberosTime((data)->ctime);
free((data)->ctime);
(data)->ctime = NULL;
}
if((data)->cusec) {
free_krb5int32((data)->cusec);
free((data)->cusec);
(data)->cusec = NULL;
}
free_KerberosTime(&(data)->stime);
free_krb5int32(&(data)->susec);
free_krb5int32(&(data)->error_code);
if((data)->crealm) {
free_Realm((data)->crealm);
free((data)->crealm);
(data)->crealm = NULL;
}
if((data)->cname) {
free_PrincipalName((data)->cname);
free((data)->cname);
(data)->cname = NULL;
}
free_Realm(&(data)->realm);
free_PrincipalName(&(data)->sname);
if((data)->e_text) {
der_free_general_string((data)->e_text);
free((data)->e_text);
(data)->e_text = NULL;
}
if((data)->e_data) {
der_free_octet_string((data)->e_data);
free((data)->e_data);
(data)->e_data = NULL;
}
}

size_t ASN1CALL
length_KRB_ERROR(const KRB_ERROR *data)
{
size_t ret = 0;
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->pvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_MESSAGE_TYPE(&(data)->msg_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->ctime){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->ctime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->cusec){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32((data)->cusec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->stime);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->susec);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->error_code);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->crealm){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->crealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->cname){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
{
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->e_text){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->e_text);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
if((data)->e_data){
size_t Top_tag_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string((data)->e_data);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB_ERROR(const KRB_ERROR *from, KRB_ERROR *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->pvno, &(to)->pvno)) goto fail;
if(copy_MESSAGE_TYPE(&(from)->msg_type, &(to)->msg_type)) goto fail;
if((from)->ctime) {
(to)->ctime = malloc(sizeof(*(to)->ctime));
if((to)->ctime == NULL) goto fail;
if(copy_KerberosTime((from)->ctime, (to)->ctime)) goto fail;
}else
(to)->ctime = NULL;
if((from)->cusec) {
(to)->cusec = malloc(sizeof(*(to)->cusec));
if((to)->cusec == NULL) goto fail;
if(copy_krb5int32((from)->cusec, (to)->cusec)) goto fail;
}else
(to)->cusec = NULL;
if(copy_KerberosTime(&(from)->stime, &(to)->stime)) goto fail;
if(copy_krb5int32(&(from)->susec, &(to)->susec)) goto fail;
if(copy_krb5int32(&(from)->error_code, &(to)->error_code)) goto fail;
if((from)->crealm) {
(to)->crealm = malloc(sizeof(*(to)->crealm));
if((to)->crealm == NULL) goto fail;
if(copy_Realm((from)->crealm, (to)->crealm)) goto fail;
}else
(to)->crealm = NULL;
if((from)->cname) {
(to)->cname = malloc(sizeof(*(to)->cname));
if((to)->cname == NULL) goto fail;
if(copy_PrincipalName((from)->cname, (to)->cname)) goto fail;
}else
(to)->cname = NULL;
if(copy_Realm(&(from)->realm, &(to)->realm)) goto fail;
if(copy_PrincipalName(&(from)->sname, &(to)->sname)) goto fail;
if((from)->e_text) {
(to)->e_text = malloc(sizeof(*(to)->e_text));
if((to)->e_text == NULL) goto fail;
if(der_copy_general_string((from)->e_text, (to)->e_text)) goto fail;
}else
(to)->e_text = NULL;
if((from)->e_data) {
(to)->e_data = malloc(sizeof(*(to)->e_data));
if((to)->e_data == NULL) goto fail;
if(der_copy_octet_string((from)->e_data, (to)->e_data)) goto fail;
}else
(to)->e_data = NULL;
return 0;
fail:
free_KRB_ERROR(to);
return ENOMEM;
}

int ASN1CALL
encode_ChangePasswdDataMS(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const ChangePasswdDataMS *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* targrealm */
if((data)->targrealm) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->targrealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* targname */
if((data)->targname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->targname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* newpasswd */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->newpasswd, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_ChangePasswdDataMS(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, ChangePasswdDataMS *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t newpasswd_datalen, newpasswd_oldlen;
Der_type newpasswd_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &newpasswd_type, 0, &newpasswd_datalen, &l);
if (e == 0 && newpasswd_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
newpasswd_oldlen = len;
if (newpasswd_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = newpasswd_datalen;
{
size_t newpasswd_Tag_datalen, newpasswd_Tag_oldlen;
Der_type newpasswd_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &newpasswd_Tag_type, UT_OctetString, &newpasswd_Tag_datalen, &l);
if (e == 0 && newpasswd_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
newpasswd_Tag_oldlen = len;
if (newpasswd_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = newpasswd_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->newpasswd, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = newpasswd_Tag_oldlen - newpasswd_Tag_datalen;
}
len = newpasswd_oldlen - newpasswd_datalen;
}
{
size_t targname_datalen, targname_oldlen;
Der_type targname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &targname_type, 1, &targname_datalen, &l);
if (e == 0 && targname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->targname = NULL;
} else {
(data)->targname = calloc(1, sizeof(*(data)->targname));
if ((data)->targname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
targname_oldlen = len;
if (targname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = targname_datalen;
e = decode_PrincipalName(p, len, (data)->targname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = targname_oldlen - targname_datalen;
}
}
{
size_t targrealm_datalen, targrealm_oldlen;
Der_type targrealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &targrealm_type, 2, &targrealm_datalen, &l);
if (e == 0 && targrealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->targrealm = NULL;
} else {
(data)->targrealm = calloc(1, sizeof(*(data)->targrealm));
if ((data)->targrealm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
targrealm_oldlen = len;
if (targrealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = targrealm_datalen;
e = decode_Realm(p, len, (data)->targrealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = targrealm_oldlen - targrealm_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_ChangePasswdDataMS(data);
return e;
}

void ASN1CALL
free_ChangePasswdDataMS(ChangePasswdDataMS *data)
{
der_free_octet_string(&(data)->newpasswd);
if((data)->targname) {
free_PrincipalName((data)->targname);
free((data)->targname);
(data)->targname = NULL;
}
if((data)->targrealm) {
free_Realm((data)->targrealm);
free((data)->targrealm);
(data)->targrealm = NULL;
}
}

size_t ASN1CALL
length_ChangePasswdDataMS(const ChangePasswdDataMS *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->newpasswd);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->targname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->targname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->targrealm){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->targrealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_ChangePasswdDataMS(const ChangePasswdDataMS *from, ChangePasswdDataMS *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_octet_string(&(from)->newpasswd, &(to)->newpasswd)) goto fail;
if((from)->targname) {
(to)->targname = malloc(sizeof(*(to)->targname));
if((to)->targname == NULL) goto fail;
if(copy_PrincipalName((from)->targname, (to)->targname)) goto fail;
}else
(to)->targname = NULL;
if((from)->targrealm) {
(to)->targrealm = malloc(sizeof(*(to)->targrealm));
if((to)->targrealm == NULL) goto fail;
if(copy_Realm((from)->targrealm, (to)->targrealm)) goto fail;
}else
(to)->targrealm = NULL;
return 0;
fail:
free_ChangePasswdDataMS(to);
return ENOMEM;
}

int ASN1CALL
encode_EtypeList(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EtypeList *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

for(i = (int)(data)->len - 1; i >= 0; --i) {
size_t Top_tag_for_oldret = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(data)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_EtypeList(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EtypeList *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t Top_Tag_origlen = len;
size_t Top_Tag_oldret = ret;
size_t Top_Tag_olen = 0;
void *Top_Tag_tmp;
ret = 0;
(data)->len = 0;
(data)->val = NULL;
while(ret < Top_Tag_origlen) {
size_t Top_Tag_nlen = Top_Tag_olen + sizeof(*((data)->val));
if (Top_Tag_olen > Top_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
Top_Tag_olen = Top_Tag_nlen;
Top_Tag_tmp = realloc((data)->val, Top_Tag_olen);
if (Top_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(data)->val = Top_Tag_tmp;
e = decode_ENCTYPE(p, len, &(data)->val[(data)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(data)->len++;
len = Top_Tag_origlen - ret;
}
ret += Top_Tag_oldret;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_EtypeList(data);
return e;
}

void ASN1CALL
free_EtypeList(EtypeList *data)
{
while((data)->len){
free_ENCTYPE(&(data)->val[(data)->len-1]);
(data)->len--;
}
free((data)->val);
(data)->val = NULL;
}

size_t ASN1CALL
length_EtypeList(const EtypeList *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
int i;
ret = 0;
for(i = (data)->len - 1; i >= 0; --i){
size_t Top_tag_for_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(data)->val[i]);
ret += Top_tag_for_oldret;
}
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_EtypeList(const EtypeList *from, EtypeList *to)
{
memset(to, 0, sizeof(*to));
if(((to)->val = malloc((from)->len * sizeof(*(to)->val))) == NULL && (from)->len != 0)
goto fail;
for((to)->len = 0; (to)->len < (from)->len; (to)->len++){
if(copy_ENCTYPE(&(from)->val[(to)->len], &(to)->val[(to)->len])) goto fail;
}
return 0;
fail:
free_EtypeList(to);
return ENOMEM;
}

int ASN1CALL
encode_AD_IF_RELEVANT(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AD_IF_RELEVANT *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_AuthorizationData(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AD_IF_RELEVANT(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AD_IF_RELEVANT *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
e = decode_AuthorizationData(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
if(size) *size = ret;
return 0;
fail:
free_AD_IF_RELEVANT(data);
return e;
}

void ASN1CALL
free_AD_IF_RELEVANT(AD_IF_RELEVANT *data)
{
free_AuthorizationData(data);
}

size_t ASN1CALL
length_AD_IF_RELEVANT(const AD_IF_RELEVANT *data)
{
size_t ret = 0;
ret += length_AuthorizationData(data);
return ret;
}

int ASN1CALL
copy_AD_IF_RELEVANT(const AD_IF_RELEVANT *from, AD_IF_RELEVANT *to)
{
memset(to, 0, sizeof(*to));
if(copy_AuthorizationData(from, to)) goto fail;
return 0;
fail:
free_AD_IF_RELEVANT(to);
return ENOMEM;
}

int ASN1CALL
encode_AD_KDCIssued(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AD_KDCIssued *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* elements */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_AuthorizationData(p, len, &(data)->elements, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* i-sname */
if((data)->i_sname) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->i_sname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* i-realm */
if((data)->i_realm) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->i_realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* ad-checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->ad_checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AD_KDCIssued(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AD_KDCIssued *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t ad_checksum_datalen, ad_checksum_oldlen;
Der_type ad_checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ad_checksum_type, 0, &ad_checksum_datalen, &l);
if (e == 0 && ad_checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ad_checksum_oldlen = len;
if (ad_checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ad_checksum_datalen;
e = decode_Checksum(p, len, &(data)->ad_checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ad_checksum_oldlen - ad_checksum_datalen;
}
{
size_t i_realm_datalen, i_realm_oldlen;
Der_type i_realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &i_realm_type, 1, &i_realm_datalen, &l);
if (e == 0 && i_realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->i_realm = NULL;
} else {
(data)->i_realm = calloc(1, sizeof(*(data)->i_realm));
if ((data)->i_realm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
i_realm_oldlen = len;
if (i_realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = i_realm_datalen;
e = decode_Realm(p, len, (data)->i_realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = i_realm_oldlen - i_realm_datalen;
}
}
{
size_t i_sname_datalen, i_sname_oldlen;
Der_type i_sname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &i_sname_type, 2, &i_sname_datalen, &l);
if (e == 0 && i_sname_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->i_sname = NULL;
} else {
(data)->i_sname = calloc(1, sizeof(*(data)->i_sname));
if ((data)->i_sname == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
i_sname_oldlen = len;
if (i_sname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = i_sname_datalen;
e = decode_PrincipalName(p, len, (data)->i_sname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = i_sname_oldlen - i_sname_datalen;
}
}
{
size_t elements_datalen, elements_oldlen;
Der_type elements_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &elements_type, 3, &elements_datalen, &l);
if (e == 0 && elements_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
elements_oldlen = len;
if (elements_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = elements_datalen;
e = decode_AuthorizationData(p, len, &(data)->elements, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = elements_oldlen - elements_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AD_KDCIssued(data);
return e;
}

void ASN1CALL
free_AD_KDCIssued(AD_KDCIssued *data)
{
free_Checksum(&(data)->ad_checksum);
if((data)->i_realm) {
free_Realm((data)->i_realm);
free((data)->i_realm);
(data)->i_realm = NULL;
}
if((data)->i_sname) {
free_PrincipalName((data)->i_sname);
free((data)->i_sname);
(data)->i_sname = NULL;
}
free_AuthorizationData(&(data)->elements);
}

size_t ASN1CALL
length_AD_KDCIssued(const AD_KDCIssued *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->ad_checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->i_realm){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->i_realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->i_sname){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->i_sname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_AuthorizationData(&(data)->elements);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AD_KDCIssued(const AD_KDCIssued *from, AD_KDCIssued *to)
{
memset(to, 0, sizeof(*to));
if(copy_Checksum(&(from)->ad_checksum, &(to)->ad_checksum)) goto fail;
if((from)->i_realm) {
(to)->i_realm = malloc(sizeof(*(to)->i_realm));
if((to)->i_realm == NULL) goto fail;
if(copy_Realm((from)->i_realm, (to)->i_realm)) goto fail;
}else
(to)->i_realm = NULL;
if((from)->i_sname) {
(to)->i_sname = malloc(sizeof(*(to)->i_sname));
if((to)->i_sname == NULL) goto fail;
if(copy_PrincipalName((from)->i_sname, (to)->i_sname)) goto fail;
}else
(to)->i_sname = NULL;
if(copy_AuthorizationData(&(from)->elements, &(to)->elements)) goto fail;
return 0;
fail:
free_AD_KDCIssued(to);
return ENOMEM;
}

int ASN1CALL
encode_AD_AND_OR(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AD_AND_OR *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* elements */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_AuthorizationData(p, len, &(data)->elements, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* condition-count */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_heim_integer(p, len, &(data)->condition_count, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AD_AND_OR(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AD_AND_OR *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t condition_count_datalen, condition_count_oldlen;
Der_type condition_count_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &condition_count_type, 0, &condition_count_datalen, &l);
if (e == 0 && condition_count_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
condition_count_oldlen = len;
if (condition_count_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = condition_count_datalen;
{
size_t condition_count_Tag_datalen, condition_count_Tag_oldlen;
Der_type condition_count_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &condition_count_Tag_type, UT_Integer, &condition_count_Tag_datalen, &l);
if (e == 0 && condition_count_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
condition_count_Tag_oldlen = len;
if (condition_count_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = condition_count_Tag_datalen;
e = der_get_heim_integer(p, len, &(data)->condition_count, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = condition_count_Tag_oldlen - condition_count_Tag_datalen;
}
len = condition_count_oldlen - condition_count_datalen;
}
{
size_t elements_datalen, elements_oldlen;
Der_type elements_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &elements_type, 1, &elements_datalen, &l);
if (e == 0 && elements_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
elements_oldlen = len;
if (elements_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = elements_datalen;
e = decode_AuthorizationData(p, len, &(data)->elements, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = elements_oldlen - elements_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AD_AND_OR(data);
return e;
}

void ASN1CALL
free_AD_AND_OR(AD_AND_OR *data)
{
der_free_heim_integer(&(data)->condition_count);
free_AuthorizationData(&(data)->elements);
}

size_t ASN1CALL
length_AD_AND_OR(const AD_AND_OR *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_heim_integer(&(data)->condition_count);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_AuthorizationData(&(data)->elements);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AD_AND_OR(const AD_AND_OR *from, AD_AND_OR *to)
{
memset(to, 0, sizeof(*to));
if(der_copy_heim_integer(&(from)->condition_count, &(to)->condition_count)) goto fail;
if(copy_AuthorizationData(&(from)->elements, &(to)->elements)) goto fail;
return 0;
fail:
free_AD_AND_OR(to);
return ENOMEM;
}

int ASN1CALL
encode_AD_MANDATORY_FOR_KDC(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AD_MANDATORY_FOR_KDC *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_AuthorizationData(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AD_MANDATORY_FOR_KDC(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AD_MANDATORY_FOR_KDC *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
e = decode_AuthorizationData(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
if(size) *size = ret;
return 0;
fail:
free_AD_MANDATORY_FOR_KDC(data);
return e;
}

void ASN1CALL
free_AD_MANDATORY_FOR_KDC(AD_MANDATORY_FOR_KDC *data)
{
free_AuthorizationData(data);
}

size_t ASN1CALL
length_AD_MANDATORY_FOR_KDC(const AD_MANDATORY_FOR_KDC *data)
{
size_t ret = 0;
ret += length_AuthorizationData(data);
return ret;
}

int ASN1CALL
copy_AD_MANDATORY_FOR_KDC(const AD_MANDATORY_FOR_KDC *from, AD_MANDATORY_FOR_KDC *to)
{
memset(to, 0, sizeof(*to));
if(copy_AuthorizationData(from, to)) goto fail;
return 0;
fail:
free_AD_MANDATORY_FOR_KDC(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_SAM_TYPE(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SAM_TYPE *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
int enumint = (int)*data;
e = der_put_integer(p, len, &enumint, &l);
if (e) return e;
p -= l; len -= l; ret += l;

}
;e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_Integer, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SAM_TYPE(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SAM_TYPE *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Integer, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
int enumint;
e = der_get_integer(p, len, &enumint, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
*data = enumint;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_SAM_TYPE(data);
return e;
}

void ASN1CALL
free_PA_SAM_TYPE(PA_SAM_TYPE *data)
{
}

size_t ASN1CALL
length_PA_SAM_TYPE(const PA_SAM_TYPE *data)
{
size_t ret = 0;
{
int enumint = *data;
ret += der_length_integer(&enumint);
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_SAM_TYPE(const PA_SAM_TYPE *from, PA_SAM_TYPE *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_PA_SAM_REDIRECT(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SAM_REDIRECT *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_HostAddresses(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SAM_REDIRECT(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SAM_REDIRECT *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
e = decode_HostAddresses(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
if(size) *size = ret;
return 0;
fail:
free_PA_SAM_REDIRECT(data);
return e;
}

void ASN1CALL
free_PA_SAM_REDIRECT(PA_SAM_REDIRECT *data)
{
free_HostAddresses(data);
}

size_t ASN1CALL
length_PA_SAM_REDIRECT(const PA_SAM_REDIRECT *data)
{
size_t ret = 0;
ret += length_HostAddresses(data);
return ret;
}

int ASN1CALL
copy_PA_SAM_REDIRECT(const PA_SAM_REDIRECT *from, PA_SAM_REDIRECT *to)
{
memset(to, 0, sizeof(*to));
if(copy_HostAddresses(from, to)) goto fail;
return 0;
fail:
free_PA_SAM_REDIRECT(to);
return ENOMEM;
}

int ASN1CALL
encode_SAMFlags(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const SAMFlags *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
unsigned char c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->must_pk_encrypt_sad) {
c |= 1<<5;
}
if((data)->send_encrypted_sad) {
c |= 1<<6;
}
if((data)->use_sad_as_key) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
if (len < 1) return ASN1_OVERFLOW;
*p-- = 0;
len -= 1;
ret += 1;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_BitString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_SAMFlags(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, SAMFlags *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_BitString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
if (len < 1) return ASN1_OVERRUN;
p++; len--; ret++;
do {
if (len < 1) break;
(data)->use_sad_as_key = (*p >> 7) & 1;
(data)->send_encrypted_sad = (*p >> 6) & 1;
(data)->must_pk_encrypt_sad = (*p >> 5) & 1;
} while(0);
p += len; ret += len;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_SAMFlags(data);
return e;
}

void ASN1CALL
free_SAMFlags(SAMFlags *data)
{
}

size_t ASN1CALL
length_SAMFlags(const SAMFlags *data)
{
size_t ret = 0;
ret += 5;
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_SAMFlags(const SAMFlags *from, SAMFlags *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

unsigned SAMFlags2int(SAMFlags f)
{
unsigned r = 0;
if(f.use_sad_as_key) r |= (1U << 0);
if(f.send_encrypted_sad) r |= (1U << 1);
if(f.must_pk_encrypt_sad) r |= (1U << 2);
return r;
}

SAMFlags int2SAMFlags(unsigned n)
{
	SAMFlags flags;

	memset(&flags, 0, sizeof(flags));

	flags.use_sad_as_key = (n >> 0) & 1;
	flags.send_encrypted_sad = (n >> 1) & 1;
	flags.must_pk_encrypt_sad = (n >> 2) & 1;
	return flags;
}

static struct units SAMFlags_units[] = {
	{"must-pk-encrypt-sad",	1U << 2},
	{"send-encrypted-sad",	1U << 1},
	{"use-sad-as-key",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_SAMFlags_units(void){
return SAMFlags_units;
}

int ASN1CALL
encode_PA_SAM_CHALLENGE_2_BODY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SAM_CHALLENGE_2_BODY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* sam-etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_etype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 9, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-nonce */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 8, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-pk-for-sad */
if((data)->sam_pk_for_sad) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, (data)->sam_pk_for_sad, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 7, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-response-prompt */
if((data)->sam_response_prompt) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_response_prompt, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 6, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-challenge */
if((data)->sam_challenge) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_challenge, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-challenge-label */
if((data)->sam_challenge_label) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_challenge_label, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-track-id */
if((data)->sam_track_id) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_track_id, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-type-name */
if((data)->sam_type_name) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_type_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-flags */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_SAMFlags(p, len, &(data)->sam_flags, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SAM_CHALLENGE_2_BODY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SAM_CHALLENGE_2_BODY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t sam_type_datalen, sam_type_oldlen;
Der_type sam_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_type_type, 0, &sam_type_datalen, &l);
if (e == 0 && sam_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_type_oldlen = len;
if (sam_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_type_datalen;
e = decode_krb5int32(p, len, &(data)->sam_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_type_oldlen - sam_type_datalen;
}
{
size_t sam_flags_datalen, sam_flags_oldlen;
Der_type sam_flags_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_flags_type, 1, &sam_flags_datalen, &l);
if (e == 0 && sam_flags_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_flags_oldlen = len;
if (sam_flags_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_flags_datalen;
e = decode_SAMFlags(p, len, &(data)->sam_flags, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_flags_oldlen - sam_flags_datalen;
}
{
size_t sam_type_name_datalen, sam_type_name_oldlen;
Der_type sam_type_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_type_name_type, 2, &sam_type_name_datalen, &l);
if (e == 0 && sam_type_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_type_name = NULL;
} else {
(data)->sam_type_name = calloc(1, sizeof(*(data)->sam_type_name));
if ((data)->sam_type_name == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_type_name_oldlen = len;
if (sam_type_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_type_name_datalen;
{
size_t sam_type_name_Tag_datalen, sam_type_name_Tag_oldlen;
Der_type sam_type_name_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_type_name_Tag_type, UT_GeneralString, &sam_type_name_Tag_datalen, &l);
if (e == 0 && sam_type_name_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_type_name_Tag_oldlen = len;
if (sam_type_name_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_type_name_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_type_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_type_name_Tag_oldlen - sam_type_name_Tag_datalen;
}
len = sam_type_name_oldlen - sam_type_name_datalen;
}
}
{
size_t sam_track_id_datalen, sam_track_id_oldlen;
Der_type sam_track_id_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_track_id_type, 3, &sam_track_id_datalen, &l);
if (e == 0 && sam_track_id_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_track_id = NULL;
} else {
(data)->sam_track_id = calloc(1, sizeof(*(data)->sam_track_id));
if ((data)->sam_track_id == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_track_id_oldlen = len;
if (sam_track_id_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_track_id_datalen;
{
size_t sam_track_id_Tag_datalen, sam_track_id_Tag_oldlen;
Der_type sam_track_id_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_track_id_Tag_type, UT_GeneralString, &sam_track_id_Tag_datalen, &l);
if (e == 0 && sam_track_id_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_track_id_Tag_oldlen = len;
if (sam_track_id_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_track_id_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_track_id, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_track_id_Tag_oldlen - sam_track_id_Tag_datalen;
}
len = sam_track_id_oldlen - sam_track_id_datalen;
}
}
{
size_t sam_challenge_label_datalen, sam_challenge_label_oldlen;
Der_type sam_challenge_label_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_challenge_label_type, 4, &sam_challenge_label_datalen, &l);
if (e == 0 && sam_challenge_label_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_challenge_label = NULL;
} else {
(data)->sam_challenge_label = calloc(1, sizeof(*(data)->sam_challenge_label));
if ((data)->sam_challenge_label == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_challenge_label_oldlen = len;
if (sam_challenge_label_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_challenge_label_datalen;
{
size_t sam_challenge_label_Tag_datalen, sam_challenge_label_Tag_oldlen;
Der_type sam_challenge_label_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_challenge_label_Tag_type, UT_GeneralString, &sam_challenge_label_Tag_datalen, &l);
if (e == 0 && sam_challenge_label_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_challenge_label_Tag_oldlen = len;
if (sam_challenge_label_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_challenge_label_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_challenge_label, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_challenge_label_Tag_oldlen - sam_challenge_label_Tag_datalen;
}
len = sam_challenge_label_oldlen - sam_challenge_label_datalen;
}
}
{
size_t sam_challenge_datalen, sam_challenge_oldlen;
Der_type sam_challenge_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_challenge_type, 5, &sam_challenge_datalen, &l);
if (e == 0 && sam_challenge_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_challenge = NULL;
} else {
(data)->sam_challenge = calloc(1, sizeof(*(data)->sam_challenge));
if ((data)->sam_challenge == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_challenge_oldlen = len;
if (sam_challenge_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_challenge_datalen;
{
size_t sam_challenge_Tag_datalen, sam_challenge_Tag_oldlen;
Der_type sam_challenge_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_challenge_Tag_type, UT_GeneralString, &sam_challenge_Tag_datalen, &l);
if (e == 0 && sam_challenge_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_challenge_Tag_oldlen = len;
if (sam_challenge_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_challenge_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_challenge, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_challenge_Tag_oldlen - sam_challenge_Tag_datalen;
}
len = sam_challenge_oldlen - sam_challenge_datalen;
}
}
{
size_t sam_response_prompt_datalen, sam_response_prompt_oldlen;
Der_type sam_response_prompt_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_response_prompt_type, 6, &sam_response_prompt_datalen, &l);
if (e == 0 && sam_response_prompt_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_response_prompt = NULL;
} else {
(data)->sam_response_prompt = calloc(1, sizeof(*(data)->sam_response_prompt));
if ((data)->sam_response_prompt == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_response_prompt_oldlen = len;
if (sam_response_prompt_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_response_prompt_datalen;
{
size_t sam_response_prompt_Tag_datalen, sam_response_prompt_Tag_oldlen;
Der_type sam_response_prompt_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_response_prompt_Tag_type, UT_GeneralString, &sam_response_prompt_Tag_datalen, &l);
if (e == 0 && sam_response_prompt_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_response_prompt_Tag_oldlen = len;
if (sam_response_prompt_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_response_prompt_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_response_prompt, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_response_prompt_Tag_oldlen - sam_response_prompt_Tag_datalen;
}
len = sam_response_prompt_oldlen - sam_response_prompt_datalen;
}
}
{
size_t sam_pk_for_sad_datalen, sam_pk_for_sad_oldlen;
Der_type sam_pk_for_sad_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_pk_for_sad_type, 7, &sam_pk_for_sad_datalen, &l);
if (e == 0 && sam_pk_for_sad_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_pk_for_sad = NULL;
} else {
(data)->sam_pk_for_sad = calloc(1, sizeof(*(data)->sam_pk_for_sad));
if ((data)->sam_pk_for_sad == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_pk_for_sad_oldlen = len;
if (sam_pk_for_sad_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_pk_for_sad_datalen;
e = decode_EncryptionKey(p, len, (data)->sam_pk_for_sad, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_pk_for_sad_oldlen - sam_pk_for_sad_datalen;
}
}
{
size_t sam_nonce_datalen, sam_nonce_oldlen;
Der_type sam_nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_nonce_type, 8, &sam_nonce_datalen, &l);
if (e == 0 && sam_nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_nonce_oldlen = len;
if (sam_nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_nonce_datalen;
e = decode_krb5int32(p, len, &(data)->sam_nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_nonce_oldlen - sam_nonce_datalen;
}
{
size_t sam_etype_datalen, sam_etype_oldlen;
Der_type sam_etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_etype_type, 9, &sam_etype_datalen, &l);
if (e == 0 && sam_etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_etype_oldlen = len;
if (sam_etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_etype_datalen;
e = decode_krb5int32(p, len, &(data)->sam_etype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_etype_oldlen - sam_etype_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_SAM_CHALLENGE_2_BODY(data);
return e;
}

void ASN1CALL
free_PA_SAM_CHALLENGE_2_BODY(PA_SAM_CHALLENGE_2_BODY *data)
{
free_krb5int32(&(data)->sam_type);
free_SAMFlags(&(data)->sam_flags);
if((data)->sam_type_name) {
der_free_general_string((data)->sam_type_name);
free((data)->sam_type_name);
(data)->sam_type_name = NULL;
}
if((data)->sam_track_id) {
der_free_general_string((data)->sam_track_id);
free((data)->sam_track_id);
(data)->sam_track_id = NULL;
}
if((data)->sam_challenge_label) {
der_free_general_string((data)->sam_challenge_label);
free((data)->sam_challenge_label);
(data)->sam_challenge_label = NULL;
}
if((data)->sam_challenge) {
der_free_general_string((data)->sam_challenge);
free((data)->sam_challenge);
(data)->sam_challenge = NULL;
}
if((data)->sam_response_prompt) {
der_free_general_string((data)->sam_response_prompt);
free((data)->sam_response_prompt);
(data)->sam_response_prompt = NULL;
}
if((data)->sam_pk_for_sad) {
free_EncryptionKey((data)->sam_pk_for_sad);
free((data)->sam_pk_for_sad);
(data)->sam_pk_for_sad = NULL;
}
free_krb5int32(&(data)->sam_nonce);
free_krb5int32(&(data)->sam_etype);
}

size_t ASN1CALL
length_PA_SAM_CHALLENGE_2_BODY(const PA_SAM_CHALLENGE_2_BODY *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_SAMFlags(&(data)->sam_flags);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_type_name){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_type_name);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_track_id){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_track_id);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_challenge_label){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_challenge_label);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_challenge){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_challenge);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_response_prompt){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_response_prompt);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_pk_for_sad){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey((data)->sam_pk_for_sad);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_etype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_SAM_CHALLENGE_2_BODY(const PA_SAM_CHALLENGE_2_BODY *from, PA_SAM_CHALLENGE_2_BODY *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->sam_type, &(to)->sam_type)) goto fail;
if(copy_SAMFlags(&(from)->sam_flags, &(to)->sam_flags)) goto fail;
if((from)->sam_type_name) {
(to)->sam_type_name = malloc(sizeof(*(to)->sam_type_name));
if((to)->sam_type_name == NULL) goto fail;
if(der_copy_general_string((from)->sam_type_name, (to)->sam_type_name)) goto fail;
}else
(to)->sam_type_name = NULL;
if((from)->sam_track_id) {
(to)->sam_track_id = malloc(sizeof(*(to)->sam_track_id));
if((to)->sam_track_id == NULL) goto fail;
if(der_copy_general_string((from)->sam_track_id, (to)->sam_track_id)) goto fail;
}else
(to)->sam_track_id = NULL;
if((from)->sam_challenge_label) {
(to)->sam_challenge_label = malloc(sizeof(*(to)->sam_challenge_label));
if((to)->sam_challenge_label == NULL) goto fail;
if(der_copy_general_string((from)->sam_challenge_label, (to)->sam_challenge_label)) goto fail;
}else
(to)->sam_challenge_label = NULL;
if((from)->sam_challenge) {
(to)->sam_challenge = malloc(sizeof(*(to)->sam_challenge));
if((to)->sam_challenge == NULL) goto fail;
if(der_copy_general_string((from)->sam_challenge, (to)->sam_challenge)) goto fail;
}else
(to)->sam_challenge = NULL;
if((from)->sam_response_prompt) {
(to)->sam_response_prompt = malloc(sizeof(*(to)->sam_response_prompt));
if((to)->sam_response_prompt == NULL) goto fail;
if(der_copy_general_string((from)->sam_response_prompt, (to)->sam_response_prompt)) goto fail;
}else
(to)->sam_response_prompt = NULL;
if((from)->sam_pk_for_sad) {
(to)->sam_pk_for_sad = malloc(sizeof(*(to)->sam_pk_for_sad));
if((to)->sam_pk_for_sad == NULL) goto fail;
if(copy_EncryptionKey((from)->sam_pk_for_sad, (to)->sam_pk_for_sad)) goto fail;
}else
(to)->sam_pk_for_sad = NULL;
if(copy_krb5int32(&(from)->sam_nonce, &(to)->sam_nonce)) goto fail;
if(copy_krb5int32(&(from)->sam_etype, &(to)->sam_etype)) goto fail;
return 0;
fail:
free_PA_SAM_CHALLENGE_2_BODY(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_SAM_CHALLENGE_2(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SAM_CHALLENGE_2 *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* sam-cksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->sam_cksum)->len - 1; i >= 0; --i) {
size_t sam_cksum_tag_tag_for_oldret = ret;
ret = 0;
e = encode_Checksum(p, len, &(&(data)->sam_cksum)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += sam_cksum_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-body */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PA_SAM_CHALLENGE_2_BODY(p, len, &(data)->sam_body, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SAM_CHALLENGE_2(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SAM_CHALLENGE_2 *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t sam_body_datalen, sam_body_oldlen;
Der_type sam_body_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_body_type, 0, &sam_body_datalen, &l);
if (e == 0 && sam_body_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_body_oldlen = len;
if (sam_body_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_body_datalen;
e = decode_PA_SAM_CHALLENGE_2_BODY(p, len, &(data)->sam_body, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_body_oldlen - sam_body_datalen;
}
{
size_t sam_cksum_datalen, sam_cksum_oldlen;
Der_type sam_cksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_cksum_type, 1, &sam_cksum_datalen, &l);
if (e == 0 && sam_cksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_cksum_oldlen = len;
if (sam_cksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_cksum_datalen;
{
size_t sam_cksum_Tag_datalen, sam_cksum_Tag_oldlen;
Der_type sam_cksum_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_cksum_Tag_type, UT_Sequence, &sam_cksum_Tag_datalen, &l);
if (e == 0 && sam_cksum_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_cksum_Tag_oldlen = len;
if (sam_cksum_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_cksum_Tag_datalen;
{
size_t sam_cksum_Tag_Tag_origlen = len;
size_t sam_cksum_Tag_Tag_oldret = ret;
size_t sam_cksum_Tag_Tag_olen = 0;
void *sam_cksum_Tag_Tag_tmp;
ret = 0;
(&(data)->sam_cksum)->len = 0;
(&(data)->sam_cksum)->val = NULL;
while(ret < sam_cksum_Tag_Tag_origlen) {
size_t sam_cksum_Tag_Tag_nlen = sam_cksum_Tag_Tag_olen + sizeof(*((&(data)->sam_cksum)->val));
if (sam_cksum_Tag_Tag_olen > sam_cksum_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
sam_cksum_Tag_Tag_olen = sam_cksum_Tag_Tag_nlen;
sam_cksum_Tag_Tag_tmp = realloc((&(data)->sam_cksum)->val, sam_cksum_Tag_Tag_olen);
if (sam_cksum_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->sam_cksum)->val = sam_cksum_Tag_Tag_tmp;
e = decode_Checksum(p, len, &(&(data)->sam_cksum)->val[(&(data)->sam_cksum)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->sam_cksum)->len++;
len = sam_cksum_Tag_Tag_origlen - ret;
}
ret += sam_cksum_Tag_Tag_oldret;
}
len = sam_cksum_Tag_oldlen - sam_cksum_Tag_datalen;
}
len = sam_cksum_oldlen - sam_cksum_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_SAM_CHALLENGE_2(data);
return e;
}

void ASN1CALL
free_PA_SAM_CHALLENGE_2(PA_SAM_CHALLENGE_2 *data)
{
free_PA_SAM_CHALLENGE_2_BODY(&(data)->sam_body);
while((&(data)->sam_cksum)->len){
free_Checksum(&(&(data)->sam_cksum)->val[(&(data)->sam_cksum)->len-1]);
(&(data)->sam_cksum)->len--;
}
free((&(data)->sam_cksum)->val);
(&(data)->sam_cksum)->val = NULL;
}

size_t ASN1CALL
length_PA_SAM_CHALLENGE_2(const PA_SAM_CHALLENGE_2 *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PA_SAM_CHALLENGE_2_BODY(&(data)->sam_body);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t sam_cksum_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->sam_cksum)->len - 1; i >= 0; --i){
size_t sam_cksum_tag_tag_for_oldret = ret;
ret = 0;
ret += length_Checksum(&(&(data)->sam_cksum)->val[i]);
ret += sam_cksum_tag_tag_for_oldret;
}
ret += sam_cksum_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_SAM_CHALLENGE_2(const PA_SAM_CHALLENGE_2 *from, PA_SAM_CHALLENGE_2 *to)
{
memset(to, 0, sizeof(*to));
if(copy_PA_SAM_CHALLENGE_2_BODY(&(from)->sam_body, &(to)->sam_body)) goto fail;
if(((&(to)->sam_cksum)->val = malloc((&(from)->sam_cksum)->len * sizeof(*(&(to)->sam_cksum)->val))) == NULL && (&(from)->sam_cksum)->len != 0)
goto fail;
for((&(to)->sam_cksum)->len = 0; (&(to)->sam_cksum)->len < (&(from)->sam_cksum)->len; (&(to)->sam_cksum)->len++){
if(copy_Checksum(&(&(from)->sam_cksum)->val[(&(to)->sam_cksum)->len], &(&(to)->sam_cksum)->val[(&(to)->sam_cksum)->len])) goto fail;
}
return 0;
fail:
free_PA_SAM_CHALLENGE_2(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_SAM_RESPONSE_2(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SAM_RESPONSE_2 *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* sam-nonce */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-enc-nonce-or-sad */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->sam_enc_nonce_or_sad, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-track-id */
if((data)->sam_track_id) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_track_id, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-flags */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_SAMFlags(p, len, &(data)->sam_flags, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SAM_RESPONSE_2(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SAM_RESPONSE_2 *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t sam_type_datalen, sam_type_oldlen;
Der_type sam_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_type_type, 0, &sam_type_datalen, &l);
if (e == 0 && sam_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_type_oldlen = len;
if (sam_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_type_datalen;
e = decode_krb5int32(p, len, &(data)->sam_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_type_oldlen - sam_type_datalen;
}
{
size_t sam_flags_datalen, sam_flags_oldlen;
Der_type sam_flags_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_flags_type, 1, &sam_flags_datalen, &l);
if (e == 0 && sam_flags_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_flags_oldlen = len;
if (sam_flags_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_flags_datalen;
e = decode_SAMFlags(p, len, &(data)->sam_flags, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_flags_oldlen - sam_flags_datalen;
}
{
size_t sam_track_id_datalen, sam_track_id_oldlen;
Der_type sam_track_id_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_track_id_type, 2, &sam_track_id_datalen, &l);
if (e == 0 && sam_track_id_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_track_id = NULL;
} else {
(data)->sam_track_id = calloc(1, sizeof(*(data)->sam_track_id));
if ((data)->sam_track_id == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_track_id_oldlen = len;
if (sam_track_id_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_track_id_datalen;
{
size_t sam_track_id_Tag_datalen, sam_track_id_Tag_oldlen;
Der_type sam_track_id_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_track_id_Tag_type, UT_GeneralString, &sam_track_id_Tag_datalen, &l);
if (e == 0 && sam_track_id_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_track_id_Tag_oldlen = len;
if (sam_track_id_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_track_id_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_track_id, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_track_id_Tag_oldlen - sam_track_id_Tag_datalen;
}
len = sam_track_id_oldlen - sam_track_id_datalen;
}
}
{
size_t sam_enc_nonce_or_sad_datalen, sam_enc_nonce_or_sad_oldlen;
Der_type sam_enc_nonce_or_sad_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_enc_nonce_or_sad_type, 3, &sam_enc_nonce_or_sad_datalen, &l);
if (e == 0 && sam_enc_nonce_or_sad_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_enc_nonce_or_sad_oldlen = len;
if (sam_enc_nonce_or_sad_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_enc_nonce_or_sad_datalen;
e = decode_EncryptedData(p, len, &(data)->sam_enc_nonce_or_sad, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_enc_nonce_or_sad_oldlen - sam_enc_nonce_or_sad_datalen;
}
{
size_t sam_nonce_datalen, sam_nonce_oldlen;
Der_type sam_nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_nonce_type, 4, &sam_nonce_datalen, &l);
if (e == 0 && sam_nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_nonce_oldlen = len;
if (sam_nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_nonce_datalen;
e = decode_krb5int32(p, len, &(data)->sam_nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_nonce_oldlen - sam_nonce_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_SAM_RESPONSE_2(data);
return e;
}

void ASN1CALL
free_PA_SAM_RESPONSE_2(PA_SAM_RESPONSE_2 *data)
{
free_krb5int32(&(data)->sam_type);
free_SAMFlags(&(data)->sam_flags);
if((data)->sam_track_id) {
der_free_general_string((data)->sam_track_id);
free((data)->sam_track_id);
(data)->sam_track_id = NULL;
}
free_EncryptedData(&(data)->sam_enc_nonce_or_sad);
free_krb5int32(&(data)->sam_nonce);
}

size_t ASN1CALL
length_PA_SAM_RESPONSE_2(const PA_SAM_RESPONSE_2 *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_SAMFlags(&(data)->sam_flags);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_track_id){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_track_id);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->sam_enc_nonce_or_sad);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_SAM_RESPONSE_2(const PA_SAM_RESPONSE_2 *from, PA_SAM_RESPONSE_2 *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->sam_type, &(to)->sam_type)) goto fail;
if(copy_SAMFlags(&(from)->sam_flags, &(to)->sam_flags)) goto fail;
if((from)->sam_track_id) {
(to)->sam_track_id = malloc(sizeof(*(to)->sam_track_id));
if((to)->sam_track_id == NULL) goto fail;
if(der_copy_general_string((from)->sam_track_id, (to)->sam_track_id)) goto fail;
}else
(to)->sam_track_id = NULL;
if(copy_EncryptedData(&(from)->sam_enc_nonce_or_sad, &(to)->sam_enc_nonce_or_sad)) goto fail;
if(copy_krb5int32(&(from)->sam_nonce, &(to)->sam_nonce)) goto fail;
return 0;
fail:
free_PA_SAM_RESPONSE_2(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_ENC_SAM_RESPONSE_ENC(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_ENC_SAM_RESPONSE_ENC *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* sam-sad */
if((data)->sam_sad) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, (data)->sam_sad, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* sam-nonce */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->sam_nonce, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_ENC_SAM_RESPONSE_ENC(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_ENC_SAM_RESPONSE_ENC *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t sam_nonce_datalen, sam_nonce_oldlen;
Der_type sam_nonce_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_nonce_type, 0, &sam_nonce_datalen, &l);
if (e == 0 && sam_nonce_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_nonce_oldlen = len;
if (sam_nonce_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_nonce_datalen;
e = decode_krb5int32(p, len, &(data)->sam_nonce, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_nonce_oldlen - sam_nonce_datalen;
}
{
size_t sam_sad_datalen, sam_sad_oldlen;
Der_type sam_sad_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &sam_sad_type, 1, &sam_sad_datalen, &l);
if (e == 0 && sam_sad_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->sam_sad = NULL;
} else {
(data)->sam_sad = calloc(1, sizeof(*(data)->sam_sad));
if ((data)->sam_sad == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
sam_sad_oldlen = len;
if (sam_sad_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_sad_datalen;
{
size_t sam_sad_Tag_datalen, sam_sad_Tag_oldlen;
Der_type sam_sad_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &sam_sad_Tag_type, UT_GeneralString, &sam_sad_Tag_datalen, &l);
if (e == 0 && sam_sad_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
sam_sad_Tag_oldlen = len;
if (sam_sad_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = sam_sad_Tag_datalen;
e = der_get_general_string(p, len, (data)->sam_sad, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = sam_sad_Tag_oldlen - sam_sad_Tag_datalen;
}
len = sam_sad_oldlen - sam_sad_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_ENC_SAM_RESPONSE_ENC(data);
return e;
}

void ASN1CALL
free_PA_ENC_SAM_RESPONSE_ENC(PA_ENC_SAM_RESPONSE_ENC *data)
{
free_krb5int32(&(data)->sam_nonce);
if((data)->sam_sad) {
der_free_general_string((data)->sam_sad);
free((data)->sam_sad);
(data)->sam_sad = NULL;
}
}

size_t ASN1CALL
length_PA_ENC_SAM_RESPONSE_ENC(const PA_ENC_SAM_RESPONSE_ENC *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->sam_nonce);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->sam_sad){
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string((data)->sam_sad);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_ENC_SAM_RESPONSE_ENC(const PA_ENC_SAM_RESPONSE_ENC *from, PA_ENC_SAM_RESPONSE_ENC *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->sam_nonce, &(to)->sam_nonce)) goto fail;
if((from)->sam_sad) {
(to)->sam_sad = malloc(sizeof(*(to)->sam_sad));
if((to)->sam_sad == NULL) goto fail;
if(der_copy_general_string((from)->sam_sad, (to)->sam_sad)) goto fail;
}else
(to)->sam_sad = NULL;
return 0;
fail:
free_PA_ENC_SAM_RESPONSE_ENC(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_S4U2Self(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_S4U2Self *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* auth */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_general_string(p, len, &(data)->auth, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_GeneralString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->cksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* realm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* name */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_S4U2Self(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_S4U2Self *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t name_datalen, name_oldlen;
Der_type name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &name_type, 0, &name_datalen, &l);
if (e == 0 && name_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
name_oldlen = len;
if (name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = name_datalen;
e = decode_PrincipalName(p, len, &(data)->name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = name_oldlen - name_datalen;
}
{
size_t realm_datalen, realm_oldlen;
Der_type realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &realm_type, 1, &realm_datalen, &l);
if (e == 0 && realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
realm_oldlen = len;
if (realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = realm_datalen;
e = decode_Realm(p, len, &(data)->realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = realm_oldlen - realm_datalen;
}
{
size_t cksum_datalen, cksum_oldlen;
Der_type cksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cksum_type, 2, &cksum_datalen, &l);
if (e == 0 && cksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cksum_oldlen = len;
if (cksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cksum_datalen;
e = decode_Checksum(p, len, &(data)->cksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cksum_oldlen - cksum_datalen;
}
{
size_t auth_datalen, auth_oldlen;
Der_type auth_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &auth_type, 3, &auth_datalen, &l);
if (e == 0 && auth_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
auth_oldlen = len;
if (auth_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = auth_datalen;
{
size_t auth_Tag_datalen, auth_Tag_oldlen;
Der_type auth_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &auth_Tag_type, UT_GeneralString, &auth_Tag_datalen, &l);
if (e == 0 && auth_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
auth_Tag_oldlen = len;
if (auth_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = auth_Tag_datalen;
e = der_get_general_string(p, len, &(data)->auth, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = auth_Tag_oldlen - auth_Tag_datalen;
}
len = auth_oldlen - auth_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_S4U2Self(data);
return e;
}

void ASN1CALL
free_PA_S4U2Self(PA_S4U2Self *data)
{
free_PrincipalName(&(data)->name);
free_Realm(&(data)->realm);
free_Checksum(&(data)->cksum);
der_free_general_string(&(data)->auth);
}

size_t ASN1CALL
length_PA_S4U2Self(const PA_S4U2Self *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->cksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_general_string(&(data)->auth);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_S4U2Self(const PA_S4U2Self *from, PA_S4U2Self *to)
{
memset(to, 0, sizeof(*to));
if(copy_PrincipalName(&(from)->name, &(to)->name)) goto fail;
if(copy_Realm(&(from)->realm, &(to)->realm)) goto fail;
if(copy_Checksum(&(from)->cksum, &(to)->cksum)) goto fail;
if(der_copy_general_string(&(from)->auth, &(to)->auth)) goto fail;
return 0;
fail:
free_PA_S4U2Self(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB5SignedPathData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB5SignedPathData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* method_data */
if((data)->method_data) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_METHOD_DATA(p, len, (data)->method_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* delegated */
if((data)->delegated) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Principals(p, len, (data)->delegated, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* authtime */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->authtime, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* client */
if((data)->client) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Principal(p, len, (data)->client, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB5SignedPathData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB5SignedPathData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t client_datalen, client_oldlen;
Der_type client_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &client_type, 0, &client_datalen, &l);
if (e == 0 && client_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->client = NULL;
} else {
(data)->client = calloc(1, sizeof(*(data)->client));
if ((data)->client == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
client_oldlen = len;
if (client_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = client_datalen;
e = decode_Principal(p, len, (data)->client, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = client_oldlen - client_datalen;
}
}
{
size_t authtime_datalen, authtime_oldlen;
Der_type authtime_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &authtime_type, 1, &authtime_datalen, &l);
if (e == 0 && authtime_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
authtime_oldlen = len;
if (authtime_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = authtime_datalen;
e = decode_KerberosTime(p, len, &(data)->authtime, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = authtime_oldlen - authtime_datalen;
}
{
size_t delegated_datalen, delegated_oldlen;
Der_type delegated_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &delegated_type, 2, &delegated_datalen, &l);
if (e == 0 && delegated_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->delegated = NULL;
} else {
(data)->delegated = calloc(1, sizeof(*(data)->delegated));
if ((data)->delegated == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
delegated_oldlen = len;
if (delegated_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = delegated_datalen;
e = decode_Principals(p, len, (data)->delegated, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = delegated_oldlen - delegated_datalen;
}
}
{
size_t method_data_datalen, method_data_oldlen;
Der_type method_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &method_data_type, 3, &method_data_datalen, &l);
if (e == 0 && method_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->method_data = NULL;
} else {
(data)->method_data = calloc(1, sizeof(*(data)->method_data));
if ((data)->method_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
method_data_oldlen = len;
if (method_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = method_data_datalen;
e = decode_METHOD_DATA(p, len, (data)->method_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = method_data_oldlen - method_data_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB5SignedPathData(data);
return e;
}

void ASN1CALL
free_KRB5SignedPathData(KRB5SignedPathData *data)
{
if((data)->client) {
free_Principal((data)->client);
free((data)->client);
(data)->client = NULL;
}
free_KerberosTime(&(data)->authtime);
if((data)->delegated) {
free_Principals((data)->delegated);
free((data)->delegated);
(data)->delegated = NULL;
}
if((data)->method_data) {
free_METHOD_DATA((data)->method_data);
free((data)->method_data);
(data)->method_data = NULL;
}
}

size_t ASN1CALL
length_KRB5SignedPathData(const KRB5SignedPathData *data)
{
size_t ret = 0;
if((data)->client){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Principal((data)->client);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->authtime);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->delegated){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Principals((data)->delegated);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->method_data){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_METHOD_DATA((data)->method_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB5SignedPathData(const KRB5SignedPathData *from, KRB5SignedPathData *to)
{
memset(to, 0, sizeof(*to));
if((from)->client) {
(to)->client = malloc(sizeof(*(to)->client));
if((to)->client == NULL) goto fail;
if(copy_Principal((from)->client, (to)->client)) goto fail;
}else
(to)->client = NULL;
if(copy_KerberosTime(&(from)->authtime, &(to)->authtime)) goto fail;
if((from)->delegated) {
(to)->delegated = malloc(sizeof(*(to)->delegated));
if((to)->delegated == NULL) goto fail;
if(copy_Principals((from)->delegated, (to)->delegated)) goto fail;
}else
(to)->delegated = NULL;
if((from)->method_data) {
(to)->method_data = malloc(sizeof(*(to)->method_data));
if((to)->method_data == NULL) goto fail;
if(copy_METHOD_DATA((from)->method_data, (to)->method_data)) goto fail;
}else
(to)->method_data = NULL;
return 0;
fail:
free_KRB5SignedPathData(to);
return ENOMEM;
}

int ASN1CALL
encode_KRB5SignedPath(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KRB5SignedPath *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* method_data */
if((data)->method_data) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_METHOD_DATA(p, len, (data)->method_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* delegated */
if((data)->delegated) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Principals(p, len, (data)->delegated, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->cksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* etype */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_ENCTYPE(p, len, &(data)->etype, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KRB5SignedPath(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KRB5SignedPath *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t etype_datalen, etype_oldlen;
Der_type etype_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &etype_type, 0, &etype_datalen, &l);
if (e == 0 && etype_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
etype_oldlen = len;
if (etype_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = etype_datalen;
e = decode_ENCTYPE(p, len, &(data)->etype, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = etype_oldlen - etype_datalen;
}
{
size_t cksum_datalen, cksum_oldlen;
Der_type cksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cksum_type, 1, &cksum_datalen, &l);
if (e == 0 && cksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cksum_oldlen = len;
if (cksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cksum_datalen;
e = decode_Checksum(p, len, &(data)->cksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cksum_oldlen - cksum_datalen;
}
{
size_t delegated_datalen, delegated_oldlen;
Der_type delegated_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &delegated_type, 2, &delegated_datalen, &l);
if (e == 0 && delegated_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->delegated = NULL;
} else {
(data)->delegated = calloc(1, sizeof(*(data)->delegated));
if ((data)->delegated == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
delegated_oldlen = len;
if (delegated_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = delegated_datalen;
e = decode_Principals(p, len, (data)->delegated, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = delegated_oldlen - delegated_datalen;
}
}
{
size_t method_data_datalen, method_data_oldlen;
Der_type method_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &method_data_type, 3, &method_data_datalen, &l);
if (e == 0 && method_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->method_data = NULL;
} else {
(data)->method_data = calloc(1, sizeof(*(data)->method_data));
if ((data)->method_data == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
method_data_oldlen = len;
if (method_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = method_data_datalen;
e = decode_METHOD_DATA(p, len, (data)->method_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = method_data_oldlen - method_data_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KRB5SignedPath(data);
return e;
}

void ASN1CALL
free_KRB5SignedPath(KRB5SignedPath *data)
{
free_ENCTYPE(&(data)->etype);
free_Checksum(&(data)->cksum);
if((data)->delegated) {
free_Principals((data)->delegated);
free((data)->delegated);
(data)->delegated = NULL;
}
if((data)->method_data) {
free_METHOD_DATA((data)->method_data);
free((data)->method_data);
(data)->method_data = NULL;
}
}

size_t ASN1CALL
length_KRB5SignedPath(const KRB5SignedPath *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_ENCTYPE(&(data)->etype);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->cksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->delegated){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Principals((data)->delegated);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->method_data){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_METHOD_DATA((data)->method_data);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KRB5SignedPath(const KRB5SignedPath *from, KRB5SignedPath *to)
{
memset(to, 0, sizeof(*to));
if(copy_ENCTYPE(&(from)->etype, &(to)->etype)) goto fail;
if(copy_Checksum(&(from)->cksum, &(to)->cksum)) goto fail;
if((from)->delegated) {
(to)->delegated = malloc(sizeof(*(to)->delegated));
if((to)->delegated == NULL) goto fail;
if(copy_Principals((from)->delegated, (to)->delegated)) goto fail;
}else
(to)->delegated = NULL;
if((from)->method_data) {
(to)->method_data = malloc(sizeof(*(to)->method_data));
if((to)->method_data == NULL) goto fail;
if(copy_METHOD_DATA((from)->method_data, (to)->method_data)) goto fail;
}else
(to)->method_data = NULL;
return 0;
fail:
free_KRB5SignedPath(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_ClientCanonicalizedNames(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_ClientCanonicalizedNames *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* mapped-name */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->mapped_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* requested-name */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->requested_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_ClientCanonicalizedNames(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_ClientCanonicalizedNames *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t requested_name_datalen, requested_name_oldlen;
Der_type requested_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &requested_name_type, 0, &requested_name_datalen, &l);
if (e == 0 && requested_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
requested_name_oldlen = len;
if (requested_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = requested_name_datalen;
e = decode_PrincipalName(p, len, &(data)->requested_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = requested_name_oldlen - requested_name_datalen;
}
{
size_t mapped_name_datalen, mapped_name_oldlen;
Der_type mapped_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &mapped_name_type, 1, &mapped_name_datalen, &l);
if (e == 0 && mapped_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
mapped_name_oldlen = len;
if (mapped_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = mapped_name_datalen;
e = decode_PrincipalName(p, len, &(data)->mapped_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = mapped_name_oldlen - mapped_name_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_ClientCanonicalizedNames(data);
return e;
}

void ASN1CALL
free_PA_ClientCanonicalizedNames(PA_ClientCanonicalizedNames *data)
{
free_PrincipalName(&(data)->requested_name);
free_PrincipalName(&(data)->mapped_name);
}

size_t ASN1CALL
length_PA_ClientCanonicalizedNames(const PA_ClientCanonicalizedNames *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->requested_name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->mapped_name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_ClientCanonicalizedNames(const PA_ClientCanonicalizedNames *from, PA_ClientCanonicalizedNames *to)
{
memset(to, 0, sizeof(*to));
if(copy_PrincipalName(&(from)->requested_name, &(to)->requested_name)) goto fail;
if(copy_PrincipalName(&(from)->mapped_name, &(to)->mapped_name)) goto fail;
return 0;
fail:
free_PA_ClientCanonicalizedNames(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_ClientCanonicalized(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_ClientCanonicalized *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* canon-checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->canon_checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* names */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PA_ClientCanonicalizedNames(p, len, &(data)->names, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_ClientCanonicalized(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_ClientCanonicalized *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t names_datalen, names_oldlen;
Der_type names_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &names_type, 0, &names_datalen, &l);
if (e == 0 && names_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
names_oldlen = len;
if (names_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = names_datalen;
e = decode_PA_ClientCanonicalizedNames(p, len, &(data)->names, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = names_oldlen - names_datalen;
}
{
size_t canon_checksum_datalen, canon_checksum_oldlen;
Der_type canon_checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &canon_checksum_type, 1, &canon_checksum_datalen, &l);
if (e == 0 && canon_checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
canon_checksum_oldlen = len;
if (canon_checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = canon_checksum_datalen;
e = decode_Checksum(p, len, &(data)->canon_checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = canon_checksum_oldlen - canon_checksum_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_ClientCanonicalized(data);
return e;
}

void ASN1CALL
free_PA_ClientCanonicalized(PA_ClientCanonicalized *data)
{
free_PA_ClientCanonicalizedNames(&(data)->names);
free_Checksum(&(data)->canon_checksum);
}

size_t ASN1CALL
length_PA_ClientCanonicalized(const PA_ClientCanonicalized *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PA_ClientCanonicalizedNames(&(data)->names);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->canon_checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_ClientCanonicalized(const PA_ClientCanonicalized *from, PA_ClientCanonicalized *to)
{
memset(to, 0, sizeof(*to));
if(copy_PA_ClientCanonicalizedNames(&(from)->names, &(to)->names)) goto fail;
if(copy_Checksum(&(from)->canon_checksum, &(to)->canon_checksum)) goto fail;
return 0;
fail:
free_PA_ClientCanonicalized(to);
return ENOMEM;
}

int ASN1CALL
encode_AD_LoginAlias(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const AD_LoginAlias *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* login-alias */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->login_alias, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_AD_LoginAlias(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, AD_LoginAlias *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t login_alias_datalen, login_alias_oldlen;
Der_type login_alias_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &login_alias_type, 0, &login_alias_datalen, &l);
if (e == 0 && login_alias_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
login_alias_oldlen = len;
if (login_alias_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = login_alias_datalen;
e = decode_PrincipalName(p, len, &(data)->login_alias, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = login_alias_oldlen - login_alias_datalen;
}
{
size_t checksum_datalen, checksum_oldlen;
Der_type checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &checksum_type, 1, &checksum_datalen, &l);
if (e == 0 && checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
checksum_oldlen = len;
if (checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = checksum_datalen;
e = decode_Checksum(p, len, &(data)->checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = checksum_oldlen - checksum_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_AD_LoginAlias(data);
return e;
}

void ASN1CALL
free_AD_LoginAlias(AD_LoginAlias *data)
{
free_PrincipalName(&(data)->login_alias);
free_Checksum(&(data)->checksum);
}

size_t ASN1CALL
length_AD_LoginAlias(const AD_LoginAlias *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->login_alias);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_AD_LoginAlias(const AD_LoginAlias *from, AD_LoginAlias *to)
{
memset(to, 0, sizeof(*to));
if(copy_PrincipalName(&(from)->login_alias, &(to)->login_alias)) goto fail;
if(copy_Checksum(&(from)->checksum, &(to)->checksum)) goto fail;
return 0;
fail:
free_AD_LoginAlias(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_SvrReferralData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SvrReferralData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* referred-realm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->referred_realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* referred-name */
if((data)->referred_name) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->referred_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SvrReferralData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SvrReferralData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t referred_name_datalen, referred_name_oldlen;
Der_type referred_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &referred_name_type, 1, &referred_name_datalen, &l);
if (e == 0 && referred_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->referred_name = NULL;
} else {
(data)->referred_name = calloc(1, sizeof(*(data)->referred_name));
if ((data)->referred_name == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
referred_name_oldlen = len;
if (referred_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = referred_name_datalen;
e = decode_PrincipalName(p, len, (data)->referred_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = referred_name_oldlen - referred_name_datalen;
}
}
{
size_t referred_realm_datalen, referred_realm_oldlen;
Der_type referred_realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &referred_realm_type, 0, &referred_realm_datalen, &l);
if (e == 0 && referred_realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
referred_realm_oldlen = len;
if (referred_realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = referred_realm_datalen;
e = decode_Realm(p, len, &(data)->referred_realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = referred_realm_oldlen - referred_realm_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_SvrReferralData(data);
return e;
}

void ASN1CALL
free_PA_SvrReferralData(PA_SvrReferralData *data)
{
if((data)->referred_name) {
free_PrincipalName((data)->referred_name);
free((data)->referred_name);
(data)->referred_name = NULL;
}
free_Realm(&(data)->referred_realm);
}

size_t ASN1CALL
length_PA_SvrReferralData(const PA_SvrReferralData *data)
{
size_t ret = 0;
if((data)->referred_name){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->referred_name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->referred_realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_SvrReferralData(const PA_SvrReferralData *from, PA_SvrReferralData *to)
{
memset(to, 0, sizeof(*to));
if((from)->referred_name) {
(to)->referred_name = malloc(sizeof(*(to)->referred_name));
if((to)->referred_name == NULL) goto fail;
if(copy_PrincipalName((from)->referred_name, (to)->referred_name)) goto fail;
}else
(to)->referred_name = NULL;
if(copy_Realm(&(from)->referred_realm, &(to)->referred_realm)) goto fail;
return 0;
fail:
free_PA_SvrReferralData(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_SERVER_REFERRAL_DATA(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_SERVER_REFERRAL_DATA *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

e = encode_EncryptedData(p, len, data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_SERVER_REFERRAL_DATA(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_SERVER_REFERRAL_DATA *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
e = decode_EncryptedData(p, len, data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
if(size) *size = ret;
return 0;
fail:
free_PA_SERVER_REFERRAL_DATA(data);
return e;
}

void ASN1CALL
free_PA_SERVER_REFERRAL_DATA(PA_SERVER_REFERRAL_DATA *data)
{
free_EncryptedData(data);
}

size_t ASN1CALL
length_PA_SERVER_REFERRAL_DATA(const PA_SERVER_REFERRAL_DATA *data)
{
size_t ret = 0;
ret += length_EncryptedData(data);
return ret;
}

int ASN1CALL
copy_PA_SERVER_REFERRAL_DATA(const PA_SERVER_REFERRAL_DATA *from, PA_SERVER_REFERRAL_DATA *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncryptedData(from, to)) goto fail;
return 0;
fail:
free_PA_SERVER_REFERRAL_DATA(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_ServerReferralData(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_ServerReferralData *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* referral-valid-until */
if((data)->referral_valid_until) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, (data)->referral_valid_until, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* requested-principal-name */
if((data)->requested_principal_name) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->requested_principal_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* true-principal-name */
if((data)->true_principal_name) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, (data)->true_principal_name, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* referred-realm */
if((data)->referred_realm) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, (data)->referred_realm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_PA_ServerReferralData(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_ServerReferralData *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t referred_realm_datalen, referred_realm_oldlen;
Der_type referred_realm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &referred_realm_type, 0, &referred_realm_datalen, &l);
if (e == 0 && referred_realm_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->referred_realm = NULL;
} else {
(data)->referred_realm = calloc(1, sizeof(*(data)->referred_realm));
if ((data)->referred_realm == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
referred_realm_oldlen = len;
if (referred_realm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = referred_realm_datalen;
e = decode_Realm(p, len, (data)->referred_realm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = referred_realm_oldlen - referred_realm_datalen;
}
}
{
size_t true_principal_name_datalen, true_principal_name_oldlen;
Der_type true_principal_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &true_principal_name_type, 1, &true_principal_name_datalen, &l);
if (e == 0 && true_principal_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->true_principal_name = NULL;
} else {
(data)->true_principal_name = calloc(1, sizeof(*(data)->true_principal_name));
if ((data)->true_principal_name == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
true_principal_name_oldlen = len;
if (true_principal_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = true_principal_name_datalen;
e = decode_PrincipalName(p, len, (data)->true_principal_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = true_principal_name_oldlen - true_principal_name_datalen;
}
}
{
size_t requested_principal_name_datalen, requested_principal_name_oldlen;
Der_type requested_principal_name_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &requested_principal_name_type, 2, &requested_principal_name_datalen, &l);
if (e == 0 && requested_principal_name_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->requested_principal_name = NULL;
} else {
(data)->requested_principal_name = calloc(1, sizeof(*(data)->requested_principal_name));
if ((data)->requested_principal_name == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
requested_principal_name_oldlen = len;
if (requested_principal_name_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = requested_principal_name_datalen;
e = decode_PrincipalName(p, len, (data)->requested_principal_name, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = requested_principal_name_oldlen - requested_principal_name_datalen;
}
}
{
size_t referral_valid_until_datalen, referral_valid_until_oldlen;
Der_type referral_valid_until_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &referral_valid_until_type, 3, &referral_valid_until_datalen, &l);
if (e == 0 && referral_valid_until_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->referral_valid_until = NULL;
} else {
(data)->referral_valid_until = calloc(1, sizeof(*(data)->referral_valid_until));
if ((data)->referral_valid_until == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
referral_valid_until_oldlen = len;
if (referral_valid_until_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = referral_valid_until_datalen;
e = decode_KerberosTime(p, len, (data)->referral_valid_until, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = referral_valid_until_oldlen - referral_valid_until_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_PA_ServerReferralData(data);
return e;
}

void ASN1CALL
free_PA_ServerReferralData(PA_ServerReferralData *data)
{
if((data)->referred_realm) {
free_Realm((data)->referred_realm);
free((data)->referred_realm);
(data)->referred_realm = NULL;
}
if((data)->true_principal_name) {
free_PrincipalName((data)->true_principal_name);
free((data)->true_principal_name);
(data)->true_principal_name = NULL;
}
if((data)->requested_principal_name) {
free_PrincipalName((data)->requested_principal_name);
free((data)->requested_principal_name);
(data)->requested_principal_name = NULL;
}
if((data)->referral_valid_until) {
free_KerberosTime((data)->referral_valid_until);
free((data)->referral_valid_until);
(data)->referral_valid_until = NULL;
}
}

size_t ASN1CALL
length_PA_ServerReferralData(const PA_ServerReferralData *data)
{
size_t ret = 0;
if((data)->referred_realm){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm((data)->referred_realm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->true_principal_name){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->true_principal_name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->requested_principal_name){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName((data)->requested_principal_name);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->referral_valid_until){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime((data)->referral_valid_until);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_PA_ServerReferralData(const PA_ServerReferralData *from, PA_ServerReferralData *to)
{
memset(to, 0, sizeof(*to));
if((from)->referred_realm) {
(to)->referred_realm = malloc(sizeof(*(to)->referred_realm));
if((to)->referred_realm == NULL) goto fail;
if(copy_Realm((from)->referred_realm, (to)->referred_realm)) goto fail;
}else
(to)->referred_realm = NULL;
if((from)->true_principal_name) {
(to)->true_principal_name = malloc(sizeof(*(to)->true_principal_name));
if((to)->true_principal_name == NULL) goto fail;
if(copy_PrincipalName((from)->true_principal_name, (to)->true_principal_name)) goto fail;
}else
(to)->true_principal_name = NULL;
if((from)->requested_principal_name) {
(to)->requested_principal_name = malloc(sizeof(*(to)->requested_principal_name));
if((to)->requested_principal_name == NULL) goto fail;
if(copy_PrincipalName((from)->requested_principal_name, (to)->requested_principal_name)) goto fail;
}else
(to)->requested_principal_name = NULL;
if((from)->referral_valid_until) {
(to)->referral_valid_until = malloc(sizeof(*(to)->referral_valid_until));
if((to)->referral_valid_until == NULL) goto fail;
if(copy_KerberosTime((from)->referral_valid_until, (to)->referral_valid_until)) goto fail;
}else
(to)->referral_valid_until = NULL;
return 0;
fail:
free_PA_ServerReferralData(to);
return ENOMEM;
}

int ASN1CALL
encode_FastOptions(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const FastOptions *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

{
unsigned char c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->kdc_follow__referrals) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
c = 0;
if((data)->hide_client_names) {
c |= 1<<6;
}
if((data)->reserved) {
c |= 1<<7;
}
if (len < 1) return ASN1_OVERFLOW;
*p-- = c; len--; ret++;
if (len < 1) return ASN1_OVERFLOW;
*p-- = 0;
len -= 1;
ret += 1;
}

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_BitString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_FastOptions(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, FastOptions *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_BitString, &Top_datalen, &l);
if (e == 0 && Top_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
if (len < 1) return ASN1_OVERRUN;
p++; len--; ret++;
do {
if (len < 1) break;
(data)->reserved = (*p >> 7) & 1;
(data)->hide_client_names = (*p >> 6) & 1;
p++; len--; ret++;
if (len < 1) break;
p++; len--; ret++;
if (len < 1) break;
(data)->kdc_follow__referrals = (*p >> 7) & 1;
} while(0);
p += len; ret += len;
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_FastOptions(data);
return e;
}

void ASN1CALL
free_FastOptions(FastOptions *data)
{
}

size_t ASN1CALL
length_FastOptions(const FastOptions *data)
{
size_t ret = 0;
ret += 5;
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_FastOptions(const FastOptions *from, FastOptions *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

unsigned FastOptions2int(FastOptions f)
{
unsigned r = 0;
if(f.reserved) r |= (1U << 0);
if(f.hide_client_names) r |= (1U << 1);
if(f.kdc_follow__referrals) r |= (1U << 16);
return r;
}

FastOptions int2FastOptions(unsigned n)
{
	FastOptions flags;

	memset(&flags, 0, sizeof(flags));

	flags.reserved = (n >> 0) & 1;
	flags.hide_client_names = (n >> 1) & 1;
	flags.kdc_follow__referrals = (n >> 16) & 1;
	return flags;
}

static struct units FastOptions_units[] = {
	{"kdc-follow--referrals",	1U << 16},
	{"hide-client-names",	1U << 1},
	{"reserved",	1U << 0},
	{NULL,	0}
};

const struct units * asn1_FastOptions_units(void){
return FastOptions_units;
}

int ASN1CALL
encode_KrbFastReq(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastReq *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* req-body */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KDC_REQ_BODY(p, len, &(data)->req_body, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* padata */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->padata)->len - 1; i >= 0; --i) {
size_t padata_tag_tag_for_oldret = ret;
ret = 0;
e = encode_PA_DATA(p, len, &(&(data)->padata)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += padata_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* fast-options */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_FastOptions(p, len, &(data)->fast_options, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastReq(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastReq *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t fast_options_datalen, fast_options_oldlen;
Der_type fast_options_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &fast_options_type, 0, &fast_options_datalen, &l);
if (e == 0 && fast_options_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
fast_options_oldlen = len;
if (fast_options_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = fast_options_datalen;
e = decode_FastOptions(p, len, &(data)->fast_options, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = fast_options_oldlen - fast_options_datalen;
}
{
size_t padata_datalen, padata_oldlen;
Der_type padata_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_type, 1, &padata_datalen, &l);
if (e == 0 && padata_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_oldlen = len;
if (padata_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_datalen;
{
size_t padata_Tag_datalen, padata_Tag_oldlen;
Der_type padata_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &padata_Tag_type, UT_Sequence, &padata_Tag_datalen, &l);
if (e == 0 && padata_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_Tag_oldlen = len;
if (padata_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_Tag_datalen;
{
size_t padata_Tag_Tag_origlen = len;
size_t padata_Tag_Tag_oldret = ret;
size_t padata_Tag_Tag_olen = 0;
void *padata_Tag_Tag_tmp;
ret = 0;
(&(data)->padata)->len = 0;
(&(data)->padata)->val = NULL;
while(ret < padata_Tag_Tag_origlen) {
size_t padata_Tag_Tag_nlen = padata_Tag_Tag_olen + sizeof(*((&(data)->padata)->val));
if (padata_Tag_Tag_olen > padata_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
padata_Tag_Tag_olen = padata_Tag_Tag_nlen;
padata_Tag_Tag_tmp = realloc((&(data)->padata)->val, padata_Tag_Tag_olen);
if (padata_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->padata)->val = padata_Tag_Tag_tmp;
e = decode_PA_DATA(p, len, &(&(data)->padata)->val[(&(data)->padata)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->padata)->len++;
len = padata_Tag_Tag_origlen - ret;
}
ret += padata_Tag_Tag_oldret;
}
len = padata_Tag_oldlen - padata_Tag_datalen;
}
len = padata_oldlen - padata_datalen;
}
{
size_t req_body_datalen, req_body_oldlen;
Der_type req_body_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &req_body_type, 2, &req_body_datalen, &l);
if (e == 0 && req_body_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
req_body_oldlen = len;
if (req_body_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = req_body_datalen;
e = decode_KDC_REQ_BODY(p, len, &(data)->req_body, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = req_body_oldlen - req_body_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastReq(data);
return e;
}

void ASN1CALL
free_KrbFastReq(KrbFastReq *data)
{
free_FastOptions(&(data)->fast_options);
while((&(data)->padata)->len){
free_PA_DATA(&(&(data)->padata)->val[(&(data)->padata)->len-1]);
(&(data)->padata)->len--;
}
free((&(data)->padata)->val);
(&(data)->padata)->val = NULL;
free_KDC_REQ_BODY(&(data)->req_body);
}

size_t ASN1CALL
length_KrbFastReq(const KrbFastReq *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_FastOptions(&(data)->fast_options);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t padata_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->padata)->len - 1; i >= 0; --i){
size_t padata_tag_tag_for_oldret = ret;
ret = 0;
ret += length_PA_DATA(&(&(data)->padata)->val[i]);
ret += padata_tag_tag_for_oldret;
}
ret += padata_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KDC_REQ_BODY(&(data)->req_body);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastReq(const KrbFastReq *from, KrbFastReq *to)
{
memset(to, 0, sizeof(*to));
if(copy_FastOptions(&(from)->fast_options, &(to)->fast_options)) goto fail;
if(((&(to)->padata)->val = malloc((&(from)->padata)->len * sizeof(*(&(to)->padata)->val))) == NULL && (&(from)->padata)->len != 0)
goto fail;
for((&(to)->padata)->len = 0; (&(to)->padata)->len < (&(from)->padata)->len; (&(to)->padata)->len++){
if(copy_PA_DATA(&(&(from)->padata)->val[(&(to)->padata)->len], &(&(to)->padata)->val[(&(to)->padata)->len])) goto fail;
}
if(copy_KDC_REQ_BODY(&(from)->req_body, &(to)->req_body)) goto fail;
return 0;
fail:
free_KrbFastReq(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbFastArmor(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastArmor *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* armor-value */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = der_put_octet_string(p, len, &(data)->armor_value, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, PRIM, UT_OctetString, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* armor-type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->armor_type, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastArmor(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastArmor *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t armor_type_datalen, armor_type_oldlen;
Der_type armor_type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &armor_type_type, 0, &armor_type_datalen, &l);
if (e == 0 && armor_type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
armor_type_oldlen = len;
if (armor_type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armor_type_datalen;
e = decode_krb5int32(p, len, &(data)->armor_type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = armor_type_oldlen - armor_type_datalen;
}
{
size_t armor_value_datalen, armor_value_oldlen;
Der_type armor_value_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &armor_value_type, 1, &armor_value_datalen, &l);
if (e == 0 && armor_value_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
armor_value_oldlen = len;
if (armor_value_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armor_value_datalen;
{
size_t armor_value_Tag_datalen, armor_value_Tag_oldlen;
Der_type armor_value_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &armor_value_Tag_type, UT_OctetString, &armor_value_Tag_datalen, &l);
if (e == 0 && armor_value_Tag_type != PRIM) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
armor_value_Tag_oldlen = len;
if (armor_value_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armor_value_Tag_datalen;
e = der_get_octet_string(p, len, &(data)->armor_value, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = armor_value_Tag_oldlen - armor_value_Tag_datalen;
}
len = armor_value_oldlen - armor_value_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastArmor(data);
return e;
}

void ASN1CALL
free_KrbFastArmor(KrbFastArmor *data)
{
free_krb5int32(&(data)->armor_type);
der_free_octet_string(&(data)->armor_value);
}

size_t ASN1CALL
length_KrbFastArmor(const KrbFastArmor *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->armor_type);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += der_length_octet_string(&(data)->armor_value);
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastArmor(const KrbFastArmor *from, KrbFastArmor *to)
{
memset(to, 0, sizeof(*to));
if(copy_krb5int32(&(from)->armor_type, &(to)->armor_type)) goto fail;
if(der_copy_octet_string(&(from)->armor_value, &(to)->armor_value)) goto fail;
return 0;
fail:
free_KrbFastArmor(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbFastArmoredReq(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastArmoredReq *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-fast-req */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_fast_req, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* req-checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->req_checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* armor */
if((data)->armor) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KrbFastArmor(p, len, (data)->armor, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastArmoredReq(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastArmoredReq *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t armor_datalen, armor_oldlen;
Der_type armor_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &armor_type, 0, &armor_datalen, &l);
if (e == 0 && armor_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->armor = NULL;
} else {
(data)->armor = calloc(1, sizeof(*(data)->armor));
if ((data)->armor == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
armor_oldlen = len;
if (armor_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armor_datalen;
e = decode_KrbFastArmor(p, len, (data)->armor, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = armor_oldlen - armor_datalen;
}
}
{
size_t req_checksum_datalen, req_checksum_oldlen;
Der_type req_checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &req_checksum_type, 1, &req_checksum_datalen, &l);
if (e == 0 && req_checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
req_checksum_oldlen = len;
if (req_checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = req_checksum_datalen;
e = decode_Checksum(p, len, &(data)->req_checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = req_checksum_oldlen - req_checksum_datalen;
}
{
size_t enc_fast_req_datalen, enc_fast_req_oldlen;
Der_type enc_fast_req_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_fast_req_type, 2, &enc_fast_req_datalen, &l);
if (e == 0 && enc_fast_req_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_fast_req_oldlen = len;
if (enc_fast_req_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_fast_req_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_fast_req, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_fast_req_oldlen - enc_fast_req_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastArmoredReq(data);
return e;
}

void ASN1CALL
free_KrbFastArmoredReq(KrbFastArmoredReq *data)
{
if((data)->armor) {
free_KrbFastArmor((data)->armor);
free((data)->armor);
(data)->armor = NULL;
}
free_Checksum(&(data)->req_checksum);
free_EncryptedData(&(data)->enc_fast_req);
}

size_t ASN1CALL
length_KrbFastArmoredReq(const KrbFastArmoredReq *data)
{
size_t ret = 0;
if((data)->armor){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KrbFastArmor((data)->armor);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->req_checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_fast_req);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastArmoredReq(const KrbFastArmoredReq *from, KrbFastArmoredReq *to)
{
memset(to, 0, sizeof(*to));
if((from)->armor) {
(to)->armor = malloc(sizeof(*(to)->armor));
if((to)->armor == NULL) goto fail;
if(copy_KrbFastArmor((from)->armor, (to)->armor)) goto fail;
}else
(to)->armor = NULL;
if(copy_Checksum(&(from)->req_checksum, &(to)->req_checksum)) goto fail;
if(copy_EncryptedData(&(from)->enc_fast_req, &(to)->enc_fast_req)) goto fail;
return 0;
fail:
free_KrbFastArmoredReq(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_FX_FAST_REQUEST(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_FX_FAST_REQUEST *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;


switch((data)->element) {
case choice_PA_FX_FAST_REQUEST_armored_data: {size_t Top_oldret = ret;
ret = 0;
e = encode_KrbFastArmoredReq(p, len, &((data))->u.armored_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_oldret;
break;
}
case choice_PA_FX_FAST_REQUEST_asn1_ellipsis: {
if (len < (data)->u.asn1_ellipsis.length)
return ASN1_OVERFLOW;
p -= (data)->u.asn1_ellipsis.length;
ret += (data)->u.asn1_ellipsis.length;
memcpy(p + 1, (data)->u.asn1_ellipsis.data, (data)->u.asn1_ellipsis.length);
break;
}
};
*size = ret;
return 0;
}

int ASN1CALL
decode_PA_FX_FAST_REQUEST(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_FX_FAST_REQUEST *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
if (der_match_tag(p, len, ASN1_C_CONTEXT, CONS, 0, NULL) == 0) {
{
size_t armored_data_datalen, armored_data_oldlen;
Der_type armored_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &armored_data_type, 0, &armored_data_datalen, &l);
if (e == 0 && armored_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
armored_data_oldlen = len;
if (armored_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armored_data_datalen;
e = decode_KrbFastArmoredReq(p, len, &(data)->u.armored_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = armored_data_oldlen - armored_data_datalen;
}
(data)->element = choice_PA_FX_FAST_REQUEST_armored_data;
}
else {
(data)->u.asn1_ellipsis.data = calloc(1, len);
if ((data)->u.asn1_ellipsis.data == NULL) {
e = ENOMEM; goto fail;
}
(data)->u.asn1_ellipsis.length = len;
memcpy((data)->u.asn1_ellipsis.data, p, len);
(data)->element = choice_PA_FX_FAST_REQUEST_asn1_ellipsis;
p += len;
ret += len;
len = 0;
}
if(size) *size = ret;
return 0;
fail:
free_PA_FX_FAST_REQUEST(data);
return e;
}

void ASN1CALL
free_PA_FX_FAST_REQUEST(PA_FX_FAST_REQUEST *data)
{
switch((data)->element) {
case choice_PA_FX_FAST_REQUEST_armored_data:
free_KrbFastArmoredReq(&(data)->u.armored_data);
break;
case choice_PA_FX_FAST_REQUEST_asn1_ellipsis:
der_free_octet_string(&(data)->u.asn1_ellipsis);
break;}
}

size_t ASN1CALL
length_PA_FX_FAST_REQUEST(const PA_FX_FAST_REQUEST *data)
{
size_t ret = 0;
switch((data)->element) {
case choice_PA_FX_FAST_REQUEST_armored_data:
{
size_t Top_oldret = ret;
ret = 0;
ret += length_KrbFastArmoredReq(&(data)->u.armored_data);
ret += 1 + der_length_len (ret);
ret += Top_oldret;
}
break;
case choice_PA_FX_FAST_REQUEST_asn1_ellipsis:
ret += (data)->u.asn1_ellipsis.length;
break;
}
return ret;
}

int ASN1CALL
copy_PA_FX_FAST_REQUEST(const PA_FX_FAST_REQUEST *from, PA_FX_FAST_REQUEST *to)
{
memset(to, 0, sizeof(*to));
(to)->element = (from)->element;
switch((from)->element) {
case choice_PA_FX_FAST_REQUEST_armored_data:
if(copy_KrbFastArmoredReq(&(from)->u.armored_data, &(to)->u.armored_data)) goto fail;
break;
case choice_PA_FX_FAST_REQUEST_asn1_ellipsis: {
int ret;
ret=der_copy_octet_string(&(from)->u.asn1_ellipsis, &(to)->u.asn1_ellipsis);
if (ret) goto fail;
break;
}
}
return 0;
fail:
free_PA_FX_FAST_REQUEST(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbFastFinished(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastFinished *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* ticket-checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->ticket_checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 5, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* checksum */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Checksum(p, len, &(data)->checksum, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* cname */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_PrincipalName(p, len, &(data)->cname, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* crealm */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Realm(p, len, &(data)->crealm, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* usec */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_krb5int32(p, len, &(data)->usec, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* timestamp */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KerberosTime(p, len, &(data)->timestamp, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastFinished(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastFinished *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t timestamp_datalen, timestamp_oldlen;
Der_type timestamp_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &timestamp_type, 0, &timestamp_datalen, &l);
if (e == 0 && timestamp_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
timestamp_oldlen = len;
if (timestamp_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = timestamp_datalen;
e = decode_KerberosTime(p, len, &(data)->timestamp, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = timestamp_oldlen - timestamp_datalen;
}
{
size_t usec_datalen, usec_oldlen;
Der_type usec_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &usec_type, 1, &usec_datalen, &l);
if (e == 0 && usec_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
usec_oldlen = len;
if (usec_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = usec_datalen;
e = decode_krb5int32(p, len, &(data)->usec, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = usec_oldlen - usec_datalen;
}
{
size_t crealm_datalen, crealm_oldlen;
Der_type crealm_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &crealm_type, 2, &crealm_datalen, &l);
if (e == 0 && crealm_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
crealm_oldlen = len;
if (crealm_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = crealm_datalen;
e = decode_Realm(p, len, &(data)->crealm, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = crealm_oldlen - crealm_datalen;
}
{
size_t cname_datalen, cname_oldlen;
Der_type cname_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &cname_type, 3, &cname_datalen, &l);
if (e == 0 && cname_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
cname_oldlen = len;
if (cname_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = cname_datalen;
e = decode_PrincipalName(p, len, &(data)->cname, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = cname_oldlen - cname_datalen;
}
{
size_t checksum_datalen, checksum_oldlen;
Der_type checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &checksum_type, 4, &checksum_datalen, &l);
if (e == 0 && checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
checksum_oldlen = len;
if (checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = checksum_datalen;
e = decode_Checksum(p, len, &(data)->checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = checksum_oldlen - checksum_datalen;
}
{
size_t ticket_checksum_datalen, ticket_checksum_oldlen;
Der_type ticket_checksum_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &ticket_checksum_type, 5, &ticket_checksum_datalen, &l);
if (e == 0 && ticket_checksum_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
ticket_checksum_oldlen = len;
if (ticket_checksum_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = ticket_checksum_datalen;
e = decode_Checksum(p, len, &(data)->ticket_checksum, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = ticket_checksum_oldlen - ticket_checksum_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastFinished(data);
return e;
}

void ASN1CALL
free_KrbFastFinished(KrbFastFinished *data)
{
free_KerberosTime(&(data)->timestamp);
free_krb5int32(&(data)->usec);
free_Realm(&(data)->crealm);
free_PrincipalName(&(data)->cname);
free_Checksum(&(data)->checksum);
free_Checksum(&(data)->ticket_checksum);
}

size_t ASN1CALL
length_KrbFastFinished(const KrbFastFinished *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KerberosTime(&(data)->timestamp);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_krb5int32(&(data)->usec);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Realm(&(data)->crealm);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_PrincipalName(&(data)->cname);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Checksum(&(data)->ticket_checksum);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastFinished(const KrbFastFinished *from, KrbFastFinished *to)
{
memset(to, 0, sizeof(*to));
if(copy_KerberosTime(&(from)->timestamp, &(to)->timestamp)) goto fail;
if(copy_krb5int32(&(from)->usec, &(to)->usec)) goto fail;
if(copy_Realm(&(from)->crealm, &(to)->crealm)) goto fail;
if(copy_PrincipalName(&(from)->cname, &(to)->cname)) goto fail;
if(copy_Checksum(&(from)->checksum, &(to)->checksum)) goto fail;
if(copy_Checksum(&(from)->ticket_checksum, &(to)->ticket_checksum)) goto fail;
return 0;
fail:
free_KrbFastFinished(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbFastResponse(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastResponse *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* finished */
if((data)->finished) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KrbFastFinished(p, len, (data)->finished, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* rep-key */
if((data)->rep_key) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKey(p, len, (data)->rep_key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* padata */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->padata)->len - 1; i >= 0; --i) {
size_t padata_tag_tag_for_oldret = ret;
ret = 0;
e = encode_PA_DATA(p, len, &(&(data)->padata)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += padata_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastResponse(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastResponse *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t padata_datalen, padata_oldlen;
Der_type padata_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &padata_type, 0, &padata_datalen, &l);
if (e == 0 && padata_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_oldlen = len;
if (padata_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_datalen;
{
size_t padata_Tag_datalen, padata_Tag_oldlen;
Der_type padata_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &padata_Tag_type, UT_Sequence, &padata_Tag_datalen, &l);
if (e == 0 && padata_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
padata_Tag_oldlen = len;
if (padata_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = padata_Tag_datalen;
{
size_t padata_Tag_Tag_origlen = len;
size_t padata_Tag_Tag_oldret = ret;
size_t padata_Tag_Tag_olen = 0;
void *padata_Tag_Tag_tmp;
ret = 0;
(&(data)->padata)->len = 0;
(&(data)->padata)->val = NULL;
while(ret < padata_Tag_Tag_origlen) {
size_t padata_Tag_Tag_nlen = padata_Tag_Tag_olen + sizeof(*((&(data)->padata)->val));
if (padata_Tag_Tag_olen > padata_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
padata_Tag_Tag_olen = padata_Tag_Tag_nlen;
padata_Tag_Tag_tmp = realloc((&(data)->padata)->val, padata_Tag_Tag_olen);
if (padata_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->padata)->val = padata_Tag_Tag_tmp;
e = decode_PA_DATA(p, len, &(&(data)->padata)->val[(&(data)->padata)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->padata)->len++;
len = padata_Tag_Tag_origlen - ret;
}
ret += padata_Tag_Tag_oldret;
}
len = padata_Tag_oldlen - padata_Tag_datalen;
}
len = padata_oldlen - padata_datalen;
}
{
size_t rep_key_datalen, rep_key_oldlen;
Der_type rep_key_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &rep_key_type, 1, &rep_key_datalen, &l);
if (e == 0 && rep_key_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->rep_key = NULL;
} else {
(data)->rep_key = calloc(1, sizeof(*(data)->rep_key));
if ((data)->rep_key == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
rep_key_oldlen = len;
if (rep_key_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = rep_key_datalen;
e = decode_EncryptionKey(p, len, (data)->rep_key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = rep_key_oldlen - rep_key_datalen;
}
}
{
size_t finished_datalen, finished_oldlen;
Der_type finished_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &finished_type, 2, &finished_datalen, &l);
if (e == 0 && finished_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->finished = NULL;
} else {
(data)->finished = calloc(1, sizeof(*(data)->finished));
if ((data)->finished == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
finished_oldlen = len;
if (finished_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = finished_datalen;
e = decode_KrbFastFinished(p, len, (data)->finished, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = finished_oldlen - finished_datalen;
}
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastResponse(data);
return e;
}

void ASN1CALL
free_KrbFastResponse(KrbFastResponse *data)
{
while((&(data)->padata)->len){
free_PA_DATA(&(&(data)->padata)->val[(&(data)->padata)->len-1]);
(&(data)->padata)->len--;
}
free((&(data)->padata)->val);
(&(data)->padata)->val = NULL;
if((data)->rep_key) {
free_EncryptionKey((data)->rep_key);
free((data)->rep_key);
(data)->rep_key = NULL;
}
if((data)->finished) {
free_KrbFastFinished((data)->finished);
free((data)->finished);
(data)->finished = NULL;
}
}

size_t ASN1CALL
length_KrbFastResponse(const KrbFastResponse *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t padata_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->padata)->len - 1; i >= 0; --i){
size_t padata_tag_tag_for_oldret = ret;
ret = 0;
ret += length_PA_DATA(&(&(data)->padata)->val[i]);
ret += padata_tag_tag_for_oldret;
}
ret += padata_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->rep_key){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKey((data)->rep_key);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->finished){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KrbFastFinished((data)->finished);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastResponse(const KrbFastResponse *from, KrbFastResponse *to)
{
memset(to, 0, sizeof(*to));
if(((&(to)->padata)->val = malloc((&(from)->padata)->len * sizeof(*(&(to)->padata)->val))) == NULL && (&(from)->padata)->len != 0)
goto fail;
for((&(to)->padata)->len = 0; (&(to)->padata)->len < (&(from)->padata)->len; (&(to)->padata)->len++){
if(copy_PA_DATA(&(&(from)->padata)->val[(&(to)->padata)->len], &(&(to)->padata)->val[(&(to)->padata)->len])) goto fail;
}
if((from)->rep_key) {
(to)->rep_key = malloc(sizeof(*(to)->rep_key));
if((to)->rep_key == NULL) goto fail;
if(copy_EncryptionKey((from)->rep_key, (to)->rep_key)) goto fail;
}else
(to)->rep_key = NULL;
if((from)->finished) {
(to)->finished = malloc(sizeof(*(to)->finished));
if((to)->finished == NULL) goto fail;
if(copy_KrbFastFinished((from)->finished, (to)->finished)) goto fail;
}else
(to)->finished = NULL;
return 0;
fail:
free_KrbFastResponse(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbFastArmoredRep(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbFastArmoredRep *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* enc-fast-rep */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptedData(p, len, &(data)->enc_fast_rep, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

*size = ret;
return 0;
}

int ASN1CALL
decode_KrbFastArmoredRep(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbFastArmoredRep *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
{
size_t Top_datalen, Top_oldlen;
Der_type Top_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &Top_type, UT_Sequence, &Top_datalen, &l);
if (e == 0 && Top_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
Top_oldlen = len;
if (Top_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = Top_datalen;
{
size_t enc_fast_rep_datalen, enc_fast_rep_oldlen;
Der_type enc_fast_rep_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &enc_fast_rep_type, 0, &enc_fast_rep_datalen, &l);
if (e == 0 && enc_fast_rep_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
enc_fast_rep_oldlen = len;
if (enc_fast_rep_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = enc_fast_rep_datalen;
e = decode_EncryptedData(p, len, &(data)->enc_fast_rep, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = enc_fast_rep_oldlen - enc_fast_rep_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbFastArmoredRep(data);
return e;
}

void ASN1CALL
free_KrbFastArmoredRep(KrbFastArmoredRep *data)
{
free_EncryptedData(&(data)->enc_fast_rep);
}

size_t ASN1CALL
length_KrbFastArmoredRep(const KrbFastArmoredRep *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptedData(&(data)->enc_fast_rep);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbFastArmoredRep(const KrbFastArmoredRep *from, KrbFastArmoredRep *to)
{
memset(to, 0, sizeof(*to));
if(copy_EncryptedData(&(from)->enc_fast_rep, &(to)->enc_fast_rep)) goto fail;
return 0;
fail:
free_KrbFastArmoredRep(to);
return ENOMEM;
}

int ASN1CALL
encode_PA_FX_FAST_REPLY(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const PA_FX_FAST_REPLY *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;


switch((data)->element) {
case choice_PA_FX_FAST_REPLY_armored_data: {size_t Top_oldret = ret;
ret = 0;
e = encode_KrbFastArmoredRep(p, len, &((data))->u.armored_data, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 0, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_oldret;
break;
}
case choice_PA_FX_FAST_REPLY_asn1_ellipsis: {
if (len < (data)->u.asn1_ellipsis.length)
return ASN1_OVERFLOW;
p -= (data)->u.asn1_ellipsis.length;
ret += (data)->u.asn1_ellipsis.length;
memcpy(p + 1, (data)->u.asn1_ellipsis.data, (data)->u.asn1_ellipsis.length);
break;
}
};
*size = ret;
return 0;
}

int ASN1CALL
decode_PA_FX_FAST_REPLY(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, PA_FX_FAST_REPLY *data, size_t *size)
{
size_t ret = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int e HEIMDAL_UNUSED_ATTRIBUTE;

memset(data, 0, sizeof(*data));
if (der_match_tag(p, len, ASN1_C_CONTEXT, CONS, 0, NULL) == 0) {
{
size_t armored_data_datalen, armored_data_oldlen;
Der_type armored_data_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &armored_data_type, 0, &armored_data_datalen, &l);
if (e == 0 && armored_data_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
armored_data_oldlen = len;
if (armored_data_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = armored_data_datalen;
e = decode_KrbFastArmoredRep(p, len, &(data)->u.armored_data, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = armored_data_oldlen - armored_data_datalen;
}
(data)->element = choice_PA_FX_FAST_REPLY_armored_data;
}
else {
(data)->u.asn1_ellipsis.data = calloc(1, len);
if ((data)->u.asn1_ellipsis.data == NULL) {
e = ENOMEM; goto fail;
}
(data)->u.asn1_ellipsis.length = len;
memcpy((data)->u.asn1_ellipsis.data, p, len);
(data)->element = choice_PA_FX_FAST_REPLY_asn1_ellipsis;
p += len;
ret += len;
len = 0;
}
if(size) *size = ret;
return 0;
fail:
free_PA_FX_FAST_REPLY(data);
return e;
}

void ASN1CALL
free_PA_FX_FAST_REPLY(PA_FX_FAST_REPLY *data)
{
switch((data)->element) {
case choice_PA_FX_FAST_REPLY_armored_data:
free_KrbFastArmoredRep(&(data)->u.armored_data);
break;
case choice_PA_FX_FAST_REPLY_asn1_ellipsis:
der_free_octet_string(&(data)->u.asn1_ellipsis);
break;}
}

size_t ASN1CALL
length_PA_FX_FAST_REPLY(const PA_FX_FAST_REPLY *data)
{
size_t ret = 0;
switch((data)->element) {
case choice_PA_FX_FAST_REPLY_armored_data:
{
size_t Top_oldret = ret;
ret = 0;
ret += length_KrbFastArmoredRep(&(data)->u.armored_data);
ret += 1 + der_length_len (ret);
ret += Top_oldret;
}
break;
case choice_PA_FX_FAST_REPLY_asn1_ellipsis:
ret += (data)->u.asn1_ellipsis.length;
break;
}
return ret;
}

int ASN1CALL
copy_PA_FX_FAST_REPLY(const PA_FX_FAST_REPLY *from, PA_FX_FAST_REPLY *to)
{
memset(to, 0, sizeof(*to));
(to)->element = (from)->element;
switch((from)->element) {
case choice_PA_FX_FAST_REPLY_armored_data:
if(copy_KrbFastArmoredRep(&(from)->u.armored_data, &(to)->u.armored_data)) goto fail;
break;
case choice_PA_FX_FAST_REPLY_asn1_ellipsis: {
int ret;
ret=der_copy_octet_string(&(from)->u.asn1_ellipsis, &(to)->u.asn1_ellipsis);
if (ret) goto fail;
break;
}
}
return 0;
fail:
free_PA_FX_FAST_REPLY(to);
return ENOMEM;
}

