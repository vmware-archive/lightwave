#!/bin/bash -e

source $(dirname $(realpath $0))/common.sh

main()
{
  load_variables
  install_dependencies
  install_logging_dependencies
}

###
# Install simple lightwave dependencies which don't require configuration
install_dependencies()
{
  echo "Install generic depdendencies"

  cp ${CONFIGDIR}/lightwave.repo /etc/yum.repos.d/lightwave.repo
  tdnf -yq makecache
  # required to create local yum repository
  tdnf install -yq sed zip unzip createrepo_c \
                   c-rest-engine-1.2-4.ph2 \
                   cyrus-sasl-2.1.26 \
                   glibc-devel-2.26 \
                   openssl-1.0.2n \
                   likewise-open-6.2.11
}

###
# Install and configure logging dependencies
install_logging_dependencies()
{
  # if LOGSTASH_ELB option is set, we need to install logging depedencies
  if [[ -n "${LOGSTASH_ELB}" ]]
  then
    echo "Install logging depdendencies"

    # install logging tools
    tdnf install -y syslog-ng logrotate
    if ! rpm -q filebeat
    then
      curl -L -O https://artifacts.elastic.co/downloads/beats/filebeat/filebeat-5.6.3-x86_64.rpm
      rpm -i filebeat-5.6.3-x86_64.rpm
      rm filebeat-5.6.3-x86_64.rpm
    fi

    # load systemd-journald config
    cp ${CONFIGDIR}/journald.conf /etc/systemd/journald.conf

    # remove the existing config of syslog-ng for lightwave
    # everything will be written to /var/log/messages
    sed -i '/lightwave.conf.d/d' /etc/syslog-ng/syslog-ng.conf
    if [[ -z $(grep "frac_digits(3)" /etc/syslog-ng/syslog-ng.conf) ]]
    then
        echo "" >> /etc/syslog-ng/syslog-ng.conf
        echo "options { frac_digits(3); };" >> /etc/syslog-ng/syslog-ng.conf
        echo "" >> /etc/syslog-ng/syslog-ng.conf
    fi

    # install logrotate and configure for syslog
    cp ${CONFIGDIR}/lightwave-syslog-logrotate.conf /etc/logrotate.d/lightwave-syslog-logrotate.conf
    chmod 444 /etc/logrotate.d/lightwave-syslog-logrotate.conf

    # configure an hourly cron job to rotate syslog
    # we don't need to keep these files as journald already has them,
    # and they're also forwarded to and saved in our ELK stack
    if ! [ -L "/etc/cron.hourly/logrotate" ] || ! [ -e "/etc/cron.hourly/logrotate" ]
    then
      /usr/bin/ln -s /etc/cron.daily/logrotate /etc/cron.hourly/logrotate
    fi

    # configuration steps referred from cascade controller codebase
    # custom directory for config files
    mkdir -p /etc/filebeat/conf.d
    # main config file specifying path to other custom ones
    sed "s/@@LOGSTASH_ELB@@/${LOGSTASH_ELB}/" ${CONFIGDIR}/filebeat.yml > /etc/filebeat/filebeat.yml
    # config for sts logs
    sed "s/@@LOGSTASH_ELB@@/${LOGSTASH_ELB}/" ${CONFIGDIR}/filebeat-sts.yml > /etc/filebeat/conf.d/filebeat-sts.yml
    # config for other lightwave logs
    sed "s/@@LOGSTASH_ELB@@/${LOGSTASH_ELB}/" ${CONFIGDIR}/filebeat-lightwave.yml > /etc/filebeat/conf.d/filebeat-lightwave.yml

    # restart logging tools
    systemctl restart syslog-ng
    systemctl restart systemd-journald
    systemctl restart filebeat
    systemctl restart crond
  fi
}

main "$@"
