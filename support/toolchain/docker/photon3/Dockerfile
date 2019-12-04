FROM photon:3.0-20191101
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

RUN tdnf update -qy  tdnf  && \
    tdnf erase -qy toybox && \
    tdnf install -qy \
    ant-contrib-1.0b3-13.ph3.noarch \
    apache-ant-1.10.5-4.ph3.noarch \
    apache-maven-3.5.4-3.ph3.x86_64 \
    apache-tomcat-8.5.46-1.ph3.noarch \
    autoconf-2.69-8.ph3.noarch \
    automake-1.16.1-1.ph3.noarch \
    binutils-2.32-3.ph3.x86_64 \
    boost-devel-1.66.0-1.ph3.x86_64 \
    c-rest-engine-devel-1.2-5.ph3.x86_64 \
    cmocka-1.1.2-1.ph3.x86_64 \
    copenapi-devel-0.0.2-3.ph3.x86_64 \
    curl-devel-7.61.1-5.ph3.x86_64 \
    cyrus-sasl-2.1.26-14.ph3.x86_64 \
    diffutils-3.6-2.ph3.x86_64 \
    e2fsprogs-devel-1.44.3-4.ph3.x86_64 \
    elfutils-0.176-1.ph3.x86_64 \
    file-5.34-2.ph3.x86_64 \
    findutils-4.6.0-5.ph3.x86_64 \
    gawk-4.2.1-1.ph3.x86_64 \
    gcc-7.3.0-4.ph3.x86_64 \
    gdbm-1.18-1.ph3.x86_64 \
    git-2.23.0-1.ph3.x86_64 \
    glibc-devel-2.28-4.ph3.x86_64 \
    go-1.13.3-1.ph3.x86_64 \
    jansson-devel-2.11-1.ph3.x86_64 \
    jq-1.5-4.ph3.x86_64 \
    krb5-devel-1.17-1.ph3.x86_64 \
    libtool-2.4.6-3.ph3.x86_64 \
    likewise-open-devel-6.2.11.13-1.ph3.x86_64 \
    linux-api-headers-4.19.84-1.ph3.noarch \
    make-4.2.1-3.ph3.x86_64 \
    nodejs-10.15.2-1.ph3.x86_64 \
    openjdk8-1.8.0.232-1.ph3.x86_64 \
    openjre8-1.8.0.232-1.ph3.x86_64 \
    openssl-devel-1.0.2t-1.ph3.x86_64 \
    procps-ng-3.3.15-1.ph3.x86_64 \
    python2-devel-2.7.17-1.ph3.x86_64 \
    rpm-4.14.2-5.ph3.x86_64 \
    rpm-build-4.14.2-5.ph3.x86_64 \
    rpm-devel-4.14.2-5.ph3.x86_64 \
    sed-4.5-1.ph3.x86_64 \
    shadow-4.6-4.ph3.x86_64 \
    sqlite-devel-3.27.2-5.ph3.x86_64 \
    utf8proc-2.2.0-1.ph3.x86_64 \
    util-linux-devel-2.32-1.ph3.x86_64 && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers
#    chmod -R o+r /opt/likewise/include
