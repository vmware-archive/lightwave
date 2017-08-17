# Test Script that performs testing for VMDNS REST API for Metrics Collection

## VARIABLES AND CONSTANTS ##

host="localhost"
port="7677"

# Admin DN
admindn="cn=administrator,cn=users,dc=vsphere,dc=local"
adminpw='Admin!23'

# Unit Test: VMDNS Get Metrics Data testing
echo "Unit Test: VMDNS Get Metrics Data testing"
echo "Expected Result: Should be able to see all the metrics in Prometheus Data Format(text/plain)"
echo "Hit Enter to continue"
read
curl -v -u $admindn:$adminpw http://$host:$port/v1/dns/metrics
echo
echo "Unit Test Done."
exit 0
