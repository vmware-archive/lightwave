#!/bin/bash -xe
#
# Script to test lightwave and proxy services

curl -k https://$LW_SERVER_ELB:$LW_SERVER_PORT
curl http://$LW_SERVER_ELB:$LW_PROXY_SERVER_PORT/v1/tenants
