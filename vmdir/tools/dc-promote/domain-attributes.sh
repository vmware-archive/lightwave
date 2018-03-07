cat <<NNNN> /tmp/domain-attributes.ldif
version: 1
dn: DOMAIN_DN
changetype: modify
add: maxPwdAge
maxPwdAge: 14
NNNN
ldapmodify -Y GSSAPI -a -f /tmp/domain-attributes.ldif

