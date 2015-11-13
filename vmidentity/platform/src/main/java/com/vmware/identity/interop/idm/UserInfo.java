/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.idm;

import java.util.List;

import com.vmware.identity.interop.Validate;

public class UserInfo {

    private final String _name;
    private final String _domain;
    private final List<String> _groups;
    private final List<String> _groupSids;
    private final String _userSid;

    public UserInfo(String name, String domain, List<String> groups, List<String> groupSids, String userSid)
    {
        Validate.validateNotEmpty(name, "Name");
        Validate.validateNotEmpty(domain, "Domain");
        Validate.validateNotNull(groups, "Groups");
        Validate.validateNotNull(groupSids, "groupSids");
        Validate.validateNotNull(userSid, "userSid");

        _name = name;
        _domain = domain;
        _groups = groups;
        _groupSids = groupSids;
        _userSid = userSid;
    }

    public String getName() {
        return _name;
    }

    public String getDomain() {
        return _domain;
    }

    public List<String> getGroups() {
        return _groups;
    }

    public List<String> getGroupSids() {
        return _groupSids;
    }

    public String getUserSid() {
        return _userSid;
    }

    public String getUPN() {
        return String.format("%s@%s", _name, _domain);
    }
}
