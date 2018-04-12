#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  if [[ "${PROMOTED}" == true ]]
  then
    start_cron
    end_upgrade
  fi
}

###
# Sets HA state to available, so clients may affinitize to current instance
end_upgrade()
{
  echo "Sleeping for 60 seconds for replication to catch up"
  sleep 60

  echo "/opt/vmware/bin/vmafd-cli end-upgrade --server-name localhost --user-name ${DOMAIN_PROMOTER_USER}"
  /opt/vmware/bin/vmafd-cli end-upgrade \
      --server-name localhost \
      --user-name "${DOMAIN_PROMOTER_USER}" \
      --password "${DOMAIN_PROMOTER_PASS}"
}

main "$@"
