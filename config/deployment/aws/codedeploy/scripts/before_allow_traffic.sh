#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  STATE=$(/opt/vmware/bin/vmafd-cli get-domain-state --server-name localhost)
  if [[ "${STATE}" == "Controller" ]]
  then
    load_variables
    load_credentials
    start_cron
    update_dns_list_after_upgrade # temporary
    end_upgrade
  fi
}

###
# (Temporary) Update DNS list after upgrade is complete
update_dns_list_after_upgrade()
{
  if [[ -f primary_dns ]]
  then
    echo "The current host is the primary DNS"
    echo "Move the current host to the beginnig of the DNS list"
    find_asg_partners
    replace_dns_list $(string_join , ${LOCAL_IPV4} ${PARTNER_IPS[@]:0:3})

    echo "Sleep for 180 seconds for clients to pick up the new dhcp option"
    sleep 180

    echo "Add scale in protection to the current host"
    aws autoscaling set-instance-protection \
        --region ${REGION} \
        --auto-scaling-group-name ${ASG} \
        --instance-ids ${INSTANCE_ID} \
        --protected-from-scale-in

    echo "Remove scale in protection from other hosts"
    for _ID in ${PARTNER_IDS[@]}
    do
      aws autoscaling set-instance-protection \
          --region ${REGION} \
          --auto-scaling-group-name ${ASG} \
          --instance-ids ${_ID} \
          --no-protected-from-scale-in
    done
  fi
}

###
# Sets HA state to available, so clients may affinitize to current instance
end_upgrade()
{
  echo "Sleep for 60 seconds for clients to re-affinitize"
  sleep 60

  echo "/opt/vmware/bin/vmafd-cli end-upgrade" \
       "--server-name localhost" \
       "--user-name ${DOMAIN_PROMOTER_USER}"
  /opt/vmware/bin/vmafd-cli end-upgrade \
      --server-name localhost \
      --user-name "${DOMAIN_PROMOTER_USER}" \
      --password "${DOMAIN_PROMOTER_PASS}"
}

main "$@"
