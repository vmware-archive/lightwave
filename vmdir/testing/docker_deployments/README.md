vmdir Multi-node docker based test
================

Scripts to setup and inspect docker network based multi-node vmdir.

These scripts allow

1. Build docker container with vmdir and necessary tools.
2. Easily add nodes or operate as single node.
3. Transparent network details using docker network.
4. Operate on nodes via helper scripts.

How to use
----------
cd to vmdir/test/docker_deployments directory.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./make_docker_image.sh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will make a docker image tagged docker_vmdirtest

To deploy, do

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./deploy.sh <number of nodes>
For eg: ./deploy.sh 4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

the above deploy script will promote the first node as primary and join the rest of them to it automatically. You can call deploy as many times as you want to add  the number of nodes.

for eg: ./deploy.sh will deploy single node

now, you can do ./deploy.sh 4 which will add 3 more and join them.

the naming conventions are vmdir1.vmdir.test, vmdir2.vmdir.test and so on.

Once deployed, you can run db stats, log stats etc on all nodes.

for eg: ./deploy.sh 4  will make a 4 node deployment with its own network in your vm.

Network details should be transparent for the purposes of this test.
If you need to know those, you can run

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
docker network -f name=vmdir_test_net
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Inspection scripts
-----------------

Let's look at the additional scripts and what they do.

check_env
---------
Iterates all nodes and runs stats on them as shown below. Stats currently are
a lost of /var/lib/vmware/vmdir/data.mdb for size and mdb_stat on the database
for database stats.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./check_env.sh

CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
19526c96ecb6        docker_vmdirtest    "/bin/bash"         4 minutes ago       Up 4 minutes                            vmdir1.vmdir.test
  Number of pages used: 2126
  Last transaction ID: 8166
  Number of readers used: 1
---------- 1 root root 8.4M Apr 16 22:02 /var/lib/vmware/vmdir/data.mdb
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

user_stats
---------

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./user_stats.sh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Show users on all nodes - not very useful but serves as an example.

remove_all
---------

Stop and delete all nodes.
