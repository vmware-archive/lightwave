#!/bin/bash

echo "" >> /var/log/start_server.sh.log
echo "" >> /var/log/start_server.sh.log
echo "" >> /var/log/start_server.sh.log
echo "" >> /var/log/start_server.sh.log

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${DIR}/start_server.sh >> /var/log/start_server.sh.log 2>&1
