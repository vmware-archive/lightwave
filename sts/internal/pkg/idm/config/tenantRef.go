package config

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"encoding/base64"
	"fmt"

	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/types"
)

type tenantRef interface {
	types.TenantInfo

	DomainDn() string
	Key() string

	getTenantsDn() string
	getConfigDn() string

	encrypt(cleartext string) (string, diag.Error)
	decrypt(encrypted string) (string, diag.Error)
}

type tenantRefBuilder interface {
	Name(n diag.TenantID)
	DomainDn(d string)
	Key(k string)

	Build() (tenantRef, diag.Error)
}

func newtenantRefBuilder() tenantRefBuilder {
	return &tenantRefBuilderImpl{}
}

type transformer func(string) (string, diag.Error)

// idm:"ldap:oc=lightwaveSTSTenantReference;ccn=Tenants;t=tenantRef"
type tenantRefImpl struct {
	name     diag.TenantID // idm:"ldap:name=cn;u=false;m=Name"
	domainDn string        // idm:"ldap:name=aliasedObjectName;u=false;m=DomainDn"
	key      string        // idm:"ldap:name=lightwaveSTSTenantKey;u=false;m=Key"
	binKey   []byte
	domain   string
}

func (t *tenantRefImpl) Name() diag.TenantID { return t.name }
func (t *tenantRefImpl) Domain() string      { return t.domain }
func (t *tenantRefImpl) DomainDn() string    { return t.domainDn }
func (t *tenantRefImpl) Key() string         { return t.key }

func (t *tenantRefImpl) getTenantsDn() string {
	// cn=Tenant,cn=Config,cn=STS,dc=tenant
	return "cn=" + t.name.String() + "," + t.getConfigDn()
}

func (t *tenantRefImpl) getConfigDn() string {
	return getConfigDnForDomain(t.Domain())
}

func (t *tenantRefImpl) encrypt(cleartext string) (string, diag.Error) {
	if len(cleartext) <= 0 {
		return "", nil
	}

	block, err := aes.NewCipher(t.binKey)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	nonce := make([]byte, aesgcm.NonceSize())
	_, err = rand.Read(nonce)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	ciphertext := aesgcm.Seal(nonce, nonce, []byte(cleartext), nil)

	return base64.RawURLEncoding.EncodeToString(ciphertext), nil
}
func (t *tenantRefImpl) decrypt(encrypted string) (string, diag.Error) {
	if len(encrypted) <= 0 {
		return "", nil
	}

	val, err := base64.RawURLEncoding.DecodeString(encrypted)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to base64 decode: %v", err), err)
	}

	block, err := aes.NewCipher(t.binKey)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to new cipher: %v", err), err)
	}

	aesgcm, err := cipher.NewGCM(block)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to new gcm: %v", err), err)
	}

	nonce := val[:aesgcm.NonceSize()]
	val = val[aesgcm.NonceSize():]

	cleartext, err := aesgcm.Open(nil, nonce, val, nil)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to gcm open: %v", err), err)
	}

	return string(cleartext), nil
}

func getConfigDnForDomain(domain string) string {
	return "cn=" + StsConfigContainer + ",cn=" + StsContainer + "," + types.DnFromDomain(domain)

}

type tenantRefBuilderImpl tenantRefImpl

func (t *tenantRefBuilderImpl) Name(n diag.TenantID) { t.name = n }
func (t *tenantRefBuilderImpl) DomainDn(d string)    { t.domainDn = d }
func (t *tenantRefBuilderImpl) Key(k string)         { t.key = k }

func (t *tenantRefBuilderImpl) Build() (tenantRef, diag.Error) {
	// todo: validations
	t.domain = types.DomainFromDn(t.domainDn)

	if len(t.key) <= 0 {
		var err diag.Error
		t.key, err = generateRandom()
		if err != nil {
			return nil, err
		}
	}

	var errf error
	t.binKey, errf = base64.RawURLEncoding.DecodeString(t.key)
	if errf != nil {
		return nil, diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to base64 decode: %v", errf), errf)
	}

	return (*tenantRefImpl)(t), nil
}

const randomLength = 32

func generateRandom() (string, diag.Error) {
	b := make([]byte, randomLength)
	_, err := rand.Read(b)
	if err != nil {
		return "", diag.MakeError(
			types.IdmErrorGeneric, fmt.Sprintf("Failed to generate random: %v", err), err)
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}

type transformedVal struct {
	val   ldap.Value
	trans transformer
}

func transformedValue(val ldap.Value, trans transformer) ldap.Value {
	return &transformedVal{val: val, trans: trans}
}

func (ev *transformedVal) Len() int {
	if ev == nil || ev.val == nil {
		return 0
	}

	return ev.val.Len()
}
func (ev *transformedVal) IterateString(f ldap.StringValueFunc) diag.Error {
	if ev == nil || ev.val == nil {
		return nil
	}

	return ev.val.IterateString(func(s string) diag.Error {
		sv, err := ev.trans(s)
		if err != nil {
			return err
		}
		return f(sv)
	})
}
func (ev *transformedVal) IterateBinary(f ldap.BinaryValueFunc) diag.Error {
	if ev == nil || ev.val == nil {
		return nil
	}
	return ev.val.IterateBinary(func(b []byte) diag.Error {
		sv, err := ev.trans(string(b))
		if err != nil {
			return err
		}
		return f([]byte(sv))
	})
}
