#!/bin/bash
set -x

mode=$1
partner='partner'
/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm start post
partner_opt="--domain-name post.test"
if [ ${mode} == ${partner} ]; then
  partner_opt=" --partner-name $POST_FIRST_NODE_NAME"
fi
/opt/vmware/bin/post-cli node promote --password $POST_PWD $partner_opt
