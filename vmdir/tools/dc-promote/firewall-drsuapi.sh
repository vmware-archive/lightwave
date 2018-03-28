#!/bin/sh

# Remove previously opened port, if one exists
if [ -f /tmp/drsuapi_port_prev.txt ]; then
  drsuapi_port=`cat /tmp/drsuapi_port_prev.txt`
  iptables -D LIGHTWAVE --proto tcp --dport $drsuapi_port -j ACCEPT
fi

lsassd_pid=`ps auxw | grep leak-check | grep -v grep  | awk '{print $2}'`
if [ -n "$lsassd_pid" ]; then
  drsuapi_port=`netstat -tanp | grep "${lsassd_pid}/"  | grep LISTEN | grep -v tcp6 | \
    sed -e 's/\(.*:\)\(.*:.*\)/\2/'  -e 's/  .*//'`
  echo lsassd pid=$lsassd_pid /tmp/lsassd-${lsassd_pid}.vg
else
  drsuapi_port=`netstat -tanp | grep lsassd | grep LISTEN | grep tcp[^6]  | \
    sed -e 's/\(.*:\)\(.*:.*\)/\2/'  -e 's/  .*//'`
fi

iptables -I LIGHTWAVE --proto tcp --dport $drsuapi_port -j ACCEPT
echo $drsuapi_port > /tmp/drsuapi_port_prev.txt
echo opened firewall drsuapi_port=$drsuapi_port
