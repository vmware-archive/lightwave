#!/bin/sh

###
# (Temporary) Get the current DNS list from the DHCP option
get_dns_list()
{
  DHCP_OPTION_ID=$(aws ec2 describe-vpcs \
      --region ${REGION} \
      --vpc-ids ${VPC_ID} \
      --query "Vpcs[].DhcpOptionsId" \
      --output text)
  echo "DHCP_OPTION_ID=${DHCP_OPTION_ID}"
  [[ ${DHCP_OPTION_ID} ]]

  DNS_IPS=($(aws ec2 describe-dhcp-options \
      --region ${REGION} \
      --filters "Name=dhcp-options-id,Values=${DHCP_OPTION_ID}" \
      --query "DhcpOptions[].DhcpConfigurations[?Key=='domain-name-servers'].Values[]" \
      --output text))
  echo "DNS_IPS=${DNS_IPS[@]}"
}

###
# (Temporary) Replace DNS list with the input string in the DHCP option
replace_dns_list()
{
  if [[ $# -ne 1 ]]
  then
    echo "Can't replace DNS list $# arguments - expected 1"
    return 1
  fi

  echo "Replacing DNS list with the input string $1"

  OLD_DHCP_OPTION_ID=$(aws ec2 describe-vpcs \
      --region ${REGION} \
      --vpc-ids ${VPC_ID} \
      --query "Vpcs[].DhcpOptionsId" \
      --output text)
  echo "OLD_DHCP_OPTION_ID=${OLD_DHCP_OPTION_ID}"
  [[ ${OLD_DHCP_OPTION_ID} ]]

  NEW_DHCP_OPTION_ID=$(aws ec2 create-dhcp-options \
      --region ${REGION} \
      --dhcp-configurations "Key=domain-name-servers,Values=$1" \
      --query "DhcpOptions.DhcpOptionsId" \
      --output text)
  echo "NEW_DHCP_OPTION_ID=${NEW_DHCP_OPTION_ID}"
  [[ ${NEW_DHCP_OPTION_ID} ]]

  echo "Apply the new dhcp option"
  aws ec2 associate-dhcp-options \
      --region ${REGION} \
      --vpc-id ${VPC_ID} \
      --dhcp-options-id ${NEW_DHCP_OPTION_ID}

  echo "Delete the old dhcp option"
  aws ec2 delete-dhcp-options \
      --region ${REGION} \
      --dhcp-options-id ${OLD_DHCP_OPTION_ID}

  echo "Waiting for AWS to synchronize..."
  sleep 10

  echo "Restart networkd and resolved"
  systemctl restart systemd-networkd systemd-resolved
}
