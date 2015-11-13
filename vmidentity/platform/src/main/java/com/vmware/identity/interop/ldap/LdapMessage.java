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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.sun.jna.Pointer;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 2:31 PM
 * To change this template use File | Settings | File Templates.
 */
class LdapMessage implements ILdapMessageEx
{
    private LdapConnection _connection;
    private Pointer _message; /* LdapMessage* */
    private final Set<String> _messageAttributes;

    LdapMessage(LdapConnection connection, Pointer message, Set<String> messageAttributes)
    {
        if ( connection == null || connection.isValid() == false)
        {
            throw new IllegalArgumentException("connection");
        }
        if ( message == null || message == Pointer.NULL)
        {
            throw new IllegalArgumentException("message");
        }

        this._connection = connection;
        this._message = message;
        this._messageAttributes = (messageAttributes != null) ? messageAttributes : new HashSet<String>();
    }

    LdapConnection getConnection()
    {
        this.validate();
        return this._connection;
    }

    boolean isValid()
    {
        return ( (this._connection != null) && (this._connection.isValid() == true)
                && (this._message != null) && (this._message != Pointer.NULL) );
    }

    boolean isMessageAttribute(String attribute)
    {
        return (this._messageAttributes.contains(attribute));
    }

    @Override
    public ILdapEntry[] getEntries()
    {
        this.validate();

        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

        List<ILdapEntry> entryList = null;

        int numberEntries = ldapClientLibrary.ldap_count_entries(this._connection.getRawConnection(), this._message);

        if ( numberEntries > 0 )
        {
            int i = 0;
            entryList = new ArrayList<ILdapEntry>(numberEntries);
            Pointer pEntry = ldapClientLibrary.ldap_first_entry(this._connection.getRawConnection(), this._message);

            while ( (pEntry != null) && (pEntry != Pointer.NULL) )
            {
                entryList.add(new LdapEntry(this, pEntry));
                i++;
                if (i < numberEntries)
                {
                    pEntry = ldapClientLibrary.ldap_next_entry( this._connection.getRawConnection(), pEntry);
                }
                else
                {
                    pEntry = Pointer.NULL;
                }
            }
        }
        return ((entryList != null) && (entryList.size() > 0)) ?
               entryList.toArray(new ILdapEntry[entryList.size()]) :
               null;
    }

    @Override
    public int getEntriesCount()
    {
        this.validate();

        ILdapClientLibrary ldapClientLibrary = getLdapLibrary();
        return ldapClientLibrary.ldap_count_entries(this._connection.getRawConnection(), this._message);
    }

    @Override
    public void close()
    {
        if ( ( this._message != null) && (this._message != Pointer.NULL) )
        {
            ILdapClientLibrary ldapClientLibrary = getLdapLibrary();

            ldapClientLibrary.ldap_msgfree(this._message);

            this._message = Pointer.NULL;
        }
        this._connection = null;
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

    private ILdapClientLibrary getLdapLibrary()
    {
        return this._connection.getLdapLibrary();
    }

    private void validate()
    {
        if ( this.isValid() == false )
        {
            throw new IllegalStateException("Accessing a closed instance of LdapMessage.");
        }
    }
}
