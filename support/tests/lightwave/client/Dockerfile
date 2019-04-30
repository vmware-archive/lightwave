FROM vmware/lightwave-base-photon2:1.0.0
ENV container=docker
ENV GOROOT=/usr/lib/golang
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."

RUN tdnf makecache -q
RUN tdnf update likewise-open --disablerepo=* --enablerepo=lightwave -qy
RUN tdnf install net-tools shadow findutils gawk jansson sqlite-devel jq gcc glibc-devel git go-1.9.1 netcat -qy
