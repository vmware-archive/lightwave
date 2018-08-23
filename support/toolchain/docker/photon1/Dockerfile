FROM vmware/photon:1.0
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

RUN tdnf update -y --refresh rpm-4.13.0.1 tdnf && \
    tdnf makecache && \
    tdnf update -y tdnf && \
    tdnf install -y \
        ant-contrib-1.0b3 \
        apache-ant-1.9.6 \
        apache-maven-3.3.9 \
        apache-tomcat-8.5.23 \
        autoconf-2.69 \
        automake-1.15 \
        binutils-2.29.1 \
        boost-devel-1.60.0 \
        c-rest-engine-devel-1.2 \
        cmocka-1.1.1 \
        copenapi-devel-0.0.1 \
        e2fsprogs-devel-1.42.13 \
        elfutils-0.169 \
        gawk-4.1.3 \
        gcc-5.3.0 \
        glibc-devel-2.22 \
        go-1.8.1 \
        jansson-devel-2.9 \
        jaxws-ri-2.2.10 \
        libtool-2.4.6 \
        likewise-open-devel-6.2.11 \
        linux-api-headers-4.4.88 \
        make-4.1 \
        nodejs-7.7.4 \
        openjdk-1.8.0.141 \
        openssl-devel \
        procps-ng-3.3.11 \
        python2-devel-2.7.13 \
        rpm-build-4.13.0.1 \
        rpm-devel-4.13.0.1 \
        sed-4.2.2 \
        shadow-4.2.1 \
        util-linux-devel-2.27.1 && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers && \
    chmod -R o+r /opt/likewise/include
