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

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.TypeMapper;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;

/**
 * Created by IntelliJ IDEA.
 * User: krishnag
 * Date: 12/19/11
 * Time: 1:46 PM
 * To change this template use File | Settings | File Templates.
 */

/**
 * Do not use this class externally.
 * It is intended for internal use by Platform package.
 * This class must stay public for interop ...
 */

public final class LdapModNative extends Structure implements AutoCloseable
{
    static int LDAP_MOD_BVALUES = 0x80;

    public int     modOp;
    public String  modType;
    public Pointer values;

    private BerValNativeArray nativeBerValArray;

    public
    LdapModNative(LdapMod mod, TypeMapper mapper)
    {
        super(mapper);
        this.nativeBerValArray = null;

        LdapModOperation op = mod.getOperation();

        modOp   = op.getCode() | LDAP_MOD_BVALUES;
        modType = mod.getModType();

        LdapValue[] modVals = mod.getValues();

        switch (op) {
        case DELETE:
           if (modVals != null && modVals.length > 0)
            {
                this.nativeBerValArray = new BerValNativeArray(modVals);
                values = this.nativeBerValArray.getPointer();
            }
            else
            {
                values = Pointer.NULL;
            }
            break;

        case ADD:
            this.nativeBerValArray = new BerValNativeArray(modVals);
            values = this.nativeBerValArray.getPointer();
            break;

        case REPLACE:
           if (modVals != null && modVals.length > 0)
           {
               this.nativeBerValArray = new BerValNativeArray(modVals);
               values = this.nativeBerValArray.getPointer();
           }
           else
           {
               values = Pointer.NULL;
           }
           break;

        default:
            throw new RuntimeException("Unsupported modification type ["
               + op.toString() + "]");
      }

      write();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "modOp", "modType", "values"
        });
    }

    @Override
    public void close()
    {
        if ( this.nativeBerValArray != null )
        {
            this.nativeBerValArray.close();
            this.nativeBerValArray = null;
        }
    }
}
