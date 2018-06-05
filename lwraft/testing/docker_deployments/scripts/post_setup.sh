#!/bin/bash
set -x

mode=$1
log_split=$2
partner='partner'

function setup_params() {
    uselogdb=0x0
    if [ $log_split == true ]; then
      uselogdb=0x1
    fi
    /opt/likewise/bin/lwregshell add_value \
    "[HKEY_THIS_MACHINE\Services\post\Parameters]" \
    "RaftUseLogDB" REG_DWORD $uselogdb
}

/opt/likewise/sbin/lwsmd --start-as-daemon

setup_params

/opt/likewise/bin/lwsm start post
partner_opt="--domain-name post.test"
if [ ${mode} == ${partner} ]; then
  partner_opt=" --partner-name $POST_FIRST_NODE_NAME"
fi
/opt/vmware/bin/post-cli node promote --password $POST_PWD $partner_opt
