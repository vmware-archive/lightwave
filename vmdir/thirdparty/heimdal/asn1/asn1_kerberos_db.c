/* Generated from kerberos-db-key.asn1 */
/* Do not edit */

#define  ASN1_LIB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <krb5-types.h>
#include <kerberos_db.h>
#include <kerberos_db-priv.h>
#include <asn1_err.h>
#include <der.h>
#include <der-private.h>
#include <asn1-template.h>
#include <parse_units.h>

int ASN1CALL
encode_UInt16(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const UInt16 *data, size_t *size)
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
decode_UInt16(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, UInt16 *data, size_t *size)
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
free_UInt16(data);
return e;
}

void ASN1CALL
free_UInt16(UInt16 *data)
{
}

size_t ASN1CALL
length_UInt16(const UInt16 *data)
{
size_t ret = 0;
ret += der_length_unsigned(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_UInt16(const UInt16 *from, UInt16 *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_UInt32(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const UInt32 *data, size_t *size)
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
decode_UInt32(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, UInt32 *data, size_t *size)
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
free_UInt32(data);
return e;
}

void ASN1CALL
free_UInt32(UInt32 *data)
{
}

size_t ASN1CALL
length_UInt32(const UInt32 *data)
{
size_t ret = 0;
ret += der_length_unsigned(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_UInt32(const UInt32 *from, UInt32 *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_Int32(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const Int32 *data, size_t *size)
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
decode_Int32(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, Int32 *data, size_t *size)
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
free_Int32(data);
return e;
}

void ASN1CALL
free_Int32(Int32 *data)
{
}

size_t ASN1CALL
length_Int32(const Int32 *data)
{
size_t ret = 0;
ret += der_length_integer(data);
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_Int32(const Int32 *from, Int32 *to)
{
memset(to, 0, sizeof(*to));
*(to) = *(from);
return 0;
}

int ASN1CALL
encode_EncryptionKeyLdapDb(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const EncryptionKeyLdapDb *data, size_t *size)
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
e = encode_Int32(p, len, &(data)->keytype, &l);
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
decode_EncryptionKeyLdapDb(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, EncryptionKeyLdapDb *data, size_t *size)
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
e = decode_Int32(p, len, &(data)->keytype, &l);
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
free_EncryptionKeyLdapDb(data);
return e;
}

void ASN1CALL
free_EncryptionKeyLdapDb(EncryptionKeyLdapDb *data)
{
free_Int32(&(data)->keytype);
der_free_octet_string(&(data)->keyvalue);
}

size_t ASN1CALL
length_EncryptionKeyLdapDb(const EncryptionKeyLdapDb *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Int32(&(data)->keytype);
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
copy_EncryptionKeyLdapDb(const EncryptionKeyLdapDb *from, EncryptionKeyLdapDb *to)
{
memset(to, 0, sizeof(*to));
if(copy_Int32(&(from)->keytype, &(to)->keytype)) goto fail;
if(der_copy_octet_string(&(from)->keyvalue, &(to)->keyvalue)) goto fail;
return 0;
fail:
free_EncryptionKeyLdapDb(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbSalt(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbSalt *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

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
/* type */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_Int32(p, len, &(data)->type, &l);
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
decode_KrbSalt(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbSalt *data, size_t *size)
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
size_t type_datalen, type_oldlen;
Der_type type_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &type_type, 0, &type_datalen, &l);
if (e == 0 && type_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
type_oldlen = len;
if (type_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = type_datalen;
e = decode_Int32(p, len, &(data)->type, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = type_oldlen - type_datalen;
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
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbSalt(data);
return e;
}

void ASN1CALL
free_KrbSalt(KrbSalt *data)
{
free_Int32(&(data)->type);
if((data)->salt) {
der_free_octet_string((data)->salt);
free((data)->salt);
(data)->salt = NULL;
}
}

size_t ASN1CALL
length_KrbSalt(const KrbSalt *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Int32(&(data)->type);
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
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbSalt(const KrbSalt *from, KrbSalt *to)
{
memset(to, 0, sizeof(*to));
if(copy_Int32(&(from)->type, &(to)->type)) goto fail;
if((from)->salt) {
(to)->salt = malloc(sizeof(*(to)->salt));
if((to)->salt == NULL) goto fail;
if(der_copy_octet_string((from)->salt, (to)->salt)) goto fail;
}else
(to)->salt = NULL;
return 0;
fail:
free_KrbSalt(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbKey(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbKey *data, size_t *size)
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
/* key */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_EncryptionKeyLdapDb(p, len, &(data)->key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* salt */
if((data)->salt) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_KrbSalt(p, len, (data)->salt, &l);
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
decode_KrbKey(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbKey *data, size_t *size)
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
size_t salt_datalen, salt_oldlen;
Der_type salt_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &salt_type, 0, &salt_datalen, &l);
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
e = decode_KrbSalt(p, len, (data)->salt, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = salt_oldlen - salt_datalen;
}
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
e = decode_EncryptionKeyLdapDb(p, len, &(data)->key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_oldlen - key_datalen;
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
free_KrbKey(data);
return e;
}

void ASN1CALL
free_KrbKey(KrbKey *data)
{
if((data)->salt) {
free_KrbSalt((data)->salt);
free((data)->salt);
(data)->salt = NULL;
}
free_EncryptionKeyLdapDb(&(data)->key);
if((data)->s2kparams) {
der_free_octet_string((data)->s2kparams);
free((data)->s2kparams);
(data)->s2kparams = NULL;
}
}

size_t ASN1CALL
length_KrbKey(const KrbKey *data)
{
size_t ret = 0;
if((data)->salt){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_KrbSalt((data)->salt);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_EncryptionKeyLdapDb(&(data)->key);
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
copy_KrbKey(const KrbKey *from, KrbKey *to)
{
memset(to, 0, sizeof(*to));
if((from)->salt) {
(to)->salt = malloc(sizeof(*(to)->salt));
if((to)->salt == NULL) goto fail;
if(copy_KrbSalt((from)->salt, (to)->salt)) goto fail;
}else
(to)->salt = NULL;
if(copy_EncryptionKeyLdapDb(&(from)->key, &(to)->key)) goto fail;
if((from)->s2kparams) {
(to)->s2kparams = malloc(sizeof(*(to)->s2kparams));
if((to)->s2kparams == NULL) goto fail;
if(der_copy_octet_string((from)->s2kparams, (to)->s2kparams)) goto fail;
}else
(to)->s2kparams = NULL;
return 0;
fail:
free_KrbKey(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbKeySet(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbKeySet *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* keys */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
for(i = (int)(&(data)->keys)->len - 1; i >= 0; --i) {
size_t keys_tag_tag_for_oldret = ret;
ret = 0;
e = encode_KrbKey(p, len, &(&(data)->keys)->val[i], &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += keys_tag_tag_for_oldret;
}
e = der_put_length_and_tag (p, len, ret, ASN1_C_UNIV, CONS, UT_Sequence, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 4, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* mkvno */
if((data)->mkvno) {
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_UInt32(p, len, (data)->mkvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 3, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* kvno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_UInt32(p, len, &(data)->kvno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 2, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* attribute-minor-vno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_UInt16(p, len, &(data)->attribute_minor_vno, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* attribute-major-vno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_UInt16(p, len, &(data)->attribute_major_vno, &l);
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
decode_KrbKeySet(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbKeySet *data, size_t *size)
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
size_t attribute_major_vno_datalen, attribute_major_vno_oldlen;
Der_type attribute_major_vno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &attribute_major_vno_type, 0, &attribute_major_vno_datalen, &l);
if (e == 0 && attribute_major_vno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
attribute_major_vno_oldlen = len;
if (attribute_major_vno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = attribute_major_vno_datalen;
e = decode_UInt16(p, len, &(data)->attribute_major_vno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = attribute_major_vno_oldlen - attribute_major_vno_datalen;
}
{
size_t attribute_minor_vno_datalen, attribute_minor_vno_oldlen;
Der_type attribute_minor_vno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &attribute_minor_vno_type, 1, &attribute_minor_vno_datalen, &l);
if (e == 0 && attribute_minor_vno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
attribute_minor_vno_oldlen = len;
if (attribute_minor_vno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = attribute_minor_vno_datalen;
e = decode_UInt16(p, len, &(data)->attribute_minor_vno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = attribute_minor_vno_oldlen - attribute_minor_vno_datalen;
}
{
size_t kvno_datalen, kvno_oldlen;
Der_type kvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &kvno_type, 2, &kvno_datalen, &l);
if (e == 0 && kvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
kvno_oldlen = len;
if (kvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = kvno_datalen;
e = decode_UInt32(p, len, &(data)->kvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = kvno_oldlen - kvno_datalen;
}
{
size_t mkvno_datalen, mkvno_oldlen;
Der_type mkvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &mkvno_type, 3, &mkvno_datalen, &l);
if (e == 0 && mkvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) {
(data)->mkvno = NULL;
} else {
(data)->mkvno = calloc(1, sizeof(*(data)->mkvno));
if ((data)->mkvno == NULL) { e = ENOMEM; goto fail; }
p += l; len -= l; ret += l;
mkvno_oldlen = len;
if (mkvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = mkvno_datalen;
e = decode_UInt32(p, len, (data)->mkvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = mkvno_oldlen - mkvno_datalen;
}
}
{
size_t keys_datalen, keys_oldlen;
Der_type keys_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &keys_type, 4, &keys_datalen, &l);
if (e == 0 && keys_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
keys_oldlen = len;
if (keys_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = keys_datalen;
{
size_t keys_Tag_datalen, keys_Tag_oldlen;
Der_type keys_Tag_type;
e = der_match_tag_and_length(p, len, ASN1_C_UNIV, &keys_Tag_type, UT_Sequence, &keys_Tag_datalen, &l);
if (e == 0 && keys_Tag_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
keys_Tag_oldlen = len;
if (keys_Tag_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = keys_Tag_datalen;
{
size_t keys_Tag_Tag_origlen = len;
size_t keys_Tag_Tag_oldret = ret;
size_t keys_Tag_Tag_olen = 0;
void *keys_Tag_Tag_tmp;
ret = 0;
(&(data)->keys)->len = 0;
(&(data)->keys)->val = NULL;
while(ret < keys_Tag_Tag_origlen) {
size_t keys_Tag_Tag_nlen = keys_Tag_Tag_olen + sizeof(*((&(data)->keys)->val));
if (keys_Tag_Tag_olen > keys_Tag_Tag_nlen) { e = ASN1_OVERFLOW; goto fail; }
keys_Tag_Tag_olen = keys_Tag_Tag_nlen;
keys_Tag_Tag_tmp = realloc((&(data)->keys)->val, keys_Tag_Tag_olen);
if (keys_Tag_Tag_tmp == NULL) { e = ENOMEM; goto fail; }
(&(data)->keys)->val = keys_Tag_Tag_tmp;
e = decode_KrbKey(p, len, &(&(data)->keys)->val[(&(data)->keys)->len], &l);
if(e) goto fail;
p += l; len -= l; ret += l;
(&(data)->keys)->len++;
len = keys_Tag_Tag_origlen - ret;
}
ret += keys_Tag_Tag_oldret;
}
len = keys_Tag_oldlen - keys_Tag_datalen;
}
len = keys_oldlen - keys_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbKeySet(data);
return e;
}

void ASN1CALL
free_KrbKeySet(KrbKeySet *data)
{
free_UInt16(&(data)->attribute_major_vno);
free_UInt16(&(data)->attribute_minor_vno);
free_UInt32(&(data)->kvno);
if((data)->mkvno) {
free_UInt32((data)->mkvno);
free((data)->mkvno);
(data)->mkvno = NULL;
}
while((&(data)->keys)->len){
free_KrbKey(&(&(data)->keys)->val[(&(data)->keys)->len-1]);
(&(data)->keys)->len--;
}
free((&(data)->keys)->val);
(&(data)->keys)->val = NULL;
}

size_t ASN1CALL
length_KrbKeySet(const KrbKeySet *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_UInt16(&(data)->attribute_major_vno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_UInt16(&(data)->attribute_minor_vno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_UInt32(&(data)->kvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
if((data)->mkvno){
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_UInt32((data)->mkvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
{
size_t keys_tag_tag_oldret = ret;
int i;
ret = 0;
for(i = (&(data)->keys)->len - 1; i >= 0; --i){
size_t keys_tag_tag_for_oldret = ret;
ret = 0;
ret += length_KrbKey(&(&(data)->keys)->val[i]);
ret += keys_tag_tag_for_oldret;
}
ret += keys_tag_tag_oldret;
}
ret += 1 + der_length_len (ret);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbKeySet(const KrbKeySet *from, KrbKeySet *to)
{
memset(to, 0, sizeof(*to));
if(copy_UInt16(&(from)->attribute_major_vno, &(to)->attribute_major_vno)) goto fail;
if(copy_UInt16(&(from)->attribute_minor_vno, &(to)->attribute_minor_vno)) goto fail;
if(copy_UInt32(&(from)->kvno, &(to)->kvno)) goto fail;
if((from)->mkvno) {
(to)->mkvno = malloc(sizeof(*(to)->mkvno));
if((to)->mkvno == NULL) goto fail;
if(copy_UInt32((from)->mkvno, (to)->mkvno)) goto fail;
}else
(to)->mkvno = NULL;
if(((&(to)->keys)->val = malloc((&(from)->keys)->len * sizeof(*(&(to)->keys)->val))) == NULL && (&(from)->keys)->len != 0)
goto fail;
for((&(to)->keys)->len = 0; (&(to)->keys)->len < (&(from)->keys)->len; (&(to)->keys)->len++){
if(copy_KrbKey(&(&(from)->keys)->val[(&(to)->keys)->len], &(&(to)->keys)->val[(&(to)->keys)->len])) goto fail;
}
return 0;
fail:
free_KrbKeySet(to);
return ENOMEM;
}

int ASN1CALL
encode_MasterKey(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const MasterKey *data, size_t *size)
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
e = encode_Int32(p, len, &(data)->keytype, &l);
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
decode_MasterKey(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, MasterKey *data, size_t *size)
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
e = decode_Int32(p, len, &(data)->keytype, &l);
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
free_MasterKey(data);
return e;
}

void ASN1CALL
free_MasterKey(MasterKey *data)
{
free_Int32(&(data)->keytype);
der_free_octet_string(&(data)->keyvalue);
}

size_t ASN1CALL
length_MasterKey(const MasterKey *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_Int32(&(data)->keytype);
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
copy_MasterKey(const MasterKey *from, MasterKey *to)
{
memset(to, 0, sizeof(*to));
if(copy_Int32(&(from)->keytype, &(to)->keytype)) goto fail;
if(der_copy_octet_string(&(from)->keyvalue, &(to)->keyvalue)) goto fail;
return 0;
fail:
free_MasterKey(to);
return ENOMEM;
}

int ASN1CALL
encode_KrbMKey(unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, const KrbMKey *data, size_t *size)
{
size_t ret HEIMDAL_UNUSED_ATTRIBUTE = 0;
size_t l HEIMDAL_UNUSED_ATTRIBUTE;
int i HEIMDAL_UNUSED_ATTRIBUTE, e HEIMDAL_UNUSED_ATTRIBUTE;

/* key */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_MasterKey(p, len, &(data)->key, &l);
if (e) return e;
p -= l; len -= l; ret += l;

e = der_put_length_and_tag (p, len, ret, ASN1_C_CONTEXT, CONS, 1, &l);
if (e) return e;
p -= l; len -= l; ret += l;

ret += Top_tag_oldret;
}
/* kvno */
{
size_t Top_tag_oldret HEIMDAL_UNUSED_ATTRIBUTE = ret;
ret = 0;
e = encode_UInt32(p, len, &(data)->kvno, &l);
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
decode_KrbMKey(const unsigned char *p HEIMDAL_UNUSED_ATTRIBUTE, size_t len HEIMDAL_UNUSED_ATTRIBUTE, KrbMKey *data, size_t *size)
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
size_t kvno_datalen, kvno_oldlen;
Der_type kvno_type;
e = der_match_tag_and_length(p, len, ASN1_C_CONTEXT, &kvno_type, 0, &kvno_datalen, &l);
if (e == 0 && kvno_type != CONS) { e = ASN1_BAD_ID; }
if(e) goto fail;
p += l; len -= l; ret += l;
kvno_oldlen = len;
if (kvno_datalen > len) { e = ASN1_OVERRUN; goto fail; }
len = kvno_datalen;
e = decode_UInt32(p, len, &(data)->kvno, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = kvno_oldlen - kvno_datalen;
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
e = decode_MasterKey(p, len, &(data)->key, &l);
if(e) goto fail;
p += l; len -= l; ret += l;
len = key_oldlen - key_datalen;
}
len = Top_oldlen - Top_datalen;
}
if(size) *size = ret;
return 0;
fail:
free_KrbMKey(data);
return e;
}

void ASN1CALL
free_KrbMKey(KrbMKey *data)
{
free_UInt32(&(data)->kvno);
free_MasterKey(&(data)->key);
}

size_t ASN1CALL
length_KrbMKey(const KrbMKey *data)
{
size_t ret = 0;
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_UInt32(&(data)->kvno);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
{
size_t Top_tag_oldret = ret;
ret = 0;
ret += length_MasterKey(&(data)->key);
ret += 1 + der_length_len (ret);
ret += Top_tag_oldret;
}
ret += 1 + der_length_len (ret);
return ret;
}

int ASN1CALL
copy_KrbMKey(const KrbMKey *from, KrbMKey *to)
{
memset(to, 0, sizeof(*to));
if(copy_UInt32(&(from)->kvno, &(to)->kvno)) goto fail;
if(copy_MasterKey(&(from)->key, &(to)->key)) goto fail;
return 0;
fail:
free_KrbMKey(to);
return ENOMEM;
}

