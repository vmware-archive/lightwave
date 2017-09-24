FROM vmware/photon:1.0
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

RUN tdnf update -y --refresh tdnf && \
    tdnf makecache && \
    tdnf update -y rpm && \
    tdnf install -y sed && \
    tdnf install -y procps-ng && \
    tdnf install -y shadow && \
    tdnf install -y binutils && \
    tdnf install -y make && \
    tdnf install -y gawk && \
    tdnf install -y autoconf && \
    tdnf install -y automake && \
    tdnf install -y libtool && \
    tdnf install -y gcc && \
    tdnf install -y glibc-devel && \
    tdnf install -y linux-api-headers && \
    tdnf install -y util-linux-devel && \
    tdnf install -y e2fsprogs-devel && \
    tdnf install -y rpm-build && \
    tdnf install -y rpm-devel && \
    tdnf install -y openjdk && \
    tdnf install -y apache-maven && \
    tdnf install -y apache-ant && \
    tdnf install -y ant-contrib && \
    tdnf install -y jaxws-ri && \
    tdnf install -y python2-devel && \
    tdnf install -y apache-tomcat && \
    tdnf install -y boost-devel && \
    tdnf install -y jansson-devel && \
    tdnf install -y openssl-devel && \
    tdnf install -y likewise-open-devel && \
    tdnf install -y copenapi-devel && \
    tdnf install -y c-rest-engine-devel && \
    tdnf install -y go && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers && \
    chmod -R o+r /opt/likewise/include
