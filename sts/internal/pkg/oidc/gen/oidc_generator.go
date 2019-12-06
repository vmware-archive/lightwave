//go:generate go run oidc_generator.go
// +build ignore
package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"text/template"
	"unicode"
	"unicode/utf8"

	"go/ast"
	"go/parser"
	"go/token"
	"log"
	"path"
)

func main() {

	enumGenFile := path.Join(".", "..", "protocol", "enums_gen.go")
	serGenFile := path.Join(".", "..", "protocol", "marshall_gen.go")
	err := os.Remove(enumGenFile)
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
	err = os.Remove(serGenFile)
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
	cg := &codeGen{
		codePath: path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "oidc", "protocol"),
		enumOutput: enumGenFile,
		serOutput:  serGenFile,
	}

	err = cg.Generate()
	if err != nil {
		log.Fatalln(err)
	}
}

func enumTemplate() string {
	bytes, err := ioutil.ReadFile(
		path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "oidc", "gen", "enum.go.tmpl"))
	if err != nil {
		log.Fatalf("Unable to read enum template file: %v", err)
	}

	return string(bytes)
}

func marshalTemplate() string {
	bytes, err := ioutil.ReadFile(
		path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "oidc", "gen", "marshal.go.tmpl"))
	if err != nil {
		log.Fatalf("Unable to read enum template file: %v", err)
	}

	return string(bytes)
}

type codeGen struct {
	codePath   string
	enumOutput string
	serOutput  string

	marshalTmpl   *template.Template
	enumConstTmpl *template.Template
	fset          *token.FileSet
	packageName   string
	sb            *strings.Builder // enum
	marshalSB     *strings.Builder

	enums    map[*ast.TypeSpec]*enumDef
	marshals map[*ast.TypeSpec]*encodingSpec
}

func (cg *codeGen) Generate() error {

	cg.sb = &strings.Builder{}
	cg.marshalSB = &strings.Builder{}

	cg.enumConstTmpl = template.Must(template.New("enumConst").Parse(enumTemplate()))
	cg.marshalTmpl = template.Must(template.New("marshal").Parse(marshalTemplate()))

	var f, err = os.Stat(cg.codePath)
	if err != nil {
		return err
	}
	cg.fset = token.NewFileSet()

	if f.IsDir() {
		pkgs, err := parser.ParseDir(cg.fset, cg.codePath, nil, parser.ParseComments)
		if err != nil {
			return err
		}
		// todo: preserve order of generated classes between runs
		for pkgName, pkg := range pkgs {
			//v := newVisitor(g, pkgName)
			cg.packageName = pkgName
			cg.enums = make(map[*ast.TypeSpec]*enumDef, 20)
			cg.marshals = make(map[*ast.TypeSpec]*encodingSpec, 20)
			cg.sb.WriteString(fmt.Sprintf(packageFmt, cg.packageName))
			cg.marshalSB.WriteString(fmt.Sprintf(serPackageFmt, cg.packageName))
			// range on files in package
			for _, file := range pkg.Files {
				ast.Inspect(file, cg.Visit)
			}
			for k, v := range cg.enums {
				cg.processEnumDef(k, v)
			}
			for k, v := range cg.marshals {
				cg.processSerialization(k, v)
			}
			cg.marshalSB.WriteString(serializationFooter)
		}
	} else {
		file, err := parser.ParseFile(cg.fset, cg.codePath, nil, parser.ParseComments)
		if err != nil {
			return err
		}
		cg.packageName = file.Name.Name
		cg.enums = make(map[*ast.TypeSpec]*enumDef, 20)
		cg.marshals = make(map[*ast.TypeSpec]*encodingSpec, 20)
		cg.sb.WriteString(fmt.Sprintf(packageFmt, cg.packageName))
		cg.marshalSB.WriteString(fmt.Sprintf(serPackageFmt, cg.packageName))
		ast.Inspect(file, cg.Visit)
		for k, v := range cg.enums {
			cg.processEnumDef(k, v)
		}
		for k, v := range cg.marshals {
			cg.processSerialization(k, v)
		}
		cg.marshalSB.WriteString(serializationFooter)
	}
	err = ioutil.WriteFile(cg.enumOutput, []byte(cg.sb.String()), 0644)
	err = ioutil.WriteFile(cg.serOutput, []byte(cg.marshalSB.String()), 0644)

	return nil
}

func (cg *codeGen) Visit(n ast.Node) bool {
	switch n := n.(type) {
	case *ast.GenDecl:
		{
			genDecl := (*ast.GenDecl)(n)
			if genDecl.Tok == token.TYPE {
				for _, t := range genDecl.Specs {
					typeSpec, ok := t.(*ast.TypeSpec)
					if ok {
						cg.processType(typeSpec, getOidcComments([]*ast.CommentGroup{genDecl.Doc, typeSpec.Comment}))
					}
				}
			}
			return false
		}
	default:
		{
			return true
		}
	}
}

func (cg *codeGen) processType(t *ast.TypeSpec, oidc []string) {
	for _, c := range oidc {
		log.Printf("Processing oidc tags: %v", c)
		ed, err := parseEnumDef(c)
		if err != nil {
			log.Fatalf("Failed generation: %v\n", err)
		}
		// enum definition
		if ed != nil {
			cg.enums[t] = ed
			//cg.processEnumDef(t, ed)
		}

		serSpec, err := parseTypeSerializationDef(c)
		if err != nil {
			log.Fatalf("Failed generation: %v\n", err)
		}
		// enum definition
		if serSpec != nil {
			//cg.processSerialization(t, serSpec)
			cg.marshals[t] = serSpec
		}
	}
}

func (cg *codeGen) processEnumDef(t *ast.TypeSpec, ed *enumDef) {
	ident, ok := t.Type.(*ast.Ident)
	if !ok {
		log.Fatalf("For enums only support built in uint types or string")
	}

	_, wasMapped := enumBaseTypes[ident.Name]
	if !(wasMapped) {
		log.Fatalf("For enums only support built in uint types or string: '%s'\n", ident.Name)
	}
	es := &enumSpec{
		TypeName:  t.Name.Name,
		UTypeName: ident.Name,
		IsSet:     ed.set,
		Values:    make([]enumValSpec, 0, len(ed.values)),
		ErrorType: ed.ErrorType,
	}
	if es.IsSet {
		if es.UTypeName == "string" {
			es.SetType = "string"
		} else {
			if len(ed.values) > 63 {
				log.Fatalf("enum sets with > 63 distinct enum values not yet supported: '%s'\n", ident.Name)
			} else if len(ed.values) <= 8 {
				es.SetType = "8"
			} else if len(ed.values) <= 16 {
				es.SetType = "16"
			} else if len(ed.values) <= 32 {
				es.SetType = "32"
			} else {
				es.SetType = "64"
			}
		}
	}
	for _, v := range ed.values {
		es.Values = append(es.Values, enumValSpec{ValueName: camelCase(v, true), Value: v})
	}

	err := cg.enumConstTmpl.Execute(cg.sb, es)
	if err != nil {
		log.Fatalf("Failed applying enum template: %v", err)
	}
	supportedTypes[es.TypeName] = []string{"enum", "enum"}
	supportedTypes[es.TypeName+"Set"] = []string{"enum", "enum"}
}

func (cg *codeGen) processSerialization(t *ast.TypeSpec, enc *encodingSpec) {
	log.Printf("Processing type for serialization: %s", t.Name.Name)
	enc.TypeName = t.Name.Name
	enc.Fields = make([]EncodingFieldSpec, 0, 10)
	structType, ok := t.Type.(*ast.StructType)
	if !ok {
		log.Printf("Skipping serialization for type - '%s' as this is not a structure type\n", t.Name.Name)
		return
	}

	if structType.Fields == nil || len(structType.Fields.List) == 0 {
		return
	}

	for _, f := range structType.Fields.List {
		if f != nil {
			var tagVal string
			var comments []string
			if f.Tag != nil && f.Tag.Kind == token.STRING && len(f.Tag.Value) > 0 {
				tagVal = getOidcComment(f.Tag.Value)
			}
			comments = getOidcComments([]*ast.CommentGroup{f.Comment})
			if len(tagVal) > 0 {
				comments = append(comments, tagVal)
			}

			for _, c := range comments {
				fs, err := parseFieldSerializationDef(c)
				if err != nil {
					log.Fatalf("Failed to parse field serialization comment: %v\n", err)
				}
				if fs == nil {
					continue
				}

				if len(fs.ErrorType) <= 0 {
					fs.ErrorType = enc.ErrorType
				}

				if len(f.Names) != 1 || f.Names[0] == nil || len(f.Names[0].Name) <= 0 {
					log.Fatalf("Structures with un-named or multiple field defs are not supported: %v\n", enc.TypeName)
				}

				fs.StructFieldName = f.Names[0].Name
				ft := f.Type
				switch ft.(type) {
				case (*ast.StarExpr):
					{
						ft = ft.(*ast.StarExpr).X
						fs.IsPointer = true
					}
				case (*ast.ArrayType):
					{
						ft = ft.(*ast.ArrayType).Elt
						fs.IsArray = true
					}
				}

				switch ft.(type) {
				case *ast.Ident:
					{
						fs.StructFieldType = ft.(*ast.Ident).Name
						if v, ok := supportedTypes[fs.StructFieldType]; ok {
							fs.StructFieldSimpleType = v[0]
							fs.StructFieldIntSize = v[1]
						} else {
							// unsupported types!
						}
					}
				case *ast.SelectorExpr:
					{ // support only qualified names
						sel := ft.(*ast.SelectorExpr)
						ident, ok := sel.X.(*ast.Ident)
						if !ok {
							log.Fatalf("Structures with un-supported field type: %v, field name=%v; type=%v\n",
								enc.TypeName, f.Names[0].Name, reflect.TypeOf(ft))
						}
						fs.StructFieldType = ident.Name + "." + sel.Sel.Name
						if v, ok := supportedTypes[fs.StructFieldType]; ok {
							fs.StructFieldSimpleType = v[0]
							fs.StructFieldIntSize = v[1]
						} else {
							// unsupported types!
						}
					}
				default:
					{
						log.Fatalf("Structures with un-supported field type: %v, field name=%v; type=%v\n",
							enc.TypeName, f.Names[0].Name, reflect.TypeOf(ft))
					}
				}

				enc.Fields = append(enc.Fields, *fs)
				break
			}
		}
		enc.FieldsNum = len(enc.Fields)
	}

	err := cg.marshalTmpl.Execute(cg.marshalSB, enc)
	if err != nil {
		log.Fatalf("Failed applying marshal template: %v", err)
	}
}

func getOidcComments(cgs []*ast.CommentGroup) []string {
	if len(cgs) <= 0 {
		return nil
	}

	res := make([]string, 0, 5)
	for _, cg := range cgs {
		if cg == nil || len(cg.List) <= 0 {
			continue
		}

		for _, val := range cg.List {
			if (val == nil) || len(val.Text) <= 0 {
				continue
			}
			v := getOidcComment(val.Text)
			if len(v) > 0 {
				res = append(res, v)
			}
		}
	}

	return res
}

func getOidcComment(val string) string {
	val = strings.TrimLeftFunc(val, unicode.IsSpace)
	if strings.HasPrefix(val, "//") {
		val = strings.TrimPrefix(val, "//")
		val = strings.TrimLeftFunc(val, unicode.IsSpace)
	} else if strings.HasPrefix(val, "`") {
		val = strings.TrimPrefix(val, "`")
		val = strings.TrimLeftFunc(val, unicode.IsSpace)
	}
	st := reflect.StructTag(val)
	v, ok := st.Lookup("oidc")
	if ok {
		return v
	}
	return ""
}

const (
	packageFmt = `// Code generated by oidc_gen. DO NOT EDIT.

package %s

import(
	"fmt"
	"math/bits"
	"strings"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

var(
	exists = struct{}{}
)
`

	serPackageFmt = `// Code generated by oidc_gen. DO NOT EDIT.

package %s

import(
	"net/url"
	"strconv"
	"fmt"
	"text/template"
	"io"
	"github.com/francoispqt/gojay"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)
`
	serializationFooter = `

const (
	PostFormID        = "oidcPostForm"
	formPostHtmlStart = ` + "`" + `<html>
  <head><title>Code Form Post</title></head>
  <body onload="document.getElementById('oidcPostForm').submit(); ">` + "`" + `

	formStartBegin = ` + "`" + `
    <form id="oidcPostForm" method="post" action="` + "`" + `
	formStartEnd = ` + "`" + `"/>` + "`" + `

	formend = ` + "`" + `
    </form>` + "`" + `

	inputField = ` + "`" + `<input type="hidden" name="%s" value="%s"/>` + "`" + `

	formPostHtmlEnd = ` + "`" + `  </body>
</html>` + "`" + `
)
`
)

//enum:vals=password refresh_token authorization_code client_credentials[,set];err=<type_of_error>
// enum:<part1>[;<part2>]
// vals=v1 v2 v3[,set]
// err=<type_of_error>
func parseEnumDef(str string) (*enumDef, error) {
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "enum:") {
		return nil, nil
	}
	str1 := str
	isSet := false
	vals := []string{}
	errType := ""

	str = strings.TrimPrefix(str, "enum:")
	str = strings.TrimSpace(str)
	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)
		if strings.HasPrefix(section, "vals=") {
			section = strings.TrimPrefix(section, "vals=")
			section = strings.TrimSpace(section)
			parts := strings.Split(section, ",")
			vals = strings.Fields(parts[0])
			if len(parts) > 1 {
				parts[1] = strings.TrimSpace(parts[1])
				isSet = parts[1] == "set"
			}
			continue
		}
		if strings.HasPrefix(section, "err=") {
			errType = readErrorType(section)
			continue
		}
	}
	if len(vals) <= 0 {
		return nil, fmt.Errorf("oidc enum tag missing values definition: '%s'", str1)
	}
	if len(errType) <= 0 {
		errType = "OidcErrorInvalidRequest"
	}
	ed := &enumDef{
		set:       isSet,
		values:    vals,
		ErrorType: errType,
	}
	return ed, nil
}

// marshal:<part1>[;<part2>]
// [enc=qjf[,url=<struct_field_name>]]
// [dec=qjf[,partial]]
// err=<type_of_error>
// ser:[enc=qjf[,url=<struct_field_name>]][,dec=qjf[,partial]],err=<type_of_error>
// encoding to query, json, form (any of) decoding from query json, form; propagate partial decode state on error
func parseTypeSerializationDef(str string) (*encodingSpec, error) {
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "marshal:") {
		return nil, nil
	}
	str1 := str

	var err error
	str = strings.TrimPrefix(str, "marshal:")
	str = strings.TrimSpace(str)
	spec := &encodingSpec{Fields: make([]EncodingFieldSpec, 10)}

	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)

		if strings.HasPrefix(section, "enc=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "enc="))
			parts := strings.Split(section, ",")

			spec.EncodingFormats, err = parseEncodings(parts[0], str1)
			if err != nil {
				return nil, err
			}
			if len(parts) > 1 {
				parts[1] = strings.TrimSpace(parts[1])
				if strings.HasPrefix(parts[1], "url=") {
					spec.FormActionFieldName = strings.TrimSpace(strings.TrimPrefix(parts[1], "url="))
				}
			}

			continue
		}

		if strings.HasPrefix(section, "dec=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "dec="))
			parts := strings.Split(section, ",")

			spec.DecodingFormats, err = parseEncodings(parts[0], str1)
			if err != nil {
				return nil, err
			}
			if len(parts) > 1 {
				parts[1] = strings.TrimSpace(parts[1])
				if strings.HasPrefix(parts[1], "partial") {
					spec.SupportPartialDecode = true
				}
			}

			continue
		}

		if strings.HasPrefix(section, "err=") {
			spec.ErrorType = readErrorType(section)
			continue
		}
	}

	if len(spec.ErrorType) <= 0 {
		spec.ErrorType = "OidcErrorInvalidRequest"
	}

	return spec, nil
}

func readErrorType(str string) string {
	errType := ""
	if strings.HasPrefix(str, "err=") {
		str = strings.TrimPrefix(str, "err=")
		errType = strings.TrimSpace(str)
	}
	return errType
}

// field
// marshal:<part1>[;<part2>]
// name=<name>
// omitempty
// err=<type_of_error>
// ser:fn=grant_type,omitempty
// encoding to query, json, form (any of) decoding from query json, form; propagate partial decode state on error
func parseFieldSerializationDef(str string) (*EncodingFieldSpec, error) {
	str1 := str
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "marshal:") {
		return nil, nil
	}

	str = strings.TrimPrefix(str, "marshal:")
	str = strings.TrimSpace(str)

	spec := &EncodingFieldSpec{}

	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)
		if strings.HasPrefix(section, "name=") {
			spec.SerFieldName = strings.TrimSpace(strings.TrimPrefix(section, "name="))
			continue
		}
		if section == "omitempty" {
			spec.OmitEmpty = true
			continue
		}
		if strings.HasPrefix(section, "err=") {
			spec.ErrorType = readErrorType(section)
			continue
		}
	}

	if len(spec.SerFieldName) <= 0 {
		return nil, fmt.Errorf("oidc marshal tag missing field serialization name: '%s'", str1)
	}

	return spec, nil
}

func parseEncodings(str string, orig string) (map[string]struct{}, error) {
	str = strings.TrimSpace(str)
	if len(str) <= 0 || len(str) > 3 {
		return nil, fmt.Errorf("oidc ser tag missing values for enc= definition: '%s'", orig)
	}
	mapRet := make(map[string]struct{}, len(str))
	if validEncoding(str[0]) {
		mapRet[string(str[0])] = struct{}{}
	} else {
		return nil, fmt.Errorf("oidc ser tag invalid format '%s'", orig)
	}

	if len(str) > 1 {
		if validEncoding(str[1]) {
			mapRet[string(str[1])] = struct{}{}
		} else {
			return nil, fmt.Errorf("oidc ser tag invalid format '%s'", orig)
		}

		if len(str) > 2 {
			if validEncoding(str[2]) {
				mapRet[string(str[2])] = struct{}{}
			} else {
				return nil, fmt.Errorf("oidc ser tag invalid format '%s'", orig)
			}
		}
	}

	return mapRet, nil
}

func validEncoding(b byte) bool {
	return b == 'q' || b == 'j' || b == 'f'
}

//enum,set,vals=password refresh_token authorization_code client_credentials

type enumDef struct {
	set       bool
	values    []string
	ErrorType string
}

type enumSpec struct {
	TypeName  string
	UTypeName string
	IsSet     bool
	SetType   string
	Values    []enumValSpec
	ErrorType string
}
type enumValSpec struct {
	ValueName string
	Value     string
}

var enumBaseTypes = map[string]string{
	"uint":   "uint",
	"uint8":  "uint8",
	"uint16": "int16",
	"uint32": "int32",
	"uint64": "int64",
	"string": "string",
}

type encodingSpec struct {
	TypeName             string
	EncodingFormats      map[string]struct{}
	DecodingFormats      map[string]struct{}
	SupportPartialDecode bool
	Fields               []EncodingFieldSpec
	FieldsNum            int
	FormActionFieldName  string
	ErrorType            string
}

type EncodingFieldSpec struct {
	StructFieldName       string
	StructFieldType       string
	StructFieldSimpleType string
	StructFieldIntSize    string
	IsArray               bool
	IsPointer             bool
	SerFieldName          string
	OmitEmpty             bool
	ErrorType             string
}

func camelCase(name string, exported bool) string {
	sb := &strings.Builder{}
	strs := strings.Split(name, "_")
	buf := make([]byte, utf8.UTFMax)
	cap := exported
	for _, s := range strs {
		for i, r := range s {
			if i == 0 {
				if cap && unicode.IsLower(r) {
					r = unicode.ToUpper(r)
				} else if !cap && unicode.IsUpper(r) {
					r = unicode.ToLower(r)
				}
			}

			len := utf8.EncodeRune(buf, r)
			sb.Write(buf[0:len])
		}
		cap = true
	}

	return sb.String()
}

// type to encoding/decoding
var supportedTypes = map[string][]string{
	"int":           []string{"int", ""},
	"int8":          []string{"int", "8"},
	"int16":         []string{"int", "16"},
	"int32":         []string{"int", "32"},
	"int64":         []string{"int", "64"},
	"uint":          []string{"uint", ""},
	"uint8":         []string{"uint", "8"},
	"uint16":        []string{"uint", "16"},
	"uint32":        []string{"uint", "32"},
	"uint64":        []string{"uint", "64"},
	"bool":          []string{"bool", ""},
	"string":        []string{"string", ""},
	"url.URL":       []string{"url.URL", ""},
	"stringSetImpl": []string{"array", ""},
}
