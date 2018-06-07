// Package vecsclient provides an interoperability layer between the C based
// Lightwave VECS client and Go based applications.
package vecsclient

/*
#cgo CFLAGS: -I/opt/vmware/include -I/opt/likewise/include
#cgo LDFLAGS: -L/opt/vmware/lib64 -lvmafdclient -Wl,-rpath=/opt/vmware/lib64
#include <stdlib.h>
#include <vmafdtypes.h>
#include <vmafdclient.h>
#include <vecsclient.h>
*/
import "C"

import (
	"fmt"
	"runtime"
)

// VecsStore holds a C pointer to a VECS Store handle.
type VecsStore struct {
	p C.PVECS_STORE
}

// VecsEntry holds a C pointer to a VECS Entry.
type VecsEntry struct {
	p C.PVECS_CERT_ENTRY_A
}

type VecsEntryType int
type VecsEntryInfoLevel int

const (
	VecsTypeUnknown             VecsEntryType = iota // 0
	VecsTypePrivateKey                               // 1
	VecsTypeSecretKey                                // 2
	VecsTypeTrustedCert                              // 3
	VecsTypeRevokedCertList                          // 4
	VecsTypeEncryptedPrivateKey                      // 5
)

const (
	VecsEntryInfoLevelUndefined VecsEntryInfoLevel = iota // 0
	VecsEntryInfoLevel1                                   // 1
	VecsEntryInfoLevel2                                   // 2
)

// VecsForceRefreshCerts triggers a force refresh of Lightwave trusted
// root certificates.  It takes in a server address, username, and pasword.
// All arguments can and should be empty strings.
// This function will return any encountered errors.
func VecsForceRefreshCerts(server string, username string, password string) (err error) {
	serverCStr := goStringToCString(server)
	userNameCStr := goStringToCString(username)
	passwordCStr := goStringToCString(password)

	defer freeCString(serverCStr)
	defer freeCString(userNameCStr)
	defer freeCString(passwordCStr)

	var e C.DWORD = C.VmAfdTriggerRootCertsRefresh(
		serverCStr,
		userNameCStr,
		passwordCStr)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to trigger force refresh of certs (%s)", cErrorToGoError(e))
	}

	return
}

// VecsCreateStore creates a new cert store and returns its handle.
// The caller must call store.Close() when the store is no longer needed.
func VecsCreateStore(server string, storeName string, password string) (store *VecsStore, err error) {
	serverCStr := goStringToCString(server)
	storeNameCStr := goStringToCString(storeName)
	passwordCStr := goStringToCString(password)

	defer freeCString(serverCStr)
	defer freeCString(storeNameCStr)
	defer freeCString(passwordCStr)

	var s C.PVECS_STORE = nil
	var e C.DWORD = C.VecsCreateCertStoreA(
		serverCStr,
		storeNameCStr,
		passwordCStr,
		&s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to create cert store (%s)", cErrorToGoError(e))
		return
	}

	store = &VecsStore{s}
	runtime.SetFinalizer(store, vecsStoreFinalize)
	return store, nil
}

// VecsLoadStore returns a handle to the opened cert store.
// The caller must call store.Close() when the store is no longer needed.
func VecsLoadStore(server string, storeName string, password string) (store *VecsStore, err error) {
	serverCStr := goStringToCString(server)
	storeNameCStr := goStringToCString(storeName)
	passwordCStr := goStringToCString(password)

	defer freeCString(serverCStr)
	defer freeCString(storeNameCStr)
	defer freeCString(passwordCStr)

	var s C.PVECS_STORE = nil
	var e C.DWORD = C.VecsOpenCertStoreA(
		serverCStr,
		storeNameCStr,
		passwordCStr,
		&s)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to load cert store (%s)", cErrorToGoError(e))
		return
	}

	store = &VecsStore{s}
	runtime.SetFinalizer(store, vecsStoreFinalize)
	return
}

// VecsDeleteStore deletes the specified cert store
func VecsDeleteStore(server string, storeName string) (err error) {
	serverCStr := goStringToCString(server)
	storeNameCStr := goStringToCString(storeName)

	defer freeCString(serverCStr)
	defer freeCString(storeNameCStr)

	var e C.DWORD = C.VecsDeleteCertStoreA(
		serverCStr,
		storeNameCStr)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to delete store (%s)", cErrorToGoError(e))
		return
	}

	return
}

// Close closes the specified cert store.
// The caller must call store.Close() when the store is no longer needed.
func (store *VecsStore) Close() (err error) {
	if store.p != nil {
		err := C.VecsCloseCertStore(store.p)
		if err != 0 {
			return fmt.Errorf("[ERROR] failed to close cert store (%s)", cErrorToGoError(err))
		}
		store.p = nil
	}

	return
}

// EntryCount returns the number of entries in the specified store.
// It returns any encountered errors.
func (store *VecsStore) EntryCount() (count int, err error) {
	var c C.DWORD = 0
	var e C.DWORD = C.VecsGetEntryCount(
		store.p,
		&c)
	if err != nil {
		err = fmt.Errorf("[ERROR] failed to get entry count from VECS store (%s)", cErrorToGoError(e))
		return
	}

	count = int(c)
	return
}

// AddEntry creates a new entry and adds it to the specified store.
func (store *VecsStore) AddEntry(entryType VecsEntryType, alias string, certificate string, privateKey string, password string, autoRefresh bool) (err error) {
	aliasCStr := goStringToCString(alias)
	certificateCStr := goStringToCString(certificate)
	privateKeyCStr := goStringToCString(privateKey)
	passwordCStr := goStringToCString(password)

	defer freeCString(aliasCStr)
	defer freeCString(certificateCStr)
	defer freeCString(passwordCStr)
	defer freeCString(privateKeyCStr)

	var e C.DWORD = C.VecsAddEntryA(
		store.p,
		C.CERT_ENTRY_TYPE(entryType),
		aliasCStr,
		certificateCStr,
		privateKeyCStr,
		passwordCStr,
		C.BOOLEAN(func() int {
			if autoRefresh {
				return 1
			} else {
				return 0
			}
		}()))
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to add entry to store (%s)", cErrorToGoError(e))
		return
	}

	return
}

// GetEntry retrieves the aliased entry from the store.
// The caller must call entry.Free() when the entry is no longer needed.
func (store *VecsStore) GetEntry(alias string, infoLevel VecsEntryInfoLevel) (entry *VecsEntry, err error) {
	aliasCStr := goStringToCString(alias)

	defer freeCString(aliasCStr)

	var n C.PVECS_CERT_ENTRY_A = nil
	var e C.DWORD = C.VecsGetEntryByAliasA(
		store.p,
		aliasCStr,
		C.ENTRY_INFO_LEVEL(infoLevel),
		&n)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to get entry from store (%s)", cErrorToGoError(e))
		return
	}

	entry = &VecsEntry{n}
	runtime.SetFinalizer(entry, vecsEntryFinalize)
	return
}

// GetEntryType retrieves the type of the aliased entry from the store.
func (store *VecsStore) GetEntryType(alias string) (entryType VecsEntryType, err error) {
	aliasCStr := goStringToCString(alias)

	defer freeCString(aliasCStr)

	var t C.CERT_ENTRY_TYPE = 0
	var e C.DWORD = C.VecsGetEntryTypeByAliasA(
		store.p,
		aliasCStr,
		&t)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to get entry type from store (%s)", cErrorToGoError(e))
		return
	}

	entryType = VecsEntryType(t)
	return
}

// IsEntryKey determines if an aliased entry is a key.
func (store *VecsStore) IsEntryKey(alias string) (isKey bool, err error) {
	entryType, err := store.GetEntryType(alias)
	if err != nil {
		return
	}

	isKey = (entryType == VecsTypeSecretKey)
	return
}

// IsEntryCert determines if an aliased entry is a certificate.
func (store *VecsStore) IsEntryCert(alias string) (isCert bool, err error) {
	entryType, err := store.GetEntryType(alias)
	if err != nil {
		return
	}

	isCert = (entryType == VecsTypeTrustedCert)
	return
}

// GetEntryDate retrieves the date of the aliased entry from the store.
func (store *VecsStore) GetEntryDate(alias string) (entryDate int, err error) {
	aliasCStr := goStringToCString(alias)

	defer freeCString(aliasCStr)

	var d C.LW_UINT32 = 0
	var e C.DWORD = C.VecsGetEntryDateByAliasA(
		store.p,
		aliasCStr,
		&d)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to get entry date from store (%s)", cErrorToGoError(e))
		return
	}

	entryDate = int(d)
	return
}

// GetEntryCert retrieves the cert from the aliased entry in the store.
func (store *VecsStore) GetEntryCert(alias string) (cert string, err error) {
	aliasCStr := goStringToCString(alias)

	defer freeCString(aliasCStr)

	var c C.PSTR = nil
	var e C.DWORD = C.VecsGetCertificateByAliasA(
		store.p,
		aliasCStr,
		&c)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to get cert from store (%s)", cErrorToGoError(e))
		return
	}

	cert = vmafdStringToGoString(c)
	return
}

// GetEntryKey retrieves the key from the aliased entry in the store.
func (store *VecsStore) GetEntryKey(alias string, password string) (privateKey string, err error) {
	aliasCStr := goStringToCString(alias)
	passwordCStr := goStringToCString(password)

	defer freeCString(aliasCStr)
	defer freeCString(passwordCStr)

	var k C.PSTR = nil
	var e C.DWORD = C.VecsGetKeyByAliasA(
		store.p,
		aliasCStr,
		passwordCStr,
		&k)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to get key from store (%s)", cErrorToGoError(e))
		return
	}

	privateKey = vmafdStringToGoString(k)
	return
}

// DeleteEntry removes the aliased entry from the store.
func (store *VecsStore) DeleteEntry(alias string) (err error) {
	aliasCStr := goStringToCString(alias)

	defer freeCString(aliasCStr)

	var e C.DWORD = C.VecsDeleteEntryA(
		store.p,
		aliasCStr)
	if e != 0 {
		err = fmt.Errorf("[ERROR] failed to delete entry from store (%s)", cErrorToGoError(e))
		return
	}

	return
}

// GetType returns the type of the entry.
func (entry *VecsEntry) GetType() (entryType VecsEntryType, err error) {
	if entry.p != nil {
		entryType = VecsEntryType(entry.p.entryType)
		return
	}

	err = fmt.Errorf("[ERROR] failed to get type from nil entry")
	return
}

// IsKey determines if the entry is a key.
func (entry *VecsEntry) IsKey() (isKey bool, err error) {
	entryType, err := entry.GetType()
	if err != nil {
		return
	}

	isKey = (entryType == VecsTypeSecretKey)
	return
}

// IsCert determines if the entry is a certificate.
func (entry *VecsEntry) IsCert() (isCert bool, err error) {
	entryType, err := entry.GetType()
	if err != nil {
		return
	}

	isCert = (entryType == VecsTypeTrustedCert)
	return
}

// GetDate returns the creation date of the entry.
func (entry *VecsEntry) GetDate() (date int, err error) {
	if entry.p != nil {
		date = int(entry.p.dwDate)
		return
	}

	err = fmt.Errorf("[ERROR] failed to get date from nil entry")
	return
}

// GetAlias returns the alias of the entry.
func (entry *VecsEntry) GetAlias() (alias string, err error) {
	if entry.p != nil {
		if entry.p.pszAlias != nil {
			alias = cStringToGoString(entry.p.pszAlias)
			return
		}
	}

	err = fmt.Errorf("[ERROR] failed to get alias from nil entry or entry alias")
	return
}

// GetCert returns the certificate of the entry.
func (entry *VecsEntry) GetCert() (cert string, err error) {
	if entry.p != nil {
		if entry.p.pszCertificate != nil {
			cert = cStringToGoString(entry.p.pszCertificate)
			return
		}
	}

	err = fmt.Errorf("[ERROR] failed to get cert from nil entry or entry cert")
	return
}

// Free cleans up memory for the entry.
// The caller must call entry.Free() when the entry is no longer needed.
func (entry *VecsEntry) Free() {
	if entry.p != nil {
		C.VecsFreeCertEntryA(entry.p)
		entry.p = nil
	}

	return
}

// vecsStoreFinalize is a wrapper function to set the finalizer for VecsStore.
func vecsStoreFinalize(s *VecsStore) {
	s.Close()
}

// vecsEntryFinalize is a wrapper function to set the finalizer for VecsEntry.
func vecsEntryFinalize(e *VecsEntry) {
	e.Free()
}
