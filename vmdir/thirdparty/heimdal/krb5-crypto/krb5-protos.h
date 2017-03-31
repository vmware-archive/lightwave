#ifndef _KRB5_PROTOS
#define _KRB5_PROTOS

struct _krb5_key_data;
struct _krb5_key_type;

typedef ssize_t krb5_ssize_t;
#ifndef KRB5_DEPRECATED_FUNCTION
#if defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1 )))
#define KRB5_DEPRECATED_FUNCTION(X) __attribute__((__deprecated__))
#else
#define KRB5_DEPRECATED_FUNCTION(X)
#endif
#endif


#ifndef KRB5_LIB
#ifndef KRB5_LIB_FUNCTION
#if defined(_WIN32)
#define KRB5_LIB_FUNCTION __declspec(dllexport)
#define KRB5_LIB_CALL __stdcall
#define KRB5_LIB_VARIABLE __declspec(dllexport)
#else
#define KRB5_LIB_FUNCTION
#define KRB5_LIB_CALL
#define KRB5_LIB_VARIABLE
#endif
#endif
#endif


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_init_context(krb5_context *context);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_free_context(krb5_context context);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_crypto_init(krb5_context context,
                 const krb5_keyblock *key,
                 krb5_enctype etype,
                 krb5_crypto *crypto);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_crypto_destroy(krb5_context context,
                    krb5_crypto crypto);



KRB5_LIB_FUNCTION const char* KRB5_LIB_CALL
krb5_config_get_string (krb5_context context,
                        const krb5_config_section *c,
                        ...);

void
_krb5_evp_schedule(krb5_context context,
                   struct _krb5_key_type *kt,
                   struct _krb5_key_data *kd);

void
_krb5_evp_cleanup(krb5_context context, struct _krb5_key_data *kd);

krb5_error_code
_krb5_SP_HMAC_SHA1_checksum(krb5_context context,
                            struct _krb5_key_data *key,
                            const void *data,
                            size_t len,
                            unsigned usage,
                            Checksum *result);

krb5_error_code
_krb5_evp_encrypt_cts(krb5_context context,
                      struct _krb5_key_data *key,
                      void *data,
                      size_t len,
                      krb5_boolean encryptp,
                      int usage,
                      void *ivec);


krb5_error_code
_krb5_HMAC_MD5_checksum(krb5_context context,
                        struct _krb5_key_data *key,
                        const void *data,
                        size_t len,
                        unsigned usage,
                        Checksum *result);

struct _krb5_checksum_type *
_krb5_find_checksum(krb5_cksumtype type);

struct _krb5_encryption_type *
_krb5_find_enctype(krb5_enctype type);

void
_krb5_free_key_data(krb5_context context, struct _krb5_key_data *key,
              struct _krb5_encryption_type *et);

void
_krb5_DES3_random_to_key(krb5_context context,
                         krb5_keyblock *key,
                         const void *data,
                         size_t size);


krb5_error_code
_krb5_evp_encrypt(krb5_context context,
                struct _krb5_key_data *key,
                void *data,
                size_t len,
                krb5_boolean encryptp,
                int usage,
                void *ivec);


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_vset_error_message (krb5_context context, krb5_error_code ret,
                         const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)));


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_vprepend_error_message(krb5_context context, krb5_error_code ret,
                            const char *fmt, va_list args)
    __attribute__ ((format (printf, 3, 0)));


KRB5_LIB_FUNCTION const char * KRB5_LIB_CALL
krb5_heim_get_error_message(krb5_context context, krb5_error_code code);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_encrypt(krb5_context context,
             krb5_crypto crypto,
             unsigned usage,
             const void *data,
             size_t len,
             krb5_data *result);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_decrypt(krb5_context context,
             krb5_crypto crypto,
             unsigned usage,
             void *data,
             size_t len,
             krb5_data *result);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_generate_random_keyblock(krb5_context context,
                              krb5_enctype type,
                              krb5_keyblock *key);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_data_free(krb5_data *p);



KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_data_alloc(krb5_data *p, int len);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_set_error_message(krb5_context context, krb5_error_code ret,
                       const char *fmt, ...);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_abortx(krb5_context context, const char *fmt, ...)
     __attribute__ ((noreturn, format (printf, 2, 3)));

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_derive_key(krb5_context context,
                const krb5_keyblock *key,
                krb5_enctype etype,
                const void *constant,
                size_t constant_len,
                krb5_keyblock **derived_key);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_free_keyblock_contents(krb5_context context,
                            krb5_keyblock *keyblock);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_keyblock(krb5_context context,
                   krb5_keyblock *keyblock);

/* HMAC according to RFC2104 */
krb5_error_code
_krb5_internal_hmac(krb5_context context,
                    struct _krb5_checksum_type *cm,
                    const void *data,
                    size_t len,
                    unsigned usage,
                    struct _krb5_key_data *keyblock,
                    Checksum *result);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_clear_error_message(krb5_context context);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_enctype_to_string(krb5_context context,
                       krb5_enctype etype,
                       char **string);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_generate_random_block(void *buf, size_t len);


KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_data(krb5_context context,
               krb5_data *p);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_keyblock (krb5_context context,
                    const krb5_keyblock *inblock,
                    krb5_keyblock **to);

krb5_error_code
_krb5_usage2arcfour(krb5_context context, unsigned *usage);


KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_data_ct_cmp(const krb5_data *data1, const krb5_data *data2);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_data_copy(krb5_data *p, const void *data, size_t len);

void
_krb5_xor (DES_cblock *key, const unsigned char *b);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
_krb5_n_fold(const void *str, size_t len, void *key, size_t size);

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
_krb5_put_int(void *buffer, unsigned long value, size_t size);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_data_zero(krb5_data *p);

void
_krb5_crc_init_table(void);

uint32_t
_krb5_crc_update (const char *p, size_t len, uint32_t res);

krb5_error_code
_krb5_des_checksum(krb5_context context,
                   const EVP_MD *evp_md,
                   struct _krb5_key_data *key,
                   const void *data,
                   size_t len,
                   Checksum *cksum);

krb5_error_code
_krb5_des_verify(krb5_context context,
                 const EVP_MD *evp_md,
                 struct _krb5_key_data *key,
                 const void *data,
                 size_t len,
                 Checksum *C);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_enctype_keysize(krb5_context context,
                     krb5_enctype type,
                     size_t *keysize);

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
_krb5_get_int(void *buffer, unsigned long *value, size_t size);

krb5_error_code
_krb5_derive_key(krb5_context context,
                 struct _krb5_encryption_type *et,
                 struct _krb5_key_data *key,
                 const void *constant,
                 size_t len);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_keyblock_contents (krb5_context context,
                             const krb5_keyblock *inblock,
                             krb5_keyblock *to);

int
wind_utf8ucs2_length(const char *in, size_t *out_len);

int
wind_utf8ucs2(const char *in, uint16_t *out, size_t *out_len);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_string_to_key_data_salt (krb5_context context,
                              krb5_enctype enctype,
                              krb5_data password,
                              krb5_salt salt,
                              krb5_keyblock *key);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_string_to_key_data_salt_opaque (krb5_context context,
                                     krb5_enctype enctype,
                                     krb5_data password,
                                     krb5_salt salt,
                                     krb5_data opaque,
                                     krb5_keyblock *key);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_string_to_key_derived(krb5_context context,
                           const void *str,
                           size_t len,
                           krb5_enctype etype,
                           krb5_keyblock *key);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_string_to_key (krb5_context context,
                    krb5_enctype enctype,
                    const char *password,
                    krb5_principal principal,
                    krb5_keyblock *key);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_set_warn_dest(krb5_context context, krb5_log_facility *fac);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_free_error_message(krb5_context context, const char *msg);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_log(krb5_context context,
         krb5_log_facility *fac,
         int level,
         const char *fmt,
         ...);

char *krb5_sprintf_alloc_buf(const char *fmt, ...); 
char *krb5_vsprintf_alloc_buf(const char *fmt, va_list); 

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_ret_uint16 (
        krb5_storage * /*sp*/,
        uint16_t * /*value*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_ret_uint32 (
        krb5_storage * /*sp*/,
        uint32_t * /*value*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_store_uint16 (
        krb5_storage * /*sp*/,
        uint16_t /*value*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_store_uint32 (
        krb5_storage * /*sp*/,
        uint32_t /*value*/);

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
krb5_storage_read (
        krb5_storage * /*sp*/,
        void * /*buf*/,
        size_t /*len*/);

KRB5_LIB_FUNCTION off_t KRB5_LIB_CALL
krb5_storage_seek (
        krb5_storage * /*sp*/,
        off_t /*offset*/,
        int /*whence*/);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_storage_set_byteorder (
        krb5_storage * /*sp*/,
        krb5_flags /*byteorder*/);

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_emem (void);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_store_int16 (
        krb5_storage * /*sp*/,
        int16_t /*value*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_storage_to_data (
        krb5_storage * /*sp*/,
        krb5_data * /*data*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_storage_free (krb5_storage * /*sp*/);

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_readonly_mem (
        const void * /*buf*/,
        size_t /*len*/);

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
krb5_storage_write (
        krb5_storage * /*sp*/,
        const void * /*buf*/,
        size_t /*len*/);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_storage_set_flags (
        krb5_storage * /*sp*/,
        krb5_flags /*flags*/);

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_mem (
        void * /*buf*/,
        size_t /*len*/);

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_checksum_is_keyed (
        krb5_context /*context*/,
        krb5_cksumtype /*type*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_crypto_init (
        krb5_context /*context*/,
        const krb5_keyblock * /*key*/,
        krb5_enctype /*etype*/,
        krb5_crypto * /*crypto*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_verify_checksum (
        krb5_context /*context*/,
        krb5_crypto /*crypto*/,
        krb5_key_usage /*usage*/,
        void * /*data*/,
        size_t /*len*/,
        Checksum * /*cksum*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_crypto_destroy (
        krb5_context /*context*/,
        krb5_crypto /*crypto*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_create_checksum (
        krb5_context /*context*/,
        krb5_crypto /*crypto*/,
        krb5_key_usage /*usage*/,
        int /*type*/,
        void * /*data*/,
        size_t /*len*/,
        Checksum * /*result*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_parse_name_flags (
        krb5_context /*context*/,
        const char * /*name*/,
        int /*flags*/,
        krb5_principal * /*principal*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_unparse_name_flags (
        krb5_context /*context*/,
        krb5_const_principal /*principal*/,
        int /*flags*/,
        char ** /*name*/);

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_principal_compare_any_realm (
        krb5_context /*context*/,
        krb5_const_principal /*princ1*/,
        krb5_const_principal /*princ2*/);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_principal (
        krb5_context /*context*/,
        krb5_principal /*p*/);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_data (
        krb5_context /*context*/,
        const krb5_data * /*indata*/,
        krb5_data ** /*outdata*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_crypto_get_checksum_type (
        krb5_context /*context*/,
        krb5_crypto /*crypto*/,
        krb5_cksumtype * /*type*/);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_checksumsize (
        krb5_context /*context*/,
        krb5_cksumtype /*type*/,
        size_t * /*size*/);

/* pac.c */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_parse(krb5_context context, const void *ptr, size_t len,
               krb5_pac *pac);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_init(krb5_context context, krb5_pac *pac);


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_add_buffer(krb5_context context, krb5_pac p,
                    uint32_t type, const krb5_data *data);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_get_buffer(krb5_context context, krb5_pac p,
                    uint32_t type, krb5_data *data);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_get_types(krb5_context context,
                   krb5_pac p,
                   size_t *len,
                   uint32_t **types);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_heim_pac_free(krb5_context context, krb5_pac pac);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_heim_pac_verify(krb5_context context,
                const krb5_pac pac,
                time_t authtime,
                krb5_const_principal principal,
                const krb5_keyblock *server,
                const krb5_keyblock *privsvr);

krb5_error_code
_krb5_heim_pac_sign(krb5_context context,
               krb5_pac p,
               time_t authtime,
               krb5_principal principal,
               const krb5_keyblock *server_key,
               const krb5_keyblock *priv_key,
               krb5_data *data);

/* principal.c */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_principal(krb5_context context,
                    krb5_principal p);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_parse_name_flags(krb5_context context,
                      const char *name,
                      int flags,
                      krb5_principal *principal);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_unparse_name_flags(krb5_context context,
                        krb5_const_principal principal,
                        int flags,
                        char **name);

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_principal_compare_any_realm(krb5_context context,
                                 krb5_const_principal princ1,
                                 krb5_const_principal princ2);

/* get_default_realm.c */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_default_realms (krb5_context context,
			 krb5_realm **realms);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_default_realm(krb5_context context,
		       krb5_realm *realm);

/* copy_host_realm.c */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_host_realm(krb5_context context,
		     const krb5_realm *from,
		     krb5_realm **to);

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_host_realm(krb5_context context,
                    const char *targethost,
                    krb5_realm **realms);

/* set_default_realm.c */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_set_default_realm(krb5_context context,
		       const char *realm);
/* free_host_realm.c */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_free_host_realm(krb5_context context,
		     krb5_realm *realmlist);

/* config_file.c */

KRB5_LIB_FUNCTION char** KRB5_LIB_CALL
krb5_config_get_strings(krb5_context context,
                        const krb5_config_section *c,
                        ...);

KRB5_LIB_FUNCTION krb5_boolean KRB5_LIB_CALL
krb5_config_get_bool_default (krb5_context context,
                              const krb5_config_section *c,
                              krb5_boolean def_value,
                              ...);

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_config_free_strings(char **strings);


#endif
