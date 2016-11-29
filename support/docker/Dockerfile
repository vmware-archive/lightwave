FROM vmware/photon:1.0
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.0.2"

EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2015 2020
ENTRYPOINT ["/usr/sbin/init"]

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
        systemd-228 \
        vmware-lightwave-server-6.6.2 && \
    rm -f /etc/systemd/system/*.wants/* && \
    rm -f /lib/systemd/system/sysinit.target.wants/systemd-tmpfiles-setup.service && \
    rm -f /lib/systemd/system/multi-user.target.wants/* && \
    rm -f /lib/systemd/system/local-fs.target.wants/* && \
    rm -f /lib/systemd/system/sockets.target.wants/*udev* && \
    rm -f /lib/systemd/system/sockets.target.wants/*initctl* && \
    rm -f /usr/lib/systemd/system/systemd-resolved.service && \
    rm -f /usr/lib/systemd/system/systemd-networkd.service && \
    mkdir -p /var/run/sshd && \
    chmod -rx /var/run/sshd && \
    rm -f /usr/lib/systemd/system/firewall.service && \
    rm -f /etc/systemd/system/multi-user.target.wants/firewall.service && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    sed -i 's/#Storage=auto/Storage=persistent/' /etc/systemd/journald.conf

ADD configure-lightwave-server.service /usr/lib/systemd/system/
ADD configure-identity-server.service /usr/lib/systemd/system/
RUN ln -s /usr/lib/systemd/system/configure-lightwave-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-lightwave-server.service
RUN ln -s /usr/lib/systemd/system/configure-identity-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-identity-server.service
