REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

echo "##########################################"
echo "Tests with indexed attribute in the filter"
echo "##########################################"
echo
echo "###"
echo "Base object search with trivial filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "objectclass=*" dn

echo "###"
echo "Base object search with a simple filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "cn=John-100" dn

echo "###"
echo "Base object search with a substring INITIAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "cn=John-100*" dn

echo "###"
echo "Base object search with a substring FINAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "cn=*John-100" dn

echo "###"
echo "Base object search with a substring ANY filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "cn=*John-100*" dn

echo "###"
echo "One-level object search with trivial filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "objectclass=*" dn

echo "###"
echo "One-level object search with simple filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "cn=John-100" dn

echo "###"
echo "One-level object search with substring INITIAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "cn=John-100*" dn

echo "###"
echo "One-level object search with substring FINAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "cn=*John-100" dn

echo "###"
echo "One-level object search with substring ANY filter => 0 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "cn=*John-100*" dn

echo "###"
echo "Subtree search with trivial filter => 103 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s subtree "objectclass=*" dn

echo "###"
echo "Subtree search with simple filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-100" dn

echo "###"
echo "Subtree search with substring INITIAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=John-100*" dn

echo "###"
echo "Subtree search with substring FINAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=*john-100" dn

echo "###"
echo "Subtree search with substring ANY filter => 0 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "cn=*John*" dn

echo "##############################################"
echo "Tests with non-indexed attribute in the filter"
echo "##############################################"
echo
echo "###"
echo "Base object search with trivial filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "sn=*" dn

echo "###"
echo "Base object search with a simple filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "sn=smith" dn

echo "###"
echo "Base object search with a substring INITIAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "sn=smit*" dn

echo "###"
echo "Base object search with a substring FINAL filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "sn=*mith" dn

echo "###"
echo "Base object search with a substring ANY filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "cn=John-100,ou=eng,dc=vmware,dc=com" -s base "sn=*smith*" dn

echo "###"
echo "One-level object search with trivial filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "sn=*" dn

echo "###"
echo "One-level object search with simple filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "sn=smith" dn

echo "###"
echo "One-level object search with substring INITIAL filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "sn=smit*" dn

echo "###"
echo "One-level object search with substring FINAL filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "sn=*mith" dn

echo "###"
echo "One-level object search with substring ANY filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s onelevel "sn=*smith*" dn

echo "###"
echo "Subtree search with trivial filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "sn=*" dn

echo "###"
echo "Subtree search with simple filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "sn=smith" dn

echo "###"
echo "Subtree search with substring INITIAL filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "sn=smit*" dn

echo "###"
echo "Subtree search with substring FINAL filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "sn=*mith" dn

echo "###"
echo "Subtree search with substring ANY filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "sn=*smith*" dn

echo "######################################################################"
echo "OR Tests with mix of indexed and non-indexed attributes in the filter"
echo "######################################################################"
echo
echo "###"
echo "Subtree object search with indexed attributes in the filter => 2 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=vmware,dc=com" -s subtree "(|(cn=john-10)(cn=john-11))" dn

echo "###"
echo "Subtree object search with indexed attributes in the filter => 2 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=vmware,dc=com" -s subtree "(|(cn=john-10)(objectClass=organizationalUnit))" dn

echo "###"
echo "Subtree object search with a (sn) non-indexed attribute in the filter => 101 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=vmware,dc=com" -s subtree "(|(sn=smith)(objectClass=organizationalUnit))" dn

echo "###"
echo "Root subtree object search with a (sn) non-indexed attribute in the filter => 53 Server is unwilling to perform. Full scan of Entry DB is required. Refine your search."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "(|(sn=smith)(objectclass=organizationalunit))" dn

echo "###"
echo "Root subtree object search with a present filter => 53 Server is unwilling to perform. Full scan of Entry DB is required. Refine your search."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "(objectclass=*)" dn

echo "###"
echo "Subtree object search with a (sn) non-indexed attribute with a non-existing value in the filter => 1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "(|(sn=smithx)(cn=john-1))" dn

echo "###"
echo "Subtree object search with a (sn) non-indexed attribute in the filter => 100 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "(|(sn=smith)(cn=john-1))" dn

echo "###"
echo "Root subtree object search with a non-indexed EQUALITY filter => 53 Server is unwilling to perform. Full scan of Entry DB is required. Refine your search."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree "(sn=smith)" dn

echo "###"
echo "Subtree object search with an indexed EQUALITY (non-existing attribute value) OR ... filter =>  1 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "dc=com" -s subtree "(|(cn=smith)(objectclass=organizationalUnit))" dn

echo "###"
echo "Subtree object search with combination of OR and NOT filters =>  102 result entry expected."
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "ou=eng,dc=vmware,dc=com" -s subtree '(|(cn=smith)(!(objectclass=organizationalUnit)))' dn

echo "###"
echo "ROOT subtree object search with combination of OR and NOT filters => 53 Server is unwilling to perform. Full scan of Entry DB is required. Refine your search." 
echo "###"
ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w 123 -b "" -s subtree '(|(cn=smith)(!(objectclass=organizationalUnit)))' dn
