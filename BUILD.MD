Building the Lightwave STS Container
====================================

 

Deliverables
------------

 

1.  Container

    1.  vmware-lightwave-sts.tar

2.  RPMs

    1.  vmware-event-devel-1.2.0-0.x86\_64.rpm

    2.  vmware-directory-client-1.2.0-0.x86\_64.rpm

    3.  vmware-directory-1.2.0-0.x86\_64.rpm

    4.  vmware-directory-client-devel-1.2.0-0.x86\_64.rpm

    5.  vmware-dns-client-1.2.0-0.x86\_64.rpm

    6.  vmware-dns-1.2.0-0.x86\_64.rpm

    7.  vmware-dns-client-devel-1.2.0-0.x86\_64.rpm

    8.  vmware-afd-client-1.2.0-0.x86\_64.rpm

    9.  vmware-afd-1.2.0-0.x86\_64.rpm

    10. vmware-afd-client-devel-1.2.0-0.x86\_64.rpm

    11. vmware-ca-1.2.0-0.x86\_64.rpm

    12. vmware-ca-client-1.2.0-0.x86\_64.rpm

    13. vmware-ca-client-devel-1.2.0-0.x86\_64.rpm

    14. vmware-sts-1.2.0-0.x86\_64.rpm

    15. vmware-sts-client-1.2.0-0.x86\_64.rpm

    16. vmware-ic-config-1.2.0-0.x86\_64.rpm

    17. vmware-lightwave-clients-1.2.0-0.x86\_64.rpm

    18. vmware-lightwave-server-1.2.0-0.x86\_64.rpm

 

Preparing the build system
--------------------------

 

1.  Start with a Photon 1.0 system

2.  tdnf install likewise-open-devel

3.  tdnf install openjdk

4.  tdnf install apache-maven

5.  tdnf install apache-ant

6.  tdnf install ant-contrib

7.  tdnf install jaxws-ri

8. tdnf install apache-tomcat

9. tdnf install boost-devel
 

Building the container
----------------------

 

From workspace root, run “make container”.

 

A successful build will yield the container at stage/vmware-lightwave-sts.tar.
This container will include lightwave packages from the local build tree.

To build a container using lightwave packages from the extern yum repository,
set your working directory to support/docker and run “docker build .”.

 

Building the RPMs
-----------------

 

From workspace root, run “make"

 

The RPMs are built as part of building the container and can be found in the
“stage” folder.

 

The default target for the top level makefile is to build the RPMs only (and not
the container).
