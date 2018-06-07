MDB WAL Tests
=============

These tests are written to test the WAL functionality.
The scripts test if WAL works fine during ungraceful shutdown of vmdir.
Similarly other scenarios like node join and server restart can be tested by making changes to these scripts.


Usage
-----

'./waltest.sh 1 1000 <Password>' will read the sample data from sampledata.txt and start adding entries. The data can be
changed according to the requirements. The first two arguments 1 and 1000 specify the range of entries to add.

ldapsearch.sh is called from waltest.sh to check if the entries that were added are present. It can also be run by
itself using the same arguments as waltest.


EXTENDING FOR MULTINODE
-----------------------

Node Join: Instead of killing vmdir, run the aws command to increase autoscaling group size 'aws autoscaling
update-auto-scaling-group'. Sleep for sometime. Get the instance id of the new vm.
Use 'aws ssm send_command' to execute ldap_check.sh at the new vm. This will check if the DB has been replicated
correctly to the new node.

Similarly other multinode cases can be handled using aws commands.
