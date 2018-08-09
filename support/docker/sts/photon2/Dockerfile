FROM vmware/lightwave-base-photon2:1.0.1
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.3.1"

EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2015 2020 7478 5005
ENTRYPOINT ["/lightwave-init"]

# Build hook

RUN tdnf install -y lightwave-1.3.1 && \
    /opt/likewise/sbin/lwsmd --start-as-daemon && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\vmafd]" \
        Arguments "/opt/vmware/sbin/vmafdd -c" && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\vmca]"  \
        Arguments "/opt/vmware/sbin/vmcad -c" && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\vmdir]" \
        Arguments "/opt/vmware/sbin/vmdird -l 0 -f /opt/vmware/share/config/vmdirschema.ldif" && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\vmdns]" \
        Arguments "/opt/vmware/sbin/vmdnsd" && \
    rpm -e --nodeps systemd && \
    rpm -e createrepo_c && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    rm -rf /tmp/vmware/lightwave

ADD lightwave-init /
