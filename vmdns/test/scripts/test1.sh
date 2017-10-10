#!/bin/bash

/opt/vmware/bin/vmdns-cli add-record --zone in-addr.arpa --type PTR --hostname check --ip 1.2.3.4 --password 'Ca$hc0w1'
/opt/vmware/bin/vmdns-cli query-record --zone in-addr.arpa --type PTR --ip 1.2.3.4 --password 'Ca$hc0w1'
nslookup 1.2.3.4
dig -x 1.2.3.4
/opt/vmware/bin/vmdns-cli del-record --zone in-addr.arpa --type PTR --ip 1.2.3.4 --hostname check --password 'Ca$hc0w1'
/opt/vmware/bin/vmdns-cli add-record --zone ip6.arpa --type PTR --hostname check --ip ::1 --password 'Ca$hc0w1'
/opt/vmware/bin/vmdns-cli query-record --zone ip6.arpa --type PTR --ip ::1 --password 'Ca$hc0w1'
nslookup ::1
dig -x ::1
/opt/vmware/bin/vmdns-cli del-record --zone ip6.arpa --type PTR --ip ::1 --hostname check --password 'Ca$hc0w1'
