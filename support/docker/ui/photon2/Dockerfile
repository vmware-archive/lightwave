FROM vmware/lightwave-base-photon2:1.0.1
MAINTAINER "Sriram Nambakam" <snambakam@vmware.com>
ENV container=docker
VOLUME ["/sys/fs/cgroup"]
LABEL vendor="VMware, Inc."
LABEL com.vmware.lightwave.ui.version="1.3.1"

EXPOSE 443
ENTRYPOINT ["/lightwave-init"]

# Build hook

RUN tdnf install -y lightwave-client-1.3.1 \
                    lightwave-ui-0.1 \
                    util-linux && \
    rpm -e --nodeps systemd && \
    rpm -e createrepo_c && \
    rm -rf /usr/share/doc/* && \
    rm -rf /usr/share/man/* && \
    rm -rf /usr/include/* && \
    rm -rf /tmp/vmware/lightwave

ADD lightwave-init /
