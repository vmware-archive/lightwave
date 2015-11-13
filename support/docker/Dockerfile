FROM vmware/photon:latest
MAINTAINER "Jonathan Brown" <brownj@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
ADD photon-extras.repo /etc/yum.repos.d/
ADD lightwave.repo /etc/yum.repos.d/
RUN mkdir /tmp/lightwave_rpms
ADD *.rpm /tmp/lightwave_rpms/
RUN rm -f /tmp/lightwave_rpms/*-devel*.rpm

# install systemd
RUN tdnf install -y systemd;\
rm -f /etc/systemd/system/*.wants/*;\
rm -f /lib/systemd/system/sysinit.target.wants/systemd-tmpfiles-setup.service;\
rm -f /lib/systemd/system/multi-user.target.wants/*;\
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*

# install sshd
RUN tdnf install -y openssh; tdnf install -y sed
RUN mkdir -p /var/run/sshd; chmod -rx /var/run/sshd
RUN ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key

# install lightwave
RUN tdnf install -y procps-ng; tdnf install -y openjdk-1.8.0.51; tdnf install -y commons-daemon; tdnf install -y apache-tomcat; tdnf install -y likewise-open-6.2.9; tdnf install -y boost; tdnf install -y gawk

RUN rpm -i /tmp/lightwave_rpms/*.rpm; rm -rf /tmp/lightwave_rpms /var/run/lwregd.pid

ADD configure-lightwave-server.service /usr/lib/systemd/system/
RUN ln -s /usr/lib/systemd/system/configure-lightwave-server.service \
          /etc/systemd/system/multi-user.target.wants/configure-lightwave-server.service

EXPOSE 22 53/udp 53 88/udp 88 389 636 2012 2014 2020

ENTRYPOINT ["/usr/sbin/init"]
