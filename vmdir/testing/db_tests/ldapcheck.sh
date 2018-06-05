#!/bin/bash

START=$1
END=$2
PASSWORD=$3

for i in `seq $START $END`;
do
	ldapsearch -Y SRP -U administrator@lw-testdom.com -h localhost -p 389 -w $PASSWORD -b "cn=Workflows$i,dc=lw-testdom,dc=com" -s base > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		echo "Not found "$i
		exit 1
	fi
	ldapsearch -Y SRP -U administrator@lw-testdom.com -h localhost -p 389 -w $PASSWORD -b "cn=Clusters$i,dc=lw-testdom,dc=com" -s base > /dev/null 2>&1
        if [ $? -ne 0 ]; then
                echo "Not found "$i
		exit 1
	fi
done;
