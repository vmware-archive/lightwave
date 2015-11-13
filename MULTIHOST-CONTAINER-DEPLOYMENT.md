Multi-Host Lightwave Container Deployment
=========================================

This example describes how to deploy multiple Lightwave instances in Docker
containers using the experimental networking features of Docker 1.8.0-dev. The
multi-host networking allows lightwave directory instances to communicate
between containers, even if they are on different Photon host machines.

Here is the Docker documentation on how to use the multi-host overlay driver to
provision two hosts:

[Overlay
Driver](<https://github.com/docker/libnetwork/blob/master/docs/overlay.md>)

Note that the multi-host networking feature is an experimental Docker feature
which is not yet released in Docker 1.7.0 that is installed in Photon TP1.

The Lightwave setup shown here provisions two Photon TP1 Docker hosts with the
multi-host overlay driver and the Consul key-value store. Then a Lightwave
server is deployed in a container on each of the Photon hosts.

Many of the steps shown below must be done on each of two Photon host machines.
If PHOTON-HOST-1 or PHOTON-HOST-2 is specified, then the step should be
performed only on that host.

Install Consul 0.5.2
--------------------

### 1. Download Consul archive 0.5.2\_linux\_amd64.zip

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# wget https://dl.bintray.com/mitchellh/consul/0.5.2_linux_amd64.zip
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 2. Extract the consul executable from the archive

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# unzip 0.5.2_linux_amd64.zip
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 3. Install the consul executable

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# mv consul /bin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 4. Check that the correct consul version has been installed

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# consul --version
Consul v0.5.2
Consul Protocol: 2 (Understands back to: 1)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 5. Create a systemd configuration for consul service

-   PHOTON-HOST-1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# cat >/lib/systemd/system/consul.service
[Unit]
Description=Consul Service
After=syslog.target network.target

[Service]
ExecStart=/bin/consul agent -server -bootstrap -data-dir /tmp/consul -bind=IPADDR2

[Install]
WantedBy=multi-user.target
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure that IPADDR2 is set to the address of the PHOTON-HOST-2 machine.

-   PHOTON-HOST-2

For the second photon host, use the following consul command arguments instead

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ExecStart=/bin/consul agent -server -data-dir /tmp/consul -bind=IPADDR1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure that IPADDR1 is set to the address of the PHOTON-HOST-1 machine.

### 6. Enable and start the consul service

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# systemctl enable consul
Created symlink from /etc/systemd/system/multi-user.target.wants/consul.service to /usr/lib/systemd/system/consul.service.
bash-4.3# systemctl daemon-reload
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 6. Start the consul service and make sure it is running

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# systemctl start consul
bash-4.3# systemctl | grep consul
consul.service                                                                      loaded active running   Consul Service
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 7. Join the consul agent

-   PHOTON-HOST-2:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# consul join IPADDR1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure that IPADDR1 is the address of the PHOTON-HOST-2 machine.  

Install Docker 1.8.0-dev
------------------------

Photon TP1 has docker-1.7.0 installed. Install docker.1.8.0-dev instead.

### 1. Download Docker archive docker-1.8.0-dev.tgz

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# wget https://experimental.docker.com/builds/Linux/x86_64/docker-1.8.0-dev.tgz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 2. Extract the docker executable from the archive

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# tar xfvz docker-1.8.0-dev.tgz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will extract the file 'usr/local/bin/docker'.

### 3. Install the docker executable

On Photon, /bin/docker is a symbolic link to docker-1.7.0. To install
docker-1.8.0-dev, copy it to /bin and change the symbolic link to point to it.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# mv usr/local/bin/docker /bin/docker-1.8.0-dev
bash-4.3# rm /bin/docker
bash-4.3# ln -s docker-1.8.0-dev /bin/docker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 4. Check that the correct docker version has been installed

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# docker --version
Docker version 1.8.0-dev, build 5fdc102, experimental
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 6. Update the systemd configuration for docker

-   PHOTON HOST

Modify the systemd configuration for docker.service with the following docker
command line:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ExecStart=/bin/docker -d --kv-store=consul:localhost:8500 --label=com.docker.network.driver.overlay.bind_interface=ens32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example above shows the network interface name 'ens32'. Make sure to
configure the correct interface name for your photon host.

-   PHOTON HOST 2

Modify the systemd configuration for docker.service with the following docker
command line:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ExecStart=/bin/docker -d --kv-store=consul:localhost:8500 --label=com.docker.network.driver.overlay.bind_interface=ens32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 7. Start the docker daemon and verify that it is running

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# systemctl daemon-reload
bash-4.3# systemctl start docker
bash-4.3# systemctl | grep docker
docker.service                                                                      loaded active running   Docker Daemon
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 

Setup Overlay Network for Lightwave
-----------------------------------

### 1. Create an overlay network

-   PHOTON HOST 1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# docker network create -d overlay prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 2. Publish lightwave services

-   PHOTON HOST 1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# docker service publish lightwave1.prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   PHOTON HOST 2

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# docker service publish lightwave2.prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Deploy Lightwave Containers
---------------------------

### 1. Import a Lightwave container image

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# docker import vmware-lightwave-sts.tar vmware/lightwave-sts:latest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 2. Run the containers and attach them to a lightwave service

-   PHOTON-HOST-1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# docker run --privileged --name lw_01 vmware/lightwave-sts
    bash-4.3# docker service attach lw_01 lightwave1.prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   PHOTON-HOST-2

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    bash-4.3# docker run --privileged --name lw_02 vmware/lightwave-sts
    bash-4.3# docker service attach lw_02 lightwave2.prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 3. Exec a shell in the running containers

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# docker exec -it lw_01 sh
sh-4.3#
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 4. Install necessary packages in the containers

The inetutils package is needed for the ifconfig command to determine the IP
address of the container. The iana-etc package is required for ping to work.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sh-4.3# tdnf install -y inetutils
sh-4.3# tdnf install -y iana-etc
sh-4.3# ifconfig eth1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example, PHOTON-HOST-1 now has IP address 172.21.0.1 and PHOTON-HOST-2
now has IP address 172.21.0.2.

### 5. Verify that ping works between the containers

-   PHOTON-HOST-1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sh-4.3# ping 172.17.42.2
    PING 172.17.42.2
    (172.17.42.2): 56 data bytes 64 bytes from 172.17.42.2: icmp_seq=0 ttl=64 time=0.051 ms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 5. Set the hostnames of the docker containers.

-   PHOTON-HOST-1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sh-4.3# hostname lw_01_prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   PHOTON-HOST-2

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sh-4.3# hostname lw_02_prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 6. Update /etc/hosts in the containers

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sh-4.3# cat >>/etc/hosts
172.21.0.1 lw_01_prod
172.21.0.2 lw_02_prod
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### 7. Configure the Lightwave servers

-   PHOTON-HOST-1

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sh-4.3# /opt/vmware/bin/configure-lightwave-server     \
                               --password ADMIN-PASSWORD   \
                               --domain vsphere.local      \
                               --hostname 172.21.0.1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This command deploys the first lightwave server in standalone mode.

Set ADMIN-PASSWORD to the desired Administrator password for the domain.

-   PHOTON-HOST-2

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    sh-4.3# /opt/vmware/bin/configure-lightwave-server   \
                               --server 172.21.0.1       \
                               --hostname 172.21.0.2     \
                               --password ADMIN-PASSWORD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This command deploys the second lightwave server in replication partner mode
with the first lightwave server.

By following these steps additional lightwave servers may be deployed in
containers with whatever replication topology is desired. Each container will be
able to communicate with all other containers no matter which Photon host the
container is deployed on.
