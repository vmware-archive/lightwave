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

package com.vmware.identity.interop.accountmanager;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.NativeMemory;
import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.Validate;
import com.vmware.identity.interop.domainmanager.DomainAdapterFactory;
import com.vmware.identity.interop.domainmanager.DomainControllerInfo;
import com.vmware.identity.interop.domainmanager.IDomainAdapter;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

/**
 * Created by IntelliJ IDEA.
 * Date: 1/6/12
 * Time: 12:05 PM
 * To change this template use File | Settings | File Templates.
 */
public class LinuxAccountAdapter extends NativeAdapter implements IAccountAdapter
{
    private static LinuxAccountAdapter ourInstance = new LinuxAccountAdapter();
    private static final Log logger = LogFactory.getLog(LinuxAccountAdapter.class);

    class LsaObjectType
    {
        public static final int LSA_OBJECT_TYPE_UNDEFINED = 0;
        public static final int LSA_OBJECT_TYPE_GROUP = 1;
        public static final int LSA_OBJECT_TYPE_USER = 2;
        public static final int LSA_OBJECT_TYPE_DOMAIN = 3;
        public static final int LSA_OBJECT_TYPE_COMPUTER = 4;
    };

    class LsaQueryType
    {
        public static final short LSA_QUERY_TYPE_UNDEFINED = 0;
        public static final short LSA_QUERY_TYPE_BY_DN = 1;
        public static final short LSA_QUERY_TYPE_BY_SID = 2;
        public static final short LSA_QUERY_TYPE_BY_NT4 = 3;
        public static final short LSA_QUERY_TYPE_BY_UPN = 4;
        public static final short LSA_QUERY_TYPE_BY_ALIAS = 5;
        public static final short LSA_QUERY_TYPE_BY_UNIX_ID = 6;
        public static final short LSA_QUERY_TYPE_BY_NAME = 7;
    };

    private interface LsaClientLibrary extends Library
    {
        LsaClientLibrary INSTANCE =
             (LsaClientLibrary) Native.loadLibrary(
                   "lsaclient",
                   LsaClientLibrary.class);

        //LW_DWORD
        //LsaOpenServer(
        //        LW_PHANDLE phConnection
        //);
        int LsaOpenServer(PointerByReference phConnection);

        //LW_DWORD
        //LsaAuthenticateUser(
        //        LW_HANDLE hLsaConnection,
        //        LW_PCSTR pszLoginName,
        //        LW_PCSTR pszPassword,
        //        LW_PSTR* ppszMessage
        //);
        int LsaAuthenticateUser(
            Pointer hLsaConnection,
            String  pszLoginName,
            String  pszPassword,
            Pointer  ppszMessage
            );

        //LW_DWORD
        //LsaGetNamesBySidList(
        //    LW_IN LW_HANDLE hLsaConnection,
        //    LW_IN size_t sCount,
        //    LW_IN LW_PSTR* ppszSidList,
        //    LW_OUT PLSA_SID_INFO* ppSIDInfoList,
        //    LW_OUT LW_OPTIONAL LW_CHAR *pchDomainSeparator
        //    );
        int
        LsaGetNamesBySidList(
            Pointer hLsaConnection,
            int sCount,
            PointerByReference ppszSidList,
            PointerByReference ppSIDInfoList,
            PointerByReference pchDomainSeparator
            );

       // LW_DWORD
       // LsaFindUserByName(
       //     LW_HANDLE hLsaConnection,
       //     LW_PCSTR pszName,
       //     LW_DWORD dwUserInfoLevel,
       //     LW_PVOID* ppUserInfo
       //     );
        int
        LsaFindUserByName(
            Pointer hLsaConnection,
            String pszName,
            int dwUserInfoLevel,
            PointerByReference ppUserInfo
            );

        //LW_DWORD
        //LsaFindGroupByName(
        //    LW_HANDLE hLsaConnection,
        //    LW_PCSTR pszGroupName,
        //    LSA_FIND_FLAGS FindFlags,
        //    LW_DWORD dwGroupInfoLevel,
        //    LW_PVOID* ppGroupInfo
        //    );
        int
        LsaFindGroupByName(
            Pointer hLsaConnection,
            String pszGroupName,
            int FindFlags,
            int dwGroupInfoLevel,
            PointerByReference ppGroupInfo
            );

        //LW_VOID
        //LsaFreeSIDInfoList(
        //    PLSA_SID_INFO ppSIDInfoList,
        //    size_t stNumSID
        //    );
        int
        LsaFreeSIDInfoList(
            Pointer pSidInfoList,
            int sCount
            );

        // LW_VOID
        // LsaFreeUserInfo(
        //    LW_DWORD dwLevel,
        //    LW_PVOID pUserInfo
        //    );
        void
        LsaFreeUserInfo(
            int dwLevel,
            Pointer pUserInfo
            );

        //LW_VOID
        //LsaFreeGroupInfo(
        //    LW_DWORD dwLevel,
        //    LW_PVOID pGroupInfo
        //    );
        void
        LsaFreeGroupInfo(
            int dwLevel,
            Pointer pGroupInfo
            );

        //LW_DWORD
        //LsaCloseServer(
        //        LW_HANDLE hConnection
        //);
        int LsaCloseServer(
            Pointer hConnection
            );
    }

    public static LinuxAccountAdapter getInstance() {
        return ourInstance;
    }

    public LinuxAccountAdapter()
    {
        if(Platform.isLinux() == false)
        {
            throw new RuntimeException(
                            "This class is only supported on Linux platform.");
        }
    }

    // IAccountAdapter
    @Override
    public boolean authenticate(String userPrincipalName, String password)
    {
        PlatformUtils.validateNotNull(userPrincipalName, "userPrincipalName");

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        int dwError = 0;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            dwError = LsaClientLibrary.INSTANCE.LsaAuthenticateUser(pLsaConnection.getValue(),
                                                                    userPrincipalName,
                                                                    password,
                                                                    Pointer.NULL);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }
        }

        return true;
    }

    @Override
    public String lookupByObjectSid(String objectSid)
    {
        PlatformUtils.validateNotNull(objectSid, "objectSid");

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        int dwError = 0;
        Pointer[] accountsPtr = null;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            PointerByReference ppSids = new PointerByReference(Pointer.NULL);
            try(NativeMemory sid = new NativeMemory(objectSid.length() + 1) )
            {
                sid.setString(0,  objectSid);
                ppSids.setValue(sid);

                PointerByReference ppSidsInfoList = new PointerByReference(Pointer.NULL);
                dwError = LsaClientLibrary.INSTANCE.LsaGetNamesBySidList(pLsaConnection.getValue(),
                                                                         1,
                                                                         ppSids,
                                                                         ppSidsInfoList,
                                                                         null);
                if (dwError != 0)
                {
                    throw new AccountManagerNativeException(dwError);
                }

                accountsPtr = ppSidsInfoList.getPointer().getPointerArray(0, 1);
                if (accountsPtr != null && accountsPtr.length > 0 && accountsPtr[0] != null)
                {
                    LsaSidInfoNative acctInfo = new LsaSidInfoNative(accountsPtr[0]);
                    if (acctInfo != null)
                    {
                        return String.format("%s\\%s", acctInfo.pszDomainName, acctInfo.pszSamAccountName);
                    }
                }
            }
            // Do not find accountInfo
            throw new AccountManagerException(String.format("Lookup objectBySid %s failed", objectSid));
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }

            if (accountsPtr != null && accountsPtr.length > 0 && accountsPtr[0] != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaFreeSIDInfoList(accountsPtr[0], 1);
            }
        }
    }

    @Override
    public AccountInfo lookupByName(String name)
    {
        AccountInfo acctName = null;

        try
        {
            acctName = lookupByUserName(name);
        }
        catch(AccountManagerNativeException ex)
        {
            logger.debug(String.format("Failed to find account as user for %s ", name) + ex.getMessage());

            acctName = lookupByGroupName(name);
        }

        return acctName;
    }

    public MachineAccountInfo getMachineAccountInfo()
    {
        // Getting joined Domain FQDN to construct machine account key path
        IDomainAdapter domainAdapter =
                DomainAdapterFactory.getInstance().getDomainAdapter();
        DomainControllerInfo joinedDcInfo = domainAdapter.getDomainJoinInfo();
        if (joinedDcInfo == null || joinedDcInfo.domainName == null || joinedDcInfo.domainName.length() == 0)
        {
            throw new AccountManagerException("Get machine account information - " +
                    "failed getting domain join info");
        }

        // Read from registry (machine samAccountName, dnsDomainName and machine password)
        String DOMAINJOIN_STATUS_ROOT_KEY = "Services\\lsass\\Parameters\\Providers\\ActiveDirectory\\DomainJoin";
        String MACHINE_ACCOUNT_INFO_KEY = DOMAINJOIN_STATUS_ROOT_KEY + "\\" + joinedDcInfo.domainName.toUpperCase()
                + "\\Pstore";

        IRegistryAdapter regAdapter =
                RegistryAdapterFactory.getInstance().getRegistryAdapter();
        IRegistryKey rootKey = null;

        try
        {
            rootKey = regAdapter.openRootKey(
                    (int) RegKeyAccess.KEY_READ);

            String samAccountName = regAdapter.getStringValue(
                      rootKey,
                      MACHINE_ACCOUNT_INFO_KEY,
                      "SamAccountName",
                      false);
            if (samAccountName == null || samAccountName.length() == 0)
            {
                throw new AccountManagerException("Get machine account information - " +
                        "failed getting machine samAccountName");
            }

            String MACHINE_PASSWORD_INFO_KEY = MACHINE_ACCOUNT_INFO_KEY + "\\PasswordInfo";
            String machinePassword = regAdapter.getStringValue(
                      rootKey,
                      MACHINE_PASSWORD_INFO_KEY,
                      "password",
                      false);
            if (machinePassword == null || machinePassword.length() == 0)
            {
                throw new AccountManagerException("Get machine account information - " +
                        "failed getting machine password information");
            }

            return new MachineAccountInfo(String.format("%s@%s", samAccountName, joinedDcInfo.domainName),
                                          machinePassword);
        }
        finally
        {
            if (rootKey != null)
            {
                rootKey.close();
            }
        }
    }

    public UserInfo findUserByName(String samAccountName, String domainName)
    {
        Validate.validateNotEmpty(samAccountName, "samAccountName");
        Validate.validateNotEmpty(domainName, "domainName");

        String name = String.format("%s@%s", samAccountName, domainName);

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        int dwError = 0;
        PointerByReference ppUserInfo = new PointerByReference(Pointer.NULL);
        Pointer accountPtr = null;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            dwError = LsaClientLibrary.INSTANCE.LsaFindUserByName(pLsaConnection.getValue(),
                                                                  name,
                                                                  2,
                                                                  ppUserInfo);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            accountPtr = ppUserInfo.getValue();
            if (accountPtr != null)
            {
                LsaUserInfoLevel2Native acctInfo = new LsaUserInfoLevel2Native(accountPtr);
                if (acctInfo != null)
                {
                    return new UserInfo(acctInfo.lsaUserInfo1.pszName,
                                        acctInfo.lsaUserInfo1.pszSid,
                                        acctInfo.bAccountLocked != 0,
                                        acctInfo.bAccountDisabled != 0);
                }
            }
            // Do not find accountInfo
            throw new AccountManagerException(String.format("FindUserByName for %s failed", name));
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }

            if (accountPtr != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaFreeUserInfo(2, accountPtr);
                ppUserInfo.setValue(Pointer.NULL);
            }
        }
    }

    private AccountInfo lookupByUserName(String name)
    {
        PlatformUtils.validateNotNull(name, "name");

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        int dwError = 0;
        Pointer accountPtr = null;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            PointerByReference ppUserInfo = new PointerByReference(Pointer.NULL);

            dwError = LsaClientLibrary.INSTANCE.LsaFindUserByName(pLsaConnection.getValue(),
                                                                  name,
                                                                  0,
                                                                  ppUserInfo);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            accountPtr = ppUserInfo.getValue();
            if (accountPtr != null)
            {
                LsaUserInfoLevel0Native acctInfo = new LsaUserInfoLevel0Native(accountPtr);
                if (acctInfo != null)
                {
                    return new AccountInfo(ACCOUNT_TYPE.USER, acctInfo.pszName, acctInfo.pszSid);
                }
            }
            // Do not find accountInfo
            throw new AccountManagerException(String.format("Lookup objectByUserName %s failed", name));
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }

            if (accountPtr != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaFreeUserInfo(0, accountPtr);
            }
        }
    }

    private AccountInfo lookupByGroupName(String name)
    {
        PlatformUtils.validateNotNull(name, "name");

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        int dwError = 0;
        Pointer accountPtr = null;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            PointerByReference ppGroupInfo = new PointerByReference(Pointer.NULL);

            dwError = LsaClientLibrary.INSTANCE.LsaFindGroupByName(pLsaConnection.getValue(),
                                                                  name,
                                                                  0,
                                                                  0,
                                                                  ppGroupInfo);
            if (dwError != 0)
            {
                throw new AccountManagerNativeException(dwError);
            }

            accountPtr = ppGroupInfo.getValue();
            if (accountPtr != null)
            {
                LsaGroupInfoLevel0Native acctInfo = new LsaGroupInfoLevel0Native(accountPtr);
                if (acctInfo != null)
                {
                    return new AccountInfo(ACCOUNT_TYPE.GROUP, acctInfo.pszName, acctInfo.pszSid);
                }
            }
            // Do not find accountInfo
            throw new AccountManagerException(String.format("Lookup objectByGroupName %s failed", name));
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }

            if (accountPtr != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaFreeGroupInfo(0, accountPtr);
            }
        }
    }
}

