//go:generate go run idm_generator.go
// +build ignore
package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"sort"
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

	cg := &codeGen{
		codePath: []string{path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "idm", "types"),
			path.Join(
				os.Getenv("GOPATH"),
				"src", "github.com",
				"vmware", "lightwave", "sts", "internal", "pkg", "idm", "config")},
		enumOutput: "types_gen.go",
		ldapOutput: path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "idm", "config", "ldap_gen.go"),
		ldapPackage:    "config",
		marshalPackage: "rest",
		marshalOutput: path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "rest", "marshal_gen.go"),
	}

	err := cg.Cleanup()
	if err != nil {
		log.Fatalln(err)
	}

	err = cg.Generate()
	if err != nil {
		log.Fatalln(err)
	}
}

type codeGen struct {
	codePath       []string
	enumOutput     string // name
	ldapOutput     string // full path
	ldapPackage    string
	marshalPackage string
	marshalOutput  string // full path

	marshalTmpl   *template.Template
	ldapTmpl      *template.Template
	enumConstTmpl *template.Template
	fset          *token.FileSet
	packageName   string
	sb            *strings.Builder // enum
	marshalSB     *strings.Builder
	ldapSB        *strings.Builder

	enums        map[*ast.TypeSpec]*enumDef
	marshals     map[*ast.TypeSpec]*encodingSpec
	marshalTypes map[string]*encodingSpec
	ldaps        map[*ast.TypeSpec]*LdapSpec
	ldapTypes    map[string]*LdapSpec
}

func (cg *codeGen) Cleanup() error {
	for _, cp := range cg.codePath {
		err := removeFile(path.Join(cp, cg.enumOutput))
		if err != nil {
			return err
		}
	}
	if len(cg.marshalOutput) > 0 {
		err := removeFile(cg.marshalOutput)
		if err != nil {
			return err
		}
	}
	err := removeFile(cg.ldapOutput)
	if err != nil {
		return err
	}
	return nil
}

func removeFile(path string) error {
	err := os.Remove(path)
	if err != nil {
		if pe, ok := err.(*os.PathError); ok {
			if os.IsNotExist(pe.Err) {
				err = nil
			}
		}
		if err != nil {
			return err
		}
	}
	return nil
}

func (cg *codeGen) Generate() error {

	cg.ldapSB = &strings.Builder{}
	cg.marshalSB = &strings.Builder{}
	cg.ldaps = make(map[*ast.TypeSpec]*LdapSpec, 20)
	cg.ldapTypes = make(map[string]*LdapSpec, 20)
	cg.marshalTypes = make(map[string]*encodingSpec, 20)
	cg.marshals = make(map[*ast.TypeSpec]*encodingSpec, 20)

	ldapImport := ""
	marshalImport := ""

	cg.enumConstTmpl = template.Must(template.New("enumConst").Parse(enumTemplate()))
	cg.ldapTmpl = template.Must(template.New("ldap").Parse(ldapTemplate()))
	cg.marshalTmpl = template.Must(template.New("marshal").Parse(marshalTemplate()))

	for _, cp := range cg.codePath {
		var f, err = os.Stat(cp)
		if err != nil {
			return err
		}
		cg.fset = token.NewFileSet()

		if f.IsDir() {
			pkgs, err := parser.ParseDir(cg.fset, cp, nil, parser.ParseComments)
			if err != nil {
				return err
			}
			if len(pkgs) > 1 {
				log.Fatalf("multiple packages in same directory unsupported")
			}
			for pkgName, pkg := range pkgs {
				cg.enums = make(map[*ast.TypeSpec]*enumDef, 20)
				cg.sb = &strings.Builder{}
				cg.packageName = pkgName
				cg.sb.WriteString(fmt.Sprintf(packageFmt, cg.packageName))
				if cg.packageName != cg.ldapPackage {
					ind := strings.Index(cp, "github.com")
					ldapImport = ldapImport + "\n	\"" + string([]byte(cp)[ind:]) + "\""
					marshalImport = marshalImport + "\n	\"" + string([]byte(cp)[ind:]) + "\""
				}
				// range on files in package
				for _, file := range pkg.Files {
					ast.Inspect(file, cg.Visit)
				}
				for k, v := range cg.enums {
					cg.processEnumDef(k, v)
				}
			}
		} else {
			file, err := parser.ParseFile(cg.fset, cp, nil, parser.ParseComments)
			if err != nil {
				return err
			}
			cg.enums = make(map[*ast.TypeSpec]*enumDef, 20)
			cg.packageName = file.Name.Name
			cg.sb.WriteString(fmt.Sprintf(packageFmt, cg.packageName))
			if cg.packageName != cg.ldapPackage {
				ind := strings.Index(cp, "github.com")
				ldapImport = ldapImport + "\n	\"" + string([]byte(cp)[ind:]) + "\""
				marshalImport = marshalImport + "\n	\"" + string([]byte(cp)[ind:]) + "\""
			}
			ast.Inspect(file, cg.Visit)
			for k, v := range cg.enums {
				cg.processEnumDef(k, v)
			}
		}
		if len(cg.enums) > 0 {
			err = ioutil.WriteFile(path.Join(cp, cg.enumOutput), []byte(cg.sb.String()), 0644)
			if err != nil {
				return err
			}
		}
	}

	cg.marshalSB.WriteString(fmt.Sprintf(serPackageFmt, cg.marshalPackage, marshalImport))
	cg.ldapSB.WriteString(fmt.Sprintf(ldapPackageFmt, cg.ldapPackage, ldapImport))

	marshalTypes := make([]*encodingSpec, 0, len(cg.marshals)+10)
	for k, v := range cg.marshals {
		log.Printf("marshal type: '%v', '%v'", k.Name.Name, v.TypeName)
		cg.processSerialization(k, v, &marshalTypes)
	}
	sort.Slice(marshalTypes, func(i, j int) bool {
		return marshalTypes[i].TypeName < marshalTypes[j].TypeName
	})
	for _, v := range marshalTypes {
		cg.genMarshal(v)
	}

	if len(marshalTypes) > 0 && len(cg.marshalOutput) > 0 {
		err := ioutil.WriteFile(cg.marshalOutput, []byte(cg.marshalSB.String()), 0644)
		if err != nil {
			return err
		}
	}

	for k, v := range cg.ldaps {
		cg.preProcessLdap(k, v)
	}

	ldapTypes := make([]*LdapSpec, 0, len(cg.ldaps)+10)
	for k, v := range cg.ldaps {
		log.Printf("ldap type: '%v', '%v'", k.Name.Name, v.TypeName)
		cg.processLdap(k, v, &ldapTypes)
	}
	for _, v := range ldapTypes {
		log.Printf("ldap type: '%v'", v.TypeName)
	}
	sort.Slice(ldapTypes, func(i, j int) bool {
		return ldapTypes[i].TypeName < ldapTypes[j].TypeName
	})
	for _, v := range ldapTypes {
		cg.genLdap(v)
	}

	if len(cg.ldapOutput) > 0 {
		err := ioutil.WriteFile(cg.ldapOutput, []byte(cg.ldapSB.String()), 0644)
		if err != nil {
			return err
		}
	}

	return nil
}

func ldapTemplate() string {
	bytes, err := ioutil.ReadFile(
		path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "idm", "gen", "ldap.go.tmpl"))
	if err != nil {
		log.Fatalf("Unable to read ldap template file: %v", err)
	}

	return string(bytes)
}

func enumTemplate() string {
	bytes, err := ioutil.ReadFile(
		path.Join(
			os.Getenv("GOPATH"),
			"src", "github.com",
			"vmware", "lightwave", "sts", "internal", "pkg", "idm", "gen", "enum.go.tmpl"))
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
			"vmware", "lightwave", "sts", "internal", "pkg", "idm", "gen", "marshal.go.tmpl"))
	if err != nil {
		log.Fatalf("Unable to read enum template file: %v", err)
	}

	return string(bytes)
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
						cg.processType(typeSpec, getIdmComments([]*ast.CommentGroup{genDecl.Doc, typeSpec.Comment}))
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

func (cg *codeGen) processType(t *ast.TypeSpec, idm []string) {
	for _, c := range idm {
		log.Printf("Processing idm tags: %v", c)
		ed, err := parseEnumDef(c)
		if err != nil {
			log.Fatalf("Failed generation: %v\n", err)
		}
		// enum definition
		if ed != nil {
			cg.enums[t] = ed
		}

		serSpec, err := parseTypeSerializationDef(c)
		if err != nil {
			log.Fatalf("Failed generation: %v\n", err)
		}
		// enum definition
		if serSpec != nil {
			serSpec.packageName = cg.packageName
			cg.marshals[t] = serSpec
			cg.marshalTypes[serSpec.TypeName] = serSpec
			if serSpec.Map {
				supportedTypes[serSpec.TypeName+"Map"] = []string{"json", "json"}
			} else {
				supportedTypes[serSpec.TypeName] = []string{"json", "json"}
			}
		}
		ldapSpec, err := parseLdapDef(c)
		if err != nil {
			log.Fatalf("Failed generation: %v\n", err)
		}
		// ldap definition
		if ldapSpec != nil {
			ldapSpec.packageName = cg.packageName
			cg.ldaps[t] = ldapSpec
			if ldapSpec.Map {
				ldapSupportedTypes[ldapSpec.TypeName+"Map"] = []string{"ldap", "ldap"}
			} else {
				ldapSupportedTypes[ldapSpec.TypeName] = []string{"ldap", "ldap"}
			}
			cg.ldapTypes[ldapSpec.TypeName] = ldapSpec
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
	ldapSupportedTypes[es.TypeName] = []string{"enum", "String", "Strings", "StringsPtr", ""}
	if es.IsSet {
		supportedTypes[es.TypeName+"Set"] = []string{"enum", "enum"}
		ldapSupportedTypes[es.TypeName+"Set"] = []string{"enumSet", "Strings", "", "", ""}
	}
}

func (cg *codeGen) processSerialization(t *ast.TypeSpec, enc *encodingSpec, specs *[]*encodingSpec) {
	log.Printf("Processing type for serialization: %s", t.Name.Name)

	if len(enc.TypeName) <= 0 {
		enc.TypeName = t.Name.Name
	}
	if enc.packageName != cg.marshalPackage {
		enc.QualifiedTypeName = enc.packageName + "." + enc.TypeName
		enc.BuilderFunction = enc.packageName + "." + enc.BuilderFunction
		enc.MapBuilderFunction = enc.packageName + "." + enc.MapBuilderFunction
		enc.ErrorType = enc.packageName + "." + enc.ErrorType
	} else {
		enc.QualifiedTypeName = enc.TypeName
	}

	enc.Fields = make([]*EncodingFieldSpec, 0, 10)
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
				tagVal = getIdmComment(f.Tag.Value)
			}
			comments = getIdmComments([]*ast.CommentGroup{f.Doc, f.Comment})
			if len(tagVal) > 0 {
				comments = append(comments, tagVal)
			}

			for _, c := range comments {
				log.Printf("marshal: checking field '%s' comment:%s", f.Names[0].Name, c)
				attr, err := parseFieldSerializationDef(c)
				if err != nil {
					log.Fatalf("Failed to parse attribute definition comment: %v\n", err)
				}
				if attr == nil {
					continue
				}

				log.Printf("marshal: matched comment:%s", c)

				if len(attr.ErrorType) <= 0 {
					attr.ErrorType = enc.ErrorType
				}

				if len(f.Names) != 1 || f.Names[0] == nil || len(f.Names[0].Name) <= 0 {
					log.Fatalf("Structures with un-named or multiple field defs are not supported: %v\n", enc.TypeName)
				}

				if len(attr.Getter) <= 0 {
					attr.Getter = f.Names[0].Name
				}
				if len(attr.Setter) <= 0 {
					attr.Setter = f.Names[0].Name
				}
				ft := f.Type

				switch ft.(type) {
				case (*ast.ArrayType):
					{
						ft = ft.(*ast.ArrayType).Elt
						attr.IsArray = true
					}
				}

				switch ft.(type) {
				case (*ast.StarExpr):
					{
						ft = ft.(*ast.StarExpr).X
						attr.IsPointer = true
					}
				}

				switch ft.(type) {
				case *ast.Ident:
					{
						attr.StructFieldType = ft.(*ast.Ident).Name
						if v, ok := supportedTypes[attr.StructFieldType]; ok {
							attr.StructFieldSimpleType = v[0]
							attr.StructFieldIntSize = v[1]
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
						attr.StructFieldType = ident.Name + "." + sel.Sel.Name
						if v, ok := supportedTypes[attr.StructFieldType]; ok {
							attr.StructFieldSimpleType = v[0]
							attr.StructFieldIntSize = v[1]
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

				if enc.packageName != cg.marshalPackage &&
					(attr.StructFieldSimpleType == "enum" || attr.StructFieldSimpleType == "enumSet" || attr.StructFieldSimpleType == "stringer") {
					index := strings.Index(attr.StructFieldType, ".")
					if index == -1 {
						attr.StructFieldType = enc.packageName + "." + attr.StructFieldType
					}
				}

				if attr.AsChild {
					enc.Children = append(enc.Children, attr)
					mT, ok := cg.marshalTypes[attr.StructFieldType]
					if ok {
						attr.ChildSpec = mT
						//mT.parent = enc
						if attr.IsArray {
							if !mT.Map {
								mT.ChildArray = true
							}
						}
					} else {
						if strings.HasSuffix(attr.StructFieldType, "Map") {
							mT, ok := cg.marshalTypes[strings.TrimSuffix(attr.StructFieldType, "Map")]
							if ok {
								//mT.parent = enc
								attr.ChildSpec = mT
								if attr.IsArray {
									log.Fatalf("marshal: unsupported base type for child array of %s", attr.StructFieldType)
								}
							} else {
								log.Fatalf("marshal: unsupported base type for child %s", attr.StructFieldType)
							}
						} else {
							log.Fatalf("marshal: unsupported base type for child %s", attr.StructFieldType)
						}
					}
				} else {
					enc.Fields = append(enc.Fields, attr)
				}
				break
			}
		}
		enc.FieldsNum = len(enc.Fields)
		enc.ChildrenNumber = len(enc.Children)
	}

	*specs = append(*specs, enc)
	for _, name := range enc.subtypes {
		name = strings.TrimSpace(name)
		stSpec := *enc
		stSpec.subtypes = []string{}
		stSpec.SubType = true
		stSpec.TypeName = name
		if stSpec.packageName != cg.marshalPackage {
			stSpec.QualifiedTypeName = stSpec.packageName + "." + stSpec.TypeName
		}
		stSpec.ParentTypeName = enc.TypeName
		stSpec.QualifiedParentTypeName = enc.QualifiedTypeName
		stSpec.ChildrenNumber = 0
		stSpec.Children = []*EncodingFieldSpec{}
		stSpec.Fields = make([]*EncodingFieldSpec, 0, enc.FieldsNum)
		if stSpec.packageName != cg.marshalPackage {
			stSpec.QualifiedTypeName = stSpec.packageName + "." + stSpec.TypeName
		}
		for _, a := range enc.Fields {
			if contains(a.inSubtypes, name) {
				stSpec.FieldsNum = stSpec.FieldsNum + 1
				stSpec.Fields = append(stSpec.Fields, a)
			}
		}
		*specs = append(*specs, &stSpec)
	}
	/*	enc.TypeName = t.Name.Name
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
					tagVal = getIdmComment(f.Tag.Value)
				}
				comments = getIdmComments([]*ast.CommentGroup{f.Comment})
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
	*/
}

func (cg *codeGen) preProcessLdap(t *ast.TypeSpec, enc *LdapSpec) {
	if (enc != nil) && (enc.DirectEncrypt || enc.ChildEncrypt) {
		p := enc.parent
		for p != nil {
			p.ChildEncrypt = true
			p = p.parent
		}
	}
}

func (cg *codeGen) processLdap(t *ast.TypeSpec, enc *LdapSpec, specs *[]*LdapSpec) {
	log.Printf("Processing type for ldap: %s", t.Name.Name)
	if len(enc.TypeName) <= 0 {
		enc.TypeName = t.Name.Name
	}
	if enc.packageName != cg.ldapPackage {
		enc.QualifiedTypeName = enc.packageName + "." + enc.TypeName
		enc.BuilderFunction = enc.packageName + "." + enc.BuilderFunction
		enc.MapBuilderFunction = enc.packageName + "." + enc.MapBuilderFunction
	} else {
		enc.QualifiedTypeName = enc.TypeName
	}

	enc.Attributes = make([]*LdapAttributeSpec, 0, 10)
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
				tagVal = getIdmComment(f.Tag.Value)
			}
			comments = getIdmComments([]*ast.CommentGroup{f.Doc, f.Comment})
			if len(tagVal) > 0 {
				comments = append(comments, tagVal)
			}

			for _, c := range comments {
				attr, err := parseAttributeDef(c)
				if err != nil {
					log.Fatalf("Failed to parse attribute definition comment: %v\n", err)
				}
				if attr == nil {
					continue
				}

				//if len(fs.ErrorType) <= 0 {
				//	fs.ErrorType = enc.ErrorType
				//}

				if attr.LdapAttribute == "cn" {
					enc.CN = attr
				}

				if attr.Encrypt {
					enc.DirectEncrypt = true
				}

				if len(f.Names) != 1 || f.Names[0] == nil || len(f.Names[0].Name) <= 0 {
					log.Fatalf("Structures with un-named or multiple field defs are not supported: %v\n", enc.TypeName)
				}

				if len(attr.Getter) <= 0 {
					attr.Getter = f.Names[0].Name
				}
				if len(attr.Setter) <= 0 {
					attr.Setter = f.Names[0].Name
				}
				ft := f.Type

				switch ft.(type) {
				case (*ast.ArrayType):
					{
						ft = ft.(*ast.ArrayType).Elt
						attr.IsArray = true
					}
				}

				switch ft.(type) {
				case (*ast.StarExpr):
					{
						ft = ft.(*ast.StarExpr).X
						attr.IsPointer = true
					}
				}

				switch ft.(type) {
				case *ast.Ident:
					{
						attr.BaseType = ft.(*ast.Ident).Name
						if v, ok := ldapSupportedTypes[attr.BaseType]; ok {
							attr.SimpleType = v[0]
							if attr.IsPointer && attr.IsArray {
								attr.CvType = v[3]
							} else if attr.IsArray {
								attr.CvType = v[2]
							} else if attr.IsPointer {
								attr.CvType = v[4]
							} else {
								attr.CvType = v[1]
							}
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
						attr.BaseType = ident.Name + "." + sel.Sel.Name
						if v, ok := ldapSupportedTypes[attr.BaseType]; ok {
							attr.SimpleType = v[0]
							if attr.IsPointer && attr.IsArray {
								attr.CvType = v[3]
							} else if attr.IsArray {
								attr.CvType = v[2]
							} else if attr.IsPointer {
								attr.CvType = v[4]
							} else {
								attr.CvType = v[1]
							}
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

				if enc.packageName != cg.ldapPackage && (attr.SimpleType == "enum" || attr.SimpleType == "enumSet" || attr.SimpleType == "stringer") {
					index := strings.Index(attr.BaseType, ".")
					if index == -1 {
						attr.BaseType = enc.packageName + "." + attr.BaseType
					}
				}

				if attr.AsChild {
					enc.Children = append(enc.Children, attr)
					ldapT, ok := cg.ldapTypes[attr.BaseType]
					if ok {
						attr.ChildSpec = ldapT
						ldapT.parent = enc
						if attr.IsArray {
							if !ldapT.Map {
								ldapT.ChildArray = true
							}
						}
					} else {
						if strings.HasSuffix(attr.BaseType, "Map") {
							ldapT, ok := cg.ldapTypes[strings.TrimSuffix(attr.BaseType, "Map")]
							if ok {
								ldapT.parent = enc
								attr.ChildSpec = ldapT
								if attr.IsArray {
									log.Fatalf("ldap: unsupported base type for child array of %s", attr.BaseType)
								}
							} else {
								log.Fatalf("ldap: unsupported base type for child %s", attr.BaseType)
							}
						} else {
							log.Fatalf("ldap: unsupported base type for child %s", attr.BaseType)
						}
					}
				} else {
					enc.Attributes = append(enc.Attributes, attr)
				}
				break
			}
		}
		enc.AttributesNumber = len(enc.Attributes)
		enc.UpdateableAttributesNumber = 0
		for _, a := range enc.Attributes {
			if a.Updateable && !(a.Readonly) {
				enc.UpdateableAttributesNumber = enc.UpdateableAttributesNumber + 1
			}
		}
		enc.ChildrenNumber = len(enc.Children)
		if enc.CN == nil && len(enc.ConstantCN) <= 0 {
			log.Fatalf("ldap: cn must be mapped or be specified as constant %s", enc.TypeName)
		}
	}

	*specs = append(*specs, enc)
	for _, name := range enc.subtypes {
		name = strings.TrimSpace(name)
		stSpec := *enc
		stSpec.subtypes = []string{}
		stSpec.SubType = true
		stSpec.TypeName = name
		if stSpec.packageName != cg.ldapPackage {
			stSpec.QualifiedTypeName = stSpec.packageName + "." + stSpec.TypeName
		}
		stSpec.ParentTypeName = enc.TypeName
		stSpec.ChildrenNumber = 0
		stSpec.UpdateableAttributesNumber = 0
		stSpec.Children = []*LdapAttributeSpec{}
		stSpec.Attributes = make([]*LdapAttributeSpec, 0, enc.AttributesNumber)
		if stSpec.packageName != cg.ldapPackage {
			stSpec.QualifiedTypeName = stSpec.packageName + "." + stSpec.TypeName
		}
		for _, a := range enc.Attributes {
			if contains(a.inSubtypes, name) {
				stSpec.AttributesNumber = stSpec.AttributesNumber + 1
				stSpec.Attributes = append(stSpec.Attributes, a)
				if a.Updateable && !(a.Readonly) {
					stSpec.UpdateableAttributesNumber = stSpec.UpdateableAttributesNumber + 1
				}
			}
		}
		*specs = append(*specs, &stSpec)
	}
	//cg.ldapTypes[enc.TypeName] = enc
}

func contains(list []string, val string) bool {
	val = strings.TrimSpace(val)
	for _, v := range list {
		if strings.TrimSpace(v) == val {
			return true
		}
	}
	return false
}

func (cg *codeGen) genMarshal(enc *encodingSpec) {
	log.Printf("Processing marshal for: %v; num Fields:%v", enc.TypeName, len(enc.Fields))
	err := cg.marshalTmpl.Execute(cg.marshalSB, enc)
	if err != nil {
		log.Fatalf("Failed applying marshal template: %v", err)
	}
}
func (cg *codeGen) genLdap(enc *LdapSpec) {
	err := cg.ldapTmpl.Execute(cg.ldapSB, enc)
	if err != nil {
		log.Fatalf("Failed applying ldap template: %v", err)
	}
}

func getIdmComments(cgs []*ast.CommentGroup) []string {
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
			v := getIdmComment(val.Text)
			if len(v) > 0 {
				res = append(res, v)
			}
		}
	}

	return res
}

func getIdmComment(val string) string {
	val = strings.TrimLeftFunc(val, unicode.IsSpace)
	if strings.HasPrefix(val, "//") {
		val = strings.TrimPrefix(val, "//")
		val = strings.TrimLeftFunc(val, unicode.IsSpace)
	} else if strings.HasPrefix(val, "`") {
		val = strings.TrimPrefix(val, "`")
		val = strings.TrimLeftFunc(val, unicode.IsSpace)
	}
	st := reflect.StructTag(val)
	v, ok := st.Lookup("idm")
	if ok {
		return v
	}

	return ""
}

const (
	enumImplTmpl = ``
	packageFmt   = `// Code generated by idm_gen. DO NOT EDIT.

package %s

import(
	"fmt"
	"math/bits"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

var(
	exists = struct{}{}
)
`

	ldapPackageFmt = `// Code generated by idm_gen. DO NOT EDIT.

package %s

import(%s
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm/ldap"
)
`
	serPackageFmt = `// Code generated by idm_gen. DO NOT EDIT.

package %s

import(
	"io"
	"time"
	"net/url"
	"github.com/francoispqt/gojay"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"%s
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
		errType = "IdmErrorInvalidArgument"
	}
	ed := &enumDef{
		set:       isSet,
		values:    vals,
		ErrorType: errType,
	}
	return ed, nil
}

// marshal:<part1>[;<part2>]
// [enc=j]
// [dec=j]
// err=<type_of_error>
// ser:[enc=j][,dec=j],err=<type_of_error>,t=Typename;subt=a,b,c
// encoding json decoding from json
func parseTypeSerializationDef(str string) (*encodingSpec, error) {
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "marshal:") {
		return nil, nil
	}
	str1 := str

	var err error
	str = strings.TrimPrefix(str, "marshal:")
	str = strings.TrimSpace(str)
	spec := &encodingSpec{Fields: make([]*EncodingFieldSpec, 10)}

	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)

		if section == "map" {
			spec.Map = true
		}

		if strings.HasPrefix(section, "enc=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "enc="))
			parts := strings.Split(section, ",")

			spec.EncodingFormats, err = parseEncodings(parts[0], str1)
			if err != nil {
				return nil, err
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

		if strings.HasPrefix(section, "subt=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "subt="))

			spec.subtypes = strings.Split(section, ",")

			continue
		}

		if strings.HasPrefix(section, "t=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "t="))
			spec.TypeName = section
			internal := false
			for _, c := range section {
				internal = unicode.IsLower(c)
				break
			}
			if internal {
				spec.BuilderFunction = "new"
				spec.MapBuilderFunction = "new"
			} else {
				spec.BuilderFunction = "New"
				spec.MapBuilderFunction = "New"
			}
			spec.BuilderFunction = spec.BuilderFunction + section + "Builder"
			spec.MapBuilderFunction = spec.MapBuilderFunction + section + "MapBuilder"

			continue
		}
	}

	if len(spec.ErrorType) <= 0 {
		spec.ErrorType = "IdmErrorInvalidArgument"
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
// m=ID;subt=a,b,c
// child
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
		if section == "child" {
			spec.AsChild = true

			continue
		}

		if strings.HasPrefix(section, "m=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "m="))
			spec.Getter = section
			spec.Setter = section

			continue
		}

		if strings.HasPrefix(section, "subt=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "subt="))

			spec.inSubtypes = strings.Split(section, ",")

			continue
		}
	}

	if len(spec.SerFieldName) <= 0 {
		return nil, fmt.Errorf("idm marshal tag missing field serialization name: '%s'", str1)
	}

	return spec, nil
}

func parseEncodings(str string, orig string) (map[string]struct{}, error) {
	str = strings.TrimSpace(str)
	if len(str) <= 0 || len(str) > 1 {
		return nil, fmt.Errorf("idm ser tag missing values for enc= definition: '%s'", orig)
	}
	mapRet := make(map[string]struct{}, len(str))
	if validEncoding(str[0]) {
		mapRet[string(str[0])] = struct{}{}
	} else {
		return nil, fmt.Errorf("idm ser tag invalid format '%s'", orig)
	}

	if len(str) > 1 {
		if validEncoding(str[1]) {
			mapRet[string(str[1])] = struct{}{}
		} else {
			return nil, fmt.Errorf("idm ser tag invalid format '%s'", orig)
		}

		if len(str) > 2 {
			if validEncoding(str[2]) {
				mapRet[string(str[2])] = struct{}{}
			} else {
				return nil, fmt.Errorf("idm ser tag invalid format '%s'", orig)
			}
		}
	}

	return mapRet, nil
}

func validEncoding(b byte) bool {
	return b == 'j'
}

// ldap:<part1>[;<part2>]
// [enc=qjf[,url=<struct_field_name>]]
// [dec=qjf[,partial]]
// err=<type_of_error>

// ldap:map:oc=vmwSTSAttributeMap;cn=ABC;ccn=Attributes;t=IDSAttribute;subt=a,b,c
// ldap:oc=vmwSTSIdentityStore;ccn=IdentityProviders;t=IDSConfig
func parseLdapDef(str string) (*LdapSpec, error) {
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "ldap:") {
		return nil, nil
	}

	//var err error
	str = strings.TrimPrefix(str, "ldap:")
	str = strings.TrimSpace(str)
	spec := &LdapSpec{Attributes: make([]*LdapAttributeSpec, 10)}

	if strings.HasPrefix(str, "map:") {
		spec.Map = true
		str = strings.TrimPrefix(str, "map:")
		str = strings.TrimSpace(str)
	}

	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)

		if strings.HasPrefix(section, "oc=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "oc="))
			spec.ObjectClass = section

			continue
		}

		if strings.HasPrefix(section, "cn=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "cn="))
			spec.ConstantCN = section

			continue
		}

		if strings.HasPrefix(section, "ccn=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "ccn="))
			spec.ContainerCN = section

			continue
		}

		if strings.HasPrefix(section, "subt=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "subt="))

			spec.subtypes = strings.Split(section, ",")

			continue
		}

		if strings.HasPrefix(section, "t=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "t="))
			spec.TypeName = section
			internal := false
			for _, c := range section {
				internal = unicode.IsLower(c)
				break
			}
			if internal {
				spec.BuilderFunction = "new"
				spec.MapBuilderFunction = "new"
			} else {
				spec.BuilderFunction = "New"
				spec.MapBuilderFunction = "New"
			}
			spec.BuilderFunction = spec.BuilderFunction + section + "Builder"
			spec.MapBuilderFunction = spec.MapBuilderFunction + section + "MapBuilder"

			continue
		}

		//if strings.HasPrefix(section, "err=") {
		//	spec.ErrorType = readErrorType(section)
		//	continue
		//}
	}

	//if len(spec.ErrorType) <= 0 {
	//	spec.ErrorType = "IdmErrorInvalidArgument"
	//}

	return spec, nil
}

// ldap:name=cn;u=false;ro=true;m=ID;subt=a,b,c
// ldap:child
func parseAttributeDef(str string) (*LdapAttributeSpec, error) {
	str = strings.TrimSpace(str)
	if !strings.HasPrefix(str, "ldap:") {
		return nil, nil
	}

	//var err error
	str = strings.TrimPrefix(str, "ldap:")
	str = strings.TrimSpace(str)
	spec := &LdapAttributeSpec{Required: true, Updateable: true}

	sections := strings.Split(str, ";")
	for _, section := range sections {
		section = strings.TrimSpace(section)

		if strings.HasPrefix(section, "name=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "name="))
			spec.LdapAttribute = section

			continue
		}

		if strings.HasPrefix(section, "u=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "u="))
			if section == "false" {
				spec.Updateable = false
			} else if section != "true" {
				return nil, fmt.Errorf("Invalid definition for updateable: %s", section)
			} else {
				spec.Updateable = true
			}

			continue
		}

		if strings.HasPrefix(section, "ro=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "ro="))
			if section == "true" {
				spec.Readonly = false
			} else if section != "false" {
				return nil, fmt.Errorf("Invalid definition for updateable: %s", section)
			} else {
				spec.Readonly = true
			}

			continue
		}

		if section == "omitempty" {
			spec.Required = false

			continue
		}

		if section == "encrypt" {
			spec.Encrypt = true

			continue
		}

		if section == "child" {
			spec.AsChild = true

			continue
		}

		if strings.HasPrefix(section, "m=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "m="))
			spec.Getter = section
			spec.Setter = section

			continue
		}

		if strings.HasPrefix(section, "subt=") {
			section = strings.TrimSpace(strings.TrimPrefix(section, "subt="))

			spec.inSubtypes = strings.Split(section, ",")

			continue
		}

		//if strings.HasPrefix(section, "err=") {
		//	spec.ErrorType = readErrorType(section)
		//	continue
		//}
	}

	//if len(spec.ErrorType) <= 0 {
	//	spec.ErrorType = "IdmErrorInvalidArgument"
	//}

	return spec, nil
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
	TypeName                string
	QualifiedTypeName       string
	EncodingFormats         map[string]struct{}
	DecodingFormats         map[string]struct{}
	SupportPartialDecode    bool
	Fields                  []*EncodingFieldSpec
	FieldsNum               int
	ErrorType               string
	Map                     bool
	Children                []*EncodingFieldSpec
	ChildrenNumber          int
	BuilderFunction         string
	MapBuilderFunction      string
	ChildArray              bool
	SubType                 bool
	ParentTypeName          string
	QualifiedParentTypeName string
	packageName             string
	parent                  *LdapSpec
	subtypes                []string
}

type EncodingFieldSpec struct {
	StructFieldType       string
	StructFieldSimpleType string
	StructFieldIntSize    string
	IsArray               bool
	IsPointer             bool
	SerFieldName          string
	OmitEmpty             bool
	ErrorType             string
	Setter                string
	Getter                string
	AsChild               bool
	ChildSpec             *encodingSpec
	inSubtypes            []string
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

type LdapSpec struct {
	TypeName                   string
	QualifiedTypeName          string
	ConstantCN                 string
	CN                         *LdapAttributeSpec
	ContainerCN                string
	ObjectClass                string
	Map                        bool
	Attributes                 []*LdapAttributeSpec
	AttributesNumber           int
	UpdateableAttributesNumber int
	Children                   []*LdapAttributeSpec
	ChildrenNumber             int
	BuilderFunction            string
	MapBuilderFunction         string
	ChildArray                 bool
	DirectEncrypt              bool
	ChildEncrypt               bool
	SubType                    bool
	ParentTypeName             string
	packageName                string
	parent                     *LdapSpec
	subtypes                   []string
}

type LdapAttributeSpec struct {
	Setter        string
	Getter        string
	BaseType      string
	LdapAttribute string
	Required      bool
	Updateable    bool
	Readonly      bool
	IsArray       bool
	IsPointer     bool
	AsChild       bool
	ChildSpec     *LdapSpec
	CvType        string
	SimpleType    string
	Encrypt       bool
	inSubtypes    []string
}

// type to encoding/decoding
var supportedTypes = map[string][]string{
	"int":               []string{"int", ""},
	"int8":              []string{"int", "8"},
	"int16":             []string{"int", "16"},
	"int32":             []string{"int", "32"},
	"int64":             []string{"int", "64"},
	"uint":              []string{"uint", ""},
	"uint8":             []string{"uint", "8"},
	"uint16":            []string{"uint", "16"},
	"uint32":            []string{"uint", "32"},
	"uint64":            []string{"uint", "64"},
	"bool":              []string{"bool", ""},
	"string":            []string{"string", ""},
	"url.URL":           []string{"url.URL", ""},
	"time.Duration":     []string{"time.Duration", ""},
	"IDSAttributeValue": []string{"stringer", ""},
	"diag.TenantID":     []string{"stringer", ""},
	"stringSetImpl":     []string{"array", ""},
}

var ldapSupportedTypes = map[string][]string{
	"int":               []string{"int", "Int", "Ints", "IntsPtr", ""},
	"uint32":            []string{"uint32", "Uint32", "", "", ""},
	"bool":              []string{"bool", "Bool", "Bools", "BoolsPtr", ""},
	"string":            []string{"string", "String", "Strings", "StringsPtr", ""},
	"url.URL":           []string{"url.URL", "Url", "Urls", "UrlsPtr", "UrlPtr"},
	"x509.Certificate":  []string{"x509.Certificate", "Cert", "Certs", "CertsPtr", "CertPtr"},
	"crypto.PrivateKey": []string{"crypto.PrivateKey", "PrivateKey", "", "", ""},
	"time.Duration":     []string{"time.Duration", "Duration", "", "", ""},
	"IDSAttributeValue": []string{"stringer", "String", "Strings", "StringsPtr", ""},
	"diag.TenantID":     []string{"stringer", "String", "Strings", "StringsPtr", ""},
}
