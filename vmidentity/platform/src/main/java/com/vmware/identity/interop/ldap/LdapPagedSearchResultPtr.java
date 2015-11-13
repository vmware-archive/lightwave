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

package com.vmware.identity.interop.ldap;

import com.sun.jna.Pointer;

class LdapPagedSearchResultPtr
{
    private Pointer ldapMessage;
    private boolean bSearchFinished;
    private Pointer bBerCookie;

    public LdapPagedSearchResultPtr(Pointer ldapMessage, boolean bSearchFinished, Pointer bBerCookie)
    {
        this.ldapMessage = ldapMessage;
        this.bSearchFinished = bSearchFinished;
        this.bBerCookie = bBerCookie;
    }

    public boolean isSearchFinished()
    {
        return this.bSearchFinished;
    }

    public Pointer getCurrentCookiePtr()
    {
        return this.bBerCookie;
    }

    public Pointer getSearchLdapMessagePtr()
    {
        return this.ldapMessage;
    }
}
