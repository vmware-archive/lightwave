FROM vmware/photon2:20180214
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave-base.version="1.0.1"

COPY lightwave.repo /etc/yum.repos.d
RUN tdnf update -y tdnf && \
    tdnf erase -y toybox && \
    tdnf install -y \
         apache-tomcat-8.5.31 \
         boost-1.63.0 \
         c-rest-engine-1.2 \
         commons-daemon-1.0.15 \
         copenapi-0.0.1 \
         createrepo_c-0.10.0 \
         findutils-4.6.0 \
         gawk-4.1.3 \
         jansson-2.9  \
         less-487 \
         likewise-open-6.2.11 \
         net-tools-1.60 \
         openjre-1.8.0.141 \
         openssl-1.0.2o \
         procps-ng-3.3.12 \
         rpm-4.13.0.1 \
         sed-4.4 \
         shadow-4.2.1 \
         vim-8.0.0533
