#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  load_variables
  load_credentials
  update_registry_values
  start_services
  enable_cdc

  # check state to see if localhost is already configured
  STATE=$(/opt/vmware/bin/vmafd-cli get-domain-state --server-name localhost)
  if [[ "${STATE}" == "None" ]]
  then
    # check if localhost is going to be the first node or a partner node
    if [[ -n ${PARTNER} ]]
    then
      promote_as_partner_node
    elif [[ -n ${ADMINISTRATOR_PASS} ]]
    then
      promote_as_first_node
    else
      echo "You must provide administrator credential to promote the first node. Skipping promote step."
      copy_first_dc_init_scripts
    fi
  fi
}

###
# Update and calibrate lightwave registry values
update_registry_values()
{
  # disable ldap simple bind to port 389
  replace_registry_val '[HKEY_THIS_MACHINE\services\vmdir\parameters]' AllowInsecureAuthentication REG_DWORD 0

  # HACK - otherwise it won't work for new nodes
  #        identity rpm install should create this path instead
  /opt/likewise/bin/lwregshell add_key '[HKEY_THIS_MACHINE\Software\VMware\Identity\Configuration]' || true

  # set SAN entry name as hostname for IDM
  replace_registry_val '[HKEY_THIS_MACHINE\Software\VMware\Identity\Configuration]' Hostname REG_SZ ${SAN_ENTRY}

  # Update vmafd registry
  replace_registry_val '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' DcCacheHeartBeat REG_DWORD 10
  replace_registry_val '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' DcCacheSyncInterval REG_DWORD 30

  # enable integrity background job
  add_registry_val '[HKEY_THIS_MACHINE\Services\vmdir\Parameters]' integrityChkJobIntervalInSec REG_DWORD 21600
  add_registry_val '[HKEY_THIS_MACHINE\Services\vmdir\Parameters]' integrityRptJobIntervalInSec REG_DWORD 86400
}

###
# Enable client affinity for DCs, for onboarding flow
enable_cdc()
{
  /opt/vmware/bin/cdc-cli client-affinity default
}

###
# Copy scripts to local directory for manual first node setup
copy_first_dc_init_scripts()
{
  mkdir -p /root/first-dc-init
  cp $(dirname $(realpath $0))/*.sh /root/first-dc-init/
  chmod +x /root/first-dc-init/*.sh
}

main "$@"
