FROM vmware/photon:1.0
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave-base.version="1.0.0"

RUN tdnf update --refresh -y \
         tdnf-1.1.0 \
         rpm-4.13.0.1 && \
    tdnf install -y \
         openssl-1.0.2o \
         createrepo-0.10.4 \
         apache-tomcat-8.5.20 \
         boost-1.60.0 \
         commons-daemon-1.0.15 \
         likewise-open-6.2.11 \
         openjre-1.8.0.141 \
         procps-ng-3.3.11 \
         sed-4.2.2 \
         jansson-2.9  \
         gawk-4.1.3 \
         copenapi-0.0.1 \
         c-rest-engine-1.1
