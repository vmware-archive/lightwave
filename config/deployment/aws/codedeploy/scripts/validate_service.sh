#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  if [[ "${PROMOTED}" == true ]]
  then
    is_idm_healthy
  fi
}

main "$@"
