FROM vmware/photon:1.0
MAINTAINER "Andrew Gormley" <agormley@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.0.2"

EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2015 2020
ENTRYPOINT ["/lightwave-init"]

# Build hook

RUN tdnf update --refresh -y \
        tdnf \
        rpm-4.11.2 \
        pcre-8.39 \
        openssl-1.0.2j \
        expat-2.2.0 \
        curl-7.51 \
        bzip2-1.0.6 && \
    tdnf install -y \
        apache-tomcat-8.0.37 \
        boost-1.60.0 \
        commons-daemon-1.0.15 \
        likewise-open-6.2.10 \
        openjre-1.8.0.102 \
        procps-ng-3.3.11 \
        sed-4.2.2 \
        vmware-lightwave-server-6.6.3 && \
    rpm -e --nodeps systemd && \
    mkdir -p /var/run/sshd && \
    chmod -rx /var/run/sshd && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    /opt/likewise/sbin/lwsmd --start-as-daemon && \
    /opt/likewise/bin/lwregshell set_value "[HKEY_THIS_MACHINE\\Services\\vmafd]" \
        Arguments "/opt/vmware/sbin/vmafdd" && \
    /opt/likewise/bin/lwregshell set_value "[HKEY_THIS_MACHINE\\Services\\vmca]"  \
        Arguments "/opt/vmware/sbin/vmcad" && \
    /opt/likewise/bin/lwregshell set_value "[HKEY_THIS_MACHINE\\Services\\vmdir]" \
        Arguments "/opt/vmware/sbin/vmdird -l 0 -f /opt/vmware/share/config/vmdirschema.ldif" && \
    /opt/likewise/bin/lwregshell set_value "[HKEY_THIS_MACHINE\\Services\\vmdns]" \
        Arguments "/opt/vmware/sbin/vmdnsd"
ADD lightwave-init /
