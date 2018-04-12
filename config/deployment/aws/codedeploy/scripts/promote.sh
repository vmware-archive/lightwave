#!/bin/sh -e

###
# Configure lightwave instance as a standalone
promote_as_first_node()
{
  echo "/opt/vmware/bin/configure-lightwave-server --domain ${LW_DOMAIN} --username ${LW_USERNAME} --site ${SITE}"
  /opt/vmware/bin/configure-lightwave-server \
      --domain "${LW_DOMAIN}" \
      --username "${ADMINISTRATOR_USER}" \
      --password "${ADMINISTRATOR_PASS}" \
      --site ${SITE}
  generate_ssl_cert
  add_dns_forwarder
}

###
# Configure lightwave instance to join as a partner
promote_as_partner_node()
{
  echo "/opt/vmware/bin/configure-lightwave-server --domain ${LW_DOMAIN} --username ${DOMAIN_PROMOTER_USER} --site ${SITE} --partner ${PARTNER}"
  /opt/vmware/bin/configure-lightwave-server \
      --domain "${LW_DOMAIN}" \
      --username "${DOMAIN_PROMOTER_USER}" \
      --password "${DOMAIN_PROMOTER_PASS}" \
      --site ${SITE} \
      --partner ${PARTNER}
  generate_ssl_cert
  fix_topology
}

###
# Generates SSL cert for new node and adds to vecs
generate_ssl_cert()
{
  VECS_DIR="/var/lib/vmware/vmafd/vecs"
  SSL_STORE="MACHINE_SSL_CERT"
  DEFAULT_CERT="__MACHINE_CERT"

  echo "Delete default machine cert"
  /opt/vmware/bin/vecs-cli entry delete --store ${SSL_STORE} --alias ${DEFAULT_CERT} -y

  echo "Enable Multiple SAN option"
  /opt/vmware/bin/certool --enableserveroption --option=multiplesan

  echo "Generate the public/private key pair for certificate signing"
  /opt/vmware/bin/certool \
      --genkey \
      --privkey=${VECS_DIR}/key.pem \
      --pubkey=${VECS_DIR}/pub_key.pem

  echo "Create the certificate config file using template"
  cp ${CONFIGDIR}/cert.cfg ./
  sed -i "s/@@SAN_ENTRY@@/${SAN_ENTRY}/" cert.cfg
  sed -i "s/@@HOSTNAME@@/${HOSTNAME}/" cert.cfg

  echo "Generating SSL cert"
  /opt/vmware/bin/certool \
      --gencert \
      --config=cert.cfg \
      --cert=${VECS_DIR}/cert.pem \
      --privkey=${VECS_DIR}/key.pem \
      --server localhost

  echo "Adding cert to vecs store"
  /opt/vmware/bin/vecs-cli entry create \
      --store ${SSL_STORE} \
      --alias ${DEFAULT_CERT} \
      --cert ${VECS_DIR}/cert.pem \
      --key ${VECS_DIR}/key.pem

  rm cert.cfg ${VECS_DIR}/key.pem ${VECS_DIR}/pub_key.pem ${VECS_DIR}/cert.pem
}

###
# Configure a DNS forwarder to the public DNS
add_dns_forwarder()
{
  echo "/opt/vmware/bin/vmdns-cli add-forwarder ${PUB_DNS} --domain ${LW_DOMAIN} --username ${DOMAIN_PROMOTER_USER}"
  /opt/vmware/bin/vmdns-cli add-forwarder ${PUB_DNS} \
      --server localhost \
      --domain "${LW_DOMAIN}" \
      --username "${DOMAIN_PROMOTER_USER}" \
      --password "${DOMAIN_PROMOTER_PASS}"
}

###
# Fixes broken replication topology using vdcrepadmin command
fix_topology()
{
  +e # TODO (PR: 1982635): Remove after this PR is resolved

  # fix intra-region replication topoloy
  echo "/opt/vmware/bin/vdcrepadmin -f enableredundanttopology -h ${HOSTNAME} -u ${DOMAIN_PROMOTER_USER} -s ${SITE} -n"
  /opt/vmware/bin/vdcrepadmin \
      -f enableredundanttopology \
      -h ${HOSTNAME} \
      -u "${DOMAIN_PROMOTER_USER}" \
      -w "${DOMAIN_PROMOTER_PASS}" \
      -s "${SITE}" \
      -n

  # fix inter-region replication topoloy
  echo "/opt/vmware/bin/vdcrepadmin -f enableredundanttopology -h ${HOSTNAME} -u ${DOMAIN_PROMOTER_USER} -n"
  /opt/vmware/bin/vdcrepadmin \
      -f enableredundanttopology \
      -h ${HOSTNAME} \
      -u "${DOMAIN_PROMOTER_USER}" \
      -w "${DOMAIN_PROMOTER_PASS}" \
      -n

  -e # TODO (PR: 1982635): Remove after this PR is resolved
}
