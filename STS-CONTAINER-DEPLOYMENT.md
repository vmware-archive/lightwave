# Build
To build the Lightwave STS Container, follow the instructions in
BUILD.md. The build process generates a saved Docker container in the
file vmware-lightwave-sts.tar.

# Overview
There are many disadvantages in having the application and the
persistent data to co-exist in a single container. Co-existing the
persistent data with the application causes issues with upgrades,
portability, backup and restore. To overcome these disadvantages,
store the persistent data in in volumes created in a data-only
container.

# Deploy Lightwave using a data-only container
The following steps show how to deploy the container image on a Photon
host.

Enable Docker on Photon machine

    systemctl status docker
    systemctl start docker

### Transfer the lightwave container image onto your docker host

    scp <lightwave-build-machine>:/root/lightwave/stage/vmware-lightwave-sts.tar .

### Load the image

     docker load < vmware-lightwave-sts.tar

### Check image list

    # docker images
    REPOSITORY             TAG                 IMAGE ID            CREATED             SIZE
    vmware/lightwave-sts   latest              1a712667c72d        About an hour ago   656.5 MB

### Create the lightwave data container

This creates a container with the needed volumes for the data. 

Note: Volumes are separate entities from containers and persist beyond
the life of a container. Application containers can use these volumes
by running with --volumes-from <data-container> commandline argument.

    # docker create -v /var/lib/vmware -v /var/lib/likewise -v /etc/likewise -v /etc/vmware-sso  --name lw_data_container vmware/lightwave-sts /bin/true
    b6c1f9206b5bcb2011bf97eb63e52c2d15923f6ebfc2f10b5513eb07be987c61

### Check the lightwave data container is created

    # docker ps -a 
    
    CONTAINER ID        IMAGE                  COMMAND                  CREATED             STATUS              PORTS               NAMES
    b6c1f9206b5b        vmware/lightwave-sts   "/usr/sbin/init /bin/"   37 seconds ago      Created                                 lw_data_container

### Create the config file

The location of the /var/lib/vmware/config directory on the host 

Name of the file is lightwave-server.cfg

*For the first node:*
    
    # cat /var/lib/vmware/config/lightwave-server.cfg
    deployment=standalone
    domain=vsphere.local
    admin=Administrator
    password=<administrator-password>
    site-name=Default-first-site
    first-instance=true
    hostname=<ip>

*For subsequent nodes that will be joined to existing node:*

    # cat /var/lib/vmware/config/lightwave-server.cfg
    deployment=partner
    domain=vsphere.local
    admin=Administrator
    password=<administrator-password>
    site-name=Default-first-site
    hostname=<ip>
    replication-partner-hostname=<partner hostname or ip>
    
### Start the Application container

This will spin up the Lightwave application container. The
--volumes-from argument has this container use the data volumes in
data container. 

    # docker run -d --name lw-sts --privileged --net=host -v /sys/fs/cgroup:/sys/fs/cgroup:ro -v /var/lib/vmware/config:/var/lib/vmware/config --volumes-from lw_data_container vmware/lightwave-sts

### Verify deployment was successful
    #  docker exec <container-id> journalctl | grep configure-lightwave-server

### Remove Lightwave configuration file
This file contains administrator credentials and should be deleted
after container is started.


    # rm /var/lib/vmware/config/lightwave-server.cfg 

Notes:
-   Choose a unique name for the container
-   This starts the container in host networking mode, meaning that it
    shares the networking configuration with the container host. Only
    one container can be running on the host in this mode.
-   The directory /var/lib/vmware/config will be mounted from the host
    to the container and the lighwave-server.cfg file created in step
    3 will be used to automatically configure Lightwave the first time
    the container is run.
