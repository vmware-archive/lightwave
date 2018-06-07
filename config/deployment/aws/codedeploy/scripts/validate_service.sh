#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  STATE=$(/opt/vmware/bin/vmafd-cli get-domain-state --server-name localhost)
  if [[ "${STATE}" == "Controller" ]]
  then
    is_idm_healthy
  fi
}

main "$@"
