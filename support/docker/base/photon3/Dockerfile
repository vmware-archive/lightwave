FROM photon:3.0-20191101
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave-base.version="3.0.2"

RUN tdnf update -y tdnf && \
    tdnf erase -y toybox && \
    tdnf install -y \
         apache-tomcat-8.5.46-1.ph3 \
         boost-1.66.0-1.ph3 \
         c-rest-engine-1.2-5.ph3 \
         commons-daemon-1.1.0-2.ph3 \
         copenapi-0.0.2-3.ph3 \
         createrepo_c-0.11.1-2.ph3 \
         findutils-4.6.0-5.ph3 \
         gawk-4.2.1-1.ph3 \
         jansson-2.11-1.ph3  \
         less-530-1.ph3 \
         likewise-open-6.2.11.13-0.ph3 \
         net-tools-1.60-11.ph3 \
         openjre8-1.8.0.232-1.ph3 \
         openssl-1.0.2t-1.ph3 \
         procps-ng-3.3.15-1.ph3 \
         rpm-4.14.2-5.ph3 \
         sed-4.5-1.ph3 \
         shadow-4.6-4.ph3 \
         vim-8.1.0388-5.ph3 && \
    chage -I -1 -m 0 -M 99999 -E -1 root
