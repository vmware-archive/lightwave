cat <<NNNN> /tmp/domain-attributes.ldif
version: 1
dn: DOMAIN_DN
changetype: modify
add: maxPwdAge
maxPwdAge: 14
NNNN
ldapmodify -a -D cn=administrator,cn=users,dc=lightwave,dc=local -f /tmp/domain-attributes.ldif -h localhost -w VMware123@
