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



#include "includes.h"

DWORD
VmwDeployStartService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    struct stat statbuf={0};

    PCSTR vmStart[] = {
            "systemctl start vmware-vmdird.service",
            "systemctl start vmware-vmafdd.service",
            "systemctl start vmware-vmcad.service",
            "systemctl start vmware-vmdnsd.service",
            "/opt/vmware/sbin/vmware-vmdird.sh start",
            "/opt/vmware/sbin/vmware-vmafdd.sh start",
            "/opt/vmware/sbin/vmware-vmcad.sh start",
            "/opt/vmware/sbin/vmware-vmdnsd.sh start"
            };

    if (stat("/.dockerenv", &statbuf) == 0)
    {
        dwIndex = 4;
    }

    if (VmStringCompareA(pszName, "vmafd", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+1]);
    }
    else if (VmStringCompareA(pszName, "vmdir", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+0]);
    }
    else if (VmStringCompareA(pszName, "vmca", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+2]);
    }
    else if (VmStringCompareA(pszName, "vmdns", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+3]);
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployStopService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    struct stat statbuf={0};

    PCSTR vmStart[] = {
            "systemctl stop vmware-vmdird.service",
            "systemctl stop vmware-vmafdd.service",
            "systemctl stop vmware-vmcad.service",
            "systemctl stop vmware-vmdnsd.service",
            "/opt/vmware/sbin/vmware-vmdird.sh stop",
            "/opt/vmware/sbin/vmware-vmafdd.sh stop",
            "/opt/vmware/sbin/vmware-vmcad.sh stop",
            "/opt/vmware/sbin/vmware-vmdnsd.sh stop"
            };

    if (stat("/.dockerenv", &statbuf) == 0)
    {
        dwIndex = 4;
    }

    if (VmStringCompareA(pszName, "vmafd", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+1]);
    }
    else if (VmStringCompareA(pszName, "vmdir", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+0]);
    }
    else if (VmStringCompareA(pszName, "vmca", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+2]);
    }
    else if (VmStringCompareA(pszName, "vmdns", FALSE) == 0)
    {
        dwError = system(vmStart[dwIndex+3]);
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmwDeployRestartService(
    PCSTR pszName
    )
{
    DWORD dwError = 0;

    dwError = VmwDeployStopService(pszName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployStartService(pszName);
    BAIL_ON_DEPLOY_ERROR(dwError);

error:

    return dwError;
}
