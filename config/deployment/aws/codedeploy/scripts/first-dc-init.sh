#!/bin/bash -e

#############################################################################
# Lightwave Standalone DC Manual Cascade Deployment Script
#
# _IMPORTANT_: This script is to be run only _ONCE_ to promote and configure
#              the first Lightwave DC for Cascade
#############################################################################

source $(dirname $(realpath $0))/common.sh

main()
{
  if [[ $# -eq 0 ]]
  then
    echo "ERROR: argument error" >&2
    usage
    exit 1
  fi

  _OPT=$(getopt -o 'hp:' --long 'help,admin-pass:' -n 'first-dc-init.sh' -- "$@")
  if [[ $? -ne 0 ]]
  then
    echo "ERROR: argument error" >&2
    usage
    exit 1
  fi
  eval set -- "${_OPT}"

  while true ; do
    case "$1" in
      -p|--admin-pass) ADMINISTRATOR_PASS=$2 ; shift 2 ;;
      -h|--help) usage ; exit 0 ;;
      --) shift ; break ;;
      *) echo "ERROR: argument error" >&2 ; usage ; exit 1 ;;
    esac
  done

  echo ""
  echo "Loading cloud variables"
  load_variables
  load_credentials

  echo ""
  echo "Configuring first Lightwave node"
  promote_as_first_node

  echo ""
  echo "Successfully configured first Lightwave node! :)"
}

usage()
{
  echo -e "Lightwave First DC Initialization Tool For Cascade\n"
  echo -e "Usage:"
  echo -e "\tfirst-dc-init.sh [--help]"
  echo -e "\t                 --admin-pass=<admin-password>"
  echo -e ""
  echo -e "Options:"
  echo -e "\t-p, --admin-pass\tLightwave system tenant administrator password"
  echo -e "\t-h, --help\tDisplay this message"
}

main "$@"
