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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map.Entry;
import java.util.TreeMap;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.*;
import com.sun.jna.ptr.IntByReference;

import org.apache.commons.logging.LogFactory;

import com.vmware.identity.interop.Validate;
import com.vmware.identity.interop.registry.RegistryUnSupportedKeyTypeException;


/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/5/12
 * Time: 1:47 PM
 * To change this template use File | Settings | File Templates.
 */
public class WinRegistryAdapter implements IRegistryAdapter {

    private static WinRegistryAdapter ourInstance = new WinRegistryAdapter();

    public static WinRegistryAdapter getInstance() {
        return ourInstance;
    }

    @Override
    public IRegistryKey createKey(IRegistryKey key, String subkey, String regClass, int access)
    {
        return createKey( key, subkey, regClass, access, false);
    }

    @Override
    public IRegistryKey createKey(IRegistryKey key, String subkey, String regClass, int access, boolean allowExisting)
    {
        Validate.validateNotNull(key, "key");
        Validate.validateNotEmpty(subkey, "subkey");
        /*
        MSDN:
        lpClass [in, optional]
            The user-defined class type of this key. This parameter may be ignored. This parameter can be NULL.
        */

        WinRegistryKey retKey = null;
        WinReg.HKEYByReference hkeyByRef = new WinReg.HKEYByReference();
        IntByReference intByRef = new IntByReference();

        WinRegistryAdapter.CheckWin32Error(
                Advapi32.INSTANCE.RegCreateKeyEx(
                        WinRegistryAdapter.getRegistryKey(key).getKey(), subkey, 0, regClass, 0, access, null, hkeyByRef, intByRef
                )
        );

        retKey = new WinRegistryKey( hkeyByRef.getValue() );

        if ( (allowExisting == false) && ( intByRef.getValue() != WinNT.REG_CREATED_NEW_KEY ) )
        {
            retKey.close();
            throw new IllegalArgumentException( String.format("Registry key '%s' already exists.", subkey) );
        }

        return retKey;
    }

    @Override
    public IRegistryKey openKey(IRegistryKey key, String subkey, int options, int access)
    {
        Validate.validateNotNull(key, "key");
        Validate.validateNotNull(subkey, "subkey"); // msdn says the function allows null for predefined keys, but for now we will limit this.

        WinReg.HKEYByReference hkey = new WinReg.HKEYByReference();

        WinRegistryAdapter.CheckWin32Error(
                Advapi32.INSTANCE.RegOpenKeyEx( WinRegistryAdapter.getRegistryKey(key).getKey(), subkey, options, access, hkey )
        );

        return new WinRegistryKey( hkey.getValue() );
    }

    public IRegistryKey openRootKey( int access )
    {
        return new WinRegistryKey(WinReg.HKEY_LOCAL_MACHINE);
    }
    
    @Override
    public void deleteKey(IRegistryKey key, String subkey)
    {
        Validate.validateNotNull(key, "key");
        Validate.validateNotNull(subkey, "subkey");

        Advapi32Util.registryDeleteKey(WinRegistryAdapter.getRegistryKey(key).getKey(), subkey);
    }

    @Override
    public void deleteTree(IRegistryKey key, String subkey)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
        lpSubKey [in, optional]
            The name of the key. This key must be a subkey of the key identified by the hKey parameter.
            If this parameter is NULL, the subkeys and values of hKey are deleted.
        */

        WinRegistryAdapter.CheckWin32Error(
                IAdvapi32Ex.INSTANCE.RegDeleteTreeA( WinRegistryAdapter.getRegistryKey(key).getKey(), subkey )
        );
    }

    @Override
    public void deleteValue(IRegistryKey key, String valuename)
    {
        Validate.validateNotNull(key, "key");

        Advapi32Util.registryDeleteValue(WinRegistryAdapter.getRegistryKey(key).getKey(), valuename);
    }

    @Override
    public String getStringValue(IRegistryKey key, String subkey, String valuename, boolean canBeNullOrEmpty)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.

            lpSubKey [in, optional]
        */

        WinReg.HKEY hKey = WinRegistryAdapter.getRegistryKey(key).getKey();
        String strValue = null;
        if ( (canBeNullOrEmpty == false) || ( Advapi32Util.registryValueExists(hKey, subkey, valuename)) )
        {
            strValue = Advapi32Util.registryGetStringValue( WinRegistryAdapter.getRegistryKey(key).getKey(), subkey, valuename );
            if ( (canBeNullOrEmpty == false ) && ((strValue == null) || (strValue.length() <= 0 )) )
            {
                IllegalStateException ex = new IllegalStateException(String.format("Mandatory value '%s' is null or empty.", valuename));
                LogFactory.getLog(WinRegistryAdapter.class).error("This value should not be null.", ex);
                throw ex;
            }
        }

        return strValue;
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
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the
            type and data for the key's unnamed or default value.

            lpSubKey [in, optional]
        */

        Collection<String> result = null;
        
        WinReg.HKEY hKey = WinRegistryAdapter.getRegistryKey(key).getKey();
        
        if ( ( canBeNullOrEmpty == false ) || (Advapi32Util.registryValueExists(hKey, subkey, valuename) ) )
        {
            IRegistryKey theDirectKey = openKey(
            								key, 
            								subkey, 
            								0, 
            								WinNT.KEY_QUERY_VALUE );
            try
            {
                WinReg.HKEY hkeyDirect =
                	WinRegistryAdapter.getRegistryKey(theDirectKey).getKey();
                
                IntByReference attrTypeByRef = new IntByReference(0);
                IntByReference attrSizeByRef = new IntByReference(0);

                WinRegistryAdapter.CheckWin32Error(
                    IAdvapi32Ex.INSTANCE.RegQueryValueExA(
                            hkeyDirect, valuename, 0, attrTypeByRef, null, attrSizeByRef)
                );

                if ( attrTypeByRef.getValue() == WinNT.REG_MULTI_SZ)
                {
                    byte[] valArray = new byte[attrSizeByRef.getValue()];
                    
                    WinRegistryAdapter.CheckWin32Error(
                        IAdvapi32Ex.INSTANCE.RegQueryValueExA(
                                hkeyDirect, valuename, 0, attrTypeByRef, valArray, attrSizeByRef)
                    );

                    result = RegistryValue.getStrings( valArray );
                    if ( (canBeNullOrEmpty == false) && ((result == null) || (result.size() == 0) ) )
                    {
                        IllegalStateException ex = new IllegalStateException(String.format("Mandatory value '%s' is null or empty.", valuename));
                        LogFactory.getLog(WinRegistryAdapter.class).error("This value should not be null.", ex);
                        throw ex;
                    }
                }
                else
                {
                    RuntimeException ex = 
                    	new RuntimeException(
	                        String.format(
	                            "Unsupported value type '%d' for value '%s'.", 
                        		attrTypeByRef.getValue(), 
                        		valuename));
                    
                    LogFactory.getLog(WinRegistryAdapter.class).error(
		                    			"Type of this value is not supported.", 
		                    			ex);
                    throw ex;
                }
            }
            finally
            {
                theDirectKey.close();
            }
        }

        return result;
    }

    @Override
    public void setStringValue(IRegistryKey key, String valuename, String value)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.
        */

        Advapi32Util.registrySetStringValue( WinRegistryAdapter.getRegistryKey(key).getKey(), valuename, value );
    }
    
	@Override
	public 
	void 
	setMultiStringValue(IRegistryKey key, String valuename, Collection<String> value)
	{
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the
            type and data for the key's unnamed or default value.
        */

        if( value != null )
        {
        	byte[] byteArray = RegistryValue.getBytes(value);

            WinRegistryAdapter.CheckWin32Error(
                    IAdvapi32Ex.INSTANCE.RegSetValueExA(
                            WinRegistryAdapter.getRegistryKey(key).getKey(),
                            valuename,
                            0, WinNT.REG_MULTI_SZ,
                            byteArray,
                            byteArray.length)
            );
        }
	}

    @Override
    public byte[] getBinaryValue(IRegistryKey key, String subkey, String valuename, boolean canBeNullOrEmpty)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.

            lpSubKey [in, optional]
        */

        byte[] binaryValue = null;
        WinReg.HKEY hKey = WinRegistryAdapter.getRegistryKey(key).getKey();
        if ( (canBeNullOrEmpty == false) || ( Advapi32Util.registryValueExists(hKey, subkey, valuename)) )
        {
            IRegistryKey theDirectKey = this.openKey( key, subkey, 0, WinNT.KEY_QUERY_VALUE );
            try
            {
                WinReg.HKEY hkeyDirect = WinRegistryAdapter.getRegistryKey(theDirectKey).getKey();
                IntByReference attrTypeByRef = new IntByReference(0);
                IntByReference attrSizeByRef = new IntByReference(0);

                WinRegistryAdapter.CheckWin32Error(
                        Advapi32.INSTANCE.RegQueryValueEx(hkeyDirect, valuename, 0, attrTypeByRef, (IntByReference)null, attrSizeByRef)
                );

                if ( attrTypeByRef.getValue() == WinNT.REG_BINARY)
                {
                    binaryValue = new byte[attrSizeByRef.getValue()];
                    WinRegistryAdapter.CheckWin32Error(
                            Advapi32.INSTANCE.RegQueryValueEx( hkeyDirect, valuename, 0, attrTypeByRef, binaryValue, attrSizeByRef)
                    );
                }
                else
                {
                    RuntimeException ex = new RuntimeException(
                            String.format("Unsupported value type '%d' for value '%s'.", attrTypeByRef.getValue(), valuename)
                    );
                    LogFactory.getLog(WinRegistryAdapter.class).error("Type of this value is not supported.", ex);
                    throw ex;
                }
            }
            finally
            {
                theDirectKey.close();
            }
        }

        return binaryValue;
    }

    @Override
    public void setBinaryValue(IRegistryKey key, String valuename, byte[] value)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.
        */

        if( value != null )
        {
            WinRegistryAdapter.CheckWin32Error(
                    Advapi32.INSTANCE.RegSetValueEx(
                            WinRegistryAdapter.getRegistryKey(key).getKey(),
                            valuename,
                            0, WinNT.REG_BINARY,
                            value,
                            value.length)
            );
        }
    }

    @Override
    public Integer getIntValue(IRegistryKey key, String subkey, String valuename, boolean canBeNullOrEmpty)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.

            lpSubKey [in, optional]
        */

        WinReg.HKEY hKey = WinRegistryAdapter.getRegistryKey(key).getKey();
        Integer integer = null;
        if ( (canBeNullOrEmpty == false) || ( Advapi32Util.registryValueExists(hKey, subkey, valuename)) )
        {
            integer = new Integer( Advapi32Util.registryGetIntValue(WinRegistryAdapter.getRegistryKey(key).getKey(), subkey, valuename ) );
        }

        return integer;
    }

    @Override
    public void setIntValue(IRegistryKey key, String valuename, int value)
    {
        Validate.validateNotNull(key, "key");
        /*
        MSDN:
            If lpValueName is NULL or an empty string, "", the function sets the type and data
            for the key's unnamed or default value.
        */

        Advapi32Util.registrySetIntValue( WinRegistryAdapter.getRegistryKey(key).getKey(), valuename, value );
    }

    @Override
    public String[] getKeys(IRegistryKey key)
    {
        Validate.validateNotNull(key, "key");

        return Advapi32Util.registryGetKeys( WinRegistryAdapter.getRegistryKey(key).getKey() );
    }

    @Override
    public boolean doesKeyExist(IRegistryKey key, String subkey)
    {
        Validate.validateNotNull(key, "key");
        Validate.validateNotEmpty(subkey, "subkey");

        return Advapi32Util.registryKeyExists(WinRegistryAdapter.getRegistryKey(key).getKey(), subkey);
    }

    @Override
    public List<RegValueType> getRegEnumValues(IRegistryKey key) {

        List<RegValueType> keys = new ArrayList<RegValueType>();
        TreeMap<String, Object> map = Advapi32Util.registryGetValues(WinRegistryAdapter.getRegistryKey(key).getKey());
        try {
            for (Entry<String, Object> entry : map.entrySet()) {
                Object obj = entry.getValue();
                if (obj instanceof String) {
                    keys.add(new RegValueType(entry.getKey(), RegistryValueType.REG_SZ.getCode()));
                } else if (obj instanceof Integer) {
                    keys.add(new RegValueType(entry.getKey(), RegistryValueType.REG_DWORD.getCode()));
                } else if (obj.getClass().isArray()) {
                    keys.add(new RegValueType(entry.getKey(), RegistryValueType.REG_MULTI_SZ.getCode()));
                } else {
                    throw new RegistryUnSupportedKeyTypeException("Unsupport Type " + obj.getClass().toString());
                }
            }
        } catch (RegistryUnSupportedKeyTypeException ex){
            System.out.println(ex.getMessage());
        }
        return keys;
    }
    // private methods

    private WinRegistryAdapter()
    { }

    private static void CheckWin32Error(int win32Error)
    {
        WinRegistryAdapter.CheckWin32Error(win32Error, false);
    }

    private static void CheckWin32Error(int win32Error, boolean logOnly)
    {
        if( win32Error != W32Errors.ERROR_SUCCESS )
        {
            Win32Exception ex = new Win32Exception(win32Error);
            LogFactory.getLog(WinRegistryAdapter.class).error(
                    String.format("Registry operation failed with native '%d'.", win32Error), ex );
            if(logOnly == false)
            {
                throw ex;
            }
        }
    }

    private interface IAdvapi32Ex extends Advapi32
    {
        IAdvapi32Ex INSTANCE = (IAdvapi32Ex) Native.loadLibrary(
                "advapi32.dll",
                IAdvapi32Ex.class);
        int RegDeleteTreeA(com.sun.jna.platform.win32.WinReg.HKEY hkey, java.lang.String s);

        int RegSetValueExA(
            com.sun.jna.platform.win32.WinReg.HKEY hkey, java.lang.String s, int i, int i1, byte[] bytes, int i2);

        int RegQueryValueExA(
            com.sun.jna.platform.win32.WinReg.HKEY hkey, java.lang.String s, int i,
            com.sun.jna.ptr.IntByReference intByReference, byte[] bytes, com.sun.jna.ptr.IntByReference intByReference1);
    }

    private static WinRegistryKey getRegistryKey(IRegistryKey key)
    {
        WinRegistryKey theKey = null;
        if ( ( key instanceof WinRegistryKey ) == false )
        {
            throw new IllegalArgumentException(
                    "Parameter key must be an instance of WinRegistryKey.");

        }

        theKey = (WinRegistryKey)key;

        return theKey;
    }

    private class WinRegistryKey implements IRegistryKey
    {
        private boolean ownkey = false;
        private WinReg.HKEY key = null;

        public WinRegistryKey( WinReg.HKEY wrappedKey )
        {
            if(wrappedKey == null)
            {
                throw new IllegalArgumentException( "wrappedKey parameter cannot be null." );
            }
            
            this.ownkey =
                       (wrappedKey != WinReg.HKEY_LOCAL_MACHINE)
                    && (wrappedKey != WinReg.HKEY_CLASSES_ROOT)
                    && (wrappedKey != WinReg.HKEY_CURRENT_CONFIG)
                    && (wrappedKey != WinReg.HKEY_CURRENT_USER)
                    && (wrappedKey != WinReg.HKEY_DYN_DATA)
                    && (wrappedKey != WinReg.HKEY_PERFORMANCE_DATA)
                    && (wrappedKey != WinReg.HKEY_PERFORMANCE_DATA)
                    && (wrappedKey != WinReg.HKEY_PERFORMANCE_NLSTEXT)
                    && (wrappedKey != WinReg.HKEY_PERFORMANCE_TEXT)
                    && (wrappedKey != WinReg.HKEY_USERS);

            this.key = wrappedKey;
        }

        @Override
        public void close()
        {
            if ( (this.ownkey == true ) && (this.key != null) && (this.key.getPointer() != Pointer.NULL))
            {
                WinRegistryAdapter.CheckWin32Error(
                        Advapi32.INSTANCE.RegCloseKey(this.key), true
                );
            }

            this.ownkey = false;
            this.key = null;
        }

        public WinReg.HKEY getKey()
        {
            return this.key;
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
}
