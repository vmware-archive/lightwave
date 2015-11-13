Building the Lightwave STS Container
====================================

 

Deliverables
------------

 

1.  Container

    1.  vmware-lightwave-sts.tar

2.  RPMs

    1.  vmware-event-devel-1.0.0-0.x86\_64.rpm

    2.  vmware-directory-client-6.0.2-0.x86\_64.rpm

    3.  vmware-directory-6.0.2-0.x86\_64.rpm

    4.  vmware-directory-client-devel-6.0.2-0.x86\_64.rpm

    5.  vmware-dns-client-6.0.0-0.x86\_64.rpm

    6.  vmware-dns-6.0.0-0.x86\_64.rpm

    7.  vmware-dns-client-devel-6.0.0-0.x86\_64.rpm

    8.  vmware-afd-client-6.0.2-0.x86\_64.rpm

    9.  vmware-afd-6.0.2-0.x86\_64.rpm

    10. vmware-afd-client-devel-6.0.2-0.x86\_64.rpm

    11. vmware-ca-6.0.2-0.x86\_64.rpm

    12. vmware-ca-client-6.0.2-0.x86\_64.rpm

    13. vmware-ca-client-devel-6.0.2-0.x86\_64.rpm

    14. vmware-sts-6.0.2-0.x86\_64.rpm

    15. vmware-sts-client-6.0.2-0.x86\_64.rpm

    16. vmware-ic-config-1.0.2-0.x86\_64.rpm

    17. vmware-lightwave-clients-6.0.2-0.x86\_64.rpm

    18. vmware-lightwave-server-6.0.2-0.x86\_64.rpm

 

Preparing the build system
--------------------------

 

1.  Start with a Photon system

2.  Make sure you have the yum repository for photon-extras configured

    -   How to configure the photon-extras repository ?

        Copy the following into /etc/yum.repos.d/photon-extras.repo

        [photon-extras]

        name=VMware Photon Extras 1.0(x86\_64)

        baseurl=https://dl.bintray.com/vmware/photon\_extras

        gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY

        gpgcheck=0

        enabled=1

        skip\_if\_unavailable=True

3.  tdnf install likewise-open-6.2.8

4.  tdnf install likewise-open-devel-6.2.8

5.  tdnf install openjdk

6.  tdnf install apache-maven

7.  tdnf install apache-ant

8.  tdnf install ant-contrib

9.  tdnf install jaxws-ri

10. tdnf install apache-tomcat

 

Building the container
----------------------

 

From workspace root, run “make container”.

 

A successful build will yield the container at stage/vmware-lightwave-sts.tar

 

Building the RPMs
-----------------

 

From workspace root, run “make"

 

The RPMs are built as part of building the container and can be found in the
“stage” folder.

 

The default target for the top level makefile is to build the RPMs only (and not
the container).
