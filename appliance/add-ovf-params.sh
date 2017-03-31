#!/bin/bash
# Copyright 2016 VMware, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed
# under the License is distributed on an "AS IS" BASIS, without warranties or
# conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the License for the
# specific language governing permissions and limitations under the License.
#
# This script adds custom ovf tools allowing us to provide configuration
# parameters to the vm created through ovftool.
packerVM=$1
shift
outputVM=$1
shift

sed -i.bak $'s@<VirtualHardwareSection@ <ProductSection ovf:required="false"> \
      <Info/> \
      <Product>Lightwave</Product> \
      <Vendor>VMware Inc.</Vendor> \
      <Version>0.0.1</Version> \
      <Property ovf:key="ip0" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>IP Address</Label> \
        <Description>The IP address for the Lightwave vm. (default: 172.16.127.67)</Description> \
      </Property> \
      <Property ovf:key="netmask0" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Netmask</Label> \
        <Description>The netmask for the Lightwave vm network. (default: 255.255.255.0)</Description> \
      </Property> \
      <Property ovf:key="gateway0" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Default Gateway</Label> \
        <Description>The default gateway address for the Lightwave vm network. (default: 172.16.127.2)</Description> \
      </Property> \
      <Property ovf:key="enable_ssh" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Enable Root SSH Logon</Label> \
        <Description>Enables password logon via ssh for the root user. (default: false)</Description> \
      </Property> \
      <Property ovf:key="DNS" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>DNS</Label> \
        <Description>The domain name servers for the Lightwave vm (comma separated). (default: 172.16.127.2)</Description> \
      </Property> \
      <Property ovf:key="ntp_servers" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>NTP Servers</Label> \
        <Description>Comma-delimited list of NTP servers. (optional)</Description> \
      </Property> \
      <Property ovf:key="root_password" ovf:userConfigurable="true" ovf:type="password"> \
        <Label>Root user password</Label> \
        <Description>This is the photon user password. (default: change on login)</Description> \
      </Property> \
      <Property ovf:key="photon_password" ovf:userConfigurable="true" ovf:type="password"> \
        <Label>Photon user password</Label> \
        <Description>This is the photon user password. (default: change on login)</Description> \
      </Property> \
      <Property ovf:key="lw_password" ovf:userConfigurable="true" ovf:type="password"> \
        <Label>Lightwave administrator password</Label> \
        <Description>This is the Lightwave administrator password. (default: L1ghtwave!)</Description> \
      </Property> \
      <Property ovf:key="lw_domain" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Lightwave domain</Label> \
        <Description>Domain of the Lightwave server. (default: photon.vmware.com)</Description> \
      </Property> \
      <Property ovf:key="lw_hostname" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Lightwave hostname</Label> \
        <Description>Hostname of the Lightwave server. (default: 172.16.127.67)</Description> \
      </Property> \
      <Property ovf:key="lw_replication" ovf:userConfigurable="true" ovf:type="string"> \
        <Label>Lightwave Replication Partner</Label> \
        <Description>IP Address of the Lightwave peer server.</Description> \
      </Property> \
    </ProductSection> \
    <VirtualHardwareSection ovf:transport="com.vmware.guestInfo"@' ${outputVM}.ovf
