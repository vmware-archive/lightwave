#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  load_variables
  load_credentials
  update_dns_list_before_upgrade # temporary
  begin_upgrade
  stop_services
}

###
# (Temporary) Update DNS list before upgrade starts
update_dns_list_before_upgrade()
{
  get_dns_list
  find_asg_partners

  for _IP in ${PARTNER_IPS[@]}
  do
    if [[ ${DNS_IPS[0]} == ${_IP} ]]
    then
      echo "The current host is a non-primary DNS - skip"
      rm -f primary_dns
      return 0
    fi
  done

  echo "The current host is the primary DNS"
  touch primary_dns

  if [[ ${#PARTNER_IPS[@]} -gt 0 ]]
  then
    echo "Move the current host to the end of the DNS list"
    replace_dns_list $(string_join , ${PARTNER_IPS[@]:0:3} ${LOCAL_IPV4})
    echo "Sleep for 240 seconds for clients to pick up the new dhcp option"
    sleep 240
  else
    echo "No partner - use ${PUB_DNS} during upgrade"
    replace_dns_list ${PUB_DNS}
  fi
}

###
# Sets HA state to unavailable and waits for clients to reaffinitize
begin_upgrade()
{
  if [[ -n `pgrep vmafdd` ]]
  then
    echo "/opt/vmware/bin/vmafd-cli begin-upgrade" \
         "--server-name localhost" \
         "--user-name ${DOMAIN_PROMOTER_USER}"
    /opt/vmware/bin/vmafd-cli begin-upgrade \
        --server-name localhost \
        --user-name "${DOMAIN_PROMOTER_USER}" \
        --password "${DOMAIN_PROMOTER_PASS}"

    echo "Sleep for 60 seconds for clients to re-affinitize"
    sleep 60
  fi
}

main "$@"
