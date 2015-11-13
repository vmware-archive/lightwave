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

import com.sun.jna.Platform;

import java.util.Collection;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * Date: 1/6/12
 * Time: 12:05 PM
 * To change this template use File | Settings | File Templates.
 */
public class LinuxRegistryAdapter implements IRegistryAdapter
{
    private static final String REG_KEY_ROOT_PATH = "HKEY_THIS_MACHINE";

    private RegistryConnection _connection = null;

    public LinuxRegistryAdapter()
    {
        if(Platform.isLinux() == false)
        {
            throw new RuntimeException(
            				"This class is only supported on Linux platform.");
        }
    }

    // IRegistryAdapter

    @Override
    public 
    IRegistryKey 
    createKey(IRegistryKey key, String subkey, String regClass, int access)
    {
        return createKey(key, subkey, regClass, access, false);
    }

    @Override
    public
    IRegistryKey 
    createKey(
    	IRegistryKey key, 
    	String       subkey, 
    	String       regClass, 
    	int          access, 
    	boolean      allowExisting
    	)
    {
        return RegistryAdapter.createKey( 
        							this.getConnection(), 
        							LinuxRegistryAdapter.getRegistryKey(key),
        							subkey, 
        							regClass, 
        							access, 
        							allowExisting );
    }

    @Override
    public 
    IRegistryKey 
    openKey(IRegistryKey key, String subkey, int options, int access)
    {
        return RegistryAdapter.openKey( 
        							this.getConnection(),
        							LinuxRegistryAdapter.getRegistryKey(key),
        							subkey, 
        							options,
        							access );
    }

    @Override
    public IRegistryKey openRootKey(int access)
    {
        return this.openKey(null, REG_KEY_ROOT_PATH, 0, access);
    }

    @Override
    public void deleteKey(IRegistryKey key, String subkey)
    {
        RegistryAdapter.deleteKey( 
        					this.getConnection(),
        					LinuxRegistryAdapter.getRegistryKey(key),
        					subkey);
    }

    @Override
    public void deleteTree(IRegistryKey key, String subkey)
    {
        RegistryAdapter.deleteTree( 
        					this.getConnection(),
        					LinuxRegistryAdapter.getRegistryKey(key),
        					subkey);
    }

    @Override
    public void deleteValue(IRegistryKey key, String valuename)
    {
        RegistryAdapter.deleteValue(
        					this.getConnection(),
        					LinuxRegistryAdapter.getRegistryKey(key),
        					valuename);
    }

    @Override
    public 
    String 
    getStringValue(
    	IRegistryKey key, 
    	String       subkey, 
    	String       valuename, 
    	boolean      canBeNullOrEmpty
    	)
    {
        return RegistryAdapter.getStringValue(
        							this.getConnection(),
        							LinuxRegistryAdapter.getRegistryKey(key),
        							subkey, 
        							valuename,
        							canBeNullOrEmpty);
    }

    @Override
    public void setStringValue(IRegistryKey key, String valuename, String value)
    {
        RegistryAdapter.setStringValue( 
        					this.getConnection(), 
        					LinuxRegistryAdapter.getRegistryKey(key), 
        					valuename, 
        					value );
    }

    @Override
    public 
    byte[] 
    getBinaryValue(
    	IRegistryKey key, 
    	String       subkey, 
    	String       valuename, 
    	boolean      canBeNullOrEmpty
    	)
    {
        return RegistryAdapter.getBinaryValue(
        							this.getConnection(),
        							LinuxRegistryAdapter.getRegistryKey(key),
        							subkey,
        							valuename,
        							canBeNullOrEmpty);
    }

    @Override
    public void setBinaryValue(IRegistryKey key, String valuename, byte[] value)
    {
        RegistryAdapter.setBinaryValue( 
        						this.getConnection(), 
        						LinuxRegistryAdapter.getRegistryKey(key), 
        						valuename, 
        						value);
    }

    @Override
    public Integer getIntValue(IRegistryKey key, String subkey, String valuename, boolean canBeNullOrEmpty)
    {
        return RegistryAdapter.getIntValue(
        							this.getConnection(),
        							LinuxRegistryAdapter.getRegistryKey(key),
        							subkey,
        							valuename,
        							canBeNullOrEmpty);
    }

    @Override
    public void setIntValue(IRegistryKey key, String valuename, int value)
    {
        RegistryAdapter.setIntValue(
        					this.getConnection(), 
        					LinuxRegistryAdapter.getRegistryKey(key), 
        					valuename, 
        					value);
    }

    @Override
    public String[] getKeys(IRegistryKey key)
    {
        return RegistryAdapter.getKeys( 
        							this.getConnection(),
        							LinuxRegistryAdapter.getRegistryKey(key));
    }

    @Override
    public boolean doesKeyExist(IRegistryKey key, String subkey)
    {
        return RegistryAdapter.keyExists(
                                    this.getConnection(),
                                    LinuxRegistryAdapter.getRegistryKey(key),
                                    subkey);
    }

    @Override
    public List<RegValueType>  getRegEnumValues(IRegistryKey key)
    {
        return RegistryAdapter.getRegEnumValues(
                                    this.getConnection(),
                                    LinuxRegistryAdapter.getRegistryKey(key)
                                    );
    }
    // privates

    private synchronized RegistryConnection
    getConnection()
    {
        if (_connection == null)
        {
            _connection = RegistryAdapter.connect();
        }

        return _connection;
    }

    private static RegistryKey getRegistryKey(IRegistryKey key)
    {
        RegistryKey theKey = null;
        if(key != null)
        {
            if ( ( key instanceof RegistryKey ) == false )
            {
                throw new IllegalArgumentException(
                        "Parameter key must be an instance of RegistryKey.");

            }

            theKey = (RegistryKey)key;
        }

        return theKey;
    }

	@Override
	public void 
	setMultiStringValue(IRegistryKey key, String valuename, Collection<String> value)
	{
        RegistryAdapter.setMultiStringValue(
								this.getConnection(), 
								LinuxRegistryAdapter.getRegistryKey(key), 
								valuename, 
								value);
		
	}

	@Override
	public 
	Collection<String>
	getMultiStringValue(
		IRegistryKey key, 
		String       subkey,
		String       valuename, 
		boolean      canBeNullOrEmpty
		) 
	{
		return RegistryAdapter.getMultiStringValue(
									this.getConnection(), 
									LinuxRegistryAdapter.getRegistryKey(key), 
									subkey, 
									valuename, 
									canBeNullOrEmpty);
	}
}

