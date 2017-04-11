int    ASN1CALL decode_MESSAGE_TYPE(const unsigned char *, size_t, MESSAGE_TYPE *, size_t *);
int    ASN1CALL encode_MESSAGE_TYPE(unsigned char *, size_t, const MESSAGE_TYPE *, size_t *);
size_t ASN1CALL length_MESSAGE_TYPE(const MESSAGE_TYPE *);
int    ASN1CALL copy_MESSAGE_TYPE  (const MESSAGE_TYPE *, MESSAGE_TYPE *);
void   ASN1CALL free_MESSAGE_TYPE  (MESSAGE_TYPE *);


int    ASN1CALL decode_krb5uint32(const unsigned char *, size_t, krb5uint32 *, size_t *);
int    ASN1CALL encode_krb5uint32(unsigned char *, size_t, const krb5uint32 *, size_t *);
size_t ASN1CALL length_krb5uint32(const krb5uint32 *);
int    ASN1CALL copy_krb5uint32  (const krb5uint32 *, krb5uint32 *);
void   ASN1CALL free_krb5uint32  (krb5uint32 *);


int    ASN1CALL decode_krb5int32(const unsigned char *, size_t, krb5int32 *, size_t *);
int    ASN1CALL encode_krb5int32(unsigned char *, size_t, const krb5int32 *, size_t *);
size_t ASN1CALL length_krb5int32(const krb5int32 *);
int    ASN1CALL copy_krb5int32  (const krb5int32 *, krb5int32 *);
void   ASN1CALL free_krb5int32  (krb5int32 *);


int    ASN1CALL decode_APOptions(const unsigned char *, size_t, APOptions *, size_t *);
int    ASN1CALL encode_APOptions(unsigned char *, size_t, const APOptions *, size_t *);
size_t ASN1CALL length_APOptions(const APOptions *);
int    ASN1CALL copy_APOptions  (const APOptions *, APOptions *);
void   ASN1CALL free_APOptions  (APOptions *);


int    ASN1CALL decode_TYPED_DATA(const unsigned char *, size_t, TYPED_DATA *, size_t *);
int    ASN1CALL encode_TYPED_DATA(unsigned char *, size_t, const TYPED_DATA *, size_t *);
size_t ASN1CALL length_TYPED_DATA(const TYPED_DATA *);
int    ASN1CALL copy_TYPED_DATA  (const TYPED_DATA *, TYPED_DATA *);
void   ASN1CALL free_TYPED_DATA  (TYPED_DATA *);


int    ASN1CALL decode_KDC_REQ(const unsigned char *, size_t, KDC_REQ *, size_t *);
int    ASN1CALL encode_KDC_REQ(unsigned char *, size_t, const KDC_REQ *, size_t *);
size_t ASN1CALL length_KDC_REQ(const KDC_REQ *);
int    ASN1CALL copy_KDC_REQ  (const KDC_REQ *, KDC_REQ *);
void   ASN1CALL free_KDC_REQ  (KDC_REQ *);


int    ASN1CALL decode_PROV_SRV_LOCATION(const unsigned char *, size_t, PROV_SRV_LOCATION *, size_t *);
int    ASN1CALL encode_PROV_SRV_LOCATION(unsigned char *, size_t, const PROV_SRV_LOCATION *, size_t *);
size_t ASN1CALL length_PROV_SRV_LOCATION(const PROV_SRV_LOCATION *);
int    ASN1CALL copy_PROV_SRV_LOCATION  (const PROV_SRV_LOCATION *, PROV_SRV_LOCATION *);
void   ASN1CALL free_PROV_SRV_LOCATION  (PROV_SRV_LOCATION *);


int    ASN1CALL decode_AD_MANDATORY_FOR_KDC(const unsigned char *, size_t, AD_MANDATORY_FOR_KDC *, size_t *);
int    ASN1CALL encode_AD_MANDATORY_FOR_KDC(unsigned char *, size_t, const AD_MANDATORY_FOR_KDC *, size_t *);
size_t ASN1CALL length_AD_MANDATORY_FOR_KDC(const AD_MANDATORY_FOR_KDC *);
int    ASN1CALL copy_AD_MANDATORY_FOR_KDC  (const AD_MANDATORY_FOR_KDC *, AD_MANDATORY_FOR_KDC *);
void   ASN1CALL free_AD_MANDATORY_FOR_KDC  (AD_MANDATORY_FOR_KDC *);


int    ASN1CALL decode_PA_SAM_TYPE(const unsigned char *, size_t, PA_SAM_TYPE *, size_t *);
int    ASN1CALL encode_PA_SAM_TYPE(unsigned char *, size_t, const PA_SAM_TYPE *, size_t *);
size_t ASN1CALL length_PA_SAM_TYPE(const PA_SAM_TYPE *);
int    ASN1CALL copy_PA_SAM_TYPE  (const PA_SAM_TYPE *, PA_SAM_TYPE *);
void   ASN1CALL free_PA_SAM_TYPE  (PA_SAM_TYPE *);


int    ASN1CALL decode_PA_SAM_REDIRECT(const unsigned char *, size_t, PA_SAM_REDIRECT *, size_t *);
int    ASN1CALL encode_PA_SAM_REDIRECT(unsigned char *, size_t, const PA_SAM_REDIRECT *, size_t *);
size_t ASN1CALL length_PA_SAM_REDIRECT(const PA_SAM_REDIRECT *);
int    ASN1CALL copy_PA_SAM_REDIRECT  (const PA_SAM_REDIRECT *, PA_SAM_REDIRECT *);
void   ASN1CALL free_PA_SAM_REDIRECT  (PA_SAM_REDIRECT *);


int    ASN1CALL decode_SAMFlags(const unsigned char *, size_t, SAMFlags *, size_t *);
int    ASN1CALL encode_SAMFlags(unsigned char *, size_t, const SAMFlags *, size_t *);
size_t ASN1CALL length_SAMFlags(const SAMFlags *);
int    ASN1CALL copy_SAMFlags  (const SAMFlags *, SAMFlags *);
void   ASN1CALL free_SAMFlags  (SAMFlags *);


int    ASN1CALL decode_PA_SAM_CHALLENGE_2_BODY(const unsigned char *, size_t, PA_SAM_CHALLENGE_2_BODY *, size_t *);
int    ASN1CALL encode_PA_SAM_CHALLENGE_2_BODY(unsigned char *, size_t, const PA_SAM_CHALLENGE_2_BODY *, size_t *);
size_t ASN1CALL length_PA_SAM_CHALLENGE_2_BODY(const PA_SAM_CHALLENGE_2_BODY *);
int    ASN1CALL copy_PA_SAM_CHALLENGE_2_BODY  (const PA_SAM_CHALLENGE_2_BODY *, PA_SAM_CHALLENGE_2_BODY *);
void   ASN1CALL free_PA_SAM_CHALLENGE_2_BODY  (PA_SAM_CHALLENGE_2_BODY *);


int    ASN1CALL decode_PA_SAM_CHALLENGE_2(const unsigned char *, size_t, PA_SAM_CHALLENGE_2 *, size_t *);
int    ASN1CALL encode_PA_SAM_CHALLENGE_2(unsigned char *, size_t, const PA_SAM_CHALLENGE_2 *, size_t *);
size_t ASN1CALL length_PA_SAM_CHALLENGE_2(const PA_SAM_CHALLENGE_2 *);
int    ASN1CALL copy_PA_SAM_CHALLENGE_2  (const PA_SAM_CHALLENGE_2 *, PA_SAM_CHALLENGE_2 *);
void   ASN1CALL free_PA_SAM_CHALLENGE_2  (PA_SAM_CHALLENGE_2 *);


int    ASN1CALL decode_PA_SAM_RESPONSE_2(const unsigned char *, size_t, PA_SAM_RESPONSE_2 *, size_t *);
int    ASN1CALL encode_PA_SAM_RESPONSE_2(unsigned char *, size_t, const PA_SAM_RESPONSE_2 *, size_t *);
size_t ASN1CALL length_PA_SAM_RESPONSE_2(const PA_SAM_RESPONSE_2 *);
int    ASN1CALL copy_PA_SAM_RESPONSE_2  (const PA_SAM_RESPONSE_2 *, PA_SAM_RESPONSE_2 *);
void   ASN1CALL free_PA_SAM_RESPONSE_2  (PA_SAM_RESPONSE_2 *);


int    ASN1CALL decode_PA_ENC_SAM_RESPONSE_ENC(const unsigned char *, size_t, PA_ENC_SAM_RESPONSE_ENC *, size_t *);
int    ASN1CALL encode_PA_ENC_SAM_RESPONSE_ENC(unsigned char *, size_t, const PA_ENC_SAM_RESPONSE_ENC *, size_t *);
size_t ASN1CALL length_PA_ENC_SAM_RESPONSE_ENC(const PA_ENC_SAM_RESPONSE_ENC *);
int    ASN1CALL copy_PA_ENC_SAM_RESPONSE_ENC  (const PA_ENC_SAM_RESPONSE_ENC *, PA_ENC_SAM_RESPONSE_ENC *);
void   ASN1CALL free_PA_ENC_SAM_RESPONSE_ENC  (PA_ENC_SAM_RESPONSE_ENC *);


int    ASN1CALL decode_FastOptions(const unsigned char *, size_t, FastOptions *, size_t *);
int    ASN1CALL encode_FastOptions(unsigned char *, size_t, const FastOptions *, size_t *);
size_t ASN1CALL length_FastOptions(const FastOptions *);
int    ASN1CALL copy_FastOptions  (const FastOptions *, FastOptions *);
void   ASN1CALL free_FastOptions  (FastOptions *);


int    ASN1CALL decode_KrbFastReq(const unsigned char *, size_t, KrbFastReq *, size_t *);
int    ASN1CALL encode_KrbFastReq(unsigned char *, size_t, const KrbFastReq *, size_t *);
size_t ASN1CALL length_KrbFastReq(const KrbFastReq *);
int    ASN1CALL copy_KrbFastReq  (const KrbFastReq *, KrbFastReq *);
void   ASN1CALL free_KrbFastReq  (KrbFastReq *);


int    ASN1CALL decode_KrbFastArmor(const unsigned char *, size_t, KrbFastArmor *, size_t *);
int    ASN1CALL encode_KrbFastArmor(unsigned char *, size_t, const KrbFastArmor *, size_t *);
size_t ASN1CALL length_KrbFastArmor(const KrbFastArmor *);
int    ASN1CALL copy_KrbFastArmor  (const KrbFastArmor *, KrbFastArmor *);
void   ASN1CALL free_KrbFastArmor  (KrbFastArmor *);


int    ASN1CALL decode_KrbFastArmoredReq(const unsigned char *, size_t, KrbFastArmoredReq *, size_t *);
int    ASN1CALL encode_KrbFastArmoredReq(unsigned char *, size_t, const KrbFastArmoredReq *, size_t *);
size_t ASN1CALL length_KrbFastArmoredReq(const KrbFastArmoredReq *);
int    ASN1CALL copy_KrbFastArmoredReq  (const KrbFastArmoredReq *, KrbFastArmoredReq *);
void   ASN1CALL free_KrbFastArmoredReq  (KrbFastArmoredReq *);


int    ASN1CALL decode_PA_FX_FAST_REQUEST(const unsigned char *, size_t, PA_FX_FAST_REQUEST *, size_t *);
int    ASN1CALL encode_PA_FX_FAST_REQUEST(unsigned char *, size_t, const PA_FX_FAST_REQUEST *, size_t *);
size_t ASN1CALL length_PA_FX_FAST_REQUEST(const PA_FX_FAST_REQUEST *);
int    ASN1CALL copy_PA_FX_FAST_REQUEST  (const PA_FX_FAST_REQUEST *, PA_FX_FAST_REQUEST *);
void   ASN1CALL free_PA_FX_FAST_REQUEST  (PA_FX_FAST_REQUEST *);


int    ASN1CALL decode_KrbFastFinished(const unsigned char *, size_t, KrbFastFinished *, size_t *);
int    ASN1CALL encode_KrbFastFinished(unsigned char *, size_t, const KrbFastFinished *, size_t *);
size_t ASN1CALL length_KrbFastFinished(const KrbFastFinished *);
int    ASN1CALL copy_KrbFastFinished  (const KrbFastFinished *, KrbFastFinished *);
void   ASN1CALL free_KrbFastFinished  (KrbFastFinished *);


int    ASN1CALL decode_KrbFastResponse(const unsigned char *, size_t, KrbFastResponse *, size_t *);
int    ASN1CALL encode_KrbFastResponse(unsigned char *, size_t, const KrbFastResponse *, size_t *);
size_t ASN1CALL length_KrbFastResponse(const KrbFastResponse *);
int    ASN1CALL copy_KrbFastResponse  (const KrbFastResponse *, KrbFastResponse *);
void   ASN1CALL free_KrbFastResponse  (KrbFastResponse *);


int    ASN1CALL decode_KrbFastArmoredRep(const unsigned char *, size_t, KrbFastArmoredRep *, size_t *);
int    ASN1CALL encode_KrbFastArmoredRep(unsigned char *, size_t, const KrbFastArmoredRep *, size_t *);
size_t ASN1CALL length_KrbFastArmoredRep(const KrbFastArmoredRep *);
int    ASN1CALL copy_KrbFastArmoredRep  (const KrbFastArmoredRep *, KrbFastArmoredRep *);
void   ASN1CALL free_KrbFastArmoredRep  (KrbFastArmoredRep *);


int    ASN1CALL decode_PA_FX_FAST_REPLY(const unsigned char *, size_t, PA_FX_FAST_REPLY *, size_t *);
int    ASN1CALL encode_PA_FX_FAST_REPLY(unsigned char *, size_t, const PA_FX_FAST_REPLY *, size_t *);
size_t ASN1CALL length_PA_FX_FAST_REPLY(const PA_FX_FAST_REPLY *);
int    ASN1CALL copy_PA_FX_FAST_REPLY  (const PA_FX_FAST_REPLY *, PA_FX_FAST_REPLY *);
void   ASN1CALL free_PA_FX_FAST_REPLY  (PA_FX_FAST_REPLY *);


