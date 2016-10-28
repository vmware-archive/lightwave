FROM vmware/photon:1.0
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.0.1"

# install systemd
RUN tdnf update --refresh -y tdnf; \
    tdnf update -y rpm; \
    tdnf install -y systemd; \
    # Remove unused systemd services
    rm -f /etc/systemd/system/*.wants/*;\
    rm -f /lib/systemd/system/sysinit.target.wants/systemd-tmpfiles-setup.service;\
    rm -f /lib/systemd/system/multi-user.target.wants/*;\
    rm -f /lib/systemd/system/local-fs.target.wants/*; \
    rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
    rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
    mkdir -p /var/run/sshd; chmod -rx /var/run/sshd; \
    # configure journald
    tdnf install -y sed; \
    sed -i 's/#Storage=auto/Storage=persistent/' /etc/systemd/journald.conf; \
    tdnf install -y procps-ng; \
    tdnf install -y commons-daemon-1.0.15-6.ph1.x86_64 \
                    openjre-1.8.0.92 \
                    apache-tomcat-8.0.35 \
                    boost-1.60.0; \
    tdnf install -y likewise-open-6.2.10; \
    tdnf install -y vmware-lightwave-server; \
    rm -f /usr/lib/systemd/system/firewall.service \
    rm -f /etc/systemd/system/multi-user.target.wants/firewall.service \
    rm -rf /usr/share/doc/*; \
    rm -rf /usr/share/man/*; \
    rm -rf /usr/include/*;

ADD configure-lightwave-server.service /usr/lib/systemd/system/
ADD configure-identity-server.service /usr/lib/systemd/system/
RUN ln -s /usr/lib/systemd/system/configure-lightwave-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-lightwave-server.service
RUN ln -s /usr/lib/systemd/system/configure-identity-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-identity-server.service

EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2015 2020

ENTRYPOINT ["/usr/sbin/init"]
