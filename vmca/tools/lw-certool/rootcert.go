package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

// GetRootCert returns the root certificate
func GetRootCert(args []string) string {
	target := &Certificate{}

	host := parseRootArguments(args)
	response, err := RestAPIRequest("", host, http.MethodGet, RootCertEndpoint, nil)
	if err != nil {
		log.Fatalf("[ERROR] REST API Request failed. Error: '%s'\n", err.Error())
	}

	if response.StatusCode != 200 {
		log.Fatalf("[ERROR] REST API Request failed with response status: %s\n", response.Status)
	}

	data, err := ioutil.ReadAll(response.Body)
	if err != nil {
		log.Fatalf("[ERROR] Failed to read http response body. Error: '%s'\n", err.Error())
	}

	err = json.Unmarshal(data, target)
	if err != nil {
		log.Fatalf("[ERROR] Failed to unmarshal json response. Error: '%s'\n", err.Error())
	}

	return target.Cert
}

func parseRootArguments(args []string) string {
	length := len(args)

	for i := 0; i < length; i++ {
		switch args[i] {
		case "--server":
			i++
			if i >= length {
				showRootUsage()
				os.Exit(1)
			}
			host = args[i]
		case "--insecure":
			disableSSL = true
		case "--help":
			showRootUsage()
			os.Exit(0)
		default:
			showRootUsage()
			os.Exit(1)
		}
	}

	return host
}

func showRootUsage() {
	fmt.Println("Usage: lw-certool ca getroot { arguments }")
	fmt.Println("Arguments (optional):")
	fmt.Println("    --server    <server ip/hostname>. Default: localhost")
	fmt.Println("    --insecure (Use this to skip certificate validation)")
}
