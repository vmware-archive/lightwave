/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


package com.vmware.af;

import com.vmware.af.interop.VmAfClientAdapter;

public class VmAfClient
{
    private String _vmAfServerUrl;
    private Object _vmAfHeartbeatHandle;

    public VmAfClient(final String vmAfServerUrl)
    {
       assert(vmAfServerUrl != null && !vmAfServerUrl.isEmpty());
       _vmAfServerUrl = vmAfServerUrl;
    }

    public int getStatus()
    {
        return VmAfClientAdapter.getStatus(_vmAfServerUrl);
    }

    public  String getDomainName()
    {
        return VmAfClientAdapter.getDomainName(_vmAfServerUrl);
    }

    public void setDomainName(String domain)
    {
        VmAfClientAdapter.setDomainName(_vmAfServerUrl, domain);
    }

    public int getRHTTPProxyPort()
    {
        return VmAfClientAdapter.getRHTTPProxyPort(_vmAfServerUrl);
    }

    public void setRHTTPProxyPort(int port)
    {
        VmAfClientAdapter.setRHTTPProxyPort(_vmAfServerUrl, port);
    }

    public void setDCPort(int port)
    {
        VmAfClientAdapter.setDCPort(_vmAfServerUrl, port);
    }

    public String getCMLocation()
    {
        return VmAfClientAdapter.getCMLocation(_vmAfServerUrl);
    }

    public String getLSLocation()
    {
        return VmAfClientAdapter.getLSLocation(_vmAfServerUrl);
    }

    public String getPNID()
    {
        return VmAfClientAdapter.getPNID(_vmAfServerUrl);
    }

    public String getPNIDUrl()
    {
        return VmAfClientAdapter.getPNIDUrl(_vmAfServerUrl);
    }

    public String getDomainController()
    {
        return VmAfClientAdapter.getDCName(_vmAfServerUrl);
    }

    public void setDomainController(String dc)
    {
        VmAfClientAdapter.setDCName(_vmAfServerUrl, dc);
    }

    public String getLDU()
    {
        return VmAfClientAdapter.getLDU(_vmAfServerUrl);
    }

    public void setLDU(String ldu)
    {
        VmAfClientAdapter.setLDU(_vmAfServerUrl, ldu);
    }

    public void promoteVmDir(String server,
                             String domain,
                             String user,
                             String password,
                             String site,
                             String partner)
    {
        VmAfClientAdapter.promoteVmDir(server, domain, user, password, site, partner);
    }

    public void demoteVmDir(String server,
                            String user,
                            String password)
    {
        VmAfClientAdapter.demoteVmDir(server, user, password);
    }

    public void joinVmDir(String server,
                          String user,
                          String password,
                          String machine,
                          String domain,
                          String orgunit)
    {
        VmAfClientAdapter.joinVmDir(server, user, password, machine, domain, orgunit);
    }

    public void leaveVmDir(String server,
                           String user,
                           String password)
    {
        VmAfClientAdapter.leaveVmDir(server, user, password);
    }

    public String createComputerAccount(
              String user,
              String password,
              String machine,
              String orgunit)
    {
        return VmAfClientAdapter.createComputerAccount(
                      user,
                      password,
                      machine,
                      orgunit);
    }

    public void joinActiveDirectory(String server,
                                    String user,
                                    String password,
                                    String domain,
                                    String orgunit)
    {
        VmAfClientAdapter.joinAD(server, user, password, domain, orgunit);
    }

    public void leaveActiveDirectory(String server,
                                     String user,
                                     String password)
    {
        VmAfClientAdapter.leaveAD(server, user, password);
    }

    public DomainInfo queryActiveDirectory(String server)
    {
        return VmAfClientAdapter.queryAD(server);
    }

    public PasswordCredential getMachineAccountCredentials()
    {
        return VmAfClientAdapter.getMachineAccountCredentials();
    }

    public String getSiteGUID()
    {
        return VmAfClientAdapter.getSiteGUID(_vmAfServerUrl);
    }

    public String getMachineID()
    {
        return VmAfClientAdapter.getMachineID(_vmAfServerUrl);
    }

    public void startHeartbeat(String ServiceName, int Port)
    {
        Object pHeartbeatHandle = VmAfClientAdapter.startHeartbeat(
                                                             ServiceName,
                                                             Port);

        _vmAfHeartbeatHandle = pHeartbeatHandle;
    }

    public void stopHeartbeat()
    {
        if (_vmAfHeartbeatHandle != null)
        {
            VmAfClientAdapter.stopHeartbeat(_vmAfHeartbeatHandle);
            _vmAfHeartbeatHandle = null;
        }
    }
}
