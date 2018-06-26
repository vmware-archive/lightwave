package main

import (
	"fmt"
	"os"
)

func main() {
	if len(os.Args) < 3 {
		showUsage()
		os.Exit(1)
	}

	if os.Args[1] != "ca" {
		showUsage()
		os.Exit(1)
	}

	switch os.Args[2] {
	case "getcert":
		cert := GenerateSignedCert(os.Args[3:])
		if certPath == "" {
			fmt.Println(cert)
		} else {
			fmt.Println("Certificate saved to file: " + certPath)
		}

	case "getroot":
		cert := GetRootCert(os.Args[3:])
		fmt.Println(cert)

	case "--help":
		showUsage()

	default:
		showUsage()
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println("Usage: lw-certool ca COMMAND { arguments }")
	fmt.Println("Commands:")
	fmt.Println("    getcert    [get signed certificate]")
	fmt.Println("    getroot    [get root certificate]")
	fmt.Println("")
	fmt.Println("Run 'lw-certool ca COMMAND --help' for more information on a particular command")
}
