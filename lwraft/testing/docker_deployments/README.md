POST Multi-node docker based test
================

Scripts to setup and inspect docker network based multi-node post. These scripts allow

1. Build docker container with post and necessary tools.
2. Easily add nodes or operate as single node.
3. Transparent network details using docker network.
4. Operate on nodes via helper scripts.

How to use
----------
cd to lwraft/docker_deployments directory.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./make_docker_image.sh
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will make a docker image tagged docker_posttest

To deploy, do

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./deploy.sh <number of nodes>
For eg: ./deploy.sh 4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

the above deploy script will promote the first node as primary and join the rest of them to it automatically. You can call deploy as many times as you want to add  the number of nodes.

for eg: ./deploy.sh will deploy single node

now, you can do ./deploy.sh 4 which will add 3 more and join them.

the naming conventions are post1.post.test, post2.post.test and so on.

Once deployed, you can run db stats, log stats etc on all nodes.

for eg: ./deploy.sh 4  will make a 4 node deployment with its own network in your vm.

Network details should be transparent for the purposes of this test.
If you need to know those, you can run

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./deploy_logsplit.sh <number of nodes>
For eg: ./deploy_logsplit.sh 4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
same functionality as deploy except that post registry sets RaftUseLogDB to 0x1
which splits the database to state and log.

docker network -f name=post_test_net
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Inspection scripts
-----------------

Let's look at the additional scripts and what they do.

check_env
---------
Iterates all nodes and runs stats on them as shown below. Stats currently are
a lost of /var/lib/vmware/post/data.mdb for size and mdb_stat on the database
for database stats.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
./check_env.sh

root@photon-e3ce278f5ae4 [ ~/post ]# ./check_env.sh
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
bbec5604f5a5        docker_posttest     "/bin/bash"         About an hour ago   Up About an hour                        post4.post.test
  Number of pages used: 513
  Last transaction ID: 1028
  Number of readers used: 1
-rw------- 1 root root 2.1M Feb  7 22:06 /var/lib/vmware/post/data.mdb
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
435f3d915bf2        docker_posttest     "/bin/bash"         About an hour ago   Up About an hour                        post3.post.test
  Number of pages used: 511
  Last transaction ID: 1030
  Number of readers used: 1
-rw------- 1 root root 2.0M Feb  7 22:06 /var/lib/vmware/post/data.mdb
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

log_stats
---------

Show raft log stat on all nodes. Help quickly identify stale nodes.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
root@photon-e3ce278f5ae4 [ ~/post ]# ./log_stats.sh
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
4d16ce50c8ca        docker_posttest     "/bin/bash"         37 seconds ago      Up 37 seconds                           post4.post.test
dn: cn=persiststate,cn=raftcontext
vmwRaftFirstLogindex: 1
vmwRaftLastApplied: 633

CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
347065e26b37        docker_posttest     "/bin/bash"         About a minute ago   Up About a minute                       post2.post.test
dn: cn=persiststate,cn=raftcontext
vmwRaftFirstLogindex: 1
vmwRaftLastApplied: 633
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

node_stats
---------

Show role, term, lastindex and lastappliedindex on all nodes (showing just one node below)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
root@photon-e3ce278f5ae4 [ ~/docker_deployments ]# ./node_stats.sh
CONTAINER ID        IMAGE               COMMAND             CREATED              STATUS              PORTS               NAMES
def26ccdc3db        docker_posttest     "/bin/bash"         About a minute ago   Up 59 seconds                           post3.post.test

Node Name                      Role       Term   LastIndex       LastAppliedIndex
------------------------------ ---------- ------ --------------- ----------------
post3.post.test                Follower   2      633             633
post2.post.test                Follower   2      633             633
post1.post.test                Leader     2      633             633
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


node_demote
----------

demote a node. specify node number. Note: this is not versatile as it assumes post1.post.test is leader.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
root@photon-e3ce278f5ae4 [ ~/docker_deployments ]# ./node_demote.sh 2
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
5144f2f6f4f1        docker_posttest     "/bin/bash"         5 minutes ago       Up 5 minutes                            post1.post.test
Persistent Objectstore Service instance post2.post.test is removed from cluster successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

node_promote
------------

promote a node by node number. works when demoted using node_demote (which means it must be part of the cluster)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
root@photon-e3ce278f5ae4 [ ~/docker_deployments ]# ./node_promote.sh 2
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
ee2dc3bdf3d0        docker_posttest     "/bin/bash"         5 minutes ago       Up 5 minutes                            post2.post.test
Initializing Persistent Objectstore Service instance ...
System Host Name: post2.post.test
Partner Server Name: post1.post.test
Target Domain Name: post.test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


remove_all
---------

Stop and delete all nodes.
