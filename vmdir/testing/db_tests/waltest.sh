#!/bin/bash

#Call script that will sleep in background and then kill vmdir
#For other test scenarios it can be replaced with a script that reboots the vm or increases size of autoscaling group by 1 etc.
START=$1
END=$2
PASSWORD=$3

./killvmdir.sh &

for i in `seq $START $END`;
do
	sed  -e "s|@@NO@@|$i|" sampledata.txt | ldapadd -h localhost -p 389
    -D"cn=Administrator,cn=Users,dc=LW-TESTDOM,dc=COM" -U administrator@lw-testdom.com -w $PASSWORD -Y SRP > /dev/null 2>&1
	if [ $? -eq 255 ]; then
		(( i=i-1 ))
		echo "Last entry number:"$i
		sleep 20
		./ldapcheck.sh $START $i $PASSWORD
		exit $?
	fi
done;
