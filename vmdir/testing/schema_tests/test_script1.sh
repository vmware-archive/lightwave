REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

echo "#########################################################"
echo "#####              TEST ATTRIBUTE TYPE              #####"
echo "#########################################################"
echo

echo "##############################"
echo "ADD POSITIVE"
echo "##############################"
echo

echo "###"
echo "Add new attribute type(s) with attributeID, cn, attributeSyntax, and isSingleValued"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrAttr001,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.001
cn: goodStrAttr001
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr002,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.002
cn: goodStrAttr002
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr003,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.003
cn: goodStrAttr003
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr004,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.004
cn: goodStrAttr004
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr005,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.005
cn: goodStrAttr005
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr006,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.006
cn: goodStrAttr006
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr007,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.007
cn: goodStrAttr007
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr008,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.008
cn: goodStrAttr008
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodStrAttr009,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.0.009
cn: goodStrAttr009
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE

dn: cn=goodIntAttr001,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.001
cn: goodIntAttr001
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr002,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.002
cn: goodIntAttr002
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr003,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.003
cn: goodIntAttr003
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr004,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.004
cn: goodIntAttr004
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr005,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.005
cn: goodIntAttr005
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr006,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.006
cn: goodIntAttr006
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr007,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.007
cn: goodIntAttr007
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr008,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.008
cn: goodIntAttr008
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE

dn: cn=goodIntAttr009,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 111.111.1.009
cn: goodIntAttr009
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.27
issinglevalued: TRUE
EOM


echo "##############################"
echo "ADD NEGATIVE - Missing Must"
echo "##############################"
echo

echo "###"
echo "Add a new attribute type without attributeID"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrAttr001,cn=schemacontext
changetype: add
objectclass: attributeschema
cn: badStrAttr001
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE
EOM

echo "###"
echo "Add a new attribute type without cn"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrAttr002,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 000.000.0.002
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
issinglevalued: TRUE
EOM

echo "###"
echo "Add a new attribute type without attributeSyntax"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrAttr004,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 000.000.0.004
cn: badStrAttr004
issinglevalued: TRUE
EOM

echo "###"
echo "Add a new attribute type without isSingleValued"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrAttr005,cn=schemacontext
changetype: add
objectclass: attributeschema
attributeid: 000.000.0.005
cn: badStrAttr005
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.15
EOM


echo "##############################"
echo "MODIFY POSITIVE"
echo "##############################"
echo

echo "###"
echo "Modify isDefunct from FALSE to TRUE"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrAttr008,cn=schemacontext
changetype: modify
add: isdefunct
isdefunct: TRUE
EOM

echo "###"
echo "Modify isSingleValued from TRUE to FALSE"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: issinglevalued
issinglevalued: FALSE
EOM


echo "##############################"
echo "MODIFY NEGATIVE - Incompatible"
echo "##############################"
echo

echo "###"
echo "Modify attributeID"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: attributeid
attributeid: 999.999.1.999
EOM

echo "###"
echo "Modify cn"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: cn
cn: badCn
EOM

echo "###"
echo "Modify attributeSyntax"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: attributesyntax
attributesyntax: 1.3.6.1.4.1.1466.115.121.1.36
EOM

echo "###"
echo "Modify schemaIDGUID (TBD - can be modified by admin)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: schemaidguid
schemaidguid: badId
EOM

echo "###"
echo "Modify isSingleValued from FALSE to TRUE"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodIntAttr007,cn=schemacontext
changetype: modify
replace: issinglevalued
issinglevalued: TRUE
EOM

echo "###"
echo "Modify isDefunct from TRUE to FALSE (TBD - not supported yet)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrAttr008,cn=schemacontext
changetype: modify
replace: isdefunct
isdefunct: FALSE
EOM


echo "#########################################################"
echo "#####               TEST OBJECT CLASS               #####"
echo "#########################################################"
echo

echo "##############################"
echo "ADD POSITIVE"
echo "##############################"
echo

echo "###"
echo "Add new object class(es) with governsID, cn, subClassOf (ABS), and objectClassCategory (ABS)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodAbsClass001,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.2.001
cn: goodAbsClass001
subclassof: top
objectclasscategory: 2
systemmustcontain: goodStrAttr001
systemmustcontain: goodStrAttr002
systemmaycontain: goodIntAttr001
systemmaycontain: goodIntAttr002

dn: cn=goodAbsClass002,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.2.002
cn: goodAbsClass002
subclassof: goodAbsClass001
objectclasscategory: 2
systemmustcontain: goodStrAttr003
systemmaycontain: goodIntAttr003

dn: cn=goodAbsClass003,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.2.003
cn: goodAbsClass003
subclassof: top
objectclasscategory: 2
EOM

echo "###"
echo "Add new object class(es) with governsID, cn, subClassOf (ABS), and objectClassCategory (AUX)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodAuxClass001,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.3.001
cn: goodAuxClass001
subclassof: goodAbsClass002
objectclasscategory: 3
systemmustcontain: goodStrAttr005

dn: cn=goodAuxClass003,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.3.003
cn: goodAuxClass003
subclassof: goodAbsClass003
objectclasscategory: 3
EOM

echo "###"
echo "Add new object class(es) with governsID, cn, subClassOf (ABS), and objectClassCategory (STR)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass001,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.4.001
cn: goodStrClass001
subclassof: goodAbsClass002
objectclasscategory: 1
systemmustcontain: goodStrAttr002
mustcontain: goodStrAttr004
systemmaycontain: goodIntAttr005
systemauxiliaryclass: goodAuxClass001

dn: cn=goodStrClass003,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.4.003
cn: goodStrClass003
subclassof: goodAbsClass003
objectclasscategory: 1
EOM

echo "###"
echo "Add new object class(es) with governsID, cn, subClassOf (AUX), and objectClassCategory (AUX)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodAuxClass002,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.3.002
cn: goodAuxClass002
subclassof: goodAuxClass001
objectclasscategory: 3
systemmaycontain: goodIntAttr004
EOM

echo "###"
echo "Add new object class(es) with governsID, cn, subClassOf (STR), and objectClassCategory (STR)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.4.002
cn: goodStrClass002
subclassof: goodStrClass001
objectclasscategory: 1
systemmustcontain: goodStrAttr003
mustcontain: goodStrAttr006
mustcontain: goodStrAttr007
auxiliaryclass: goodAuxClass002

dn: cn=goodStrClass004,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 111.111.4.004
cn: goodStrClass004
subclassof: goodStrClass001
objectclasscategory: 1
EOM


echo "##############################"
echo "ADD NEGATIVE - Missing Must"
echo "##############################"
echo

echo "###"
echo "Add a new object class without governsID"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrClass001,cn=schemacontext
changetype: add
objectclass: classschema
cn: badStrClass001
subclassof: top
objectclasscategory: 1
EOM

echo "###"
echo "Add a new object class without cn"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrClass002,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.002
subclassof: top
objectclasscategory: 1
EOM

echo "###"
echo "Add a new object class without subClassOf"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrClass003,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.003
cn: badStrClass003
objectclasscategory: 1
EOM

echo "###"
echo "Add a new object class without objectClassCategory"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrClass004,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.004
cn: badStrClass004
subclassof: top
EOM


echo "##############################"
echo "ADD NEGATIVE - Invalid Branch"
echo "##############################"
echo

echo "###"
echo "Add a new object class with governsID, cn, subClassOf (AUX), and objectClassCategory (ABS)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badAbsClass007,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.007
cn: badAbsClass007
subclassof: goodAuxClass002
objectclasscategory: 2
EOM

echo "###"
echo "Add a new object class with governsID, cn, subClassOf (AUX), and objectClassCategory (STR)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badStrClass008,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.008
cn: badStrClass008
subclassof: goodAuxClass002
objectclasscategory: 1
EOM

echo "###"
echo "Add a new object class with governsID, cn, subClassOf (STR), and objectClassCategory (ABS)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badAbsClass009,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.009
cn: badAbsClass009
subclassof: goodStrClass002
objectclasscategory: 2
EOM

echo "###"
echo "Add a new object class with governsID, cn, subClassOf (STR), and objectClassCategory (AUX)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badAuxClass010,cn=schemacontext
changetype: add
objectclass: classschema
governsid: 000.000.5.010
cn: badAuxClass010
subclassof: goodStrClass002
objectclasscategory: 3
EOM


echo "##############################"
echo "MODIFY POSITIVE"
echo "##############################"
echo

echo "###"
echo "Modify by adding attribute type(s) to systemMayContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
add: systemmaycontain
systemmaycontain: goodIntAttr006
EOM

echo "###"
echo "Modify by adding attribute type(s) to mayContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
add: maycontain
maycontain: goodIntAttr007
EOM

echo "###"
echo "Modify by adding object class(es) to systemAuxiliaryClass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass003,cn=schemacontext
changetype: modify
add: systemauxiliaryclass
systemauxiliaryclass: goodAuxClass003
EOM

echo "###"
echo "Modify by adding object class(es) to auxiliaryClass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
add: auxiliaryclass
auxiliaryclass: goodAuxClass003
EOM

echo "###"
echo "Modify defaultObjectCategory"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: defaultobjectcategory
defaultobjectcategory: cn=config
EOM

echo "###"
echo "Modify isDefunct from FALSE to TRUE"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass004,cn=schemacontext
changetype: modify
replace: isdefunct
isdefunct: TRUE
EOM


echo "##############################"
echo "MODIFY NEGATIVE - Incompatible"
echo "##############################"
echo

echo "###"
echo "Modify governsID"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: governsid
governsid: 999.999.4.999
EOM

echo "###"
echo "Modify cn"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: cn
cn: badCn
EOM

echo "###"
echo "Modify schemaIDGUID (TBD - can be modified by admin)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: schemaidguid
schemaidguid: badGuid
EOM

echo "###"
echo "Modify subClassOf"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: subclassof
subclassof: goodAbsClass001
EOM

echo "###"
echo "Modify objectClassCategory"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
replace: objectclasscategory
objectclasscategory: 2
EOM

echo "###"
echo "Modify by adding attribute type(s) to systemMustContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
add: systemmustcontain
systemmustcontain: goodIntAttr008
EOM

echo "###"
echo "Modify by adding attribute type(s) to mustContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
add: mustcontain
mustcontain: goodIntAttr009
EOM

echo "###"
echo "Modify by removing attribute type(s) from systemMustContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
delete: systemmustcontain
systemmustcontain: goodStrAttr003
EOM

echo "###"
echo "Modify by removing attribute type(s) from mustContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
delete: mustcontain
mustcontain: goodStrAttr006
EOM

echo "###"
echo "Modify by removing attribute type(s) from systemMayContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
delete: systemmaycontain
systemmaycontain: goodIntAttr006
EOM

echo "###"
echo "Modify by removing attribute type(s) from mayContain"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
delete: maycontain
maycontain: goodIntAttr007
EOM

echo "###"
echo "Modify by removing object class(es) from systemAuxiliaryClass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass001,cn=schemacontext
changetype: modify
delete: systemauxiliaryclass
systemauxiliaryclass: goodAuxClass001
EOM

echo "###"
echo "Modify by removing object class(es) from auxiliaryClass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass002,cn=schemacontext
changetype: modify
delete: auxiliaryclass
auxiliaryclass: goodAuxClass002
EOM

echo "###"
echo "Modify isDefunct from TRUE to FALSE (TBD - not supported yet)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodStrClass004,cn=schemacontext
changetype: modify
replace: isdefunct
isdefunct: FALSE
EOM


echo "#########################################################"
echo "#####                  TEST ENTRY                   #####"
echo "#########################################################"
echo

echo "##############################"
echo "ADD POSITIVE"
echo "##############################"
echo

echo "###"
echo "Add a new entry with a structural objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodIntAttr001: 1
goodIntAttr002: 2
goodIntAttr003: 3
EOM

echo "###"
echo "Add a new entry with a structural objectclass and an abstract objectclass from the same class branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry002,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAbsClass001
objectclass: goodStrClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodIntAttr001: 1
goodIntAttr002: 2
goodIntAttr003: 3
EOM

echo "###"
echo "Add a new entry with a structural objectclass and auxiliary objectclass(es) from the same class branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAuxClass001
objectclass: goodAuxClass002
objectclass: goodAuxClass003
objectclass: goodStrClass002
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodStrAttr005: mno
goodStrAttr006: pqr
goodStrAttr007: stu
goodIntAttr001: 1
goodIntAttr002: 2
goodIntAttr003: 3
EOM

echo "###"
echo "Add a new entry with a structural objectclass, an abstract objectclass, and an auxiliary objectclass from the same class branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry004,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAbsClass001
objectclass: goodAuxClass002
objectclass: goodStrClass002
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodStrAttr005: mno
goodStrAttr006: pqr
goodStrAttr007: stu
goodIntAttr001: 1
goodIntAttr002: 2
goodIntAttr003: 3
EOM

echo "###"
echo "Add a new entry with multiple structural objectclasses from the same class branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
objectclass: goodStrClass002
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodStrAttr006: pqr
goodStrAttr007: stu
goodIntAttr001: 1
goodIntAttr002: 2
goodIntAttr003: 3
EOM


echo "##############################"
echo "ADD NEGATIVE - Invalid Class Combination"
echo "##############################"
echo

echo "###"
echo "Add a new entry with an abstract objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry001,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAbsClass001
goodStrAttr001: abc
goodStrAttr002: def
EOM

echo "###"
echo "Add a new entry with an auxiliary objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry002,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAuxClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
EOM

echo "###"
echo "Add a new entry with an abstract objectclass and an auxiliary objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry003,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodAbsClass001
objectclass: goodAuxClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
EOM

echo "###"
echo "Add a new entry with a structural objectclass and an abstract objectclass from different class branches"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry004,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
objectclass: goodAbsClass003
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
EOM

echo "###"
echo "Add a new entry with a structural objectclass and an auxiliary objectclass from different class branches"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry005,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
objectclass: goodAuxClass003
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
EOM

echo "###"
echo "Add a new entry with multiple structural objectclass which are from different class branches"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry006,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
objectclass: goodStrClass003
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
EOM

echo "###"
echo "Add a new entry with an undefined objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry007,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: undefined
EOM


echo "##############################"
echo "ADD NEGATIVE - Invalid Attribute Combination"
echo "##############################"
echo

echo "###"
echo "Add a new entry with missing must attribute(s)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry008,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
EOM

echo "###"
echo "Add a new entry with an attribute that's not in any may/must list"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry009,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
goodStrAttr007: stu
EOM


echo "##############################"
echo "ADD NEGATIVE - Invalid Attribute Value"
echo "##############################"
echo

echo "###"
echo "Add a new entry with attribute(s) using incorrect syntax"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=badEntry010,cn=Users,dc=vmware,dc=com
changetype: add
objectclass: goodStrClass001
goodStrAttr001: abc
goodStrAttr002: def
goodStrAttr003: ghi
goodStrAttr004: jkl
goodIntAttr001: xyz
EOM


echo "##############################"
echo "MODIFY POSITIVE"
echo "##############################"
echo

echo "###"
echo "Modify by removing a non-violating auxiliary objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodAuxClass003
EOM

echo "###"
echo "Modify by adding a leaf's ancestor abstract objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodAbsClass001
EOM

echo "###"
echo "Modify by adding a valid auxiliary objectclass "
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodAuxClass001
-
add: goodStrAttr005
goodStrAttr005: mno
EOM

echo "###"
echo "Modify by removing an existing may attribute"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: modify
delete: goodIntAttr003
goodIntAttr003: 3
EOM

echo "###"
echo "Modify by adding a valid may attribute"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: modify
add: goodIntAttr004
goodIntAttr004: 4
EOM


echo "##############################"
echo "MODIFY NEGATIVE - Invalid Class Modification"
echo "##############################"
echo

echo "###"
echo "Modify by removing the leaf structural objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodStrClass002
EOM

echo "###"
echo "Modify by removing a must-condition violating auxiliary objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodAuxClass001
EOM

echo "###"
echo "Modify by removing a may-condition violating auxiliary objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodAuxClass002
EOM

echo "###"
echo "Modify by adding an abstract objectclass from another branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodAbsClass003
EOM

echo "###"
echo "Modify by adding a structural objectclass from another branch"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodStrClass003
EOM

echo "###"
echo "Modify by adding an invalid auxiliary objectclass "
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodAuxClass003
EOM

echo "###"
echo "Modify by adding an undefined objectclass"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: undefined
EOM


echo "##############################"
echo "MODIFY NEGATIVE - Non-Trivial Cases"
echo "##############################"
echo

echo "###"
echo "Modify by removing an abstract objectclass (we don't persist abstract object classes)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry002,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodAbsClass001
EOM

echo "###"
echo "Modify by removing a non-leaf structural objectclass (we don't allow struct oc mod)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry005,cn=Users,dc=vmware,dc=com
changetype: modify
delete: objectclass
objectclass: goodStrClass001
EOM

echo "###"
echo "Modify by adding a leaf's ancestor structural objectclass (we don't allow struct oc mod)"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry004,cn=Users,dc=vmware,dc=com
changetype: modify
add: objectclass
objectclass: goodStrClass001
EOM


echo "##############################"
echo "MODIFY NEGATIVE - Invalid Attribute Modification"
echo "##############################"
echo

echo "###"
echo "Modify by removing a must attribute"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry003,cn=Users,dc=vmware,dc=com
changetype: modify
delete: goodStrAttr001
goodStrAttr001: abc
EOM

echo "###"
echo "Modify by adding an undefined attribute"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: undefined
undefined: x
EOM

echo "###"
echo "Modify by adding multiple values to an isSingleValued may attribute"
echo "###"
echo

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 <<EOM
dn: cn=goodEntry001,cn=Users,dc=vmware,dc=com
changetype: modify
add: goodIntAttr005
goodIntAttr005: 5
goodIntAttr005: 6
EOM


