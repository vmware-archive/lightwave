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

public class OsSamConstants
{
    public static final int UF_ACCOUNT_DISABLE  =     0x0002;
    public static final int UF_LOCKOUT          =     0x0010;
    public static final int UF_PASSWORD_EXPIRED = 0x00080000;

    static final int MAX_PREFERRED_LENGTH = -1;
    static final int NERR_Success = 0;

    static final int NERR_GroupNotFound = 2220;
    static final int NERR_UserNotFound = 2221;
    static final int NERR_InternalError = 2140;

    static final int LG_INCLUDE_INDIRECT = 0x0001;
    static final int FILTER_NORMAL_ACCOUNT = 0x0002;

    static final int SidTypeUser = 1;
    static final int SidTypeGroup = 2;
    static final int SidTypeDomain = 3;
    static final int SidTypeAlias = 4;
    static final int SidTypeWellKnownGroup = 5;
    static final int SidTypeDeletedAccount = 6;
    static final int SidTypeInvalid = 7;
    static final int SidTypeUnknown = 8;
    static final int SidTypeComputer = 9;
    static final int SidTypeLabel = 10;

    static final int ERROR_NO_SUCH_ALIAS = 1376; // local group does not exist
	static final int ERROR_NO_SUCH_GROUP = 1319; // local group does not exist
    static final int ERROR_NO_SUCH_USER  = 1317; // local user does not exist
}
