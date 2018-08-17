FROM vmware/lightwave-base-photon2:1.0.1
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.3.1"

# POST LDAP  PORT: 38900
# POST LDAPS PORT: 63600
# POST HTTPS PORT: 7578
EXPOSE 38900 63600 7578
ENTRYPOINT ["/lightwave-init"]

# Build hook

RUN tdnf install -y lightwave-post-1.3.1 && \
    /opt/likewise/sbin/lwsmd --start-as-daemon && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\vmafd]" \
        Arguments "/opt/vmware/sbin/vmafdd -c" && \
    /opt/likewise/bin/lwregshell \
        set_value "[HKEY_THIS_MACHINE\\Services\\post]" \
        Arguments "/opt/vmware/sbin/postd -l 0 -f /opt/vmware/share/config/postschema.ldif" && \
    rpm -e --nodeps systemd && \
    rpm -e createrepo_c && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    rm -rf /tmp/vmware/lightwave

ADD lightwave-init /
