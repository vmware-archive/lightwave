FROM vmware/photon:latest
ENV container=docker
VOLUME ["/sys/fs/cgroup"]

#so that we have all the dependencies
RUN tdnf install -y lightwave-post

#manually run "make postrpm" from lwraft build folder 
#and copy the lightwave-post*.rpm to this
#directory to work with local built artifacts.
COPY lightwave-*.rpm /
RUN rpm -Uvh --nodeps --force /lightwave*.rpm

RUN curl -L https://vmware.bintray.com/photon_publish_rpms/x86_64/lmdb-0.9.21-1.ph2.x86_64.rpm -o lmdb.rpm
RUN rpm -ivh lmdb.rpm

#set env vars
ENV POST_FIRST_NODE_NAME "post1.post.test"
ENV POST_BIND_DN         "cn=administrator,cn=users,dc=post,dc=test"
ARG POST_PWD_IN
ENV POST_PWD=$POST_PWD_IN

#copy setup and inspect scripts
COPY scripts/post_*.sh /

RUN chmod +x /post_*.sh
CMD ["/bin/bash"]
