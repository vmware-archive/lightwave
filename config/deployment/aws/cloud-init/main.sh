#!/bin/bash -xe

# execute sub-scripts
/root/cloud-init/install-tooling.sh
/root/cloud-init/retrieve-variables.sh
/root/cloud-init/configure-host.sh
/root/cloud-init/install-ssm.sh
/root/cloud-init/install-code-deploy.sh
