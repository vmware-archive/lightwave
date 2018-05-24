FROM vmware/lightwave-base:1.0.0
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.3.1"

EXPOSE 22
ENTRYPOINT ["/lightwave-init"]

# Build hook

RUN tdnf install -y lightwave-client-1.3.1 && \
    /opt/likewise/sbin/lwsmd --start-as-daemon && \
    /opt/likewise/bin/lwregshell set_value "[HKEY_THIS_MACHINE\\Services\\vmafd]" \
        Arguments "/opt/vmware/sbin/vmafdd -c" && \
    rpm -e --nodeps systemd && \
    rpm -e createrepo && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    rm -rf /tmp/vmware/lightwave

ADD lightwave-init /
