# Multi-Container Lightwave cluster in one VM
These scripts use docker to create a network on which 3 Lighwtave containers
are created and form a 3 node cluster.

## Requirements
1. You need following tools installed
  * [docker](https://docs.docker.com/engine/installation/) (Version >= 1.12.1)
  * For running these scripts on *MacBook* you need
    * [docker-machine](https://docs.docker.com/machine/install-machine/)
    * VMware Fusion (or VirtualBox)

2. Following docker image files present in current directory
  * vmware-lightwave-sts.tar

## IP address distribution in docker overlay network.

* Subnet: 192.168.114/27
* LW Container Node IPs: 192.168.114.2 - 192.168.114.4
* Docker network name: lightwave

## Running the scripts
Following steps are for Linux with Docker. For Macbook you need to first prepare using `docker-machine` as mentioned at the end of this section.

1. Load docker images by running `./load-images.sh`
2. Create LW cluster by running `./make-lw-cluster.sh`
5. Create demo users and groups by running `./make-users.sh`

For running above scripts in *MacBook* you would need to have `docker-machine` and `virtualbox` installed.
Use following steps to prepare your MacBook to run the scripts listed above.
* `docker-machine create -d virtualbox --virtualbox-memory 4096 default`
* `eval $(docker-machine eval default)`
* `docker-machine scp ./prepare-docker-machine.sh default:/tmp`
* `docker-machien ssh default:/tmp/prepare-docker-machine.sh`

# Multi-Host Multi-Container Lightwave cluster in three VMs.
To create all containers in different VMs and create overlay network for docker containers,
we can leverage docker-machine and its swarm feature.
Make sure your have latest `docker-machine` installed. Then run following script to create three
swarm nodes (VMs).
```
./make-vms.sh
```
After VMs are created, you can run following command to set your docker client to point to swarm cluster
just created.
```
eval $(docker-machine env --swarm mhs-demo0)
```
Now you are ready to run all the scripts mentioned in section "Runninig the scripts" above.
