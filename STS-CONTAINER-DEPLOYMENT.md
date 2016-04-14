Secure Token Server Container Deployment
========================================

 

Build
-----

To build the Lightwave STS Container, follow the instructions in BUILD.md. The
build process generates a saved Docker container in the file
vmware-lightwave-sts.tar.

 

Deployment
----------

The following steps show how to deploy the container image on a Photon host.

 

### 1. Make sure that the docker daemon is running.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# systemctl status docker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the daemon is not active, start it with the command:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# systemctl start docker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 

### 2. Load the container image:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# docker load < vmware-lightwave-sts.tar
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 

### 3. Create a configuration file

Example data to be placed in /var/lib/vmware/config/lightwave-server.cfg:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
deployment=standalone
domain=vsphere.local
admin=Administrator
password=<Administrator password>
site-name=Default-first-site
first-instance=true
hostname=<Host IP address>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 

### 4. Run a container

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bash-4.3# docker run -d --name <name> --privileged --net=host \
                     -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
                     -v /var/lib/vmware/config:/var/lib/vmware/config \
                     vmware/lightwave-sts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notes:

-   Choose a unique name for the container

-   This starts the container in host networking mode, meaning that it shares
    the networking configuration with the container host. Only one container can
    be running on the host in this mode.

-   The directory /var/lib/vmware/config will be mounted from the host to the
    container and the lighwave-server.cfg file created in step 3 will be used to
    automatically configure Lightwave the first time the container is run.

 
