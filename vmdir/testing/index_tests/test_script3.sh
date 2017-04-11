REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

host="localhost"
port=389
admindn="cn=administrator,cn=users,dc=vmware,dc=com"
adminpw='123'



./generate_data_big_bad.sh > ./bad_data.ldif
ldapadd -c -h $host -p $port -x -D $admindn -w $adminpw -f ./bad_data.ldif > ./addObjects.out6 2>&1


