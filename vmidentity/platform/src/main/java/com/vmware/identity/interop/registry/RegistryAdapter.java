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

/**
 * VMware Identity Service
 *
 * Registry Java to Native Adapter
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.interop.registry;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.StringTokenizer;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.WString;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.NativeCallException;
import com.vmware.identity.interop.NativeMemory;
import com.vmware.identity.interop.NoMoreDataException;
import com.vmware.identity.interop.registry.RegValueType;

public class RegistryAdapter extends NativeAdapter
{
    private static final boolean UNICODE_ENABLED = false;
    private static final int     MAX_KEY_NAME_LENGTH = 255;

    public interface RegClientLibrary extends Library
    {
        RegClientLibrary INSTANCE =
                (RegClientLibrary) Native.loadLibrary(
                                        "regclient",
                                        RegClientLibrary.class);

        int
        LwRegOpenServer(PointerByReference phConnection);

        int
        LwRegCreateKeyExA(
            Pointer            hConnection,
            Pointer            hKey,
            String             pszSubKey,
            int                dwReserved,
            String             pszClass,
            int                dwOptions,
            int                dwDesiredAccess,
            Pointer            pSecurityDescriptor,
            PointerByReference phkResult,
            IntByReference     pdwDisposition
        );

        int
        LwRegCreateKeyExW(
            Pointer            hConnection,
            Pointer            hKey,
            WString            pwszSubKey,
            int                dwReserved,
            WString            pwszClass,
            int                dwOptions,
            int                dwDesiredAccess,
            Pointer            pSecurityDescriptor,
            PointerByReference phkResult,
            IntByReference     pdwDisposition
            );

        int
        LwRegOpenKeyExA(
            Pointer            hConnection,
            Pointer            hKey,
            String             pszSubKey,
            int                dwOptions,
            int                dwDesiredAccess,
            PointerByReference phkResult
            );

        int
        LwRegOpenKeyExW(
            Pointer            hConnection,
            Pointer            hKey,
            WString            pwszSubKey,
            int                dwOptions,
            int                dwDesiredAccess,
            PointerByReference phkResult
            );

        int
        LwRegEnumKeyExA(
            Pointer        hRegConnection,
            Pointer        hKey,
            int            dwIndex,
            Pointer        pszName,
            IntByReference pcName,
            IntByReference pReserved,
            Pointer        pszClass,
            IntByReference pcClass,
            Pointer        pftLastWriteTime
        );

        int
        LwRegEnumKeyExW(
            Pointer        hRegConnection,
            Pointer        hKey,
            int            dwIndex,
            Pointer        pwszName,
            IntByReference pcName,
            IntByReference pReserved,
            Pointer        pwszClass,
            IntByReference pcClass,
            Pointer        pftLastWriteTime
            );

        int
        LwRegEnumValueA(
            Pointer         hRegConnection,
            Pointer         hKey,
            int             dwIndex,
            Pointer         pszValueName,
            IntByReference  pcchValueName,
            IntByReference  pReserved,
            IntByReference  pdwType,
            Pointer         pData,
            IntByReference  pcbData
            );

        int
        LwRegEnumValueW(
            Pointer         hRegConnection,
            Pointer         hKey,
            int             dwIndex,
            Pointer         pszValueName,
            IntByReference  pcchValueName,
            IntByReference  pReserved,
            IntByReference  pdwType,
            Pointer         pData,
            IntByReference  pcbData
            );

        int
        LwRegGetValueA(
            Pointer        hRegConnection,
            Pointer        hKey,
            String         pszSubKey,
            String         pszValueName,
            int            dwFlags,
            IntByReference pdwType,
            Pointer        pvData,
            IntByReference pcbData
        );

        int
        LwRegGetValueW(
            Pointer        hRegConnection,
            Pointer        hKey,
            WString        pwszSubKey,
            WString        pwszValueName,
            int            dwFlags,
            IntByReference pdwType,
            Pointer        pvData,
            IntByReference pcbData
            );

        int
        LwRegSetValueExA(
            Pointer hRegConnection,
            Pointer hKey,
            String  pszValueName,
            int     dwReserved,
            int     dwType,
            Pointer pData,
            int     cbData
        );

        int
        LwRegSetValueExW(
            Pointer hRegConnection,
            Pointer hKey,
            WString pwszValueName,
            int     dwReserved,
            int     dwType,
            Pointer pData,
            int     cbData
            );

        int
        LwRegDeleteKeyA(
            Pointer hRegConnection,
            Pointer hKey,
            String  pszSubKey
            );

        int
        LwRegDeleteKeyW(
            Pointer hRegConnection,
            Pointer hKey,
            WString pwszSubKey
            );

        int
        LwRegDeleteTreeA(
                Pointer hRegConnection,
                Pointer hKey,
                String  pszSubKey
        );

        int
        LwRegDeleteTreeW(
                Pointer hRegConnection,
                Pointer hKey,
                WString pwszSubKey
        );

        int
        LwRegDeleteValueA(
            Pointer hRegConnection,
            Pointer hKey,
            String  pszValueName
            );

        int
        LwRegDeleteValueW(
            Pointer hRegConnection,
            Pointer hKey,
            WString pwszValueName
            );

        int
        LwRegCloseKey(
            Pointer hRegConnection,
            Pointer pKey
            );

        void
        LwRegCloseServer(Pointer hConnection);
    }

    public static
    RegistryConnection
    connect()
    {
        PointerByReference hConnection = new PointerByReference();

        int errCode =
               RegistryAdapter.RegClientLibrary.INSTANCE.LwRegOpenServer(
                                                             hConnection);
        checkNativeErrorCode(errCode);

        return new RegistryConnection(hConnection.getValue());
    }

    public static
    RegistryKey
    createKey(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             regClass,
        int                access,
        boolean allowExisting
        )
    {
        int reserved = 0;
        int options  = 0;
        PointerByReference phkResult = new PointerByReference();
        IntByReference pdwDisposition = new IntByReference();
        RegistryKey prevKey = key;

        int errCode;

        if (connection == null)
        {
            throw new IllegalArgumentException(
                            "A valid registry connection is required");
        }
        if (subkey == null)
        {
            throw new IllegalArgumentException(
                            "A valid registry subkey is required");
        }

        StringTokenizer tokenizer = new StringTokenizer(subkey, "\\");

        while (tokenizer.hasMoreTokens())
        {
            String token = tokenizer.nextToken();

            if (UNICODE_ENABLED)
            {
                WString wszSubkey   = new WString(token);
                WString wszRegClass = new WString(regClass);

                errCode =
                    RegistryAdapter.RegClientLibrary.INSTANCE.LwRegCreateKeyExW(
                                        connection.getConnection(),
                                        prevKey != null ?
                                                prevKey.getKey() : Pointer.NULL,
                                        wszSubkey,
                                        reserved,
                                        wszRegClass,
                                        options,
                                        access,
                                        Pointer.NULL,
                                        phkResult,
                                        pdwDisposition);
            }
            else
            {
                errCode =
                    RegistryAdapter.RegClientLibrary.INSTANCE.LwRegCreateKeyExA(
                                        connection.getConnection(),
                                        prevKey != null ?
                                                prevKey.getKey() : Pointer.NULL,
                                        token,
                                        reserved,
                                        regClass,
                                        options,
                                        access,
                                        Pointer.NULL,
                                        phkResult,
                                        pdwDisposition);
            }

            checkNativeErrorCode(errCode);

            if (prevKey != null && prevKey != key)
            {
                prevKey.close();
            }

            prevKey = new RegistryKey(
                                connection,
                                phkResult.getValue(),
                                RegistryKeyDisposition.parse(
                                                    pdwDisposition.getValue()));
        }

        if ( (prevKey != null) &&
             (allowExisting == false) &&
             (prevKey.getDisposition() !=
                     RegistryKeyDisposition.REGISTRY_KEY_DISPOSITION_NEW ) )
        {
            prevKey.close();

            throw new IllegalArgumentException(
                                String.format(
                                        "Registry key '%s' already exists.",
                                        subkey) );
        }

        return prevKey;
    }

    public static
    RegistryKey
    openKey(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        int                options,
        int                access
        )
    {
        PointerByReference phkResult = new PointerByReference();

        if (connection == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry connection is required");
        }
        if (subkey == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry subkey is required");
        }

        int errCode;

        if (UNICODE_ENABLED)
        {
            WString wszSubkey   = new WString(subkey);

            errCode =
                    RegistryAdapter.RegClientLibrary.INSTANCE.LwRegOpenKeyExW(
                            connection.getConnection(),
                            key != null ? key.getKey() : Pointer.NULL,
                            wszSubkey,
                            options,
                            access,
                            phkResult);
        }
        else
        {
            errCode =
                    RegistryAdapter.RegClientLibrary.INSTANCE.LwRegOpenKeyExA(
                            connection.getConnection(),
                            key != null ? key.getKey() : Pointer.NULL,
                            subkey,
                            options,
                            access,
                            phkResult);
        }

        checkNativeErrorCode(errCode);

        return new RegistryKey(
                        connection,
                        phkResult.getValue(),
                        RegistryKeyDisposition.REGISTRY_KEY_DISPOSITION_OPENED);
    }

    public static
    void
    deleteKey(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey
        )
    {
        int errCode;

        if (connection == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry connection is required");
        }
        if (subkey == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry subkey is required");
        }

        if (UNICODE_ENABLED)
        {
            WString wszSubkey   = new WString(subkey);

            errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteKeyW(
                                    connection.getConnection(),
                                    key != null ? key.getKey() : Pointer.NULL,
                                    wszSubkey);
        }
        else
        {
            errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteKeyA(
                                    connection.getConnection(),
                                    key != null ? key.getKey() : Pointer.NULL,
                                    subkey);
        }

        checkNativeErrorCode(errCode);
    }

    public static
    void
    deleteTree(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey
        )
    {
        int errCode;

        if (connection == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry connection is required");
        }
        if (key == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry key is required");
        }

        if (UNICODE_ENABLED)
        {
            WString wszSubkey   = subkey != null ? new WString(subkey) : null;

            errCode =
                RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteTreeW(
                                    connection.getConnection(),
                                    key.getKey(),
                                    wszSubkey);
        }
        else
        {
            errCode =
                RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteTreeA(
                                    connection.getConnection(),
                                    key.getKey(),
                                    subkey);
        }

        checkNativeErrorCode(errCode);
    }

    public static
    void
    deleteValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename
        )
    {
        int errCode;

        if (connection == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry connection is required");
        }
        if (valuename == null)
        {
            throw new IllegalArgumentException(
                    "A valid registry value name is required");
        }

        if (UNICODE_ENABLED)
        {
            WString wszValueName   = new WString(valuename);

            errCode =
                RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteValueW(
                        connection.getConnection(),
                        key != null ? key.getKey() : Pointer.NULL,
                        wszValueName);
        }
        else
        {
            errCode =
                RegistryAdapter.RegClientLibrary.INSTANCE.LwRegDeleteValueA(
                        connection.getConnection(),
                        key != null ? key.getKey() : Pointer.NULL,
                        valuename);
        }

        checkNativeErrorCode(errCode);
    }

    public static
    void
    setValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename,
        RegistryValueType  valueType,
        byte[]             data
        )
    {
        int errCode;
        int reserved = 0;
        NativeMemory pData = null;

        try
        {

            if (connection == null)
            {
                throw new IllegalArgumentException(
                        "A valid registry connection is required");
            }

            if (data != null && data.length > 0)
            {
                pData = new NativeMemory(data.length);

                pData.write(0, data, 0, data.length);
            }

            if (UNICODE_ENABLED)
            {
                WString wszValueName   = null;

                if (valuename != null)
                {
                    wszValueName = new WString(valuename);
                }

                errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegSetValueExW(
                                connection.getConnection(),
                                key != null ? key.getKey() : Pointer.NULL,
                                wszValueName,
                                reserved,
                                valueType.getCode(),
                                pData,
                                data != null ? data.length : 0);
            }
            else
            {
                errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegSetValueExA(
                                connection.getConnection(),
                                key != null ? key.getKey() : Pointer.NULL,
                                valuename,
                                reserved,
                                valueType.getCode(),
                                pData,
                                data != null ? data.length : 0);
            }

            checkNativeErrorCode(errCode);
        }
        finally
        {
            if (pData != null)
            {
                pData.close();
                pData = null;
            }
        }
    }

    public static
    RegistryValue
    getValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             valuename,
        int                flags
        )
    {
        int errCode;
        NativeMemory pData = null;
        IntByReference pdwType = new IntByReference();
        IntByReference pcbData = new IntByReference();

        try
        {
            if (connection == null)
            {
                throw new IllegalArgumentException(
                        "A valid registry connection is required");
            }

            if (UNICODE_ENABLED)
            {
                WString wszSubkey      = null;
                WString wszValueName   = null;

                if (subkey != null)
                {
                    wszSubkey = new WString(subkey);
                }
                if (valuename != null)
                {
                    wszValueName = new WString(valuename);
                }

                errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegGetValueW(
                                        connection.getConnection(),
                                        key != null ? key.getKey() : Pointer.NULL,
                                        wszSubkey,
                                        wszValueName,
                                        flags,
                                        pdwType,
                                        pData,
                                        pcbData);

                checkNativeErrorCode(errCode);

                if (pcbData.getValue() > 0)
                {
                    pData = new NativeMemory(pcbData.getValue());

                    errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegGetValueW(
                            connection.getConnection(),
                            key != null ? key.getKey() : Pointer.NULL,
                            wszSubkey,
                            wszValueName,
                            flags,
                            pdwType,
                            pData,
                            pcbData);

                    checkNativeErrorCode(errCode);
                }
            }
            else
            {
                errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegGetValueA(
                                        connection.getConnection(),
                                        key != null ? key.getKey() : Pointer.NULL,
                                        subkey,
                                        valuename,
                                        flags,
                                        pdwType,
                                        pData,
                                        pcbData);

                checkNativeErrorCode(errCode);

                if (pcbData.getValue() > 0)
                {
                    pData = new NativeMemory(pcbData.getValue());

                    errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegGetValueA(
                                        connection.getConnection(),
                                        key != null ? key.getKey() : Pointer.NULL,
                                        subkey,
                                        valuename,
                                        flags,
                                        pdwType,
                                        pData,
                                        pcbData);

                    checkNativeErrorCode(errCode);
                }
            }

            return new RegistryValue(
                            RegistryValueType.parse(pdwType.getValue()),
                            pData != Pointer.NULL ?
                            pData.getByteArray(0, pcbData.getValue()) : null,
                            pcbData.getValue());
        }
        finally
        {
            if (pData != null)
            {
                pData.close();
                pData = null;
            }
        }
    }

    public static
    String
    getStringValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             valuename,
        boolean            canBeNullOrEmpty
        )
    {
        String result = null;

        try
        {
            RegistryValue value =
                    RegistryAdapter.getValue(
                        connection,
                        key,
                        subkey,
                        valuename,
                        (int)RegistryDataTypeFlag.DATA_TYPE_FLAG_REG_SZ.getCode());

            int length = value.getLength();

            if (length <= 0)
            {
                if (!canBeNullOrEmpty)
                {
                    throw new IllegalStateException("Mandatory value is empty");
                }
            }
            else
            {
                result = new String(value.getValue());
                if (result.charAt(result.length()-1) == '\0') // trim trailing zero
                {
                    result = result.substring(0, result.length()-1);
                }
            }
        }
        catch(RegistryNoSuchKeyOrValueException ex)
        {
            if (!canBeNullOrEmpty)
            {
                throw new IllegalStateException("Mandatory value is empty");
            }
        }

        return result;
    }

    public static
    void
    setStringValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename,
        String             value
        )
    {
        RegistryValue regValue = RegistryValue.build(value);

        RegistryAdapter.setValue(
                connection,
                key,
                valuename,
                RegistryValueType.REG_SZ,
                regValue.getValue());
    }

    public static
    Collection<String>
    getMultiStringValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             valuename,
        boolean            canBeNullOrEmpty
        )
    {
        Collection<String> result = null;

        try
        {
            RegistryValue value =
                    RegistryAdapter.getValue(
                        connection,
                        key,
                        subkey,
                        valuename,
                        (int)RegistryDataTypeFlag.DATA_TYPE_FLAG_REG_MULTI_SZ.getCode());

            int length = value.getLength();

            if (length <= 0)
            {
                if (!canBeNullOrEmpty)
                {
                    throw new IllegalStateException("Mandatory value is empty");
                }
            }
            else
            {
                result = RegistryValue.getStrings(value.getValue());
            }
        }
        catch(RegistryNoSuchKeyOrValueException ex)
        {
            if (!canBeNullOrEmpty)
            {
                throw new IllegalStateException("Mandatory value is empty");
            }
        }

        return result;
    }

    public static
    void
    setMultiStringValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename,
        Collection<String> value
        )
    {
        RegistryValue regValue = RegistryValue.build(value);

        RegistryAdapter.setValue(
                            connection,
                            key,
                            valuename,
                            RegistryValueType.REG_MULTI_SZ,
                            regValue.getValue());
    }

    public static
    byte[]
    getBinaryValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             valuename,
        boolean            canBeNullOrEmpty
        )
    {
        try
        {
            RegistryValue value =
                RegistryAdapter.getValue(
                    connection,
                    key,
                    subkey,
                    valuename,
                    (int)RegistryDataTypeFlag.DATA_TYPE_FLAG_REG_BINARY.getCode());

            int length = value.getLength();

            if (length <= 0)
            {
                if (!canBeNullOrEmpty)
                {
                    throw new IllegalStateException("Mandatory value is empty");
                }
            }
            else
            {
                return value.getValue();
            }
        }
        catch(RegistryNoSuchKeyOrValueException ex)
        {
            if (!canBeNullOrEmpty)
            {
                throw new IllegalStateException("Mandatory value is empty");
            }
        }

        return null;
    }

    public static
    void
    setBinaryValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename,
        byte[]             value
        )
    {
        RegistryValue regValue = RegistryValue.build(value);

        RegistryAdapter.setValue(
                connection,
                key,
                valuename,
                RegistryValueType.REG_BINARY,
                regValue.getValue());
    }

    public static
    Integer
    getIntValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             subkey,
        String             valuename,
        boolean            canBeNullOrEmpty
        )
    {
        try
        {
            RegistryValue value =
                RegistryAdapter.getValue(
                    connection,
                    key,
                    subkey,
                    valuename,
                    (int)RegistryDataTypeFlag.DATA_TYPE_FLAG_REG_DWORD.getCode());

            int length = value.getLength();

            if (length <= 0)
            {
                if (!canBeNullOrEmpty)
                {
                    throw new IllegalStateException("Mandatory value is empty");
                }
            }
            else
            {
                ByteBuffer byteBuf = ByteBuffer.wrap(value.getValue());

                byteBuf.order(ByteOrder.LITTLE_ENDIAN);

                return byteBuf.asIntBuffer().get();
            }
        }
        catch(RegistryNoSuchKeyOrValueException ex)
        {
            if (!canBeNullOrEmpty)
            {
                throw new IllegalStateException("Mandatory value is empty");
            }
        }

        return null;
    }

    public static
    void
    setIntValue(
        RegistryConnection connection,
        RegistryKey        key,
        String             valuename,
        int                value
        )
    {
        RegistryValue regValue = RegistryValue.build(value);

        RegistryAdapter.setValue(
                            connection,
                            key,
                            valuename,
                            RegistryValueType.REG_DWORD,
                            regValue.getValue());
    }

    public static
    String[]
    getKeys(
        RegistryConnection connection,
        RegistryKey        key
        )
    {
        List<String> keys = new ArrayList<String>();

        try
        {
            String value = null;
            int iIndex = 0;

            do
            {
               value = getNextKey(connection, key, iIndex++);

               if (value != null && value.length() > 0)
               {
                   keys.add(value);
               }

            } while (value != null && !value.isEmpty());
        }
        catch(NoMoreDataException e)
        {
        }
        catch (RegistryNoMoreKeysOrValuesException e1)
        {
        }

        return keys.toArray(new String[keys.size()]);
    }

    public static
    List<RegValueType>
    getRegEnumValues (
        RegistryConnection connection,
        RegistryKey        key
        )
    {
        int iIndex = 0;
        RegValueType valueType;
        List<RegValueType> keys = new ArrayList<RegValueType>();
        try
        {
            do {
                valueType = getNextValue(connection, key, iIndex++);
                if (valueType != null) {
                    keys.add(valueType);
                }

            } while (valueType != null);
        }
        catch(NoMoreDataException e)
        {
        }
        catch (RegistryNoMoreKeysOrValuesException e1)
        {
        }
        return keys;
    }

    public static
    RegValueType
    getNextValue (
            RegistryConnection connection,
            RegistryKey        key,
            int                index
            )
    {
        int errCode;
        IntByReference pcName;
        NativeMemory pName = null;
        IntByReference pReserved = null;
        IntByReference pdwType = null;
        IntByReference  pcbData = null;
        Pointer pClass = Pointer.NULL;
        IntByReference pcClass = null;
        RegValueType keyValue = null;
        String [] temp = {};

        try {
            if (connection == null) {
                throw new IllegalArgumentException("A valid registry connection is required");
            }
            if (key == null) {
                throw new IllegalArgumentException("A valid registry key is required");
            }
            if (UNICODE_ENABLED)
            {
                pdwType = new IntByReference();
                pcName = new IntByReference((MAX_KEY_NAME_LENGTH + 1) * Native.WCHAR_SIZE);
                pName = new NativeMemory(pcName.getValue());

                errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegEnumValueW(
                                connection.getConnection(),
                                               key.getKey(),
                                               index,
                                               pName,
                                               pcName,
                                               pReserved,
                                               pdwType,
                                               Pointer.NULL,
                                               pcbData);
            }
            else
            {
                pdwType = new IntByReference();
                pcName = new IntByReference(MAX_KEY_NAME_LENGTH + 1);
                pName = new NativeMemory(pcName.getValue());

                errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegEnumValueA(
                                    connection.getConnection(),
                                               key.getKey(),
                                               index,
                                               pName,
                                               pcName,
                                               pReserved,
                                               pdwType,
                                               Pointer.NULL,
                                               pcbData);
            }
            checkNativeErrorCode(errCode);

	         if (pcName.getValue() > 0)
	         {
	             if (UNICODE_ENABLED)
	             {
	                 try
	                 {
	                     String keyName = new String(pName.getByteArray(0, pcName.getValue() * Native.WCHAR_SIZE), "UTF-16");
	                     keyValue = new RegValueType(keyName,pdwType.getValue());
	                 }
	                 catch(UnsupportedEncodingException e)
	                 {
	                     throw new RuntimeException(
	                                     "Unsupported encoding - UTF-16");
	                 }
	             }
	             else
	             {
                    String keyName = new String(pName.getByteArray(0, pcName.getValue()));
	                 keyValue = new RegValueType(keyName,pdwType.getValue());
	             }
	         }

            return keyValue;
        } finally {
            if (pName != null) {
                pName.close();
                pName = null;
            }
        }
    }

    public static
    String
    getNextKey(
        RegistryConnection connection,
        RegistryKey        key,
        int                index
        )
    {
        int errCode;
        IntByReference pcName;
        NativeMemory pName = null;
        IntByReference pReserved = null;
        Pointer pClass = Pointer.NULL;
        IntByReference pcClass = null;
        String keyName = "";

        try
        {
            if (connection == null)
            {
                throw new IllegalArgumentException(
                        "A valid registry connection is required");
            }
            if (key == null)
            {
                throw new IllegalArgumentException(
                        "A valid registry key is required");
            }

            if (UNICODE_ENABLED)
            {
                pcName = new IntByReference(
                                    (MAX_KEY_NAME_LENGTH + 1) * Native.WCHAR_SIZE);
                pName = new NativeMemory(pcName.getValue());
                System.out.println(pcName.getValue());
                errCode = RegistryAdapter.RegClientLibrary.INSTANCE.LwRegEnumKeyExW(
                                            connection.getConnection(),
                                            key.getKey(),
                                            index,
                                            pName,
                                            pcName,
                                            pReserved,
                                            pClass,
                                            pcClass,
                                            Pointer.NULL);
            }
            else
            {
                pcName = new IntByReference(MAX_KEY_NAME_LENGTH + 1);
                pName = new NativeMemory(pcName.getValue());

                errCode =
                        RegistryAdapter.RegClientLibrary.INSTANCE.LwRegEnumKeyExA(
                                connection.getConnection(),
                                key.getKey(),
                                index,
                                pName,
                                pcName,
                                pReserved,
                                pClass,
                                pcClass,
                                Pointer.NULL);
            }

            checkNativeErrorCode(errCode);

            if (pcName.getValue() > 0)
            {
                if (UNICODE_ENABLED)
                {
                    try
                    {
                        keyName = new String(pName.getByteArray(
                                                        0,
                                                        pcName.getValue() *
                                                                Native.WCHAR_SIZE),
                                             "UTF-16");
                    }
                    catch(UnsupportedEncodingException e)
                    {
                        throw new RuntimeException(
                                        "Unsupported encoding - UTF-16");
                    }
                }
                else
                {
                    keyName = new String(pName.getByteArray(
                                                    0,
                                                    pcName.getValue()));
                }
            }

            return keyName;
        }
        finally
        {
            if (pName != null)
            {
                pName.close();
                pName = null;
            }
        }
    }

    public static
    boolean
    keyExists(RegistryConnection connection, RegistryKey key, String subkey)
    {
        boolean bKeyExists = false;

        try
        {
            RegistryKey key2 = openKey(
                    connection,
                    key,
                    subkey,
                    0,
                    (int) RegKeyAccess.KEY_READ);

            assert(key2 != null);

            key2.close();

            bKeyExists = true;
        }
        catch(Exception ex)
        {
        }

        return bKeyExists;
    }

    private static
    void
    checkNativeErrorCode(int errorCode)
    {
        switch (errorCode)
        {
            case 0:

                break;

            case NoMoreDataException.ERROR_NO_MORE_ITEMS:

                throw new NoMoreDataException();

            case RegistryNoMoreKeysOrValuesException.ERROR_CODE:

                throw new RegistryNoMoreKeysOrValuesException();

            case RegistryNoSuchKeyOrValueException.ERROR_CODE:

                throw new RegistryNoSuchKeyOrValueException();

            default:

                throw new NativeCallException(errorCode);
        }
    }
}
