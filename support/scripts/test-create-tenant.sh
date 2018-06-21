#!/bin/bash

function print_usage() {
    echo "Usage: $0 SYSTEM_TENANT PASSWORD COUNT"
}

# Creates tenant in IDM
function create_tenant() {
    tok=$1
    tenant=$2
    curl -X POST --insecure \
      https://localhost/idm/tenant \
      -H "authorization: Bearer ${tok}" \
      -H "content-type: application/json" \
      -d "{
              \"name\":\"${tenant}\",
              \"username\":\"administrator@${tenant}\",
              \"password\":\"Ca\$hc0w1\"
        }"
}

# Deletes a tenant in IDM
function delete_tenant() {
    tok=$1
    tenant=$2
    curl -k -X DELETE --insecure \
      https://localhost/idm/tenant/"${tenant}" \
      -H "authorization: Bearer ${TOK}" \
      -H 'content-type: application/json'
}

function create_tenant_loop() {
    tok=$1
    loop=$2
    tag=$3

    TENANTS=()
    for ((i=1; i<=$loop; i++)); do
        rand=$(head /dev/urandom  | tr -dc A-Za-z0-9 | head -c 5 ; echo '')
        tenant="tenant-${tag}-${rand}"
        echo "Creating $tenant"
        create_tenant $tok $tenant
        echo ""
        TENANTS+=("${tenant}")
    done

    for tenant in "${TENANTS[@]}"
    do
        echo "Deleting $tenant"
        delete_tenant $tok $tenant
        echo ""
    done
}

if [ "$#" -ne 3 ]; then
    echo "Illegal number of params"
    print_usage
    exit 1
fi

SYSTEM_TENANT=$1
PASSWORD=$2
COUNT=$3

TOK=$(curl -X POST --insecure \
    https://localhost:443/openidconnect/token/${SYSTEM_TENANT} \
    -H "content-type: application/x-www-form-urlencoded" \
    -d "grant_type=password"\
    -d "username=administrator@${SYSTEM_TENANT}" \
    -d "password=${PASSWORD}" \
    -d "scope=openid offline_access id_groups at_groups rs_admin_server openid+rs_vmdir" \
    | cut -f1 -d, | cut -f2 -d: | tr -d '"')

for ((i=0; i<${COUNT}; i++)); do
    create_tenant_loop "${TOK}" 5 $i &
done

wait

echo ""
echo "Done"
