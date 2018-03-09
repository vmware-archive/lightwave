FROM vmware/photon:1.0
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

RUN tdnf update -y --refresh rpm-4.13.0.1 tdnf && \
    tdnf makecache && \
    tdnf update -y tdnf && \
    tdnf install -y sed-4.2.2 && \
    tdnf install -y procps-ng-3.3.11 && \
    tdnf install -y shadow-4.2.1 && \
    tdnf install -y binutils-2.29.1 && \
    tdnf install -y make-4.1 && \
    tdnf install -y gawk-4.1.3 && \
    tdnf install -y autoconf-2.69 && \
    tdnf install -y automake-1.15 && \
    tdnf install -y libtool-2.4.6 && \
    tdnf install -y gcc-5.3.0 && \
    tdnf install -y glibc-devel-2.22 && \
    tdnf install -y linux-api-headers-4.4.88 && \
    tdnf install -y util-linux-devel-2.27.1 && \
    tdnf install -y e2fsprogs-devel-1.42.13 && \
    tdnf install -y rpm-build-4.13.0.1 && \
    tdnf install -y rpm-devel-4.13.0.1 && \
    tdnf install -y openjdk-1.8.0.141 && \
    tdnf install -y apache-maven-3.3.9 && \
    tdnf install -y apache-ant-1.9.6 && \
    tdnf install -y ant-contrib-1.0b3 && \
    tdnf install -y jaxws-ri-2.2.10 && \
    tdnf install -y python2-devel-2.7.13 && \
    tdnf install -y apache-tomcat-8.5.23 && \
    tdnf install -y boost-devel-1.60.0 && \
    tdnf install -y jansson-devel-2.9 && \
    tdnf install -y openssl-devel && \
    tdnf install -y likewise-open-devel-6.2.11 && \
    tdnf install -y copenapi-devel-0.0.1 && \
    tdnf install -y c-rest-engine-devel-1.2 && \
    tdnf install -y go-1.8.1 && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers && \
    chmod -R o+r /opt/likewise/include
