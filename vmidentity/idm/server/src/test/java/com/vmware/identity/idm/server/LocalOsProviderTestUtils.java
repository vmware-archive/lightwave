/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */
package com.vmware.identity.idm.server;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.InputStreamReader;
import java.util.List;

import com.sun.jna.Native;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.win32.StdCallLibrary;
import com.sun.jna.win32.W32APIOptions;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.interop.NativeAdapter;

class UserInfo
{
    private String _name;
    private String _firstName;
    private String _lastName;
    private String _password;
    private String _description;
    boolean _isLocked;
    boolean _isDisabled;

    UserInfo(String name, String firstName, String lastName,
             String password, String description, boolean isLocked, boolean isDisabled)
    {
        ValidateUtil.validateNotEmpty(name, "name");

        this._name=  name;
        this._firstName = firstName;
        this._lastName = lastName;
        this._password = password;
        this._description = description;
        this._isDisabled = isDisabled;
        this._isLocked = isLocked;
    }

    String getName() { return this._name; }
    String getFirstName() { return this._firstName; }
    String getLastName() { return this._lastName; }
    String getPassword() { return this._password; }
    String getDescription() { return this._description; }
    boolean isDisabled() { return this._isDisabled; }
    boolean isLocked() {  return this._isLocked; }
}

class GroupInfo
{
    private String _name;
    private String _description;

    GroupInfo(String name, String description)
    {
        ValidateUtil.validateNotEmpty( name, "name" );
        this._name = name;
        this._description = description;
    }
    String getName() { return this._name; }
    String getDescription() { return this._description; }
}

interface ISecurityAccountsLibrary extends Closeable
{
    void AddUser( String userName, String userPassword, String fullName,
                  String description, boolean disabled, boolean locked );
    void AddGroup( String groupName, String description );
    void AddUsersToGroup( String groupName, List<String> usersToAdd );
    void DeleteUser( String userName );
    void DeleteGroup( String groupName );

    @Override
    void close(); // we don't want dispose to throw
}


class WinSecurityAccountsLibrary implements ISecurityAccountsLibrary
{
    @Override
    public void AddUser(String userName, String userPassword, String fullName,
                        String description,  boolean disabled, boolean locked )
    {
        UserInfo1 info = new UserInfo1();
        info.name = userName;
        info.password = userPassword;
        info.comment = description;
        info.flags = UF_NORMAL_ACCOUNT;
        if(disabled == true)
        {
            info.flags = info.flags | UF_ACCOUNT_DISABLE;
        }
        if( locked)
        {
            //There is only one way that an account can be locked out: the user tries to log on more than the number
            //of times that are specified in the Password policy setting in Group Policy.
            //If you want to prevent the use of an account, disable it.
            //A user whose account has been disabled cannot log on until a member of the
            //Administrators group enables the account.
            //
        }
        info.privs = USER_PRIV_USER;

        CheckError(
                samLib.INSTANCE.NetUserAdd(null, UserInfo1.Level, info, null)
        );

        if ( (fullName != null) && (fullName.isEmpty() == false) )
        {
            UserFullNameInfo fullNameInfo = new UserFullNameInfo();
            fullNameInfo.fullName = fullName;

            CheckError(
                    samLib.INSTANCE.NetUserSetInfo(null, userName, UserFullNameInfo.Level, fullNameInfo, null)
            );
        }
    }

    @Override
    public void AddGroup(String groupName, String description)
    {
        LocalGroupInfo1 info = new LocalGroupInfo1();
        info.name = groupName;
        info.comment = description;

        CheckError(
                samLib.INSTANCE.NetLocalGroupAdd(null, LocalGroupInfo1.Level, info, null)
        );
    }

    @Override
    public void AddUsersToGroup(String groupName, List<String> usersToAdd)
    {
        LocalGroupMembersInfo info = new LocalGroupMembersInfo();
        LocalGroupMembersInfo[] infos = (LocalGroupMembersInfo[])info.toArray(usersToAdd.size());
        for(int i = 0; i < usersToAdd.size(); i++)
        {
            infos[i].domainName = usersToAdd.get(i);
        }

        CheckError(
                samLib.INSTANCE.NetLocalGroupAddMembers(
                        null, groupName, LocalGroupMembersInfo.Level, infos, usersToAdd.size())
        );
    }

    @Override
    public void DeleteUser(String userName)
    {
        CheckError(
                samLib.INSTANCE.NetUserDel(null, userName)
        );
    }

    @Override
    public void DeleteGroup(String groupName)
    {
        CheckError(
                samLib.INSTANCE.NetLocalGroupDel(null, groupName)
        );
    }

    @Override
    public void close()
    {
        // close is a no-op for this one ...
    }

    private void CheckError(int errorCode)
    {
        if( errorCode != 0 )
        {
            throw new Win32Exception( errorCode );
        }
    }

    private static final int UF_ACCOUNT_DISABLE = 0x0002;
    // private static final int UF_LOCKOUT = 0x0010;
    private static final int UF_NORMAL_ACCOUNT = 0x0200;

    private static final int USER_PRIV_USER = 1;

    interface samLib extends StdCallLibrary
    {
        samLib INSTANCE =
                (samLib) Native.loadLibrary(
                        "Netapi32.dll",
                        samLib.class, W32APIOptions.UNICODE_OPTIONS);

        //NET_API_STATUS NetUserAdd(
        //        __in   LMSTR servername,
        //        __in   DWORD level,
        //        __in   LPBYTE buf,
        //        __out  LPDWORD parm_err
        //);
        int NetUserAdd(String serverName, int level, UserInfo1 userInfos, IntByReference parmErr);

        //NET_API_STATUS NetUserSetInfo(
        //        __in   LPCWSTR servername,
        //        __in   LPCWSTR username,
        //        __in   DWORD level,
        //        __in   LPBYTE buf,
        //        __out  LPDWORD parm_err
        //);
        int NetUserSetInfo(String serverName, String userName, int level,
                           UserFullNameInfo userFullNameInfo, IntByReference parmErr);

        //NET_API_STATUS NetLocalGroupAdd(
        //        __in   LPCWSTR servername,
        //        __in   DWORD level,
        //        __in   LPBYTE buf,
        //        __out  LPDWORD parm_err
        //);
        int NetLocalGroupAdd(String serverName, int level, LocalGroupInfo1 groupInfos, IntByReference parmErr);

        //NET_API_STATUS NetLocalGroupAddMembers(
        //        __in  LPCWSTR servername,
        //        __in  LPCWSTR groupname,
        //        __in  DWORD level,
        //        __in  LPBYTE buf,
        //        __in  DWORD totalentries
        //);
        int NetLocalGroupAddMembers( String serverName, String groupName, int level,
                                     LocalGroupMembersInfo[] members, int totalEntries );

        //NET_API_STATUS NetUserDel(
        //        __in  LPCWSTR servername,
        //        __in  LPCWSTR username
        //);
        int NetUserDel(String serverName, String userName);

        //NET_API_STATUS NetLocalGroupDel(
        //        __in  LPCWSTR servername,
        //        __in  LPCWSTR groupname
        //);
        int NetLocalGroupDel(String serverName, String groupName);
    }
}

class LinuxSecurityAccountsLibrary extends NativeAdapter implements ISecurityAccountsLibrary
{
    private boolean _initialized;
    private Runtime _runtime;

    public LinuxSecurityAccountsLibrary()
    {
        this._runtime = Runtime.getRuntime();
        this._initialized = true;
    }

    private String runCommand(String[] cmd)
    {
        String retval = "";
        try
        {
            Process pr = this._runtime.exec(cmd);
            pr.waitFor();
            BufferedReader buf = new BufferedReader(new InputStreamReader(pr.getInputStream()));
            String line = null;
            while ((line=buf.readLine())!=null) {
                if (retval.length() > 0) {
                    retval = retval + "\n";
                }
                retval = retval + line;
            }
        }
        catch (Exception e)
        {
            throw new IllegalStateException(e);
        }
        return retval;
    }

    @Override
    public void AddUser(String userName, String userPassword, String fullName,
                        String description,  boolean disabled, boolean locked )
    {
        String[] createPasswordCmd = new String[] {
                "perl",
                "-e",
                "print crypt(\"" + userPassword + "\", \"salt\"),\"\n\""
        };
        String passwdHash = runCommand(createPasswordCmd);

        String[] addUserCmd = new String[] {
                "/usr/sbin/useradd",
                "-p",
                passwdHash,
                userName};
        String[] setFullNameCmd = new String[] {
                "chfn",
                "-f",
                fullName,
                userName};
        String[] disableUserCmd = new String[] {
                "passwd",
                "-l",
                userName};

        runCommand(addUserCmd);
        if ( (fullName != null) && (fullName.isEmpty() == false) )
        {
            runCommand(setFullNameCmd);
        }
        if (disabled)
        {
            runCommand(disableUserCmd);
        }
        // ignore description for now
        if( locked)
        {
            // There is only one way that an account can be locked out: the user tries to log on more than the number
            //of times that are specified in the Password policy setting in Group Policy.
            //If you want to prevent the use of an account, disable it.
            //A user whose account has been disabled cannot log on until a member of the
            //Administrators group enables the account.
            //
        }
    }

    @Override
    public void AddGroup(String groupName, String description)
    {
        String[] addGroupCmd = new String[] {
                "/usr/sbin/groupadd",
                groupName};
        runCommand(addGroupCmd);
        // ignore description for now
    }

    @Override
    public void AddUsersToGroup(String groupName, List<String> usersToAdd)
    {
        for(int i = 0; i < usersToAdd.size(); i++)
        {
            String userName = usersToAdd.get(i);
            String[] getUsersGroups = new String[] {
                    "groups",
                    userName
            };
            String groups = runCommand(getUsersGroups);
            groups = groups.replace(userName + " : users ", "");
            groups = groups.replace(' ', ',');
            groups = groupName + "," + groups;
            String[] addUserToGroupCmd = new String[] {
                    "/usr/sbin/usermod",
                    "-G",
                    groups,
                    userName};
            // for example:
            //  usermod -G developers,`groups dkalin | sed 's/^dkalin : users //' | tr ' ' ','` dkalin
            runCommand(addUserToGroupCmd);
        }
    }

    @Override
    public void DeleteUser(String userName)
    {
        String[] deleteUserCmd = new String[] {
                "/usr/sbin/userdel",
                userName};
        runCommand(deleteUserCmd);
    }

    @Override
    public void DeleteGroup(String groupName)
    {
        String[] deleteGroupCmd = new String[] {
                "/usr/sbin/groupdel",
                groupName};
        runCommand(deleteGroupCmd);
    }

    @Override
    public void close()
    {
        if(this._initialized == true)
        {
            this._initialized = false;
        }
    }

    @Override
    protected void finalize() throws Throwable
    {
        this.close();
    }
}

