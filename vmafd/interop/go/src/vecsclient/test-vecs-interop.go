package main

import (
	"fmt"
	"io/ioutil"
	"log"

	"github.com/vmware/lightwave/vmafd/interop/go/src/vecsclient"
)

func main() {
	fmt.Println("Testing VECS Go Interop\n")

	store, err := vecsclient.VecsCreateStore("localhost", "go-interop-test", "password")
	if err != nil {
		log.Fatalf("Failed to load cert store (%+v)", err)
	}

	err = store.Close()
	if err != nil {
		log.Fatalf("Failed to close cert store (%+v)", err)
	}

	store, err = vecsclient.VecsLoadStore("localhost", "go-interop-test", "password")
	if err != nil {
		log.Fatalf("Failed to load cert store (%+v)", err)
	}
	defer store.Close()

	cert, err := ioutil.ReadFile("/root/cert.crt")
	if err != nil {
		log.Fatalf("Failed to read cert file (%+v)", err)
	}

	err = store.AddEntry(vecsclient.VecsTypeTrustedCert, "my-alias", string(cert), "", "password", false)
	if err != nil {
		log.Fatalf("Failed to add entry to store (%+v)", err)
	}

	entry, err := store.GetEntry("my-alias", vecsclient.VecsEntryInfoLevel2)
	if err != nil {
		log.Fatalf("Failed to get entry from store (%+v)", err)
	}
	defer entry.Free()

	alias, err := entry.GetAlias()
	if err != nil {
		log.Fatalf("Failed to get alias from entry (%+v)", err)
	}
	fmt.Printf("Alias: %s\n", alias)

	date, err := store.GetEntryDate("my-alias")
	if err != nil {
		log.Fatalf("Failed to get date from entry (%+v)", err)
	}
	fmt.Printf("Date: %d\n", date)

	certa, err := entry.GetCert()
	if err != nil {
		log.Fatalf("Failed to get cert from entry (%+v)", err)
	}
	fmt.Printf("Cert:\n%s\n", certa)

	isCert, err := store.IsEntryCert("my-alias")
	if err != nil {
		log.Fatalf("Failed to do isEntryCert (%+v)", err)
	}
	if isCert {
		fmt.Printf("cert")
	}

	err = store.DeleteEntry("my-alias")
	if err != nil {
		log.Fatalf("Failed to delete entry (%+v)", err)
	}

	err = store.Close()
	if err != nil {
		log.Fatalf("Failed to close cert store (%+v)", err)
	}

	err = vecsclient.VecsDeleteStore("localhost", "go-interop-test")
	if err != nil {
		log.Fatalf("Failed to delete cert store (%+v)", err)
	}

	fmt.Println("\nSuccess :)")
}
