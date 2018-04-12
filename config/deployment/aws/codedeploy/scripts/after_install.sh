#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  create_lw_repo
  install_lightwave
  install_telegraf
}

###
# Create local repository using rpm files from the archive file
create_lw_repo()
{
  # re-point lightwave repo to /tmp/vmware/lightwave
  sed -i -e "s/https:\/\/.*/file:\/\/\/tmp\/vmware\/lightwave/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo

  # remove existing repo
  rm -rf /tmp/vmware/lightwave
  mkdir -p /tmp/vmware/lightwave/x86_64

  # symlink rpm files instead of cp
  find ${ROOTDIR} -name "*.rpm" |\
  while read org
  do
    f=`awk -F'/' '{ print $NF; }' <<< ${org}`
    tgt="/tmp/vmware/lightwave/x86_64/${f}"
    ln -nfs ${org} ${tgt}
  done

  createrepo "/tmp/vmware/lightwave"
  tdnf -yq makecache
}

###
# Check whether to upgrade or install lightwave, then tdnf install
# refresh lwsm after install/upgrade
install_lightwave()
{
  RPM_VERSION=$(rpm -qp --queryformat '%{VERSION}-%{RELEASE}' $(find ${ROOTDIR} -name "lightwave-server-*.rpm"))
  tdnf install -y lightwave-client-${RPM_VERSION} \
                  lightwave-server-${RPM_VERSION} \
                  lightwave-${RPM_VERSION}
}

###
# Install and configure telegraf
install_telegraf()
{
  # if WAVEFRONT_PROXY_ELB option is set, we need to install telegraf
  if [[ -n "${WAVEFRONT_PROXY_ELB}" ]]
  then
    cp ${CONFIGDIR}/wavefront_telegraf.repo /etc/yum.repos.d/wavefront_telegraf.repo
    tdnf makecache
    # (PR 2021327) install 1.4.0 until we resolve issue with 1.5.0 installation
    tdnf install -y telegraf-1.4.0~34b7a4c

    sed '/User=telegraf/s/^/#/g' /usr/lib/telegraf/scripts/telegraf.service >/usr/lib/systemd/system/telegraf.service
    sed "s/@@WAVEFRONT_PROXY@@/${WAVEFRONT_PROXY_ELB}/" /opt/vmware/share/config/telegraf.conf >/etc/telegraf/telegraf.conf

    find /opt/vmware -name "*-telegraf.conf" | xargs cp -t /etc/telegraf/telegraf.d

    systemctl daemon-reload
    systemctl restart telegraf
  fi
}

main "$@"
