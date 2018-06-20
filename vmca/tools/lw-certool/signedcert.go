package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/vmware/lightwave/vmafd/interop/go/src/afdclient"
)

// GeneratePrivateKey saves a new signed certificate in given path
func GenerateSignedCert(args []string) string {
	var encodedKeyBytes []byte
	var err error
	target := &Certificate{}

	host, domain, username, password := parseSignedArguments(args)
	token, err := GetBearerToken(host, domain, username, password)
	if err != nil {
		log.Fatalf("[ERROR] Failed to get access token. Error: '%s'\n", err.Error())
	}

	if _, err := os.Stat(privKeyPath); os.IsNotExist(err) {
		encodedKeyBytes, err = GeneratePrivateKey(privKeyPath)
		if err != nil {
			log.Fatalf("[ERROR] Failed to create new private key. Error: '%s'\n", err.Error())
		}
	} else {
		encodedKeyBytes, err = ReadPrivateKey(privKeyPath)
		if err != nil {
			log.Fatalf("[ERROR] Failed to read given private key (If you want to create new private key, provide a new non existing file: --privkey <new_filepath>). Error: '%s'\n", err.Error())
		}
	}

	csr, err := GenerateCSR(encodedKeyBytes)
	if err != nil || csr == "" {
		log.Fatalf("[ERROR] Failed to generate certificate signing request. Error: '%s'\n", err.Error())
	}

	epochTime := time.Now().Unix()

	jsonData := map[string]string{
		"csr":       csr,
		"notBefore": strconv.FormatInt(epochTime, 10),
		"duration":  conf.CertDuration}

	jsonValue, err := json.Marshal(jsonData)
	if err != nil {
		log.Fatalf("[ERROR] Failed to marshal json body. Error: '%s'\n", err.Error())
	}

	response, err := RestAPIRequest(token, host, http.MethodPut, SignedCertEndpoint, bytes.NewBuffer(jsonValue))
	if err != nil {
		log.Fatalf("[ERROR] REST API Request failed. Error: '%s'\n", err.Error())
	}

	if response.StatusCode != 200 {
		log.Fatalf("[ERROR] REST API Request failed with response status: '%s'\n", response.Status)
	}

	data, err := ioutil.ReadAll(response.Body)
	if err != nil {
		log.Fatalf("[ERROR] Failed to read http response body. Error: '%s'\n", err.Error())
	}

	err = json.Unmarshal(data, target)
	if err != nil {
		log.Fatalf("[ERROR] Failed to unmarshal json response. Error: '%s'\n", err.Error())
	}

	if certPath != "" {
		SaveCertToFile(target.Cert)
	}

	return target.Cert
}

// SaveCertToFile saves the certificate in the given file
func SaveCertToFile(cert string) {
	certFileHandle, err := os.Create(certPath)
	if err != nil {
		log.Fatalf("[ERROR] Failed to create cert file. File Location: '%s', Error: '%s'\n", certPath, err.Error())
	}
	defer certFileHandle.Close()

	_, err = certFileHandle.WriteString(cert)
	if err != nil {
		log.Fatalf("[ERROR] Failed to write cert key to file. File Location: '%s', Error '%s'\n", certPath, err.Error())
	}

	os.Chmod(certPath, 0400)
}

func parseSignedArguments(args []string) (hostname string, domain string, username string, password string) {
	var err error
	length := len(args)

	if length < 2 {
		showSignedUsage()
		os.Exit(1)
	}

	for i := 0; i < length; i++ {
		switch args[i] {
		case "--server":
			i++
			if i >= length {
				showRootUsage()
				os.Exit(1)
			}
			host = args[i]
		case "--domain":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			domain = args[i]
		case "--username":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			username = args[i]
		case "--password":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			password = args[i]
		case "--config":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			conf, err = getConfig(args[i])
			if err != nil {
				log.Fatalf("[ERROR] Failed to read/parse config file. Error: '%s'\n", err.Error())
			}
		case "--privkey":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			privKeyPath = args[i]
		case "--cert":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			certPath = args[i]
		case "--keysize":
			i++
			if i >= length {
				showSignedUsage()
				os.Exit(1)
			}
			keySize, err = strconv.Atoi(args[i])
			if err != nil {
				log.Fatalf("[ERROR] Invalid key size provided\n")
			}
		case "--insecure":
			disableSSL = true
		case "--help":
			showSignedUsage()
			os.Exit(0)
		default:
			showSignedUsage()
			os.Exit(1)
		}
	}

	if conf == nil {
		log.Printf("[ERROR] No config file provided\n\n")
		showSignedUsage()
		os.Exit(1)
	}

	if privKeyPath == "" {
		log.Printf("[ERROR] No private key file provided. To generate a new private key, provide a new file: --privkey /path/new-privkey.pem\n\n")
		showSignedUsage()
		os.Exit(1)
	}

	if username == "" || password == "" || domain == "" {
		var err error
		username, password, err = afdclient.VmAfdGetMachineAccountInfo()
		if err != nil {
			log.Fatalf("[ERROR] Failed to get machine account credentials. Error: '%s'\n", err.Error())
		}

		split := strings.Split(username, ".")
		domain = strings.Join(split[1:], ".")
	}

	hostname = host

	return
}

func showSignedUsage() {
	fmt.Println("Usage: certificate signed { arguments }")
	fmt.Println("Arguments:")
	fmt.Println("  Required:")
	fmt.Println("    --config    <path to .json config file>")
	fmt.Println("    --privkey   <path to get or store private key>")
	fmt.Println("                - If file exists, private key from the file will be used")
	fmt.Println("                - If file does not exists, new private key will be created and stored in the given file location")
	fmt.Println("  Optional:")
	fmt.Println("    --server    <server ip/hostname>")
	fmt.Println("                Default: localhost")
	fmt.Println("    --domain    <fully qualified domain name>")
	fmt.Println("                Default: Derived from machine account username")
	fmt.Println("    --username  <account name>")
	fmt.Println("                Default: Machine account username")
	fmt.Println("    --password  <password>")
	fmt.Println("                Default: Machine account password")
	fmt.Println("    --cert      <path to store certificate, overwrite if exists>")
	fmt.Println("                Default: None. Certificate printed to console")
	fmt.Println("    --keysize   <The size of private key to be generated (use with --privkey /path/new-privkey.pem)>")
	fmt.Println("                Default: 2048")
	fmt.Println("    --insecure (Use this to skip certificate validation)")
}
