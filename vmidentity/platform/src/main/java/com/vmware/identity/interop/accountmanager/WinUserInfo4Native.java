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

import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;

public class WinUserInfo4Native extends Structure
{
    public String pszName;
    public String pszPassword;
    public int userPasswordAge;
    public int userPriv;
    public String userHomeDir;
    public String userComment;
    public int userFlags;
    public String userScriptPath;
    public int userAuthFlags;
    public String userFullName;
    public String userUsrComment;
    public String userParms;
    public String userWorkstations;
    public int userLastLogon;
    public int userLastLogoff;
    public int userAcctExpires;
    public int userMaxStorage;
    public int userUnitsPerWeek;
    public Pointer userLogonHours; //PBYTE
    public int userBadPwCount;
    public int userNumLogons;
    public String userLogonServer;
    public int userCountryCode;
    public int userCodePage;
    public Pointer pUserSid; //PSID
    public int userPrimaryGroupId;
    public String userProfile;
    public String userHomeDirDrive;
    public int userPasswordExpired;

    public WinUserInfo4Native()
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : null);
    }

    public WinUserInfo4Native(Pointer p)
    {
        this();
        useMemory(p);
        read();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "pszName", "pszPassword", "userPasswordAge", "userPriv",
                "userHomeDir", "userComment", "userFlags", "userScriptPath",
                "userAuthFlags", "userFullName", "userUsrComment", "userParms",
                "userWorkstations", "userLastLogon", "userLastLogoff", "userAcctExpires",
                "userMaxStorage", "userUnitsPerWeek", "userLogonHours","userBadPwCount",
                "userNumLogons", "userLogonServer", "userCountryCode", "userCodePage",
                "pUserSid", "userPrimaryGroupId", "userProfile", "userHomeDirDrive", "userPasswordExpired"
        });
    }
}
