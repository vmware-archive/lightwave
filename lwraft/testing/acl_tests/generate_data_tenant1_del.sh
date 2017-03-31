#!/bin/sh

echo "dn: ou=eng,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

for i in {1..10}
do
   echo "dn: cn=John-$i,ou=eng,dc=tenant1,dc=com"
   echo "changetype: delete"
   echo ""
done

echo "dn: ou=eng,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=administrator,cn=users,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=password and lockout policy,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=ForeignSecurityPrincipals,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=users,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=Administrators,cn=Builtin,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=Users,cn=BuiltIn,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""

echo "dn: cn=BuiltIn,dc=tenant1,dc=com"
echo "changetype: delete"
echo ""


