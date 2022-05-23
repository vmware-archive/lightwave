package oidc

import (
	"bytes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"strings"
	"time"

	jose "gopkg.in/square/go-jose.v2"
)

const (
	// ClaimSubject is the name of the subject token claim
	ClaimSubject = "sub"
	// ClaimIssuer is the name of the issuer token claim
	ClaimIssuer = "iss"
	// ClaimNonce is the name of the nonce token claim
	ClaimNonce = "nonce"
	// ClaimTokenType is the name of the token type token claim
	ClaimTokenType = "token_type"
	// ClaimTokenClass is the name of the token class claim
	ClaimTokenClass = "token_class"
	// ClaimHotKJWK is the name of the HOTK JWK set claim
	ClaimHotKJWK = "hotk"
	// ClaimAudience is the name of the audience token claim
	ClaimAudience = "aud"
	// ClaimExpiration is the name of the exiration token claim
	ClaimExpiration = "exp"
	//ClaimIssuedAt is the time the token was issued
	ClaimIssuedAt = "iat"
	// ClaimGroups is the name of the groups token claim
	ClaimGroups = "groups"
	//ClaimSessionID cotains the idp session id
	ClaimSessionID = "sid"

	// BearerTokenType is type of a Bearer token
	BearerTokenType = "Bearer"
	// HOKTokenType is type of a holder-of-key token
	HOKTokenType = "hotk-pk"
	// AccessTokenClass is the class of an access token
	AccessTokenClass = "access_token"
	// IDTokenClass is the class of an id token
	IDTokenClass = "id_token"
)

// JWT interface represents a parsed/validated JWT
type JWT interface {
	Issuer() string
	Nonce() (string, bool)
	Groups() ([]string, bool)
	Type() string
	Class() string
	Subject() string
	Expiration() time.Time
	IssuedAt() time.Time
	IdpSessionID() (string, bool)
	Audience() ([]string, bool)
	Hotk() (*jose.JSONWebKeySet, error)
	Claim(claimName string) (interface{}, bool)
}

func parseTenantInToken(token string) (string, error) {
	var tenant string

	parts := strings.Split(strings.TrimSpace(token), ".")
	if len(parts) != 3 {
		return "", OIDCTokenInvalidError.MakeError("A valid token must have 3 parts", nil)
	}

	payload, err := base64.RawURLEncoding.DecodeString(parts[1])
	if err != nil {
		return "", OIDCTokenInvalidError.MakeError("Error when decoding token payload", err)
	}

	rawClaims, err := decodePayload(payload)
	if err != nil {
		return "", err
	}

	claim, ok := rawClaims["tenant"]
	if !ok {
		return "", OIDCTokenInvalidError.MakeError("No tenant claim in token", nil)
	}

	if tenant, ok = claim.(string); !ok {
		return "", OIDCTokenInvalidError.MakeError("Invalid tenant claim in token", nil)
	}

	return tenant, nil
}

func parseToken(
	token string, issuer string, audience string, nonce string, signers IssuerSigners, tokenType string, logger Logger) (JWT, error) {
	var err error

	if signers == nil {
		return nil, OIDCInvalidArgError.MakeError("signers must be provided", nil)
	}

	s, ok := signers.(*signersImpl)
	if ok == false || s.signers == nil {
		return nil, OIDCInvalidArgError.MakeError("signers argument is invalid", nil)
	}

	if len(issuer) <= 0 {
		return nil, OIDCInvalidArgError.MakeError("issuer must be provided", nil)
	}

	if logger == nil {
		logger = NewLogger()
		if err != nil {
			return nil, err
		}
	}

	tok, err := verifyToken(token, s.signers, issuer, audience, nonce, defaultClockToleranceSecs, logger)
	if err != nil {
		return nil, err
	}

	if !strings.EqualFold(tokenType, tok.Class()) {
		PrintLog(logger, LogLevelError, "Failed to verify token: token class %s does not match expected %s", tokenType, tok.Class())
		return nil, OIDCTokenInvalidError.MakeError("Failed to validate "+tok.Class()+" token", nil)
	}

	return tok, nil
}

func verifyToken(token string, signers *jose.JSONWebKeySet, issuer string, audience string, nonce string,
	clockTolerance int, logger Logger) (JWT, error) {
	jwt, err := parseAndValidateSignedToken(token, issuer, signers, clockTolerance)

	if err != nil {
		PrintLog(logger, LogLevelError, "verifyToken: Parse signed token failed. Error: '%v'", err)
		return nil, err
	}

	// validate nonce
	if len(nonce) > 0 {
		tokenNonce, ok := jwt.Nonce()
		if !ok || nonce != tokenNonce {
			PrintLog(logger, LogLevelError,
				"verifyToken: Token 'nonce' claim is invalid. Nonce claim present = '%t', expected='%s', actual='%s'",
				ok, nonce, tokenNonce)
			return nil, OIDCTokenInvalidError.MakeError("Token 'nonce' claim is invalid", nil)
		}
	}

	// validate audience
	if len(audience) > 0 {
		aud, ok := jwt.Audience()
		if !ok || !contains(aud, audience) {
			PrintLog(logger, LogLevelError,
				"verifyToken: Token audience verification failed. Audience claim present = '%t', expected='%s', actual='%v'",
				ok, audience, aud)
			return nil, OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Token does not have '%s' as an audience", audience), nil)
		}
	}

	return jwt, nil
}

func validateAndNormalizeClaims(rawClaims *map[string]interface{}, issuer string, clockToleranceSecs int) error {
	// issuer must match
	// type, class, subject must be present
	// expiration should not be in the past
	// issued at should not be in the future

	err := ensureTimeClaim(rawClaims, ClaimExpiration)
	if err != nil {
		return err
	}
	err = ensureTimeClaim(rawClaims, ClaimIssuedAt)
	if err != nil {
		return err
	}

	err = validateExpiration(rawClaims, clockToleranceSecs)
	if err != nil {
		return err
	}

	err = ensureStringClaimValue(rawClaims, ClaimIssuer, issuer)
	if err != nil {
		return err
	}
	err = ensureStringClaim(rawClaims, ClaimTokenType)
	if err != nil {
		return err
	}
	err = ensureStringClaim(rawClaims, ClaimTokenClass)
	if err != nil {
		return err
	}
	err = ensureOptionalHotkJWKSetClaim(rawClaims, ClaimHotKJWK)
	if err != nil {
		return err
	}
	err = ensureStringClaim(rawClaims, ClaimSubject)
	if err != nil {
		return err
	}
	err = ensureOptionalStringClaim(rawClaims, ClaimNonce)
	if err != nil {
		return err
	}
	err = ensureOptionalStringClaim(rawClaims, ClaimSessionID)
	if err != nil {
		return err
	}
	err = ensureOptionalStringArrayClaim(rawClaims, ClaimGroups)
	if err != nil {
		return err
	}

	err = validateAudienceClaim(rawClaims)
	if err != nil {
		return err
	}

	return nil
}

func ensureStringClaim(claims *map[string]interface{}, claimName string) error {
	c := *claims
	val, ok := c[claimName]
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim missing", claimName), nil)
	}
	valStr, ok := val.(string)
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim must be string", claimName), nil)
	}
	if len(valStr) <= 0 {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim must not be empty", claimName), nil)
	}
	return nil
}

func ensureOptionalStringClaim(claims *map[string]interface{}, claimName string) error {
	c := *claims
	val, ok := c[claimName]
	if ok {
		_, ok := val.(string)
		if !ok {
			return OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Invalid token: '%s' claim must be string", claimName), nil)
		}
	}
	return nil
}

func ensureOptionalStringArrayClaim(claims *map[string]interface{}, claimName string) error {
	c := *claims
	val, ok := c[claimName]
	if ok {
		_, ok := val.([]string)
		if !ok {
			genericArray, ok := val.([]interface{})
			if !ok {
				return OIDCTokenInvalidError.MakeError(
					fmt.Sprintf("Invalid token: '%s' claim must be a string array", claimName), nil)
			}

			strArray := make([]string, 0, len(genericArray))
			for _, v := range genericArray {
				if v == nil {
					return OIDCTokenInvalidError.MakeError(
						fmt.Sprintf("Invalid token: '%s' claim must be a string array", claimName), nil)
				}
				s, ok := v.(string)
				if !ok {
					return OIDCTokenInvalidError.MakeError(
						fmt.Sprintf("Invalid token: '%s' claim must be a string array", claimName), nil)
				}
				strArray = append(strArray, s)
			}

			c[claimName] = strArray
		}
	}
	return nil
}

func ensureStringClaimValue(claims *map[string]interface{}, claimName string, claimValue string) error {
	c := *claims
	val, ok := c[claimName]
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim missing", claimName), nil)
	}
	valStr, ok := val.(string)
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim must be string", claimName), nil)
	}
	if valStr != claimValue {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim has invalid value '%s', expected '%s'",
				claimName, valStr, claimValue), nil)
	}

	return nil
}

func ensureTimeClaim(claims *map[string]interface{}, claimName string) error {
	c := *claims
	val, ok := c[claimName]
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim missing", claimName), nil)
	}
	var valInt64 int64
	valFl64, ok := val.(float64)
	if !ok {
		valInt64, ok = val.(int64)
		if !ok {
			return OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Invalid token: '%s' claim must be integer", claimName), nil)
		}
	} else {
		valInt64 = int64(valFl64)
	}

	exp := time.Unix(valInt64, 0).UTC()

	c[claimName] = exp

	return nil
}

func ensureOptionalHotkJWKSetClaim(claims *map[string]interface{}, claimName string) error {
	var ok bool
	var val interface{}
	var hotk jose.JSONWebKeySet
	var tokType string

	c := *claims
	if val, ok = c[ClaimTokenType]; !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim missing", ClaimTokenType), nil)
	}
	tokType = val.(string)

	if tokType == HOKTokenType {
		if val, ok = c[claimName]; !ok {
			return OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Invalid token: '%s' token missing '%s' claim", tokType, ClaimHotKJWK), nil)
		}

		rawHotk, err := json.Marshal(val)
		if err != nil {
			return OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Failed to marshal hotk claim"), err)
		}

		var hotkTmp struct {
			Keys []json.RawMessage `json:"keys"`
		}
		json.Unmarshal(rawHotk, &hotkTmp)

		if len(hotkTmp.Keys) > 0 {
			for _, k := range hotkTmp.Keys {
				var jwk jose.JSONWebKey
				err := json.Unmarshal([]byte(k), &jwk)
				if err != nil {
					return OIDCTokenInvalidError.MakeError(
						fmt.Sprintf("Failed to unmarshal JWK from HOTK claim"), err)
				}
				hotk.Keys = append(hotk.Keys, jwk)
			}
			return nil
		}
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("TokenType %s needs %s claim", tokType, claimName), err)
	}

	// Return nil here because tokenType is not hotk-pk and we don't check for
	// or require the HOTK claim
	return nil
}

func validateExpiration(claims *map[string]interface{}, clockToleranceSecs int) error {
	c := *claims

	expiration := c[ClaimExpiration].(time.Time)
	issuedAt := c[ClaimIssuedAt].(time.Time)

	now := time.Now().UTC()

	expWithTolerance := expiration.Add(time.Duration(clockToleranceSecs) * time.Second)
	issuedWithTolerance := issuedAt.Add(-time.Duration(clockToleranceSecs) * time.Second)
	if now.After(expWithTolerance) {
		return OIDCTokenExpiredError.MakeError(
			fmt.Sprintf("Expired token: token expired at '%v' now is '%v'", expiration, now), nil)
	}
	if now.Before(issuedWithTolerance) {
		return OIDCTokenNotYetValidError.MakeError(
			fmt.Sprintf("Not yet valid token: token issued at '%v' now is '%v'. Possible time skew", issuedAt, now), nil)
	}

	return nil
}

func validateAudienceClaim(claims *map[string]interface{}) error {
	c := *claims
	val, ok := c[ClaimAudience]
	if !ok {
		return OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Invalid token: '%s' claim missing", ClaimAudience), nil)
	}
	// audience can be aither array or string
	valStr, ok := val.(string)
	if ok {
		c[ClaimAudience] = []string{valStr}
	} else {
		_, ok := val.([]string)
		if !ok {
			genericArray, ok := val.([]interface{})
			if !ok {
				return OIDCTokenInvalidError.MakeError(
					fmt.Sprintf("Invalid token: '%s' claim must be a string or string array", ClaimAudience), nil)
			}
			strArray := make([]string, 0, len(genericArray))
			for _, val := range genericArray {
				if val == nil {
					OIDCTokenInvalidError.MakeError(
						fmt.Sprintf("Invalid token: '%s' claim must be a string or string array",
							ClaimAudience), nil)
				}
				s, ok := val.(string)
				if !ok {
					OIDCTokenInvalidError.MakeError(
						fmt.Sprintf("Invalid token: '%s' claim must be a string or string array",
							ClaimAudience), nil)
				}

				strArray = append(strArray, s)
			}

			c[ClaimAudience] = strArray
		}
	}
	return nil
}

type tokensImpl struct {
	IDTokenField      string
	AccessTokenField  string
	RefreshTokenField string
	TokenTypeField    string
	ExpiresInField    int
}

// Tokens interface

func (c *tokensImpl) AccessToken() string {
	return c.AccessTokenField
}

func (c *tokensImpl) RefreshToken() string {
	return c.RefreshTokenField
}

func (c *tokensImpl) IDToken() string {
	return c.IDTokenField
}

func (c *tokensImpl) TokenType() string {
	return c.TokenTypeField
}

func (c *tokensImpl) ExpiresIn() int {
	return c.ExpiresInField
}

type jwtImpl struct {
	claims map[string]interface{}
}

// parseSignedToken parses jwt token from its string representation
func parseAndValidateSignedToken(token string, issuer string, keySet *jose.JSONWebKeySet, clockToleranceSecs int) (JWT, error) {
	jwt, err := parseSignedToken(token, keySet)
	if err != nil {
		return nil, err
	}
	jwti := jwt.(*jwtImpl)
	// validate and normalize expected claims.
	err = validateAndNormalizeClaims(&jwti.claims, issuer, clockToleranceSecs)
	if err != nil {
		return nil, err
	}
	return jwt, nil
}

// parseSignedToken parses jwt token from its string representation
func parseSignedToken(token string, keySet *jose.JSONWebKeySet) (JWT, error) {
	jws, err := jose.ParseSigned(token)
	if err != nil {
		return nil, OIDCTokenInvalidError.MakeError("Token format is invalid", err)
	}

	if len(jws.Signatures) != 1 {
		return nil, OIDCTokenInvalidError.MakeError("Token has either no signature or multiple signatures", nil)
	}

	if jws.Signatures[0].Header.Algorithm != string(jose.RS256) {
		return nil, OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("Unsupported signature algorithm '%s'", jws.Signatures[0].Header.Algorithm), nil)
	}

	keysToValidateAgainst := keySet.Keys
	if len(jws.Signatures[0].Header.KeyID) > 0 {
		// check if keySet contains this specific key
		keysToValidateAgainst = keySet.Key(jws.Signatures[0].Header.KeyID)
	}

	var payload []byte
	for _, k := range keysToValidateAgainst {
		payload, err = jws.Verify(k)
		if err == nil {
			break
		}
	}

	if err != nil {
		return nil, OIDCTokenInvalidSignatureError.MakeError(
			"Token signature invalid.", err)
	}

	tokenBody, err := decodePayload(payload)
	if err != nil {
		return nil, err
	}
	return &jwtImpl{claims: tokenBody}, nil
}

func (t *jwtImpl) Issuer() string {
	return t.claims[ClaimIssuer].(string)
}

func (t *jwtImpl) Nonce() (string, bool) {
	strVal := ""
	val, ok := t.claims[ClaimNonce]
	if ok {
		strVal = val.(string)
	}

	return strVal, ok
}

func (t *jwtImpl) Groups() ([]string, bool) {
	var arrayVal []string
	val, ok := t.claims[ClaimGroups]
	if ok {
		arrayVal = val.([]string)
	}

	return arrayVal, ok
}

func (t *jwtImpl) Type() string {
	return t.claims[ClaimTokenType].(string)
}

func (t *jwtImpl) Class() string {
	return t.claims[ClaimTokenClass].(string)
}

func (t *jwtImpl) Subject() string {
	return t.claims[ClaimSubject].(string)
}

func (t *jwtImpl) Expiration() time.Time {
	return t.claims[ClaimExpiration].(time.Time)
}

func (t *jwtImpl) IssuedAt() time.Time {
	return t.claims[ClaimIssuedAt].(time.Time)
}

func (t *jwtImpl) Audience() ([]string, bool) {
	var arrayVal []string
	val, ok := t.claims[ClaimAudience]
	if ok {
		arrayVal = val.([]string)
	}

	return arrayVal, ok
}

func (t *jwtImpl) Hotk() (*jose.JSONWebKeySet, error) {
	if t.Type() == HOKTokenType {
		if val, ok := t.claims[ClaimHotKJWK]; ok {
			var hotk jose.JSONWebKeySet
			rawHotk, err := json.Marshal(val)
			if err != nil {
				return nil, OIDCJsonParseError.MakeError(
					fmt.Sprintf("Failed to marshal hotk claim"), err)
			}

			var hotkTmp struct {
				Keys []json.RawMessage `json:"keys"`
			}
			json.Unmarshal(rawHotk, &hotkTmp)

			if len(hotkTmp.Keys) > 0 {
				for _, k := range hotkTmp.Keys {
					var jwk jose.JSONWebKey
					err := json.Unmarshal([]byte(k), &jwk)
					if err != nil {
						return nil, OIDCJsonParseError.MakeError(
							fmt.Sprintf("Failed to unmarshal JWK from HOTK claim"), err)
					}
					hotk.Keys = append(hotk.Keys, jwk)
				}
				return &hotk, nil
			}
			return nil, OIDCTokenInvalidError.MakeError(
				fmt.Sprintf("Failed to find HOTK JWKs"), err)
		}
		return nil, OIDCTokenInvalidError.MakeError(
			fmt.Sprintf("hotk-pk JWT must have hotk claim"), nil)
	}

	return nil, nil
}

func (t *jwtImpl) IdpSessionID() (string, bool) {
	strVal := ""
	val, ok := t.claims[ClaimSessionID]
	if ok {
		strVal = val.(string)
	}

	return strVal, ok
}

func (t *jwtImpl) Claim(claimName string) (interface{}, bool) {
	val, ok := t.claims[claimName]
	return val, ok
}

type signersImpl struct {
	signers *jose.JSONWebKeySet
}

func (s *signersImpl) Combine(signers ...IssuerSigners) IssuerSigners {
	res := &jose.JSONWebKeySet{
		Keys: make([]jose.JSONWebKey, 0, 5),
	}
	if s != nil && s.signers != nil {
		res.Keys = append(res.Keys, s.signers.Keys...)
	}
	if signers != nil && len(signers) > 0 {
		for _, k := range signers {
			if si, ok := k.(*signersImpl); ok && si.signers != nil {
				res.Keys = append(res.Keys, si.signers.Keys...)
			}
		}
	}

	return &signersImpl{signers: res}
}

func decodePayload(payload []byte) (map[string]interface{}, error) {
	var tokenBody map[string]interface{}

	jsonDecoder := json.NewDecoder(bytes.NewReader(payload))
	if err := jsonDecoder.Decode(&tokenBody); err != nil {
		return nil, OIDCTokenInvalidError.MakeError("Unable to unmarshal jwt token", err)
	}

	return tokenBody, nil
}

func parseTokenMulti(
	token string, audience string, nonce string, providerInfo []ProviderInfo, tokenType string, logger Logger) (JWT, int, error) {
	var tok JWT
	var index int
	var err error
	var info ProviderInfo

	for index, info = range providerInfo {
		tok, err = parseToken(token, info.Issuer, audience, nonce, info.Signers, tokenType, logger)
		if err == nil {
			break
		}
	}

	return tok, index, err
}
