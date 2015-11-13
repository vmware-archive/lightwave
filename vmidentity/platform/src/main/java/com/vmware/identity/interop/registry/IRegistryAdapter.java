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

package com.vmware.identity.interop.registry;

import java.util.Collection;
import java.util.List;

import com.vmware.identity.interop.registry.RegValueType;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/5/12
 * Time: 11:53 AM
 * To change this template use File | Settings | File Templates.
 */
public interface IRegistryAdapter {

    public IRegistryKey  createKey(
            IRegistryKey        key,
            String             subkey,
            String             regClass,
            int                access
    );

    public IRegistryKey  createKey(
            IRegistryKey        key,
            String             subkey,
            String             regClass,
            int                access,
            boolean            allowExisting
    );

    public IRegistryKey openKey(
            IRegistryKey        key,
            String             subkey,
            int                options,
            int                access
    );

    public IRegistryKey openRootKey(
            int                access
    );

    public void deleteKey(
            IRegistryKey        key,
            String             subkey
    );

    public void deleteTree(
            IRegistryKey        key,
            String             subkey
    );

    public void deleteValue(
            IRegistryKey        key,
            String             valuename
    );

    public String getStringValue(
            IRegistryKey        key,
            String             subkey,
            String             valuename,
            boolean            canBeNullOrEmpty
    );

    public void setStringValue(
            IRegistryKey        key,
            String             valuename,
            String             value
    );
    
    public Collection<String> getMultiStringValue(
    		IRegistryKey       key,
    		String             subkey,
    		String             valuename,
    		boolean            canBeNullOrEmpty
    		);
    
    public void setMultiStringValue(
            IRegistryKey       key,
            String             valuename,
            Collection<String> value
    );

    public byte[] getBinaryValue(
            IRegistryKey        key,
            String             subkey,
            String             valuename,
            boolean            canBeNullOrEmpty
    );

    public void setBinaryValue(
            IRegistryKey        key,
            String             valuename,
            byte[]             value
    );

    public Integer getIntValue(
            IRegistryKey        key,
            String             subkey,
            String             valuename,
            boolean            canBeNullOrEmpty
    );

    public void setIntValue(
            IRegistryKey        key,
            String             valuename,
            int                value
    );

    public String[] getKeys(
            IRegistryKey        key
    );

    public boolean doesKeyExist(
            IRegistryKey key,
            String subkey
    );

    public  List<RegValueType> getRegEnumValues (
IRegistryKey key
    );

}
