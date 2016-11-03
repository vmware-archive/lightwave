FROM vmware/photon:1.0
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.version="1.0.1"

EXPOSE 22 53/udp 53 88/udp 88 389 443 636 2012 2014 2015 2020
ENTRYPOINT ["/usr/sbin/init"]

# Update tdnf and rpm
RUN tdnf update --refresh -y \
        tdnf \
        rpm-4.11.2

# Install needed packages
RUN tdnf install -y \
        apache-tomcat-8.0.35 \
        boost-1.60.0 \
        commons-daemon-1.0.15-6.ph1.x86_64 \
        likewise-open-6.2.10 \
        openjre-1.8.0.92 \
        procps-ng-3.3.11-3.ph1 \
        sed-4.2.2-2.ph1.x86_64 \
        systemd-228-31.ph1 \
        vmware-lightwave-server-6.6.1

# Remove unused files and systemd services, and configure services
RUN rm -f /etc/systemd/system/*.wants/* && \
    rm -f /lib/systemd/system/sysinit.target.wants/systemd-tmpfiles-setup.service && \
    rm -f /lib/systemd/system/multi-user.target.wants/* && \
    rm -f /lib/systemd/system/local-fs.target.wants/* && \
    rm -f /lib/systemd/system/sockets.target.wants/*udev* && \
    rm -f /lib/systemd/system/sockets.target.wants/*initctl* && \
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
