#!/bin/bash

/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local --type PTR --hostname check --ip 1.2.3.4 --password 'Ca$hc0w1'
/opt/vmware/bin/vmdns-cli add-record --zone lightwave.local --type PTR --hostname check --ip ::1 --password 'Ca$hc0w1'
