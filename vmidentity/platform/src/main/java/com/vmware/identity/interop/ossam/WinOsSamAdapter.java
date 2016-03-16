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

package com.vmware.identity.interop.ossam;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.platform.win32.*;
import com.sun.jna.win32.W32APIOptions;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.win32.StdCallLibrary;

import java.util.*;

class WinOsSamAdapter implements IOsSamAdapter
{
    private static final Log logger = LogFactory
         .getLog(WinOsSamAdapter.class);
    private static final WinOsSamAdapter _instance = new WinOsSamAdapter();
    public static WinOsSamAdapter getInstance() { return _instance; }

    private WinOsSamAdapter() {}

    @Override
    public void LogonUser(String userName, String password)
    {
        WinNT.HANDLEByReference handleByReference = new WinNT.HANDLEByReference();
        try
        {
            boolean result = false;

            if( Advapi32.INSTANCE.LogonUser( userName, ".", password,
                WinBase.LOGON32_LOGON_NETWORK, WinBase.LOGON32_PROVIDER_DEFAULT,
                handleByReference ) == false )
            {
                Win32Exception ex = new Win32Exception( Kernel32.INSTANCE.GetLastError() );
                throw new OsSamException( OsSamConstants.NERR_InternalError, ex.getMessage(), ex );
            }
        }
        finally
        {
            if ( ( handleByReference != null ) && (handleByReference.getValue() != null)
                 && (handleByReference.getValue().getPointer() != Pointer.NULL ) )
            {
                Kernel32.INSTANCE.CloseHandle( handleByReference.getValue() );
            }
        }
    }

    @Override
    public List<String> GetLocalUserGroups(String userName, boolean recursive)
    {
        PointerByReference groupsRef = new PointerByReference(Pointer.NULL);
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();

        try
        {
            WinOsSamAdapter.CheckError(
                NetApiLibrary.INSTANCE.NetUserGetLocalGroups(
                    null, userName, GroupInfoNative.Level,
                    (recursive == true) ? OsSamConstants.LG_INCLUDE_INDIRECT : 0,
                    groupsRef, OsSamConstants.MAX_PREFERRED_LENGTH, entriesRead, totalEntries
                )
            );

            ArrayList<String> localGroups = new ArrayList<String>();

            if ( ( entriesRead.getValue() > 0 ) && ( groupsRef.getPointer() != null )
                 && ( groupsRef.getPointer() != Pointer.NULL ) )
            {
                GroupInfoNative info = new GroupInfoNative( groupsRef.getValue() );
                GroupInfoNative[] infos = (GroupInfoNative[])info.toArray( entriesRead.getValue() );

                for(GroupInfoNative gi : infos)
                {
                    localGroups.add( gi.groupName );
                }
            }

            return localGroups;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer(groupsRef);
        }
    }

    @Override
    public GroupInfo getLocalGroupInfo(String groupName)
    {
        PointerByReference groupRef = new PointerByReference(Pointer.NULL);

        try
        {
            WinOsSamAdapter.CheckError(
                    NetApiLibrary.INSTANCE.NetLocalGroupGetInfo(
                            null, groupName, GroupInfoExNative.Level, groupRef
                    )
            );

            GroupInfo group = null;

            if ( ( groupRef.getPointer() != null )
                 && ( groupRef.getPointer() != Pointer.NULL ) )
            {
                GroupInfoExNative info = new GroupInfoExNative( groupRef.getValue() );
                group = new GroupInfo( info.groupName, info.comment );
            }

            return group;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer( groupRef );
        }
    }

    @Override
    public List<GroupInfo> getLocalGroups()
    {
        PointerByReference groupsRef = new PointerByReference();
        groupsRef.setValue( Pointer.NULL );
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        PointerByReference resumeHandle = new PointerByReference();
        resumeHandle.setValue( Pointer.NULL );

        try
        {
            ArrayList<GroupInfo> groups = new ArrayList<GroupInfo>();
            do
            {
                WinOsSamAdapter.CheckError(
                        NetApiLibrary.INSTANCE.NetLocalGroupEnum(
                                null, GroupInfoExNative.Level, groupsRef, OsSamConstants.MAX_PREFERRED_LENGTH,
                                entriesRead, totalEntries, resumeHandle
                        )
                );

                if ( ( entriesRead.getValue() > 0 ) && ( groupsRef.getPointer() != null )
                        && ( groupsRef.getPointer() != Pointer.NULL ) )
                {
                    GroupInfoExNative info = new GroupInfoExNative( groupsRef.getValue() );
                    GroupInfoExNative[] infos = (GroupInfoExNative[])info.toArray( entriesRead.getValue() );
    
                    for( GroupInfoExNative giEx : infos )
                    {
                        groups.add(
                            new GroupInfo( giEx.groupName, giEx.comment )
                        );
                    }
                }
                WinOsSamAdapter.FreeNetBuffer( groupsRef );
            }
            while( (entriesRead.getValue() > 0 ) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return groups;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer( groupsRef );
        }
    }

    @Override
    public UserInfo getLocalUserInfo(String userName)
    {
        PointerByReference userRef = new PointerByReference(Pointer.NULL);

        try
        {
            WinOsSamAdapter.CheckError(
                    NetApiLibrary.INSTANCE.NetUserGetInfo(
                            null, userName, UserInfoNative.Level, userRef
                    )
            );

            UserInfo user = null;

            if ( ( userRef.getPointer() != null )
                    && ( userRef.getPointer() != Pointer.NULL ) )
            {
                UserInfoNative info = new UserInfoNative( userRef.getValue() );
                user = new UserInfo( info.name, info.fullName, info.comment, info.flags );
            }

            return user;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer( userRef );
        }
    }

    @Override
    public List<UserInfo> getLocalUsers()
    {
        PointerByReference usersRef = new PointerByReference();
        usersRef.setValue( Pointer.NULL );
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        IntByReference resumeHandle = new IntByReference(0);

        try
        {
            ArrayList<UserInfo> users = new ArrayList<UserInfo>();
            do
            {
                WinOsSamAdapter.CheckError(
                        NetApiLibrary.INSTANCE.NetUserEnum(
                                null, UserInfoNative.Level, OsSamConstants.FILTER_NORMAL_ACCOUNT,
                                usersRef, OsSamConstants.MAX_PREFERRED_LENGTH,
                                entriesRead, totalEntries, resumeHandle
                        )
                );

                if ( ( entriesRead.getValue() > 0 ) && ( usersRef.getPointer() != null )
                        && ( usersRef.getPointer() != Pointer.NULL ) )
                {
                    UserInfoNative info = new UserInfoNative( usersRef.getValue() );
                    UserInfoNative[] infos = (UserInfoNative[])info.toArray( entriesRead.getValue() );

                    for( UserInfoNative uiNative : infos )
                    {
                        users.add(
                            new UserInfo( uiNative.name, uiNative.fullName, uiNative.comment, uiNative.flags )
                        );
                    }
                }
                WinOsSamAdapter.FreeNetBuffer( usersRef );
            }
            while( (entriesRead.getValue() > 0) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return users;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer( usersRef );
        }
    }

    @Override
    public List<String> GetUsersInGroup(String groupName)
    {
        PointerByReference membersRef = new PointerByReference(Pointer.NULL);
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        PointerByReference resumeHandle = new PointerByReference(Pointer.NULL);

        try
        {
            ArrayList<String> users = new ArrayList<String>();
            do
            {
                WinOsSamAdapter.CheckError(
                        NetApiLibrary.INSTANCE.NetLocalGroupGetMembers(
                                null, groupName, GroupMembersInfoNative.Level, membersRef,
                                OsSamConstants.MAX_PREFERRED_LENGTH,
                                entriesRead, totalEntries, resumeHandle
                        )
                );

                if ( ( entriesRead.getValue() > 0 ) && ( membersRef.getPointer() != null )
                        && ( membersRef.getPointer() != Pointer.NULL ) )
                {
                    GroupMembersInfoNative info = new GroupMembersInfoNative( membersRef.getValue() );
                    GroupMembersInfoNative[] infos = (GroupMembersInfoNative[])info.toArray( entriesRead.getValue() );

                    for( GroupMembersInfoNative gmiNative : infos )
                    {
                        if( gmiNative.sidUsage == OsSamConstants.SidTypeUser )
                        {
                            users.add( gmiNative.name );
                        }
                    }
                }
                WinOsSamAdapter.FreeNetBuffer( membersRef );
            }
            while( (entriesRead.getValue() > 0) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return users;
        }
        finally
        {
            WinOsSamAdapter.FreeNetBuffer( membersRef );
        }
    }

    private static void CheckError(int errorCode)
    {
        if ( errorCode != OsSamConstants.NERR_Success )
        {
            // TODO: log
            String errorMessage = null;
            try
            {
                Win32Exception win32Ex = new Win32Exception(errorCode);
                errorMessage = win32Ex.getMessage();
            }
            catch (Exception e)
            {
                logger.error("Failed to convert error code: " + errorCode
                        + " to Win32Exception. Continuing CheckError.", e);
                errorMessage = e.getMessage();
            }

            if ( errorCode == OsSamConstants.NERR_GroupNotFound )
            {
                throw new OsSamGroupNotFoundException(errorMessage);
            }
            else if ( errorCode == OsSamConstants.NERR_UserNotFound )
            {
                throw new OsSamUserNotFoundException(errorMessage);
            }
            else if ( errorCode == OsSamConstants.ERROR_NO_SUCH_ALIAS )
            {
                throw new OsSamGroupNotFoundException(errorMessage);
            }
            else
            {
                throw new OsSamException(errorCode, errorMessage);
            }
        }
    }

    private static void FreeNetBuffer(PointerByReference buffer)
    {
        if ( (buffer != null)
              &&
             ( buffer.getPointer() != null )
             &&
             (buffer.getPointer() != Pointer.NULL) )
        {
            NetApiLibrary.INSTANCE.NetApiBufferFree(buffer.getValue());
            buffer.setPointer( Pointer.NULL );
        }
    }

    private interface NetApiLibrary extends StdCallLibrary
    {
        NetApiLibrary INSTANCE =
                (NetApiLibrary) Native.loadLibrary(
                        "Netapi32.dll",
                        NetApiLibrary.class, W32APIOptions.UNICODE_OPTIONS);


        //NET_API_STATUS NetLocalGroupGetInfo(
        //    __in   LPCWSTR servername, __in   LPCWSTR groupname,
        //    __in   DWORD level, __out  LPBYTE *bufptr
        //);
        /**
         * The NetLocalGroupGetInfo function retrieves information about a particular local group account on a server.
         *
         * @param serverName Pointer to a constant string that specifies the DNS or NetBIOS name of the remote server
         *                   on which the function is to execute. If this parameter is NULL,
         *                   the local computer is used.
         *
         * @param groupName Pointer to a constant string that specifies the name of the local group account
         *                  for which the information will be retrieved.
         *
         * @param level     Specifies the information level of the data. This parameter can be the following value.
         *                  1 -- Return the comment associated with the local group.
         *                  The bufptr parameter points to a LOCALGROUP_INFO_1 structure.
         *
         * @param groupInfo Pointer to the address of the buffer that receives the return information structure.
         *                  This buffer is allocated by the system and must be freed
         *                  using the NetApiBufferFree function.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         */
        int NetLocalGroupGetInfo( String serverName, String groupName, int level, PointerByReference groupInfo);
        
        //NET_API_STATUS NetUserGetLocalGroups(
        //    __in   LPCWSTR servername, __in   LPCWSTR username, __in   DWORD level,
        //    __in   DWORD flags, __out  LPBYTE *bufptr, __in   DWORD prefmaxlen,
        //    __out  LPDWORD entriesread, __out  LPDWORD totalentries
        //);
        /**
         *  The NetUserGetLocalGroups function retrieves a list of local groups to which a specified user belongs.
         *
         * @param serverName A pointer to a constant string that specifies the DNS or NetBIOS name of
         *                   the remote server on which the function is to execute.
         *                   If this parameter is NULL, the local computer is used.
         *
         * @param userName A pointer to a constant string that specifies the name of the user for which
         *                 to return local group membership information.
         *                 If the string is of the form DomainName\UserName the user name is expected
         *                 to be found on that domain.
         *                 If the string is of the form UserName, the user name is expected
         *                 to be found on the server specified by the servername parameter.
         *
         * @param level The information level of the data. This parameter can be the following value.
         *              0 -- Return the names of the local groups to which the user belongs.
         *              The bufptr parameter points to an array of LOCALGROUP_USERS_INFO_0 structures.
         *
         * @param flags A bitmask of flags that affect the operation.
         *              Currently, only the value defined is LG_INCLUDE_INDIRECT.
         *              If this bit is set, the function also returns the names of the local groups in which
         *              the user is indirectly a member (that is, the user has membership in a global group
         *              that is itself a member of one or more local groups).
         *
         * @param groupInfo A pointer to the buffer that receives the data. The format of this data depends
         *                  on the value of the level parameter.
         *                  This buffer is allocated by the system and must be freed using
         *                  the NetApiBufferFree function. Note that you must free the buffer even
         *                  if the function fails with ERROR_MORE_DATA.
         *
         * @param maxLen The preferred maximum length, in bytes, of the returned data.
         *               If MAX_PREFERRED_LENGTH is specified in this parameter, the function allocates the amount
         *               of memory required for the data. If another value is specified in this parameter,
         *               it can restrict the number of bytes that the function returns.
         *               If the buffer size is insufficient to hold all entries, the function returns ERROR_MORE_DATA.
         *
         * @param entriesRead A pointer to a value that receives the count of elements actually enumerated.
         *
         * @param totalEntries A pointer to a value that receives the total number of entries that
         *                     could have been enumerated.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         *
         */
        int NetUserGetLocalGroups(
            String serverName, String userName, int level, int flags,
            PointerByReference groupInfo, int maxLen,
            IntByReference entriesRead, IntByReference totalEntries );
        
        //NET_API_STATUS NetLocalGroupEnum(
        //    __in     LPCWSTR servername, __in     DWORD level, __out    LPBYTE *bufptr,
        //    __in     DWORD prefmaxlen, __out    LPDWORD entriesread, __out    LPDWORD totalentries,
        //    __inout  PDWORD_PTR resumehandle
        //);
        /**
         * The NetLocalGroupEnum function returns information about each local group account on the specified server.
         * @param servername Pointer to a constant string that specifies the DNS or NetBIOS name of the remote
         *                   server on which the function is to execute. If this parameter is NULL,
         *                   the local computer is used.
         *
         * @param level Specifies the information level of the data. This parameter can be one of the following values.
         *              0 -- Return local group names. The bufptr parameter points to an array of LOCALGROUP_INFO_0
         *                   structures.
         *              1 -- Return local group names and the comment associated with each group.
         *                   The bufptr parameter points to an array of LOCALGROUP_INFO_1 structures.
         *
         * @param groupInfo Pointer to the address of the buffer that receives the information structure.
         *                  The format of this data depends on the value of the level parameter.
         *                  This buffer is allocated by the system and must be freed using
         *                  the NetApiBufferFree function. Note that you must free the buffer even
         *                  if the function fails with ERROR_MORE_DATA.
         *
         * @param maxLen Specifies the preferred maximum length of returned data, in bytes.
         *               If you specify MAX_PREFERRED_LENGTH, the function allocates the amount of memory
         *               required for the data.
         *               If you specify another value in this parameter, it can restrict the number of bytes
         *               that the function returns. If the buffer size is insufficient to hold all entries,
         *               the function returns ERROR_MORE_DATA.
         *
         * @param entriesRead Pointer to a value that receives the count of elements actually enumerated.
         *
         * @param totalEntries Pointer to a value that receives the approximate total number of entries that could
         *                     have been enumerated from the current resume position.
         *                     The total number of entries is only a hint.
         *
         * @param resumeHandle Pointer to a value that contains a resume handle that is used to continue
         *                     an existing local group search.
         *                     The handle should be zero on the first call and left unchanged for subsequent calls.
         *                     If this parameter is NULL, then no resume handle is stored.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         *
         */
        int NetLocalGroupEnum( 
            String servername, int level, PointerByReference groupInfo, int maxLen,
            IntByReference entriesRead, IntByReference totalEntries, PointerByReference resumeHandle
        ); 
        
        //NET_API_STATUS NetUserGetInfo(
        //    __in   LPCWSTR servername, __in   LPCWSTR username,
        //    __in   DWORD level, __out  LPBYTE *bufptr
        //);
        /**
         *  The NetUserGetInfo function retrieves information about a particular user account on a server.
         *
         * @param server A pointer to a constant string that specifies the DNS or NetBIOS name of the remote
         *               server on which the function is to execute. If this parameter is NULL,
         *               the local computer is used.
         *
         * @param userName A pointer to a constant string that specifies the name of the user account
         *                 for which to return information.
         *
         * @param level The information level of the data. This parameter can be one of the following values.
         *              0 -- Return the user account name. The bufptr parameter points to a USER_INFO_0 structure.
         *              1 -- Return detailed information about the user account. The bufptr parameter points
         *                   to a USER_INFO_1 structure.
         *              2 -- Return detailed information and additional attributes about the user account.
         *                   The bufptr parameter points to a USER_INFO_2 structure.
         *              3 -- Return detailed information and additional attributes about the user account.
         *                   This level is valid only on servers.
         *                   The bufptr parameter points to a USER_INFO_3 structure.
         *                   Note that it is recommended that you use USER_INFO_4 instead.
         *              4 -- Return detailed information and additional attributes about the user account.
         *                   This level is valid only on servers.
         *                   The bufptr parameter points to a USER_INFO_4 structure.
         *              10 -- Return user and account names and comments.
         *                    The bufptr parameter points to a USER_INFO_10 structure.
         *              11 -- Return detailed information about the user account.
         *                    The bufptr parameter points to a USER_INFO_11 structure.
         *              20 -- Return the user's name and identifier and various account attributes.
         *                    The bufptr parameter points to a USER_INFO_20 structure.
         *              23 -- Return the user's name and identifier and various account attributes.
         *                    The bufptr parameter points to a USER_INFO_23 structure.
         *
         *
         * @param userInfo A pointer to the buffer that receives the data.
         *                 The format of this data depends on the value of the level parameter.
         *                 This buffer is allocated by the system and must be freed
         *                 using the NetApiBufferFree function.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         *
         */
        int NetUserGetInfo( String server, String userName, int level, PointerByReference userInfo );
        
        //NET_API_STATUS NetUserEnum(
        //   __in     LPCWSTR servername, __in     DWORD level, __in     DWORD filter,
        //   __out    LPBYTE *bufptr, __in     DWORD prefmaxlen, __out    LPDWORD entriesread,
        //    __out    LPDWORD totalentries, __inout  LPDWORD resume_handle
        //);
        /**
         * The NetUserEnum function retrieves information about all user accounts on a server.
         *
         * @param serverName A pointer to a constant string that specifies the DNS or NetBIOS name
         *                   of the remote server on which the function is to execute.
         *                   If this parameter is NULL, the local computer is used.
         *
         * @param level Specifies the information level of the data. This parameter can be one of the following values.
         *              0 -- Return user account names.
         *                   The bufptr parameter points to an array of USER_INFO_0 structures.
         *              1 -- Return detailed information about user accounts.
         *                   The bufptr parameter points to an array of USER_INFO_1 structures.
         *              2 -- Return detailed information about user accounts, including authorization levels
         *                   and logon information.
         *                   The bufptr parameter points to an array of USER_INFO_2 structures.
         *              3 -- Return detailed information about user accounts, including authorization levels,
         *                   logon information, RIDs for the user and the primary group, and profile information.
         *                   The bufptr parameter points to an array of USER_INFO_3 structures.
         *              10 -- Return user and account names and comments.
         *                    The bufptr parameter points to an array of USER_INFO_10 structures.
         *              11 -- Return detailed information about user accounts.
         *                    The bufptr parameter points to an array of USER_INFO_11 structures.
         *              20 -- Return the user's name and identifier and various account attributes.
         *                    The bufptr parameter points to an array of USER_INFO_20 structures.
         *              23 -- Return the user's name and identifier and various account attributes.
         *                    The bufptr parameter points to an array of USER_INFO_23 structures.
         *
         * @param filter A value that specifies the user account types to be included in the enumeration.
         *               A value of zero indicates that all normal user, trust data, and machine account
         *               data should be included.
         *               This parameter can also be a combination of the following values.
         *               FILTER_TEMP_DUPLICATE_ACCOUNT: Enumerates account data for users whose primary account
         *               is in another domain. This account type provides user access to this domain,
         *               but not to any domain that trusts this domain. The User Manager refers to this account
         *               type as a local user account.
         *               FILTER_NORMAL_ACCOUNT: Enumerates normal user account data.
         *               This account type is associated with a typical user.
         *               FILTER_INTERDOMAIN_TRUST_ACCOUNT: Enumerates interdomain trust account data.
         *               This account type is associated with a trust account for a domain that trusts other domains.
         *               FILTER_WORKSTATION_TRUST_ACCOUNT: Enumerates workstation or member server trust account data.
         *               This account type is associated with a machine account for a computer that is
         *               a member of the domain.
         *               FILTER_SERVER_TRUST_ACCOUNT: Enumerates member server machine account data.
         *               This account type is associated with a computer account for a backup domain
         *               controller that is a member of the domain.
         *
         * @param userInfo A pointer to the buffer that receives the data. The format of this data depends
         *                 on the value of the level parameter.
         *                 The buffer for this data is allocated by the system and the application must call
         *                 the NetApiBufferFree function to free the allocated memory when the data returned
         *                 is no longer needed. Note that you must free the buffer even if the NetUserEnum
         *                 function fails with ERROR_MORE_DATA.
         *
         * @param maxLen The preferred maximum length, in bytes, of the returned data.
         *               If you specify MAX_PREFERRED_LENGTH, the NetUserEnum function allocates
         *               the amount of memory required for the data.
         *               If you specify another value in this parameter, it can restrict the number of bytes
         *               that the function returns. If the buffer size is insufficient to hold all entries,
         *               the function returns ERROR_MORE_DATA.
         *
         * @param entriesRead A pointer to a value that receives the count of elements actually enumerated.
         *
         * @param totalEntries A pointer to a value that receives the total number of entries that
         *                     could have been enumerated from the current resume position.
         *                     Note that applications should consider this value only as a hint.
         *
         * @param resumeHandle A pointer to a value that contains a resume handle which is used to continue
         *                     an existing user search. The handle should be zero on the first call and left
         *                     unchanged for subsequent calls. If this parameter is NULL,
         *                     then no resume handle is stored.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         *
         */
        int NetUserEnum(
            String serverName, int level, int filter, PointerByReference userInfo, int maxLen,
            IntByReference entriesRead, IntByReference totalEntries, IntByReference resumeHandle
        );

        //NET_API_STATUS NetLocalGroupGetMembers(
        //       __in     LPCWSTR servername, __in     LPCWSTR localgroupname, __in     DWORD level,
        //       __out    LPBYTE *bufptr, __in     DWORD prefmaxlen, __out    LPDWORD entriesread,
        //        __out    LPDWORD totalentries, __inout  PDWORD_PTR resumehandle
        //);
        /**
         * The NetLocalGroupGetMembers function retrieves a list of the members of a particular local group
         * in the security database, which is the security accounts manager (SAM) database or,
         * in the case of domain controllers, the Active Directory.
         * Local group members can be users or global groups.
         *
         * @param serverName Pointer to a constant string that specifies the DNS or NetBIOS name of the
         *                   remote server on which the function is to execute.
         *                   If this parameter is NULL, the local computer is used.
         *
         * @param localGroupName Pointer to a constant string that specifies the name of the local group
         *                       whose members are to be listed.
         *
         * @param level Specifies the information level of the data. This parameter can be one of the following values.
         *              0 -- Return the security identifier (SID) associated with the local group member.
         *              The bufptr parameter points to an array of LOCALGROUP_MEMBERS_INFO_0 structures.
         *              1 -- Return the SID and account information associated with the local group member.
         *              The bufptr parameter points to an array of LOCALGROUP_MEMBERS_INFO_1 structures.
         *              2 -- Return the SID, account information, and the domain name associated with
         *              the local group member.
         *              The bufptr parameter points to an array of LOCALGROUP_MEMBERS_INFO_2 structures.
         *              3 -- Return the account and domain names of the local group member.
         *              The bufptr parameter points to an array of LOCALGROUP_MEMBERS_INFO_3 structures.
         *
         * @param membersInfo Pointer to the address that receives the return information structure.
         *                    The format of this data depends on the value of the level parameter.
         *                    This buffer is allocated by the system and must be freed using
         *                    the NetApiBufferFree function.
         *                    Note that you must free the buffer even if the function fails with ERROR_MORE_DATA.
         *
         * @param maxLen Specifies the preferred maximum length of returned data, in bytes.
         *               If you specify MAX_PREFERRED_LENGTH, the function allocates the amount of memory required
         *               for the data.
         *               If you specify another value in this parameter, it can restrict the number of bytes
         *               that the function returns. If the buffer size is insufficient to hold all entries,
         *               the function returns ERROR_MORE_DATA.
         *
         * @param entriesRead Pointer to a value that receives the count of elements actually enumerated.
         *
         * @param totalEntries Pointer to a value that receives the total number of entries that
         *                     could have been enumerated from the current resume position.
         *
         * @param resumeHandle  Pointer to a value that contains a resume handle which is used to continue
         *                      an existing group member search. The handle should be zero on the first call
         *                      and left unchanged for subsequent calls.
         *                      If this parameter is NULL, then no resume handle is stored.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value can be one of the following error codes.
         *
         */
        int NetLocalGroupGetMembers(
            String serverName, String localGroupName, int level, PointerByReference membersInfo,
            int maxLen, IntByReference entriesRead, IntByReference totalEntries, PointerByReference resumeHandle
        );
        
        
        //NET_API_STATUS NetApiBufferFree( __in  LPVOID Buffer );
        /**
         *  The NetApiBufferFree function frees the memory that the NetApiBufferAllocate function allocates.
         *  Applications should also call NetApiBufferFree to free the memory that other network management
         *  functions use internally to return information.
         *
         * @param buffer A pointer to a buffer returned previously by another network management function
         *               or memory allocated by calling the NetApiBufferAllocate function.
         *
         * @return If the function succeeds, the return value is NERR_Success.
         *         If the function fails, the return value is a system error code.
         *
         */
        int NetApiBufferFree( Pointer buffer );
    }
}
