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

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 2:31 PM
 * To change this template use File | Settings | File Templates.
 */
class LdapPagedSearchResult implements ILdapPagedSearchResult
{
    private LdapMessage _ldapMessage;
    private Pointer _berCookie;
    private boolean _isSearchFinished;

    LdapPagedSearchResult(LdapMessage ldapMessage, Pointer berCookie, boolean isSearchFinished)
    {
        if ( ldapMessage == null || ldapMessage.isValid() == false)
        {
            throw new IllegalArgumentException("ldapMessage");
        }
        this._ldapMessage = ldapMessage;
        this._berCookie = berCookie;
        this._isSearchFinished = isSearchFinished;
    }

    public Pointer berCookiePtr()
    {
        return this._berCookie;
    }

    @Override
    public boolean isSearchFinished()
    {
        return this._isSearchFinished;
    }

    @Override
    public ILdapEntry[] getEntries()
    {
        return this._ldapMessage.getEntries();
    }

    @Override
    public void close()
    {
        if (this._berCookie != null && this._berCookie != Pointer.NULL)
        {
            ILdapClientLibrary ldapClientLibrary = this._ldapMessage.getConnection().getLdapLibrary();

            ldapClientLibrary.ber_bvfree(this._berCookie);

            this._berCookie = Pointer.NULL;
        }

        if ( this._ldapMessage != null )
        {
            this._ldapMessage.close();
        }
    }

    @Override
    protected void finalize() throws Throwable
    {
        try
        {
            this.close();
        }
        finally
        {
            super.finalize();
        }
    }
}
