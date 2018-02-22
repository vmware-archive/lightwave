FROM vmware/photon2:20180214
MAINTAINER "Dheeraj Shetty" <dheerajs@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

COPY ./lightwave.repo /etc/yum.repos.d/

RUN tdnf update -qy  tdnf  && \
    tdnf erase -qy toybox && \
    tdnf install -qy rpm-4.13.0.1 && \
    tdnf install -qy sed-4.4 && \
    tdnf install -qy procps-ng-3.3.12 && \
    tdnf install -qy shadow-4.2.1 && \
    tdnf install -qy binutils-2.29.1 && \
    tdnf install -qy make-4.2.1 && \
    tdnf install -qy gawk-4.1.4 && \
    tdnf install -qy autoconf-2.69 && \
    tdnf install -qy automake-1.15 && \
    tdnf install -qy libtool-2.4.6 && \
    tdnf install -qy diffutils && \
    tdnf install -qy findutils && \
    tdnf install -qy file && \
    tdnf install -qy gcc-6.3.0 && \
    tdnf install -qy glibc-devel-2.26 && \
    tdnf install -qy linux-api-headers-4.9.74 && \
    tdnf install -qy util-linux-devel-2.29.2 && \
    tdnf install -qy e2fsprogs-devel-1.43.4 && \
    tdnf install -qy rpm-build-4.13.0.1 && \
    tdnf install -qy rpm-devel-4.13.0.1 && \
    tdnf install -qy openjdk8-1.8.0.152 && \
    tdnf install -qy apache-maven-3.5.0 && \
    tdnf install -qy apache-ant-1.10.1 && \
    tdnf install -qy ant-contrib-1.0b3 && \
    tdnf install -qy python2-devel-2.7.13 && \
    tdnf install -qy apache-tomcat-8.5.24 && \
    tdnf install -qy boost-devel-1.63.0 && \
    tdnf install -qy jansson-devel-2.10 && \
    tdnf install -qy openssl-devel && \
    tdnf install -qy likewise-open-devel-6.2.11 && \
    tdnf install -qy copenapi-devel-0.0.2 && \
    tdnf install -qy c-rest-engine-devel-1.1 && \
    tdnf install -qy go-1.9.1 && \
    tdnf install -qy curl-devel-7.58.0 && \
    tdnf install -qy krb5-devel-1.16 && \
    tdnf install -qy sqlite-devel-3.21.0 && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers && \
    chmod -R o+r /opt/likewise/include


