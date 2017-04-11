/* Generated from kerberos-db-key.asn1 */
/* Do not edit */

#ifndef __kerberos_db_h__
#define __kerberos_db_h__

#include <stddef.h>
#include <time.h>

#ifndef __asn1_common_definitions__
#define __asn1_common_definitions__

typedef struct heim_integer {
  size_t length;
  void *data;
  int negative;
} heim_integer;

typedef struct heim_octet_string {
  size_t length;
  void *data;
} heim_octet_string;

typedef char *heim_general_string;

typedef char *heim_utf8_string;

typedef struct heim_octet_string heim_printable_string;

typedef struct heim_octet_string heim_ia5_string;

typedef struct heim_bmp_string {
  size_t length;
  uint16_t *data;
} heim_bmp_string;

typedef struct heim_universal_string {
  size_t length;
  uint32_t *data;
} heim_universal_string;

typedef char *heim_visible_string;

typedef struct heim_oid {
  size_t length;
  unsigned *components;
} heim_oid;

typedef struct heim_bit_string {
  size_t length;
  void *data;
} heim_bit_string;

typedef struct heim_octet_string heim_any;
typedef struct heim_octet_string heim_any_set;

#define ASN1_MALLOC_ENCODE(T, B, BL, S, L, R)                  \
  do {                                                         \
    (BL) = length_##T((S));                                    \
    (B) = malloc((BL));                                        \
    if((B) == NULL) {                                          \
      (R) = ENOMEM;                                            \
    } else {                                                   \
      (R) = encode_##T(((unsigned char*)(B)) + (BL) - 1, (BL), \
                       (S), (L));                              \
      if((R) != 0) {                                           \
        free((B));                                             \
        (B) = NULL;                                            \
      }                                                        \
    }                                                          \
  } while (0)

#ifdef _WIN32
#ifndef ASN1_LIB
#define ASN1EXP  __declspec(dllimport)
#else
#define ASN1EXP
#endif
#define ASN1CALL __stdcall
#else
#define ASN1EXP
#define ASN1CALL
#endif
struct units;

#endif

/*
UInt16 ::= INTEGER (0..2147483647)
*/

typedef unsigned int UInt16;

/*
UInt32 ::= INTEGER (0..-1)
*/

typedef unsigned int UInt32;

/*
Int32 ::= INTEGER (-2147483648..2147483647)
*/

typedef int Int32;

/*
EncryptionKeyLdapDb ::= SEQUENCE {
  keytype         [0] Int32,
  keyvalue        [1] OCTET STRING,
}
*/

typedef struct EncryptionKeyLdapDb {
  Int32 keytype;
  heim_octet_string keyvalue;
} EncryptionKeyLdapDb;

ASN1EXP int    ASN1CALL decode_EncryptionKeyLdapDb(const unsigned char *, size_t, EncryptionKeyLdapDb *, size_t *);
ASN1EXP int    ASN1CALL encode_EncryptionKeyLdapDb(unsigned char *, size_t, const EncryptionKeyLdapDb *, size_t *);
ASN1EXP size_t ASN1CALL length_EncryptionKeyLdapDb(const EncryptionKeyLdapDb *);
ASN1EXP int    ASN1CALL copy_EncryptionKeyLdapDb  (const EncryptionKeyLdapDb *, EncryptionKeyLdapDb *);
ASN1EXP void   ASN1CALL free_EncryptionKeyLdapDb  (EncryptionKeyLdapDb *);


/*
KrbSalt ::= SEQUENCE {
  type            [0] Int32,
  salt            [1] OCTET STRING OPTIONAL,
}
*/

typedef struct KrbSalt {
  Int32 type;
  heim_octet_string *salt;
} KrbSalt;

ASN1EXP int    ASN1CALL decode_KrbSalt(const unsigned char *, size_t, KrbSalt *, size_t *);
ASN1EXP int    ASN1CALL encode_KrbSalt(unsigned char *, size_t, const KrbSalt *, size_t *);
ASN1EXP size_t ASN1CALL length_KrbSalt(const KrbSalt *);
ASN1EXP int    ASN1CALL copy_KrbSalt  (const KrbSalt *, KrbSalt *);
ASN1EXP void   ASN1CALL free_KrbSalt  (KrbSalt *);


/*
KrbKey ::= SEQUENCE {
  salt            [0] KrbSalt OPTIONAL,
  key             [1] EncryptionKeyLdapDb,
  s2kparams       [2] OCTET STRING OPTIONAL,
  ...,
}
*/

typedef struct KrbKey {
  KrbSalt *salt;
  EncryptionKeyLdapDb key;
  heim_octet_string *s2kparams;
} KrbKey;

ASN1EXP int    ASN1CALL decode_KrbKey(const unsigned char *, size_t, KrbKey *, size_t *);
ASN1EXP int    ASN1CALL encode_KrbKey(unsigned char *, size_t, const KrbKey *, size_t *);
ASN1EXP size_t ASN1CALL length_KrbKey(const KrbKey *);
ASN1EXP int    ASN1CALL copy_KrbKey  (const KrbKey *, KrbKey *);
ASN1EXP void   ASN1CALL free_KrbKey  (KrbKey *);


/*
KrbKeySet ::= SEQUENCE {
  attribute-major-vno   [0] UInt16,
  attribute-minor-vno   [1] UInt16,
  kvno                  [2] UInt32,
  mkvno                 [3] UInt32 OPTIONAL,
  keys                  [4] SEQUENCE OF KrbKey,
  ...,
}
*/

typedef struct KrbKeySet {
  UInt16 attribute_major_vno;
  UInt16 attribute_minor_vno;
  UInt32 kvno;
  UInt32 *mkvno;
  struct KrbKeySet_keys {
    unsigned int len;
    KrbKey *val;
  } keys;
} KrbKeySet;

ASN1EXP int    ASN1CALL decode_KrbKeySet(const unsigned char *, size_t, KrbKeySet *, size_t *);
ASN1EXP int    ASN1CALL encode_KrbKeySet(unsigned char *, size_t, const KrbKeySet *, size_t *);
ASN1EXP size_t ASN1CALL length_KrbKeySet(const KrbKeySet *);
ASN1EXP int    ASN1CALL copy_KrbKeySet  (const KrbKeySet *, KrbKeySet *);
ASN1EXP void   ASN1CALL free_KrbKeySet  (KrbKeySet *);


/*
MasterKey ::= SEQUENCE {
  keytype         [0] Int32,
  keyvalue        [1] OCTET STRING,
}
*/

typedef struct MasterKey {
  Int32 keytype;
  heim_octet_string keyvalue;
} MasterKey;

ASN1EXP int    ASN1CALL decode_MasterKey(const unsigned char *, size_t, MasterKey *, size_t *);
ASN1EXP int    ASN1CALL encode_MasterKey(unsigned char *, size_t, const MasterKey *, size_t *);
ASN1EXP size_t ASN1CALL length_MasterKey(const MasterKey *);
ASN1EXP int    ASN1CALL copy_MasterKey  (const MasterKey *, MasterKey *);
ASN1EXP void   ASN1CALL free_MasterKey  (MasterKey *);


/*
KrbMKey ::= SEQUENCE {
  kvno            [0] UInt32,
  key             [1] MasterKey,
}
*/

typedef struct KrbMKey {
  UInt32 kvno;
  MasterKey key;
} KrbMKey;

ASN1EXP int    ASN1CALL decode_KrbMKey(const unsigned char *, size_t, KrbMKey *, size_t *);
ASN1EXP int    ASN1CALL encode_KrbMKey(unsigned char *, size_t, const KrbMKey *, size_t *);
ASN1EXP size_t ASN1CALL length_KrbMKey(const KrbMKey *);
ASN1EXP int    ASN1CALL copy_KrbMKey  (const KrbMKey *, KrbMKey *);
ASN1EXP void   ASN1CALL free_KrbMKey  (KrbMKey *);


#endif /* __kerberos_db_h__ */
