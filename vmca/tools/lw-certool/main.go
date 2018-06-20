package main

import (
	"fmt"
	"os"
)

func main() {
	if len(os.Args) < 2 {
		showUsage()
		os.Exit(1)
	}

	switch os.Args[1] {
	case "getsignedcert":
		cert := GenerateSignedCert(os.Args[2:])
		if certPath == "" {
			fmt.Println(cert)
		} else {
			fmt.Println("Certificate saved to file: " + certPath)
		}

	case "getrootcert":
		cert := GetRootCert(os.Args[2:])
		fmt.Println(cert)

	case "--help":
		showUsage()

	default:
		showUsage()
		os.Exit(1)
	}
}

func showUsage() {
	fmt.Println("Usage: lw-certool COMMAND { arguments }")
	fmt.Println("Commands:")
	fmt.Println("    getsignedcert    [get signed certificate]")
	fmt.Println("    getrootcert      [get root certificate]")
	fmt.Println("")
	fmt.Println("Run 'lw-certool COMMAND --help' for more information on a particular command")
}
