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

package com.vmware.identity.interop.ossam;

import com.vmware.identity.interop.Validate;

public class UserInfo
{
    private String _userName;
    private String _userFullName;
    private String _comment;
    private int _flags;

    UserInfo( String userName, String fullName, String comment, int flags )
    {
        Validate.validateNotEmpty( userName, "userName" );
        this._userName = userName;
        this._userFullName = fullName;
        this._comment = comment;
        this._flags = flags;
        
    }

    public String getName()
    {
        return this._userName;
    }
    
    public String getFullName()
    {
        return this._userFullName;
    }
    
    public String getComment()
    {
        return this._comment;
    }

    public int getFlags()
    {
        return this._flags;
    }
}
