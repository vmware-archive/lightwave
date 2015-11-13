/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm.server.provider.activedirectory;

import com.vmware.identity.idm.ValidateUtil;

public class GroupName
{
    private final String _samAccountName;
    private final String _domainName;
    private final String _domainNetBios;
    private final String _groupSid;

    public GroupName(String samAccountName, String domainName, String domainNetBios, String groupSid)
    {
        ValidateUtil.validateNotEmpty(samAccountName, "Group account name");
        ValidateUtil.validateNotEmpty(domainName, "Group domain name");
        ValidateUtil.validateNotEmpty(domainNetBios, "Group netbios name");
        ValidateUtil.validateNotEmpty(groupSid, "Group Sid");

        _samAccountName = samAccountName;
        _domainName = domainName;
        _domainNetBios = domainNetBios;
        _groupSid = groupSid;
    }

    public String getGroupAccountName()
    {
        return _samAccountName;
    }

    public String getGroupDomainFQDN()
    {
        return _domainName;
    }

    public String getGroupDomainNetbios()
    {
        return _domainNetBios;
    }

    public String getGroupSid()
    {
        return _groupSid;
    }
}
