package protocol

import (
	"bytes"
	"crypto"
	"crypto/rand"
	"crypto/sha256"
	"crypto/x509"
	"encoding/base64"
	"fmt"
	"strings"
	"time"

	"github.com/francoispqt/gojay"
	"github.com/vmware/lightwave/sts/internal/pkg/auth"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/config"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
	"github.com/vmware/lightwave/sts/internal/pkg/session"
	jose "gopkg.in/square/go-jose.v2"
)

type Issuer interface {
	auth.Validator

	IssueTokens(req OidcRequest, scope ScopeSet, nonce string, user types.UserID,
		code string, session session.SessionID, ctxt diag.RequestContext) (OidcTokenResponse, diag.Error)

	Signers(ctxt diag.RequestContext) (*jose.JSONWebKeySet, diag.Error)

	Issuer(ctxt diag.RequestContext) (string, diag.Error)
}

func NewTokenIssuer(sc config.StsConfig, auth idm.Authenticator, sr config.ServerRelativePathFunc, logger diag.Logger) (Issuer, diag.Error) {
	return &issuerImpl{sc: sc, auth: auth, sr: sr}, nil
}

type issuerImpl struct {
	sc   config.StsConfig
	auth idm.Authenticator
	sr   config.ServerRelativePathFunc
}

func (i *issuerImpl) Issuer(ctxt diag.RequestContext) (string, diag.Error) {
	return i.sc.Issuer(ctxt.Tenant(), i.sr, ctxt)
}

func (i *issuerImpl) IssueTokens(req OidcRequest, scope ScopeSet, nonce string, user types.UserID,
	code string, session session.SessionID, ctxt diag.RequestContext) (OidcTokenResponse, diag.Error) {

	var err diag.Error
	var at, it, rt, tokenType string
	logger := ctxt.Logger()

	tp, err := i.sc.TokenPolicy(ctxt.Tenant(), ctxt)
	if err != nil {
		return nil, err
	}

	signer, err := i.sc.SignerKey(ctxt.Tenant(), ctxt)
	if err != nil {
		return nil, err
	}
	cert, err := i.sc.SignerCert(ctxt.Tenant(), ctxt)
	if err != nil {
		return nil, err
	}
	kid := getIssuerKeyID(cert)

	bearerLifetimeSecs := int(tp.MaxBearerLifetime().Seconds())

	// todo: customize attributes

	var attrValues types.AttributeValues

	attrValues, err = i.auth.GetUserAttributes(ctxt.Tenant(), ctxt, user,
		types.AttributeIDUserIdentity, types.AttributeIDFirstName, types.AttributeIDLastName,
		types.AttributeIDGroupIdentities)
	if err != nil {
		return nil, err
	}
	var rts ResponseTypeSet

	authzReq, ok := req.(AuthzRequest)
	if ok {
		rts = authzReq.ResponseTypes()
	} else { // token request
		rts.add(ResponseTypeToken)
		if scope.Oidc() {
			rts.add(ResponseTypeIdToken)
		}
	}

	if rts.Contains(ResponseTypeToken) {
		logger.Tracef(diag.OIDC, "req.ResponseTypes() contains ResponseTypeToken = true")
		at, tokenType, err = i.buildAccessToken(scope, attrValues, tp, signer, kid, ctxt)
		if err != nil {
			return nil, err
		}
	}

	if rts.Contains(ResponseTypeIdToken) {
		it, err = i.buildIDToken(req, scope, nonce, session, attrValues, tp, signer, at, code, kid, ctxt)
		if err != nil {
			return nil, err
		}
	}

	// todo refresh token

	respCtxt, _ := req.(OidcResponseCtxt)
	// TODO: we should only return scopes which we honored
	return NewTokenResponse(
		at, tokenType, it, rt, code, bearerLifetimeSecs, scope, respCtxt), nil
}

func (i *issuerImpl) TokenKind() string { return auth.TokenKindJWT }

func (i *issuerImpl) ValidateToken(tkn string, ctxt diag.RequestContext) (auth.Token, diag.Error) {
	jws, err1 := jose.ParseSigned(tkn)
	if err1 != nil {
		return nil, diag.MakeError(OidcErrorTokenFormatError, "Token format is invalid", err1)
	}

	if len(jws.Signatures) != 1 {
		return nil, diag.MakeError(OidcErrorTokenFormatError, "Token format is invalid", nil)
	}

	if jws.Signatures[0].Header.Algorithm != string(jose.RS256) {
		return nil, diag.MakeError(OidcErrorTokenFormatError, fmt.Sprintf("Unsupported signature algorithm '%s'", jws.Signatures[0].Header.Algorithm), nil)
	}

	keySet, err := i.Signers(ctxt)
	if err != nil {
		return nil, err
	}

	keysToValidateAgainst := keySet.Keys
	if len(jws.Signatures[0].Header.KeyID) > 0 {
		// check if keySet contains this specific key
		keysToValidateAgainst = keySet.Key(jws.Signatures[0].Header.KeyID)
	}

	var validationErr error
	var payload []byte
	for _, k := range keysToValidateAgainst {
		payload, err1 = jws.Verify(k)
		if err1 == nil {
			validationErr = nil
			break
		}
		if validationErr == nil {
			validationErr = err1
		}
	}

	if validationErr != nil {
		return nil, diag.MakeError(OidcErrorSignatureValidationError, "Token signature invalid.", validationErr)
	}

	tok := &token{}

	r := bytes.NewReader(payload)
	dec := gojay.BorrowDecoder(r)
	defer dec.Release()
	err1 = dec.Object(tok)
	if err1 != nil {
		return nil, diag.MakeError(OidcErrorTokenFormatError, "failed decoding object", err1)
	}

	return tok, nil
}

func (i *issuerImpl) Signers(ctxt diag.RequestContext) (*jose.JSONWebKeySet, diag.Error) {

	certs, err := i.sc.SignerCerts(ctxt.Tenant(), ctxt)
	if err != nil {
		return nil, err
	}

	ks := &jose.JSONWebKeySet{
		Keys: make([]jose.JSONWebKey, 0, len(certs)),
	}
	for _, c := range certs {
		ks.Keys = append(
			ks.Keys,
			jose.JSONWebKey{
				KeyID:        getIssuerKeyID(c),
				Key:          c.PublicKey,
				Use:          "sig",
				Algorithm:    string(jose.RS256),
				Certificates: []*x509.Certificate{c},
			})
	}

	return ks, nil
}

func (i *issuerImpl) buildAccessToken(scope ScopeSet, attrValues types.AttributeValues,
	tp types.TokenPolicy, signer crypto.PrivateKey, kid string,
	ctxt diag.RequestContext) (string, string, diag.Error) {

	var err diag.Error

	tok := &token{}
	err = i.setTokenSubject(tok, attrValues, tp, ctxt)
	if err != nil {
		return "", "", err
	}
	err = i.setTokenGroups(tok, attrValues, tp, ctxt)
	if err != nil {
		return "", "", err
	}
	tok.issuer, err = i.sc.Issuer(ctxt.Tenant(), i.sr, ctxt)
	if err != nil {
		return "", "", err
	}

	tok.tokenClass = "access_token"
	tok.tokenType = "Bearer"
	tok.tenant = ctxt.Tenant().String()

	// TODO: audience
	tok.audience = stringSetImpl{}
	scope.Iterate(func(v Scope) diag.Error {
		// reimplement this is a hack that was there in current impl
		if strings.HasPrefix(v.String(), "rs_") {
			tok.audience.Add(v.String())
		}
		return nil
	})
	tok.scope = scope
	tok.id, err = generateRandom()
	if err != nil {
		return "", "", err
	}

	now := time.Now()

	tok.expiration = now.Add(tp.MaxBearerLifetime()).Unix()
	tok.issuedAt = now.Unix()

	res, err := i.signToken(tok, signer, kid, ctxt)
	if err != nil {
		return "", "", err
	}

	return res, "Bearer", nil
}

func (i *issuerImpl) buildIDToken(ci ClientInfo, scope ScopeSet, nonce string, session session.SessionID, attrValues types.AttributeValues,
	tp types.TokenPolicy, signer crypto.PrivateKey,
	at string, code string, kid string, ctxt diag.RequestContext) (string, diag.Error) {
	var err diag.Error
	tok := &token{}
	err = i.setTokenSubject(tok, attrValues, tp, ctxt)
	if err != nil {
		return "", err
	}
	err = i.setTokenGroups(tok, attrValues, tp, ctxt)
	if err != nil {
		return "", err
	}
	tok.issuer, err = i.sc.Issuer(ctxt.Tenant(), i.sr, ctxt)
	if err != nil {
		return "", err
	}

	tok.tokenClass = "id_token"
	tok.tokenType = "Bearer"
	tok.audience = stringSetImpl{ci.ClientID(): exists}
	tok.tenant = ctxt.Tenant().String()

	// todo: only honored scopes
	tok.scope = scope
	tok.id, err = generateRandom()
	if err != nil {
		return "", err
	}

	now := time.Now()

	tok.expiration = now.Add(tp.MaxBearerLifetime()).Unix()
	tok.issuedAt = now.Unix()

	tok.sid = session.String()
	tok.nonce = nonce

	if len(at) > 0 {
		tok.atHash = i.calculateHash(at)
	}
	if len(code) > 0 {
		// if we issued code, we should include this claim
		tok.cHash = i.calculateHash(code)
	}
	res, err := i.signToken(tok, signer, kid, ctxt)
	if err != nil {
		return "", err
	}

	return res, nil
}

func (i *issuerImpl) signToken(tok *token, signerKey crypto.PrivateKey, kid string, ctxt diag.RequestContext) (string, diag.Error) {
	sb := &strings.Builder{}
	bytes, err := gojay.MarshalJSONObject(tok)
	if err != nil {
		return "", diag.MakeError(OidcErrorEncodeError,
			fmt.Sprintf("Unable to encode jwt token '%v'", err), err)
	}

	sb.Write(bytes)

	signer, err := jose.NewSigner(
		jose.SigningKey{
			Algorithm: jose.RS256,
			Key: jose.JSONWebKey{
				KeyID:     kid,
				Key:       signerKey,
				Use:       "sig",
				Algorithm: string(jose.RS256),
			},
		}, nil)
	if err != nil {
		return "", diag.MakeError(OidcErrorJWSError,
			fmt.Sprintf("Unable to create signer '%v'", err), err)
	}

	payload := sb.String()

	jws, err := signer.Sign([]byte(payload))

	if err != nil {
		return "", diag.MakeError(OidcErrorJWSError,
			fmt.Sprintf("Unable to sign '%v'", err), err)
	}

	serialized, err := jws.CompactSerialize()
	if err != nil {
		return "", diag.MakeError(OidcErrorEncodeError,
			fmt.Sprintf("Unable to serialize JWS '%v'", err), err)
	}

	return serialized, nil
}

func (i *issuerImpl) calculateHash(val string) string {
	// https://openid.net/specs/openid-connect-core-1_0.html
	//at_hash/c_hash
	// Its value is the base64url encoding of the left-most half of the hash of the octets of the ASCII representation
	// of the access_token value, where the hash algorithm used is the hash algorithm used in the alg Header Parameter
	// of the ID Token's JOSE Header.
	// For instance, if the alg is RS256, hash the access_token value with SHA-256, then take the left-most 128 bits
	// and base64url encode them. The at_hash value is a case sensitive string.

	sha := sha256.Sum256(([]byte)(val))
	return base64.RawURLEncoding.EncodeToString(sha[:sha256.Size/2])
}

func (i *issuerImpl) setTokenSubject(tok *token, attrValues types.AttributeValues,
	tp types.TokenPolicy, ctxt diag.RequestContext) diag.Error {

	attrV, ok := attrValues.Value(types.AttributeIDUserIdentity)
	if !ok || attrV == nil || attrV.Len() <= 0 {
		return diag.MakeError(OidcErrorServerError, "Unable to identify token subject", nil)
	}
	if attrV.Len() != 1 {
		return diag.MakeError(OidcErrorServerError, "Unable to identify token subject", nil)
	}
	attrV.Iterate(func(v string) diag.Error {
		tok.subject = v
		return nil
	})

	return nil
}

func (i *issuerImpl) setTokenGroups(tok *token, attrValues types.AttributeValues,
	tp types.TokenPolicy, ctxt diag.RequestContext) diag.Error {

	attrV, ok := attrValues.Value(types.AttributeIDGroupIdentities)
	if !ok || attrV == nil || attrV.Len() <= 0 {
		tok.groups = stringSetImpl{}
	} else {
		tok.groups = make(stringSetImpl, attrV.Len())
		attrV.Iterate(func(v string) diag.Error {
			tok.groups.Add(v)
			return nil
		})
	}
	return nil
}

func getIssuerKeyID(c *x509.Certificate) string {
	// todo we should combine issuer with serial number
	id := ""
	if c != nil && c.SerialNumber != nil {
		id = c.SerialNumber.String()
	}

	return id
}

const randomLength = 32

func generateRandom() (string, diag.Error) {
	b := make([]byte, randomLength)
	_, err := rand.Read(b)
	if err != nil {
		return "", diag.MakeError(
			OidcErrorRandomGenError, fmt.Sprintf("Failed to generate random: %v", err), err)
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}
