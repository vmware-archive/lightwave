FROM vmware/photon:latest
ENV container=docker
VOLUME ["/sys/fs/cgroup"]

#so that we have all the dependencies
RUN tdnf install -y lightwave-server net-tools inetutils iana-etc

RUN curl -L https://vmware.bintray.com/photon_publish_rpms/x86_64/lmdb-0.9.21-1.ph2.x86_64.rpm -o lmdb.rpm
RUN rpm -ivh lmdb.rpm

#set env vars
ENV VMDIR_FIRST_NODE_NAME "vmdir1.vmdir.test"
ENV VMDIR_BIND_DN         "cn=administrator,cn=users,dc=vmdir,dc=test"
ARG VMDIR_PWD_IN
ENV VMDIR_PWD=$VMDIR_PWD_IN

#copy setup and inspect scripts
COPY scripts/vmdir_setup.sh /
COPY scripts/vmdir_user_stats.sh /

RUN chmod +x /vmdir_user_stats.sh
CMD ["/bin/bash"]
