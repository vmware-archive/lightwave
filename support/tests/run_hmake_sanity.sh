#!/bin/bash

#wait till lightwave init done

#prepare by installing rpms built in this build
rpm -Uvh --nodeps ./build/rpmbuild/RPMS/x86_64/lightwave-client*.rpm

/opt/likewise/sbin/lwsmd --start-as-daemon
/opt/likewise/bin/lwsm start vmafd

#note the following names carry docker-compose conventions
#<compose dir>_<service>_<instance count>
primary=lightwave_lightwave-server_1
partner=lightwave_lightwave-client_1

#wait for server to promote
response='000'
while [ $response -ne '404' ]; do
  response=$(curl -k --write-out %{http_code} --silent --output /dev/null https://$primary:7478)
  echo "waiting for $primary, response=$response"
  sleep 5
done

#wait a bit for join to happen
sleep 5

#run a lame sanity test. replace with tests batch run
curl -kv https://$primary:7478
