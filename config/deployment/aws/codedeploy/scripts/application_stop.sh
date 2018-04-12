#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  begin_upgrade
  stop_services
}

###
# Sets HA state to unavailable and waits for clients to reaffinitize
begin_upgrade()
{
  if [[ -n `pgrep vmafdd` ]]
  then
    echo "/opt/vmware/bin/vmafd-cli begin-upgrade --server-name localhost --user-name ${DOMAIN_PROMOTER_USER}"
    /opt/vmware/bin/vmafd-cli begin-upgrade \
        --server-name localhost \
        --user-name "${DOMAIN_PROMOTER_USER}" \
        --password "${DOMAIN_PROMOTER_PASS}"

    echo "Sleeping for 60 seconds for clients to re-affinitize"
    sleep 60
  fi
}

main "$@"
