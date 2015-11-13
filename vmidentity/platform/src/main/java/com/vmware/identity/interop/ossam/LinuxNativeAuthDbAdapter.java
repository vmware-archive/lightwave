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
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.NativeCallException;
import com.vmware.identity.interop.NoMoreDataException;

import java.io.Closeable;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class LinuxNativeAuthDbAdapter
	extends    NativeAdapter
	implements IOsSamAdapter, Closeable
{
	private static final
	LinuxNativeAuthDbAdapter _instance = new LinuxNativeAuthDbAdapter();

	public static LinuxNativeAuthDbAdapter getInstance() { return _instance; }

    private static final Log logger =
    							LogFactory.getLog(
    											LinuxNativeAuthDbAdapter.class);

	protected LinuxNativeAuthDbAdapter()
	{
	}

	@Override
	public void close() throws IOException
	{
	}

    @Override
    protected void finalize() throws Throwable
    {
        this.close();
    }

	@Override
	public void LogonUser(String userName, String password)
	{
		logger.info(String.format("Authenticating user [%s]", userName));

		int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthCredentials(
															userName,
															password);
		checkNativeErrorCode(errCode);
	}

	@Override
	public List<String> GetLocalUserGroups(String userName, boolean recursive)
	{
		logger.info(
				String.format(
						"Getting local groups for user [%s]. Recursive ? [%s]",
						userName,
						recursive ? "yes" : "no"));

		List<String> groups = new ArrayList<String>();

		PointerByReference ppGroups = new PointerByReference();
		IntByReference     pdwNumGroups = new IntByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthFindGroupsForUser(
																userName,
																ppGroups,
																pdwNumGroups);
			if (errCode != NoMoreDataException.ERROR_NO_MORE_ITEMS)
			{
			    checkNativeErrorCode(errCode);
			}

			if (pdwNumGroups.getValue() > 0)
			{
				LinuxGroupNative nativeGroup = new LinuxGroupNative(
														ppGroups.getValue());
				LinuxGroupNative nativeGroups[] =
						(LinuxGroupNative[])nativeGroup.toArray(
													pdwNumGroups.getValue());

				for (LinuxGroupNative grp : nativeGroups)
				{
					groups.add(new String(grp.name));
				}
			}
		}
		finally
		{
			if (ppGroups != null &&
			    ppGroups.getPointer() != Pointer.NULL &&
		        ppGroups.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeGroupInfoArray(
												ppGroups.getValue(),
												pdwNumGroups.getValue());
			}
		}

		return groups;
	}

	@Override
	public GroupInfo getLocalGroupInfo(String groupName)
	{
		PointerByReference ppGroup = new PointerByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthFindGroup(
																groupName,
																ppGroup);
			checkNativeErrorCode(errCode);

			LinuxGroupNative nativeGroup = new LinuxGroupNative(
													ppGroup.getValue());

			return buildGroupInfo(nativeGroup);
		}
		finally
		{
			if (ppGroup.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeGroupInfo(
													ppGroup.getValue());
			}
		}
	}

	@Override
	public List<GroupInfo> getLocalGroups()
	{
		List<GroupInfo> groups = new ArrayList<GroupInfo>();

		PointerByReference ppContext = new PointerByReference();
		PointerByReference ppGroupInfo = new PointerByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthBeginEnumGroups(
																	ppContext);
			checkNativeErrorCode(errCode);

			boolean done = false;

			do
			{
				if (ppGroupInfo.getValue() != Pointer.NULL)
				{
					VmDirAuthLibrary.INSTANCE.VmDirAuthFreeGroupInfo(
													ppGroupInfo.getValue());
					ppGroupInfo.setValue(Pointer.NULL);
				}

				errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthGetNextGroup(
														ppContext.getValue(),
														ppGroupInfo);

				if (errCode == NoMoreDataException.ERROR_NO_MORE_ITEMS ||
				    ppGroupInfo.getPointer() == Pointer.NULL)
				{
					done = true;
				}
				else
				{
					LinuxGroupNative nativeGroup =
										new LinuxGroupNative(
													ppGroupInfo.getValue());

					groups.add(buildGroupInfo(nativeGroup));
				}

			} while (!done);
		}
		finally
		{
			if (ppGroupInfo != null &&
		        ppGroupInfo.getPointer() != Pointer.NULL &&
		        ppGroupInfo.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeGroupInfo(
												ppGroupInfo.getValue());
				ppGroupInfo.setValue(Pointer.NULL);
			}

			if (ppContext != null &&
		        ppContext.getPointer() != Pointer.NULL &&
		        ppContext.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthEndEnumGroups(
													ppContext.getValue());
			}
		}

		return groups;
	}

	@Override
	public UserInfo getLocalUserInfo(String userName)
	{
		PointerByReference ppUser = new PointerByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthFindUser(
																userName,
																ppUser);
			checkNativeErrorCode(errCode);

			LinuxUserNative nativeUser = new LinuxUserNative(
													ppUser.getValue());

			return buildUserInfo(nativeUser);
		}
		finally
		{
			if (ppUser.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeUserInfo(
													ppUser.getValue());
			}
		}
	}

	@Override
	public List<UserInfo> getLocalUsers()
	{
		List<UserInfo> users = new ArrayList<UserInfo>();

		PointerByReference ppContext = new PointerByReference();
		PointerByReference ppUserInfo = new PointerByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthBeginEnumUsers(
																	ppContext);
			checkNativeErrorCode(errCode);

			boolean done = false;

			do
			{
				if (ppUserInfo.getValue() != Pointer.NULL)
				{
					VmDirAuthLibrary.INSTANCE.VmDirAuthFreeUserInfo(
													ppUserInfo.getValue());
					ppUserInfo.setValue(Pointer.NULL);
				}

				errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthGetNextUser(
														ppContext.getValue(),
														ppUserInfo);

				if (errCode == NoMoreDataException.ERROR_NO_MORE_ITEMS ||
			        ppUserInfo.getValue() == Pointer.NULL)
				{
					done = true;
				}
				else
				{
					LinuxUserNative nativeUser =
										new LinuxUserNative(
													ppUserInfo.getValue());

					users.add(buildUserInfo(nativeUser));
				}

			} while (!done);
		}
		finally
		{
			if (ppUserInfo != null &&
		        ppUserInfo.getPointer() != Pointer.NULL &&
		        ppUserInfo.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeUserInfo(
												ppUserInfo.getValue());
				ppUserInfo.setValue(Pointer.NULL);
			}

			if (ppContext != null &&
		        ppContext.getPointer() != Pointer.NULL &&
		        ppContext.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthEndEnumUsers(
													ppContext.getValue());
			}
		}

		return users;
	}

	@Override
	public List<String> GetUsersInGroup(String groupName)
	{
		List<String> users = new ArrayList<String>();

		PointerByReference ppUsers = new PointerByReference();
		IntByReference pdwNumUsers = new IntByReference();

		try
		{
			int errCode = VmDirAuthLibrary.INSTANCE.VmDirAuthFindUsersInGroup(
														groupName,
														ppUsers,
														pdwNumUsers);

			checkNativeErrorCode(errCode);

			if (pdwNumUsers.getValue() > 0)
			{
				LinuxUserNative nativeUser = new LinuxUserNative(
													ppUsers.getValue());
				LinuxUserNative nativeUsers[] =
						(LinuxUserNative[]) nativeUser.toArray(
														pdwNumUsers.getValue());

				for (LinuxUserNative user : nativeUsers)
				{
					users.add(new String(user.name));
				}
			}
		}
		finally
		{
			if (ppUsers.getValue() != Pointer.NULL)
			{
				VmDirAuthLibrary.INSTANCE.VmDirAuthFreeUserInfoArray(
												ppUsers.getValue(),
												pdwNumUsers.getValue());
			}
		}

		return users;
	}

	private static UserInfo buildUserInfo(LinuxUserNative nativeUser)
	{
		return new UserInfo(
						new String(nativeUser.name),
						new String(nativeUser.fullName),
						nativeUser.comment != null ?
								new String(nativeUser.comment) : null,
						0);
	}

	private static GroupInfo buildGroupInfo(LinuxGroupNative nativeGroup)
	{
		return new GroupInfo(
						new String(nativeGroup.name),
						nativeGroup.comment != null ?
								new String(nativeGroup.comment) : null);
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

			case OsSamConstants.ERROR_NO_SUCH_GROUP :

				throw new OsSamGroupNotFoundException("Group not found");

			case OsSamConstants.ERROR_NO_SUCH_USER:

				throw new OsSamUserNotFoundException("User not found");

			default:

				throw new NativeCallException(errorCode);

    	}
    }

	private interface VmDirAuthLibrary extends Library
    {
		VmDirAuthLibrary INSTANCE =
					(VmDirAuthLibrary) Native.loadLibrary(
							                        "libvmdirauth",
							                        VmDirAuthLibrary.class);

		int
		VmDirAuthFindUser(
		    String             pszUsername,     /* IN     */
		    PointerByReference ppUserInfo       /*    OUT */
		    );

		int
		VmDirAuthBeginEnumUsers(
			PointerByReference ppContext     /*    OUT */
		    );

		int
		VmDirAuthGetNextUser(
		    Pointer            pContext,      /* IN     */
		    PointerByReference ppUserInfo     /*    OUT */
		    );

		int
		VmDirAuthEndEnumUsers(
		    Pointer pContext       /* IN OUT */
		    );

		int
		VmDirAuthFindGroup(
			String             pszGroupname,   /* IN     */
		    PointerByReference ppGroupInfo     /*    OUT */
		    );

		int
		VmDirAuthBeginEnumGroups(
		    PointerByReference ppContext     /*    OUT */
		    );

		int
		VmDirAuthGetNextGroup(
		    Pointer            pContext,      /* IN     */
		    PointerByReference ppGroupInfo    /*    OUT */
		    );

		int
		VmDirAuthEndEnumGroups(
		    Pointer pContext       /* IN OUT */
		    );

		int
		VmDirAuthFindUsersInGroup(
			String             pszGroupname,   /* IN     */
		    PointerByReference ppUsers,        /*    OUT */
		    IntByReference     pdwNumUsers     /* IN OUT */
		    );

		int
		VmDirAuthFindGroupsForUser(
			String             pszUsername,    /* IN     */
		    PointerByReference ppGroups,       /*    OUT */
		    IntByReference     pdwNumGroups    /* IN OUT */
		    );

		int
		VmDirAuthCredentials(
			String pszUsername,          /* IN     */
			String pszPassword           /* IN     */
		    );

		void
		VmDirAuthFreeUserInfoArray(
		    Pointer pUserInfoArray,   /* IN OUT */
		    int     dwNumUsers        /* IN     */
		    );

		void
		VmDirAuthFreeUserInfo(
		    Pointer pUserInfo         /* IN OUT */
		    );

		void
		VmDirAuthFreeGroupInfoArray(
		    Pointer pGroupInfoArray, /* IN OUT */
		    int     dwNumGroups      /* IN     */
		    );

		void
		VmDirAuthFreeGroupInfo(
		    Pointer pGroupInfo       /* IN OUT */
		    );
    }
}
