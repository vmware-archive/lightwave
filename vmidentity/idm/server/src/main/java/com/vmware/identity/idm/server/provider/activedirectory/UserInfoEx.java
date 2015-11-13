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

import java.util.List;

import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.interop.Validate;

public class UserInfoEx
{
    private final String _userSid;
    private final String _samAccountName;
    private final String _upnName;
    private final List<GroupName> _resolvedGroupNames;

    public UserInfoEx(String samAccountName, String upnName, List<GroupName> resolvedGroupNames, String userSid)
    {
        ValidateUtil.validateNotEmpty(samAccountName, "User samAccountName");
        ValidateUtil.validateNotEmpty(upnName, "User UPN name");
        ValidateUtil.validateNotNull(userSid, "userSid");

        _samAccountName = samAccountName;
        _upnName = upnName;
        _resolvedGroupNames = resolvedGroupNames;
        _userSid = userSid;
    }

    public String getUserSamAccount()
    {
        return _samAccountName;
    }

    public String getUserUpnName()
    {
        return _upnName;
    }

    public List<GroupName> getGroupNamesInfo()
    {
        return _resolvedGroupNames;
    }

    public String getUserSid()
    {
        return _userSid;
    }
}
