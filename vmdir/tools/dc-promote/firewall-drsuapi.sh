#!/bin/sh
sleep infinity &  PID=$!
trap "kill $PID" 1 2 3 15

# Wait for lsassd to be ready!
tcpports=0
i=0
while [ \( $tcpports -lt 2 \) -a \( $i -lt 5 \) ]; do
  tcpports=`netstat -tanp | grep -e lsassd -e valgrind | grep 'tcp[^6]' | grep LISTEN |
             grep -v grep | wc -l`
  sleep 2
  i=$(($i + 1))
done

if [ $i -ge 5 ]; then
  # timed out waiting for lsassd to be ready
  exit 1
fi

# Remove previously opened port, if one exists
if [ -f /tmp/drsuapi_port_prev.txt ]; then
  drsuapi_port=`cat /tmp/drsuapi_port_prev.txt`
  for port in $drsuapi_port; do
    iptables -D LIGHTWAVE --proto tcp --dport $port -j ACCEPT
  done
fi

lsassd_pid=`ps auxw | grep -e lsass -e valgrind | grep -v grep  | awk '{print $2}'`
if [ -n "$lsassd_pid" ]; then
  drsuapi_port=`netstat -tanp | grep "${lsassd_pid}/"  | grep LISTEN | grep -v tcp6 | \
    sed -e 's/\(.*:\)\(.*:.*\)/\2/'  -e 's/  .*//'`
  echo lsassd pid=$lsassd_pid /tmp/lsassd-${lsassd_pid}.vg
else
  drsuapi_port=`netstat -tanp | grep lsassd | grep LISTEN | grep tcp[^6]  | \
    sed -e 's/\(.*:\)\(.*:.*\)/\2/'  -e 's/  .*//'`
fi

for port in $drsuapi_port; do
  iptables -I LIGHTWAVE --proto tcp --dport $port -j ACCEPT
done
echo $drsuapi_port > /tmp/drsuapi_port_prev.txt

rule135=`iptables -L -n | grep 'ACCEPT.*tcp dpt:135'`
if [ -z "$rule135" ]; then
  # Insure all ports for Lightwave DC are open
  iptables -I LIGHTWAVE --proto icmp -j ACCEPT &&
      iptables -I LIGHTWAVE --proto udp --dport 53 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto tcp --dport 53 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto udp --dport 88 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto tcp --dport 88 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto udp --dport 389 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto tcp --dport 445 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto tcp --dport 135 -j ACCEPT && 
      iptables -I LIGHTWAVE --proto tcp --dport 139 -j ACCEPT &&
      iptables -I LIGHTWAVE --proto tcp --dport 389 -j ACCEPT &&
      iptables -I OUTPUT --proto tcp --dport 389 -j ACCEPT
fi

# Notify lwsm I have started
dd if=/dev/zero bs=1 count=1 2> /dev/null 1>&3

# Wait until notified by lwsmd to go away
wait $PID
