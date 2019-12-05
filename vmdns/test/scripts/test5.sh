#!/bin/bash

/opt/vmware/bin/vmdns-cli add-zone 1.in-addr.arpa --ns-host ns1 --ns-ip 172.16.1.1 --type reverse --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli add-record --zone 1.in-addr.arpa --type PTR --hostname check-sub --ip 1.2.3.4 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli query-record --zone 1.in-addr.arpa --ip 1.2.3.4 --type PTR --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli del-record --zone 1.in-addr.arpa --type PTR --hostname check-sub --ip 1.2.3.4 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli del-zone 1.in-addr.arpa --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli add-zone 1.0.0.0.ip6.arpa --ns-host ns1 --ns-ip 172.16.1.1 --type reverse --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli add-record --zone 1.0.0.0.ip6.arpa --type PTR --hostname check-sub --ip 1::0 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli query-record --zone 1.0.0.0.ip6.arpa --ip 1::0 --type PTR --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli del-record --zone 1.0.0.0.ip6.arpa --type PTR --hostname check-sub --ip 1::0 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli del-zone 1.0.0.0.ip6.arpa --password 'Passw0rd$'
