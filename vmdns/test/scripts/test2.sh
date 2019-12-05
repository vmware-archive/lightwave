#!/bin/bash

/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local --type PTR --hostname check --ip 1.2.3.4 --password 'Passw0rd$'
/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local --type PTR --hostname check --ip ::1 --password 'Passw0rd$'
