REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -b "ou=eng,dc=vmware,dc=com" -s subtree "cn=*" dn

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -b "" -s subtree "cn=John-50" dn

./generate_data.sh > data.ldif

ldapmodify -c -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -f data.ldif

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -b "" -s subtree "cn=John-50" dn

ldapsearch -h $host -p $port -x -D "cn=Administrator,cn=Users,dc=vmware,dc=com" -w '123' -b "ou=eng,dc=vmware,dc=com" -s subtree "cn=*" dn
