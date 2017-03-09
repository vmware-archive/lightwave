FROM vmware/photon:1.0
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
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
    # setup tools required for development
    tdnf install -y procps-ng; \
    tdnf install -y binutils; \
    tdnf install -y git; \
    tdnf install -y make; \
    tdnf install -y gawk; \
    tdnf install -y autoconf; \
    tdnf install -y automake; \
    tdnf install -y libtool; \
    tdnf install -y gcc; \
    tdnf install -y glibc-devel; \
    tdnf install -y linux-api-headers; \
    tdnf install -y util-linux-devel; \
    tdnf install -y e2fsprogs-devel; \
    tdnf install -y rpm-build; \
    tdnf install -y rpm-devel; \
    tdnf install -y openjdk; \
    tdnf install -y apache-maven; \
    tdnf install -y apache-ant; \
    tdnf install -y ant-contrib; \
    tdnf install -y jaxws-ri; \
    tdnf install -y python2-devel; \
    tdnf install -y apache-tomcat; \
    tdnf install -y boost-devel; \
    tdnf install -y openssl-devel; \
    tdnf install -y likewise-open-devel;

EXPOSE 22

ENTRYPOINT ["/bin/bash"]
