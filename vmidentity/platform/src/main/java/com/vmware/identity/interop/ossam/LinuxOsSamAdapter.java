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

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.TypeMapper;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.NativeCallException;

import java.io.Closeable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

class LinuxOsSamAdapter extends NativeAdapter implements IOsSamAdapter, Closeable
{
    private boolean _initialized;

    private static final LinuxOsSamAdapter _instance = new LinuxOsSamAdapter();
    public static LinuxOsSamAdapter getInstance() { return _instance; }

    private LinuxOsSamAdapter()
    {
        LinuxOsSamAdapter.CheckError(
                NetApiLibrary.INSTANCE.NetApiInitialize()
        );

        this._initialized = true;
    }

    @Override
    public void LogonUser(String userName, String password)
    {
        PointerByReference serverHandle = new PointerByReference( Pointer.NULL );

        LinuxOsSamAdapter.CheckError(LsaLibrary.INSTANCE.LsaOpenServer(serverHandle));

        try
        {
            LinuxOsSamAdapter.CheckError(
                    LsaLibrary.INSTANCE.LsaAuthenticateUser(serverHandle.getValue(), userName, password, null)
            );
        }
        finally
        {
            if ( serverHandle.getValue() != null )
            {
                LsaLibrary.INSTANCE.LsaCloseServer( serverHandle.getValue() );
                serverHandle.setValue( Pointer.NULL );
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
            LinuxOsSamAdapter.CheckError(
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
            LinuxOsSamAdapter.FreeNetBuffer(groupsRef);
        }
    }

    @Override
    public GroupInfo getLocalGroupInfo(String groupName)
    {
        PointerByReference groupRef = new PointerByReference(Pointer.NULL);

        try
        {
            LinuxOsSamAdapter.CheckError(
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
            LinuxOsSamAdapter.FreeNetBuffer( groupRef );
        }
    }

    @Override
    public List<GroupInfo> getLocalGroups()
    {
        PointerByReference groupsRef = new PointerByReference(Pointer.NULL);
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        IntByReference resumeHandle = new IntByReference();

        try
        {
            ArrayList<GroupInfo> groups = new ArrayList<GroupInfo>();
            do
            {
                LinuxOsSamAdapter.CheckError(
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
                LinuxOsSamAdapter.FreeNetBuffer( groupsRef );
            }
            while( (entriesRead.getValue() > 0 ) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return groups;
        }
        finally
        {
            LinuxOsSamAdapter.FreeNetBuffer( groupsRef );
        }
    }

    @Override
    public UserInfo getLocalUserInfo(String userName)
    {
        PointerByReference userRef = new PointerByReference(Pointer.NULL);

        try
        {
            LinuxOsSamAdapter.CheckError(
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
            LinuxOsSamAdapter.FreeNetBuffer( userRef );
        }
    }

    @Override
    public List<UserInfo> getLocalUsers()
    {
        PointerByReference usersRef = new PointerByReference(Pointer.NULL);
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        IntByReference resumeHandle = new IntByReference();

        try
        {
            ArrayList<UserInfo> users = new ArrayList<UserInfo>();
            do
            {
                LinuxOsSamAdapter.CheckError(
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
                LinuxOsSamAdapter.FreeNetBuffer( usersRef );
            }
            while( (entriesRead.getValue() > 0) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return users;
        }
        finally
        {
            LinuxOsSamAdapter.FreeNetBuffer( usersRef );
        }
    }

    @Override
    public List<String> GetUsersInGroup(String groupName)
    {
        PointerByReference membersRef = new PointerByReference(Pointer.NULL);
        IntByReference entriesRead = new IntByReference();
        IntByReference totalEntries = new IntByReference();
        IntByReference resumeHandle = new IntByReference();

        try
        {
            ArrayList<String> users = new ArrayList<String>();
            do
            {
                LinuxOsSamAdapter.CheckError(
                        NetApiLibrary.INSTANCE.NetLocalGroupGetMembers(
                                null, groupName, GroupMembersInfo3Native.Level, membersRef,
                                OsSamConstants.MAX_PREFERRED_LENGTH,
                                entriesRead, totalEntries, resumeHandle
                        )
                );

                if ( ( entriesRead.getValue() > 0 ) && ( membersRef.getPointer() != null )
                        && ( membersRef.getPointer() != Pointer.NULL ) )
                {
                    GroupMembersInfo3Native info = new GroupMembersInfo3Native( membersRef.getValue() );
                    GroupMembersInfo3Native[] infos = (GroupMembersInfo3Native[])info.toArray( entriesRead.getValue() );

                    for( GroupMembersInfo3Native gmi3Native : infos )
                    {
                        users.add( LinuxOsSamAdapter.getName(gmi3Native.domainAndName) );
                    }
                }
                LinuxOsSamAdapter.FreeNetBuffer( membersRef );
            }
            while( (entriesRead.getValue() > 0) && ( totalEntries.getValue() > entriesRead.getValue() ) );

            return users;
        }
        finally
        {
            LinuxOsSamAdapter.FreeNetBuffer( membersRef );
        }
    }

    @Override
    public void close()
    {
        if (this._initialized == true)
        {
            NetApiLibrary.INSTANCE.NetApiShutdown();
            this._initialized = false;
        }
    }

    @Override
    protected void finalize() throws Throwable
    {
        this.close();
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

    private static String getName(String domainAndName)
    {
        String name = null;

        if( domainAndName != null )
        {
            int index = domainAndName.indexOf('\\');
            if( index >= 0 )
            {
                if( index < domainAndName.length() - 1 )
                {
                    name = domainAndName.substring( index + 1 );
                }
                else
                {
                    name = "";
                }
            }
            else
            {
                name = domainAndName;
            }
        }

        return name;
    }

    private static void CheckError(int errorCode)
    {
        if( errorCode != OsSamConstants.NERR_Success )
        {
            NativeCallException ex = (new NativeCallException(errorCode));
            if( errorCode == OsSamConstants.NERR_GroupNotFound )
            {
                throw new OsSamGroupNotFoundException(ex.getMessage());
            }
            else if( errorCode == OsSamConstants.NERR_UserNotFound )
            {
                throw new OsSamUserNotFoundException(ex.getMessage());
            }
            if(errorCode == OsSamConstants.ERROR_NO_SUCH_ALIAS)
            {
                throw new OsSamGroupNotFoundException(ex.getMessage());
            }
            else
            {
                throw new OsSamException( errorCode, ex.getMessage() );
            }
        }
    }

    private interface NetApiLibrary extends Library
    {
        NetApiLibrary INSTANCE =
                (NetApiLibrary) Native.loadLibrary(
                        "liblwnetapi",
                        NetApiLibrary.class,
                            new HashMap<String, TypeMapper>() {
                            {
                                put(OPTION_TYPE_MAPPER, LwApiTypeMapper.UNICODE);
                            }
                        }
                );

        //DWORD NetApiInitialize();
        int NetApiInitialize();

        //NET_API_STATUS NetApiShutdown( VOID );
        int NetApiShutdown();

        //NET_API_STATUS
        //NetLocalGroupGetInfo(
        //        PCWSTR  pwszHostname,
        //        PCWSTR  pwszGroupname,
        //        DWORD   dwLevel,
        //        PVOID  *ppBuffer
        //);
        int NetLocalGroupGetInfo( String pwszHostname, String pwszGroupname, int dwLevel, PointerByReference ppBuffer);

        //NET_API_STATUS
        //NetUserGetLocalGroups(
        //        PCWSTR  pwszHostname,
        //        PCWSTR  pwszUsername,
        //        DWORD   dwLevel,
        //        DWORD   dwFlags,
        //        PVOID  *ppBuffer,
        //        DWORD   dwMaxBufferSize,
        //        PDWORD  pdwNumEntries,
        //        PDWORD  pdwTotalNumEntries
        //);
        int NetUserGetLocalGroups( String pwszHostname, String pwszUsername, int dwLevel, int dwFlags,
                                   PointerByReference ppBuffer, int dwMaxBufferSize, IntByReference pdwNumEntries,
                                   IntByReference pdwTotalNumEntries);

        //NET_API_STATUS
        //NetLocalGroupEnum(
        //        PCWSTR  pwszHostname,
        //        DWORD   dwLevel,
        //        PVOID  *ppBuffer,
        //        DWORD   dwBufferMaxSize,
        //        PDWORD  pdwNumEntries,
        //        PDWORD  pdwTotalNumEntries,
        //        PDWORD  pdwResume
        //);
        int NetLocalGroupEnum( String pwszHostname, int dwLevel, PointerByReference ppBuffer, int dwBufferMaxSize,
                               IntByReference pdwNumEntries, IntByReference pdwTotalNumEntries, IntByReference pdwResume);

        //NET_API_STATUS
        //NetUserGetInfo(
        //        PCWSTR  pwszHostname,
        //        PCWSTR  pwszUsername,
        //        DWORD   dwLevel,
        //        PVOID  *ppBuffer
        //);
        int NetUserGetInfo( String pwszHostname, String pwszUsername, int dwLevel, PointerByReference ppBuffer );

        //NET_API_STATUS
        //NetUserEnum(
        //        PCWSTR  pwszHostname,
        //        DWORD   dwLevel,
        //        DWORD   dwFilter,
        //        PVOID  *ppBuffer,
        //        DWORD   dwMaxBufferSize,
        //        PDWORD  pdwNumEntries,
        //        PDWORD  pdwTotalNumEntries,
        //        PDWORD  pdwResume
        //);
        int NetUserEnum( String pwszHostname, int dwLevel, int dwFilter, PointerByReference ppBuffer,
                         int dwMaxBufferSize, IntByReference pdwNumEntries, IntByReference pdwTotalNumEntries,
                         IntByReference pdwResume);

        //NET_API_STATUS
        //NetLocalGroupGetMembers(
        //        PCWSTR  pwszHostname,
        //        PCWSTR  pwszLocalGroupName,
        //        DWORD   dwLevel,
        //        PVOID  *ppBuffer,
        //        DWORD   dwMaxBufferSize,
        //        PDWORD  pdwNumEntries,
        //        PDWORD  pdwTotalNumEntries,
        //        PDWORD  pdwResume
        //);
        int NetLocalGroupGetMembers( String pwszHostname, String pwszLocalGroupName, int dwLevel,
                                     PointerByReference ppBuffer, int dwMaxBufferSize, IntByReference pdwNumEntries,
                                     IntByReference pdwTotalNumEntries, IntByReference pdwResume);

        //NET_API_STATUS
        //NetApiBufferFree(
        //        PVOID   pBuffer
        //);
        int NetApiBufferFree(Pointer pBuffer);

    }

    private interface LsaLibrary extends Library
    {
        LsaLibrary INSTANCE =
                (LsaLibrary) Native.loadLibrary(
                        "liblsaclient",
                        LsaLibrary.class);

        //LW_DWORD
        //LsaOpenServer(
        //        LW_PHANDLE phConnection
        //);
        int LsaOpenServer( PointerByReference phConnection );

        //LW_DWORD
        //LsaAuthenticateUser(
        //        LW_HANDLE hLsaConnection,
        //        LW_PCSTR pszLoginName,
        //        LW_PCSTR pszPassword,
        //        LW_PSTR* ppszMessage
        //);
        int LsaAuthenticateUser( Pointer hLsaConnection, String pszLoginName, String pszPassword,
                                 Pointer ppszMessage ); // we will not receive message, it is unclear how to free it

        //LW_DWORD
        //LsaCloseServer(
        //        LW_HANDLE hConnection
        //);
        int LsaCloseServer( Pointer hConnection );
        
    }

}
