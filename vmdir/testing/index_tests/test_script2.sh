REL_DIR=`dirname $0`
cd $REL_DIR

source ../header.sh

host="localhost"
port=389
admindn="cn=administrator,cn=users,dc=vmware,dc=com"
adminpw='123'

op=$1

if (( "$op" == 0 || "$op" == 2 ))
then

# 0.
# any prep here
./generate_data_big.sh > ./data.ldif
ldapadd -c -h $host -p $port -x -D $admindn -w $adminpw -f ./data.ldif > ./addObjects.out5 2>&1

fi

if (( "$op" == 1 || "$op" == 2 ))
then

# 1.
# start index
echo "*** Start indexing for uid"
ldapmodify -c -h $host -p $port -x -D $admindn -w $adminpw <<EOM
dn: cn=uid,cn=schemacontext
changetype: modify
add: searchFlags
searchFlags: 1
-
add: vmwAttrUniquenessScope
vmwAttrUniquenessScope: ou=org-A,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-B,dc=vmware,dc=com
vmwAttrUniquenessScope: ou=org-C,dc=vmware,dc=com
EOM
echo

fi



