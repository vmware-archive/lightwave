FROM vmware/photon2:20180214
MAINTAINER "Dheeraj Shetty" <dheerajs@vmware.com>
ENV container=docker
ENV GOROOT=/usr/lib/golang

COPY ./lightwave.repo /etc/yum.repos.d/

RUN tdnf update -qy  tdnf  && \
    tdnf erase -qy toybox && \
    tdnf install -qy \
        ant-contrib-1.0b3 \
        apache-ant-1.10.1 \
        apache-maven-3.5.0 \
        apache-tomcat-8.5.29 \
        autoconf-2.69 \
        automake-1.15 \
        aws-sdk-kms-1.4.33 \
        binutils-2.29.1 \
        boost-devel-1.63.0 \
        c-rest-engine-devel-1.2 \
        cmocka-1.1.1 \
        copenapi-devel-0.0.2 \
        curl-devel-7.58.0 \
        diffutils \
        e2fsprogs-devel-1.43.4 \
        elfutils-0.169 \
        file \
        findutils \
        gawk-4.1.4 \
        gcc-6.3.0 \
        git-2.14.2 \
        glibc-devel-2.26 \
        go-1.9.1 \
        jansson-devel-2.10 \
        jq-1.5 \
        krb5-devel-1.16 \
        libtool-2.4.6 \
        likewise-open-devel-6.2.11 \
        linux-api-headers-4.9.74 \
        make-4.2.1 \
        nodejs-7.7.4 \
        openjdk8-1.8.0.152 \
        openssl-devel \
        procps-ng-3.3.12 \
        python2-devel-2.7.13 \
        rpm-4.13.0.1 \
        rpm-build-4.13.0.1 \
        rpm-devel-4.13.0.1 \
        sed-4.4 \
        shadow-4.2.1 \
        sqlite-devel-3.21.0 \
        util-linux-devel-2.29.2 && \
    echo 'ALL ALL=NOPASSWD: ALL' >>/etc/sudoers && \
    chmod -R o+r /opt/likewise/include && \
    curl -sSLf https://github.com/go-swagger/go-swagger/releases/download/0.11.0/swagger_linux_amd64 >/usr/local/bin/swagger && \
    chmod a+rx /usr/local/bin/swagger
