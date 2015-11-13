/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

public class ActiveDirectoryJoinInfo {

    public static enum JoinStatus
    {
        ACTIVE_DIRECTORY_JOIN_STATUS_UNKNOWN,
        ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP,
        ACTIVE_DIRECTORY_JOIN_STATUS_DOMAIN
    }

    private final JoinStatus _status;
    private final String _name;
    private final String _alias;
    private final String _dn;

    public ActiveDirectoryJoinInfo(String name, String alias, String dn)
    {
        ValidateUtil.validateNotEmpty(name, "name");

        if (name.equalsIgnoreCase("WORKGROUP"))
        {
            _name = "WORKGROUP";
            _alias = "";
            _dn = null;
            _status = JoinStatus.ACTIVE_DIRECTORY_JOIN_STATUS_WORKGROUP;
        }
        else
        {
            ValidateUtil.validateNotEmpty(alias, "Alias");

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
