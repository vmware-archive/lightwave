//go:generate go run assets_generator.go

package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"path"
	"strings"
	"time"

	"github.com/shurcooL/vfsgen"
)

func main() {

	assetsFile := path.Join(".", "..", "static", "config_vfsdata.go")
	err := os.MkdirAll(path.Join(".", "..", "static"), os.ModePerm)
	if err != nil {
		log.Fatalln(err)
	}	
	err = os.Remove(assetsFile)
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

	fs := &splicedFileSystem{
		name:     "/",
		selfPath: "/",
		dirs: map[string]http.FileSystem{
			"configs": http.Dir(
				path.Join(
					os.Getenv("GOPATH"),
					"src", "github.com", "vmware", "lightwave", "sts", "configs")),
			"init": http.Dir(
				path.Join(
					os.Getenv("GOPATH"),
					"src", "github.com", "vmware", "lightwave", "sts", "init")),
		},
	}

	err = vfsgen.Generate(
		fs,
		vfsgen.Options{
			PackageName:  "static",
			VariableName: "SetupAssets",
			Filename:     assetsFile,
		})
	if err != nil {
		log.Fatalln(err)
	}

}

type splicedFileSystem struct {
	name     string
	selfPath string
	dirs     map[string]http.FileSystem
}

func (d *splicedFileSystem) Read([]byte) (int, error) {
	return 0, fmt.Errorf("cannot Read from directory %s", d.name)
}
func (d *splicedFileSystem) Close() error               { return nil }
func (d *splicedFileSystem) Stat() (os.FileInfo, error) { return d, nil }

func (d *splicedFileSystem) Name() string       { return d.name }
func (d *splicedFileSystem) Size() int64        { return 0 }
func (d *splicedFileSystem) Mode() os.FileMode  { return 0755 | os.ModeDir }
func (d *splicedFileSystem) ModTime() time.Time { return time.Now() }
func (d *splicedFileSystem) IsDir() bool        { return true }
func (d *splicedFileSystem) Sys() interface{}   { return nil }

func (d *splicedFileSystem) Seek(offset int64, whence int) (int64, error) {
	return 0, nil
}

func (d *splicedFileSystem) Readdir(count int) ([]os.FileInfo, error) {
	res := make([]os.FileInfo, 0, len(d.dirs))
	for name, fs := range d.dirs {
		file, err := fs.Open("/")
		if err != nil {
			fmt.Println("error for file " + name)
			return []os.FileInfo{}, err
		}
		fi, err := file.Stat()
		if err != nil {
			fmt.Println("error for file stat" + name)
			return []os.FileInfo{}, err
		}

		res = append(res, fi)
	}
	return res, nil
}

func (d *splicedFileSystem) Open(filepath string) (http.File, error) {
	if strings.EqualFold(filepath, d.selfPath) {
		return d, nil
	}

	for name, fs := range d.dirs {
		if strings.HasPrefix(filepath, path.Join(d.selfPath, name)) {
			return fs.Open(path.Join("/", strings.TrimPrefix(filepath, path.Join(d.selfPath, name))))
		}
	}

	return nil, fmt.Errorf("No such file or directory: " + filepath)
}
