# How to perform container upgrade

The following steps require that Lightwave has been configured to use
data volumes. (See STS-CONTAINER-DEPLOYMENT.md)

For multiple replicas, backup/upgrade/rollback can be performed on
individual nodes one at a time, only temporarily bringing down the
node currently being operated on. In the case of an upgrade failure,
only the failed node will need to be rolled back. Having nodes with
different versions in a domain is supported.

**Tag images**

New images may have same name as the old image. To keep them
organized, tag the image before loading a new image:

    # docker tag vmware/lightwave-sts vmware/lightwave-sts-old

## Create a backup of data from volumes

Before performing upgrade, backup all data for use in rollback in the
case of upgrade failure.

#### Create a backup directory on the host
    # mkdir backup-6-6-0

#### Create a new 'backup' container

This container will map a volume to a backup directory on the host,
and can be used for backup and restore operations.

**Create backup container:**

    # docker run -d --name backup --volumes-from lw_data -v $(pwd)/backup-6-6-0:/backup vmware/lightwave-sts

**Install tar**

For upgrading from lightwave 1.0 beta or rc, tar must be installed

    # docker exec backup tdnf install tar

**Create tars of all data volumes**

    # docker exec backup tar cvfP /backup/lib-vmware.tar /var/lib/vmware
    # docker exec backup tar cvfP /backup/lib-likewise.tar /var/lib/likewise
    # docker exec backup tar cvfP /backup/etc-likewise.tar /etc/likewise/
    # docker exec backup tar cvfP /backup/etc-vmware-sso.tar /etc/vmware-sso

**Remove backup container**

    # docker stop <backup container id>
    # docker rm <backup container id>

## Deploy Upgraded Container

**Stop running Lightwave container**

    # docker stop <lw container name>

**Create/edit config file so that domain, deployment and vmdir
  password in the lightwave-server config**

    # cat /var/lib/vmware/config/lightwave-server.cfg
    deployment=standalone
    domain=vsphere.local   <<<<
    admin=Administrator    <<<<
    password=Admin!23
    first-instance=true
    site-name=Default-first-site
    hostname=10.118.97.160

**Load new container:**

    # docker load < vmware-lightwave-sts.tar

**Deploy a container with the new image using same data volumes as
  previous installation**

Giving the container a distinguishing name may help with managing the
many containers that may get used during the upgrade process.

During initialization, the container will detect if upgrade logic is
to executed, put the directory into non-replication state and perform
any data patching needed.

Example:

    # docker run -d --name lightwave-1-1-0 --privileged --net=host -v /sys/fs/cgroup:/sys/fs/cgroup:ro  -v /var/lib/vmware/config:/var/lib/vmware/config --volumes-from lw_data_container vmware/lightwave-sts
    a689a33d718ff41692d230f4c39b3422e759f68d764c0a4a0638aca5af9af80f

If upgrade is successful, the directory will be taken out of
non-replication state.

### Verify that upgrade was successful

**Check journalctl for upgrade completion:**

    # docker exec lightwave-1-1-0 journalctl | grep configure-lightwave-server
    <snip>
    Sep 10 00:12:10 photon-ga.eng.vmware.com configure-lightwave-server[64]: Running vdcupgrade
    Sep 10 00:12:10 photon-ga.eng.vmware.com configure-lightwave-server[64]: Directory upgrade success.
    Sep 10 00:12:10 photon-ga.eng.vmware.com configure-lightwave-server[64]: Upgrade complete.

**Remove lightwave config file**

This file contains administrator credentials and should be deleted
when finished with upgrade.

    # rm /var/lib/vmware/config/lightwave-server.cfg

## Restore due to failed upgrade

If journalctl indicates an upgrade failure, a rollback may be
performed to revert Lightwave container back to known good state.

**Create a new 'rollback' container**

As with backup, this container will map a volume to the backup
directory on the host.

*Note: If a data volumes container was created during installation,
 the failed container can be killed at this point, and the data
 container can provide the data volumes. Otherwise, the running failed
 container can provide the volumes since it is in a non-replication
 state.*

**Create rollback container using volumes from the container to be
  restored:**

    # docker run -d --name rollback --volumes-from lw_data -v $(pwd)/backup-6-6-0:/backup vmware/lightwave-sts

**Extract tars of all data volumes**

    # docker exec rollback tar xvfP backup/lib-vmware.tar
    # docker exec rollback tar xvfP backup/lib-likewise.tar
    # docker exec rollback tar xvfP backup/etc-likewise.tar
    # docker exec rollback tar xvfP backup/etc-vmware-sso.tar

**Remove rollback container**

    # docker stop <rollback container id>
    # docker rm <rollback container id>

*If the failed upgrade container has not been stopped, do so before
 starting a container using the old image.*

**Restart old version container or use docker run to start a new
  instance of the old Lightwave container using the restored data:**

    # docker run -d --name vmsts-restored --privileged --net=host -v /sys/fs/cgroup:/sys/fs/cgroup:ro --volumes-from <backup image id>  <original LW image id>
