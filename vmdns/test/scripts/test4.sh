#!/bin/bash

/opt/vmware/bin/vmdns-cli query-record --zone in-addr.arpa --type PTR --ip 1.2.3.4 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli query-record --zone ip6.arpa --type PTR --ip ::1 --password 'Passw0rd$'
