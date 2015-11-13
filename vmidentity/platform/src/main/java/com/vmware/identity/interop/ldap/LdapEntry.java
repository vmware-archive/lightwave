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
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;

class LdapEntry implements ILdapEntry
{
    private final LdapMessage _message;
    private final Pointer _entry; /* LdapMessage* */
    private HashSet<String> _attributeNamesInEntry;
    private boolean _bAttributeNamesProcessed = false;

    LdapEntry(LdapMessage message, Pointer entry)
    {
        if ( message == null || message.isValid() == false)
        {
            throw new IllegalArgumentException("connection");
        }
        if ( entry == null || entry == Pointer.NULL)
        {
            throw new IllegalArgumentException("message");
        }

        this._message = message;
        this._entry = entry;
        this._attributeNamesInEntry = null;
     }

    @Override
    public String getDN()
    {
        this.validate();

        Pointer pDN = Pointer.NULL;
        ILdapClientLibrary ldapLibrary = getLdapLibrary();

        try
        {
            pDN = ldapLibrary.ldap_get_dn(this._message.getConnection().getRawConnection(),
                                                this._entry);

            return ldapLibrary.getString(pDN);
        }
        finally
        {
            if (pDN != Pointer.NULL)
            {
                ldapLibrary.ldap_memfree(pDN);
            }
        }
    }

    @Override
    public String[] getAttributeNames()
    {
        this.validate();

        List<String> attrList = new ArrayList<String>();

        ILdapClientLibrary ldapLibrary = getLdapLibrary();
        Pointer pAttr = Pointer.NULL;
        PointerByReference ppBerElem = new PointerByReference();

        try
        {
            pAttr =
                    ldapLibrary.ldap_first_attribute(this._message.getConnection().getRawConnection(),
                                                     this._entry,
                                                     ppBerElem);

            while ( (pAttr != null) && (pAttr != Pointer.NULL) )
            {
                attrList.add(ldapLibrary.getString(pAttr));

                if (pAttr != Pointer.NULL)
                {
                    ldapLibrary.ldap_memfree(pAttr);
                    pAttr = Pointer.NULL;
                }

                pAttr = ldapLibrary.ldap_next_attribute(this._message.getConnection().getRawConnection(),
                                                        this._entry,
                                                        ppBerElem.getValue());
            }
        }
        finally
        {
            if (pAttr != Pointer.NULL)
            {
                ldapLibrary.ldap_memfree(pAttr);
            }
            if (ppBerElem.getValue() != Pointer.NULL)
            {
                /*
                    OpenLdap:
                    The caller is solely responsible for freeing the BerElement pointed to by berptr
                    when it     is no longer needed  by  calling  ber_free(3).
                    When  calling  ber_free(3) in this instance, be sure the second argument is 0.

                   MSDN:
                   When you have finished stepping through a list of attributes
                   and ptr is non-NULL, free the pointer by calling ber_free( ptr, 0 ).
                   Be aware that you must pass the second parameter as 0 (zero) in this call.
               */
               ldapLibrary.ber_free(ppBerElem.getValue(), 0);
            }
        }

        String[] attributeNames = attrList.size() > 0 ? attrList.toArray(new String[attrList.size()]) : null;

        // if this._attributeNamesInEntry is null, cache it
        if (this._attributeNamesInEntry == null && attributeNames != null && attributeNames.length > 0)
        {
            String[] attributeNamesInEntry = new String[attributeNames.length];

            for (int i = 0; i < attributeNames.length; i++)
            {
                attributeNamesInEntry[i] = attributeNames[i].toLowerCase();
            }

            this._attributeNamesInEntry = new HashSet<String>(Arrays.asList(attributeNamesInEntry));
        }

        return attributeNames;
    }

    @Override
    public LdapValue[] getAttributeValues(String attributeName)
    {
        this.validate();

        List<LdapValue> valueList = new ArrayList<LdapValue>();

        ILdapClientLibrary ldapLibrary = getLdapLibrary();

        Pointer pValues = null;

        // if attributeNames are not yet processed, process it and set nameCached to 1
        if (!this._bAttributeNamesProcessed)
        {
            // this.getAttributeNames fill in this._attributeNamesInEntry
            this.getAttributeNames();
            this._bAttributeNamesProcessed = true;
        }

        try
        {
            // make sure AttributeNames are processed
            // In case (1) we find no attribute value; or
            // (2) Cached attributeNames do not contain the attribute
            if (this._bAttributeNamesProcessed &&
                 (this._attributeNamesInEntry == null ||
                  (this._attributeNamesInEntry != null && !this._attributeNamesInEntry.contains(attributeName.toLowerCase())))
               )
            {
                throw new NoSuchAttributeLdapException(LdapErrors.LDAP_NO_SUCH_ATTRIBUTE.getCode(),
                                                       String.format("attribute %s does not exist in this LdapEntry", attributeName));
            }

            pValues = ldapLibrary.ldap_get_values_len(this._message.getConnection().getRawConnection(),
                                                      this._entry,
                                                      attributeName);

            if (pValues != null)
            {
                int numValues = ldapLibrary.ldap_count_values_len(pValues);

                if (numValues > 0)
                {
                    for (BerValNative value : BerValNative.fromPointerArray(pValues,
                                                                            numValues))
                    {
                        if (value.length > 0)
                        {
                            byte[] bytes = value.value.getByteArray(0, value.length);

                            valueList.add(new LdapValue(bytes));
                        }
                    }
                }
            }
        }
        catch(NoSuchAttributeLdapException ex)
        {
            if( this._message.isMessageAttribute( attributeName ) == false )
            {
                throw ex;
            }

            // optional attributes might not even come back in the message
            // in this case we want to handle it here returning a null value,
            // instead of throwing an exception ....
            valueList = new ArrayList<LdapValue>();
        }
        finally
        {
            if (pValues != null)
            {
                ldapLibrary.ldap_value_free_len(pValues);
            }
        }

        return valueList.size() > 0 ? valueList.toArray(new LdapValue[valueList.size()]) : null;
    }

    private void validate()
    {
        if ( (this._message == null) || (this._message.isValid() == false)
                || (this._entry == null) || (this._entry == Pointer.NULL) )
        {
            throw new IllegalStateException("Accessing a closed instance of LdapMessage.");
        }
    }

    private ILdapClientLibrary getLdapLibrary()
    {
        return this._message.getConnection().getLdapLibrary();
    }
}
