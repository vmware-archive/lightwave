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

package com.vmware.identity.idm;

import java.io.Serializable;

public class ActiveDirectoryJoinInfo implements Serializable {

    private static final long serialVersionUID = -7530720126011282811L;

    public enum JoinStatus
    {
        ACTIVE_DIRECTORY_JOIN_STATUS_UNKNOWN,
        ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP,
        ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN
    }

    private final JoinStatus _status;
    private final String _name;
    private final String _alias;
    private final String _dn;

    public ActiveDirectoryJoinInfo()
    {
        _status = JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_UNKNOWN;
        _name = "";
        _alias = "";
        _dn = null;
    }

    public ActiveDirectoryJoinInfo(String name, String alias)
    {
       this(name, alias, null);
    }

    public ActiveDirectoryJoinInfo(String name, String alias, String dn)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        ValidateUtil.validateNotEmpty(alias, "Alias");

        if (name.equalsIgnoreCase("WORKGROUP"))
        {
            _name = "WORKGROUP";
            _alias = "";
            _dn = null;
            _status = JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP;
        }
        else
        {
            _name = name;
            _alias = alias;
            _dn = dn;
            _status = JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN;
        }
    }

    public String getName()
    {
        return _name;
    }

    public String getAlias()
    {
        return _alias;
    }

    public String getDn()
    {
       return _dn;
    }

    public JoinStatus getJoinStatus()
    {
        return _status;
    }
}
