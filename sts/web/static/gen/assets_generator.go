//go:generate go run assets_generator.go
// +build ignore
package main

import (
	"log"
	"net/http"
	"os"
	"path"

	"github.com/shurcooL/vfsgen"
)

func main() {

	assetsFile := path.Join(".", "..", "assets", "assets_vfsdata.go")
	err := os.Remove(assetsFile)
	if err != nil {
		if pe, ok := err.(*os.PathError); ok {
			if os.IsNotExist(pe.Err) {
				err = nil
			}
		}
		if err != nil {
			log.Fatalln(err)
		}
	}

	err = vfsgen.Generate(
		http.Dir(
			path.Join(
				os.Getenv("GOPATH"),
				"src", "github.com", "vmware", "lightwave", "sts", "web", "static", "assets")),
		vfsgen.Options{
			PackageName:  "assets",
			VariableName: "Assets",
			Filename:     assetsFile,
		})
	if err != nil {
		log.Fatalln(err)
	}

	tempatesFile := path.Join(".", "..", "templates", "templates_vfsdata.go")

	err = os.Remove(tempatesFile)
	if err != nil {
		if pe, ok := err.(*os.PathError); ok {
			if os.IsNotExist(pe.Err) {
				err = nil
			}
		}
		if err != nil {
			log.Fatalln(err)
		}
	}

	err = vfsgen.Generate(
		http.Dir(
			path.Join(
				os.Getenv("GOPATH"),
				"src", "github.com", "vmware", "lightwave", "sts", "web", "static", "templates")),
		vfsgen.Options{
			PackageName:  "templates",
			VariableName: "Templates",
			Filename:     tempatesFile,
		})
	if err != nil {
		log.Fatalln(err)
	}

}
