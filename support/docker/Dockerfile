FROM vmware/photon:latest
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]

# install systemd
RUN tdnf update -y tdnf; \
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
    tdnf install -y commons-daemon \
                    apache-tomcat \
                    boost; \
    tdnf install -y likewise-open-6.2.10; \
    tdnf install -y vmware-lightwave-server; \
    rm -rf /usr/share/doc/*; \
    rm -rf /usr/share/man/*; \
    rm -rf /usr/include/*;

ADD configure-lightwave-server.service /usr/lib/systemd/system/
RUN ln -s /usr/lib/systemd/system/configure-lightwave-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-lightwave-server.service

EXPOSE 22 53/udp 53 88/udp 88 389 636 2012 2014 2020

ENTRYPOINT ["/usr/sbin/init"]
