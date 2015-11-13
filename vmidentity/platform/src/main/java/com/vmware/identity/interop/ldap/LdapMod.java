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

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 1:44 PM
 * To change this template use File | Settings | File Templates.
 */
public class LdapMod
{
    public enum LdapModOperation
    {
        ADD (0),
        DELETE (1),
        REPLACE (2);

        private int _code;

        private LdapModOperation(int code)
        {
            _code = code;
        }

        public int getCode()
        {
            return _code;
        }
    }

    private LdapModOperation _operation = LdapModOperation.ADD;
    private String           _modType;
    private LdapValue[]      _values;

    public
    LdapMod(LdapModOperation operation, String modType, LdapValue value)
    {
        this( operation, modType, new LdapValue[] { value } );
    }

    public 
    LdapMod(LdapModOperation operation, String modType, LdapValue[] values)
    {
        this.init(operation, modType, values);
    }

    public
    LdapMod(LdapModOperation operation, String modType, int value)
    {
        this( operation, modType, new LdapValue[] { new LdapValue(value) } );
    }

    public
    LdapMod(LdapModOperation operation, String modType, int[] values)
    {
        LdapValue[] ldapValues = null;
        if( values != null )
        {
            ldapValues = new LdapValue[values.length];
            for(int i = 0; i < values.length; i++)
            {
                ldapValues[i] = new LdapValue(values[i]);
            }
        }

        this.init(operation, modType, ldapValues);
    }

    public
    LdapMod(LdapModOperation operation, String modType, Integer value)
    {
        this( operation, modType, new LdapValue[] { new LdapValue(value) } );
    }

    public
    LdapMod(LdapModOperation operation, String modType, Integer[] values)
    {
        LdapValue[] ldapValues = null;
        if( values != null )
        {
            ldapValues = new LdapValue[values.length];
            for(int i = 0; i < values.length; i++)
            {
                ldapValues[i] = new LdapValue(values[i]);
            }
        }

        this.init(operation, modType, ldapValues);
    }

    public
    LdapMod(LdapModOperation operation, String modType, String value)
    {
        this( operation, modType, new LdapValue[] { new LdapValue(value) } );
    }

    public
    LdapMod(LdapModOperation operation, String modType, String[] values)
    {
        LdapValue[] ldapValues = null;
        if( values != null )
        {
            ldapValues = new LdapValue[values.length];
            for(int i = 0; i < values.length; i++)
            {
                ldapValues[i] = new LdapValue(values[i]);
            }
        }

        this.init(operation, modType, ldapValues);
    }

    public LdapModOperation getOperation()
    {
        return _operation;
    }

    public String getModType()
    {
        return _modType;
    }

    public LdapValue[] getValues()
    {
        return _values;
    }
    
    private void init(LdapModOperation operation, String modType, LdapValue[] values)
    {
        _operation = operation;
        _modType   = modType;
        _values    = values;
    }

}
