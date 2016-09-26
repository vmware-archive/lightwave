#!/bin/sh -xe

# boot2docker VMs do not have this directory present. Create it for systemd in lightwave container.
sudo mkdir -p /sys/fs/cgroup/systemd
sudo mount -t cgroup -o none,name=systemd cgroup /sys/fs/cgroup/systemd || true
sudo mkdir -p /sys/fs/cgroup/systemd/user
echo $$ | sudo tee -a /sys/fs/cgroup/systemd/user/cgroup.procs
