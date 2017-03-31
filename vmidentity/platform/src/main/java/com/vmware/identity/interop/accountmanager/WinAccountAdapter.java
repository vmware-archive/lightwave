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
import com.sun.jna.platform.win32.Advapi32;
import com.sun.jna.platform.win32.Advapi32Util;
import com.sun.jna.platform.win32.Advapi32Util.Account;
import com.sun.jna.platform.win32.Kernel32;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.platform.win32.WinBase;
import com.sun.jna.platform.win32.WinNT.HANDLEByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.win32.W32APIOptions;
import com.vmware.identity.interop.PlatformUtils;
import com.vmware.identity.interop.Validate;

enum WinNetSetupJoinStatus {
    NET_SETUP_UNKNOWN_STATUS(0),
    NET_SETUP_UNJOINED(1),
    NET_SETUP_WORKGROUP_NAME(2),
    NET_SETUP_DOMAIN_NAME(3);

    private int _status;

    private WinNetSetupJoinStatus(int status)
    {
        _status = status;
    }

    public int getStatus()
    {
        return _status;
    }
}

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 3/13/13
 * Time: 1:47 PM
 * To change this template use File | Settings | File Templates.
 */
public class WinAccountAdapter implements IAccountAdapter {

    private static WinAccountAdapter ourInstance = new WinAccountAdapter();
    private static final Log logger = LogFactory.getLog(WinAccountAdapter.class);

    public static final int ERROR_PASSWORD_MUST_CHANGE = 1907;
    public static final int ERROR_LOGON_FAILURE = 1326;
    public static final int ERROR_ACCOUNT_RESTRICTION = 1327;
    public static final int ERROR_ACCOUNT_DISABLED = 1331;
    public static final int ERROR_INVALID_LOGON_HOURS = 1328;
    public static final int ERROR_NO_LOGON_SERVERS = 1311;
    public static final int ERROR_INVALID_WORKSTATION = 1329;
    //It gives this error if the account is locked, REGARDLESS OF WHETHER VALID CREDENTIALS WERE PROVIDED!!!
    public static final int ERROR_ACCOUNT_LOCKED_OUT = 1909;
    public static final int ERROR_ACCOUNT_EXPIRED = 1793;
    public static final int ERROR_PASSWORD_EXPIRED = 1330;

    public static final int SID_TYPE_USER = 1;
    public static final int SID_TYPE_GROUP = 2;
    public static final int SID_TYPE_ALIAS = 4; // SidTypeAlias means it is a local group Sid
    public static final int SID_WELLKNOWN_GROUP = 5;

    public static final int USER_ACCOUNT_DISABLED_FLAG = 0x0002;
    public static final int USER_ACCOUNT_LOCKOUT_FLAG = 0x0010;

    private interface WinAdvApi32 extends Library
    {
        WinAdvApi32 INSTANCE =
             (WinAdvApi32) Native.loadLibrary(
                   "Advapi32.dll",
                   WinAdvApi32.class, W32APIOptions.UNICODE_OPTIONS);

        //boolean
        // ConvertSidToStringSid(
        //    IN PSID pSid,
        //    IN LPTSTR *ppStringSid
        //    );
        boolean
        ConvertSidToStringSid(
            Pointer pSid,
            PointerByReference ppStringSid
        );
    }

    private interface WinNetApi32 extends Library
    {
        WinNetApi32 INSTANCE =
             (WinNetApi32) Native.loadLibrary(
                   "Netapi32.dll",
                   WinNetApi32.class, W32APIOptions.UNICODE_OPTIONS);

        //NET_API_STATUS
        // NetUserGetInfo(
        //    IN LPTSTR ServerName,
        //    IN LPTSTR userName,
        //    IN DWORD level,
        //    OUT LPBYTE *buffer
        //    );
        int
        NetUserGetInfo(
            String ServerName,
            String userName,
            int level,
            PointerByReference ppUserInfo
        );

        // NET_API_STATUS
        // NetApiBufferFree(
        //     LPVOID buffer
        //     )
        int NetApiBufferFree(Pointer pBuf);
    }

    public static WinAccountAdapter getInstance() {
        return ourInstance;
    }

    public WinAccountAdapter()
    {
        if(Platform.isWindows() == false)
        {
            throw new RuntimeException(
                          "This class is only supported on Windows platform.");
        }
    }

    // IAccountAdapter

    @Override
    public boolean authenticate(String userPrincipalName, String password)
    {
        PlatformUtils.validateNotNull(userPrincipalName, "userPrincipalName");

        HANDLEByReference token = new HANDLEByReference();

        try {
            boolean isAuthenticated = Advapi32.INSTANCE.LogonUser(
                                           userPrincipalName,
                                           null,
                                           password,
                                           PlatformUtils.getWindowsLogonMode(),
                                           PlatformUtils.getWindowsLogonProvider(),
                                           token);
            if (isAuthenticated == false)
            {
                int errCode = Kernel32.INSTANCE.GetLastError();
                String errMsg = String.format("Authenticating user %s failed with error code (%d)",
                        userPrincipalName, errCode);
                switch (errCode)
                {
                    case WinAccountAdapter.ERROR_ACCOUNT_LOCKED_OUT:
                        throw new AccountLockedOutException(errMsg);
                    case WinAccountAdapter.ERROR_PASSWORD_EXPIRED:
                        throw new AccountPasswordExpiredException(errMsg);

                    default:
                        throw new AccountManagerException(errMsg);
                }
            }

            return true;
        }
        finally
        {
            if ( (token.getValue() != null) &&
                 (token.getPointer() != null) &&
                 (token.getPointer() != Pointer.NULL) &&
                 (token.getValue() != WinBase.INVALID_HANDLE_VALUE) )
            {
                Kernel32.INSTANCE.CloseHandle(token.getValue());
            }
        }
    }

    @Override
    public String lookupByObjectSid(String objectSid)
    {
        Validate.validateNotEmpty(objectSid, "objectSid");

        try {
            // if not found, exception it thrown
            Account id = Advapi32Util.getAccountBySid(objectSid);

            return id.fqn;
        }
        catch (Win32Exception e)
        {
            String errMsg = String.format("Failed to find account with sid (%s) - ",
                                          objectSid) + e.getMessage();
            logger.debug(errMsg);
            throw new AccountManagerException(errMsg);
        }
    }

    @Override
    public AccountInfo lookupByName(String acctName)
    {
        Validate.validateNotEmpty(acctName, "acctName");
        Account id_with_sid = null;

        try {
            // if not found, exception it thrown
            id_with_sid = Advapi32Util.getAccountByName(acctName);
        }
        catch (Win32Exception e)
        {
            String errMsg = String.format("Failed to find account with name (%s) - ",
                    acctName) + e.getMessage();
            logger.debug(errMsg);
            throw new AccountManagerException(errMsg);
        }

        try {
            Account id = Advapi32Util.getAccountBySid(id_with_sid.sidString);
            ACCOUNT_TYPE acctType = id.accountType == SID_TYPE_USER ? ACCOUNT_TYPE.USER :
                ((id.accountType == SID_TYPE_GROUP ||
                  id.accountType == SID_TYPE_ALIAS ||
                  id.accountType == SID_WELLKNOWN_GROUP) ? ACCOUNT_TYPE.GROUP : ACCOUNT_TYPE.OTHER);

            // Windows 2003 server returns id.name already containing domain part
            // Windows 2008 server returns id.name not containing domain part
            // Add logic to strip out @domainName part in case it is return as part of id.name
            String samAccountName = id.name;
            int idx = samAccountName.indexOf('@');
            if (idx > 0)
            {
                samAccountName = samAccountName.substring(0, idx);
            }

            return new AccountInfo(acctType,
                                   String.format("%s\\%s", id.domain, samAccountName),
                                   id_with_sid.sidString);
        }
        catch (Win32Exception e)
        {
            String errMsg = String.format("Failed to find account with sid (%s) - ",
                    id_with_sid.sidString) + e.getMessage();
            logger.debug(errMsg);
            throw new AccountManagerException(errMsg);
        }
    }

    public MachineAccountInfo getMachineAccountInfo()
    {
        return new MachineAccountInfo(null, null);
    }

    public UserInfo findUserByName(String samAccountName, String domainName)
    {
        Validate.validateNotEmpty(samAccountName, "samAccountName");
        Validate.validateNotEmpty(domainName, "domainName");
        UserInfo userInfo = null;
        String userName = String.format("%s@%s", samAccountName, domainName);

        try
        {
            userInfo = findUserByNameInternal(samAccountName, domainName);
        }
        catch(Exception e)
        {
            logger.error(String.format("Failed to findUserByNameInternal for user %s", userName), e);

            AccountInfo acctInfo = lookupByName(userName);
            if (acctInfo.acctType != ACCOUNT_TYPE.USER)
            {
                throw new AccountManagerException(String.format("Failed to findUserByName for user %s", userName));
            }
            // When lookup a user in 1-way trusted domain, using domain users works, but not machine account.
            // This is consistent with Likewise behavior, the bIsLocked and bIsDisabled are always set to false
            // when looking up a user in 1-way trusted domain
            // We need keep investigate, when SPN support is in place,
            // 1-way trusted domain user look up may be fully supported
            userInfo = new UserInfo(acctInfo.accountName,
                                    acctInfo.accountSid,
                                    false,
                                    false);
        }

        return userInfo;
    }
    private UserInfo findUserByNameInternal(String samAccountName, String domainName)
    {
        PointerByReference ppUserInfo = new PointerByReference(Pointer.NULL);
        Pointer pUserInfo = Pointer.NULL;
        PointerByReference ppSid = new PointerByReference(Pointer.NULL);
        Pointer pSid = Pointer.NULL;

        try
        {
            int dwError = 0;
            String name = String.format("%s@%s", samAccountName, domainName);

            // NetUserGetInfo
            dwError = WinNetApi32.INSTANCE.NetUserGetInfo(domainName, samAccountName, 4, ppUserInfo);
            if (dwError != 0)
            {
                String errMsg = String.format("findUserByName failed - NetUserGetInfo for user [%s] with error [%d]",
                                               name, dwError);
                logger.error(errMsg);
                throw new AccountManagerException(errMsg);
            }
            pUserInfo = ppUserInfo.getValue();
            if (pUserInfo == Pointer.NULL)
            {
                String errMsg = String.format("findUserByName failed - " +
                        "NetUserGetInfo for user [%s] returns NULL user info.", name);
                logger.error(errMsg);
                throw new AccountManagerException(errMsg);
            }
            WinUserInfo4Native userInfo = new WinUserInfo4Native(pUserInfo);

            // Convert SID to sid string
            boolean bSuccess = WinAdvApi32.INSTANCE.ConvertSidToStringSid(userInfo.pUserSid, ppSid);
            if (!bSuccess)
            {
                int errCode = Kernel32.INSTANCE.GetLastError();
                String errMsg = String.format("findUserByName failed - ConvertSidToStringSid for user [%s] with error [%d]",
                                              name, errCode);
                logger.error(errMsg);
                throw new AccountManagerException(errMsg);
            }

            pSid = ppSid.getValue();
            if (pSid == Pointer.NULL)
            {
                String errMsg = String.format("findUserByName failed - " +
                                              "ConvertSidToStringSid for user [%s] returns NULL sid", name);
                logger.error(errMsg);
                throw new AccountManagerException(errMsg);
            }
            String pszSid = pSid.getString(0, true);

            boolean bIsLocked = (userInfo.userFlags & USER_ACCOUNT_LOCKOUT_FLAG) != 0;
            boolean bIsDisabled = (userInfo.userFlags & USER_ACCOUNT_DISABLED_FLAG) != 0;

            return new UserInfo(String.format("%s\\%s", domainName, userInfo.pszName),
                                pszSid,
                                bIsLocked,
                                bIsDisabled);
        }
        finally
        {
            if (pUserInfo != Pointer.NULL)
            {
                WinNetApi32.INSTANCE.NetApiBufferFree(pUserInfo);
                ppUserInfo.setValue(Pointer.NULL);
            }
            if (pSid != Pointer.NULL)
            {
                Kernel32.INSTANCE.LocalFree(pSid);
                ppSid.setValue(Pointer.NULL);
            }
        }
    }

}
