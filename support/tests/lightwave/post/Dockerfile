FROM vmware/lightwave-base-photon2:1.0.0
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."

RUN tdnf makecache -q
RUN tdnf update likewise-open --disablerepo=* --enablerepo=lightwave -qy
RUN tdnf install net-tools shadow findutils gawk jansson sqlite-devel copenapi apache-tomcat netcat -qy
