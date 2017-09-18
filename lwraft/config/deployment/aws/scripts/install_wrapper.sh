#!/bin/bash

echo "" >> /var/log/install.sh.log
echo "" >> /var/log/install.sh.log
echo "" >> /var/log/install.sh.log
echo "" >> /var/log/install.sh.log

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
${DIR}/install.sh >> /var/log/install.sh.log 2>&1