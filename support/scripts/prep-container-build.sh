#!/bin/bash


inject_docker_file_photon_1()
{
    #
    # Assumes that sed and createrepo are already installed
    #
    tmpfile=$(mktemp /tmp/lw.XXXXXX)
    cat >$tmpfile <<-EOF
	COPY x86_64 /tmp/vmware/lightwave/x86_64
	RUN sed -i -e "s/https:\/\/dl.bintray.com/file:\/\/\/tmp/" -e "s/gpgcheck=1/gpgcheck=0/" /etc/yum.repos.d/lightwave.repo && \
	createrepo /tmp/vmware/lightwave && \
	tdnf makecache
	EOF
    sed -i -e "/# Build hook/r $tmpfile" -e "//d" $DOCKER_ROOT/Dockerfile
    rm $tmpfile
}

inject_docker_file_photon_2()
{
    #
    # Assumes that sed and createrepo are already installed
    #
    tmpfile=$(mktemp /tmp/lw.XXXXXX)

    cat >$tmpfile <<-EOF
	COPY x86_64 /tmp/vmware/lightwave/x86_64
	COPY lightwave.repo /etc/yum.repos.d
	RUN createrepo /tmp/vmware/lightwave && \
	tdnf makecache
	EOF

    sed -i -e "/# Build hook/r $tmpfile" -e "//d" $DOCKER_ROOT/Dockerfile
    rm $tmpfile
}

#
# Main
#

PROJECT_ROOT=$(pwd)

DOCKER_ROOT=$PROJECT_ROOT/build/docker

OSVER=
FLAVOR=
if [ $# -gt 1 ]; then
    OSVER=$1
    FLAVOR=$2
fi

case "$OSVER" in
    photon1)
        case "$FLAVOR" in
            server)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/sts/photon1
                ;;
            client)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/client/photon1
                ;;
        esac
        ;;
    photon2)
        case "$FLAVOR" in
            server)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/sts/photon2
                ;;
            client)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/client/photon2
                ;;
            ui)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/ui/photon2
                ;;
            sample)
                DOCKER_SRC_ROOT=$PROJECT_ROOT/support/docker/sample/photon2
                ;;
        esac
        ;;
    *)
        echo "Error: Unsupported o/s version: $OSVER"
        exit 1
        ;;
esac

mkdir -p $DOCKER_ROOT

rm -rf $DOCKER_ROOT/*

cp -r $PROJECT_ROOT/build/rpmbuild/RPMS/x86_64 $DOCKER_ROOT

case "$FLAVOR" in
    ui)
        cp -r $PROJECT_ROOT/ui/stage/RPMS/x86_64/*.rpm $DOCKER_ROOT/x86_64/
    ;;
esac

cp $DOCKER_SRC_ROOT/lightwave-init $DOCKER_ROOT
LIGHTWAVE_REPO_FILE=$DOCKER_SRC_ROOT/lightwave.repo
if [ -f $LIGHTWAVE_REPO_FILE ]; then
    cp $LIGHTWAVE_REPO_FILE $DOCKER_ROOT
fi
cp $DOCKER_SRC_ROOT/Dockerfile $DOCKER_ROOT

# modify Dockerfile to use local lightwave yum repository

case "$OSVER" in
    photon1)
        inject_docker_file_photon_1
        ;;
    photon2)
        inject_docker_file_photon_2
        ;;
esac

