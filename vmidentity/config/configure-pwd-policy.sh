#!/bin/bash

usage() {
    echo -e "Configure tenant password policy"
    echo -e "\tUsage:"
    echo -e "\t./configure-pwd-policy.sh -t TENANT_NAME -b TENANT_BASE_DN -u TENANT_ADMIN -p TENANT_PWD -d PWD_LIFETIME_IN_DAYS"
}

while getopts "t:b:u:p:d:h" opt ; do
    case "${opt}" in
        t)  TENANT_NAME="${OPTARG}" ;;
        b)  TENANT_DN="${OPTARG}" ;;
        u)  TENANT_ADMIN="${OPTARG}" ;;
        p)  TENANT_PWD="${OPTARG}" ;;
        d)  PWD_LIFETIME="${OPTARG}" ;;
        h)
            usage
            exit 0
            ;;
        \?)
            echo "ERROR! Invalid option -${OPTARG}" >&2
            usage
            exit 1
            ;;
        :)
            echo "ERROR! Option -${OPTARG} requires an argument" >&2
            usage
            exit 1
            ;;
    esac
done

if [ -z $TENANT_NAME ] || [ -z $TENANT_DN ] || [ -z $TENANT_ADMIN ] || [ -z $TENANT_PWD ] || [ -z $PWD_LIFETIME ]; then
    echo "ERROR! One or more arguments are missing." >&2
    usage
    exit 1
fi

#print out current setting
echo "Current policy:"
ldapsearch -h localhost -Y SRP -U $TENANT_ADMIN -w $TENANT_PWD -o ldif-wrap=no -b "cn=password and lockout policy,$TENANT_DN" '*'
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

# ldap modify the entry
echo "Updating policy..."
cat << EOF > /tmp/pwdlifetimemod.txt
dn: cn=password and lockout policy,$TENANT_DN
changetype: modify
replace: vmwPasswordLifetimeDays
vmwPasswordLifetimeDays: $PWD_LIFETIME
EOF

cat /tmp/pwdlifetimemod.txt

ldapmodify -h localhost -Y SRP -U $TENANT_ADMIN -w $TENANT_PWD -f /tmp/pwdlifetimemod.txt
rc=$?

rm /tmp/pwdlifetimemod.txt

if [[ $rc != 0 ]]; then exit $rc; fi

# print out final setting
echo "New policy:"
ldapsearch -h localhost -Y SRP -U $TENANT_ADMIN -w $TENANT_PWD -o ldif-wrap=no -b "cn=password and lockout policy,$TENANT_DN" '*'
