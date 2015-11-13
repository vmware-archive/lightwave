#!/bin/bash

PHOTON_EXTRAS_REPO=/etc/yum.repos.d/photon-extras.repo

if [ ! -f $PHOTON_EXTRAS_REPO ]
then

cat > $PHOTON_EXTRAS_REPO <<EOF
[photon-extras]
name=VMWare Photon Extras 1.0(x86_64)
baseurl=https://dl.bintray.com/vmware/photon_extras
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=0
enabled=1
skip_if_unavailable=True
EOF

fi

tdnf install -y openjdk
tdnf install -y apache-maven
tdnf install -y apache-ant
tdnf install -y ant-contrib
tdnf install -y jaxws-ri
tdnf install -y likewise-open-6.2.2
