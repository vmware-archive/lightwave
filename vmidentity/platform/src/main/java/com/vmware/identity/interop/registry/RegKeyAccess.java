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

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/13/11
 * Time: 5:52 PM
 * To change this template use File | Settings | File Templates.
 */
public class RegKeyAccess
{
    public static final long DELETE                   = 0x00010000;
    public static final long READ_CONTROL             = 0x00020000;
    public static final long SYNCHRONIZE              = 0x00100000;

    public static final long STANDARD_RIGHTS_REQUIRED = 0x000F0000;
    public static final long STANDARD_RIGHTS_READ     = READ_CONTROL;
    public static final long STANDARD_RIGHTS_WRITE    = READ_CONTROL;

    public static final long KEY_QUERY_VALUE        = 0x0001;
    public static final long KEY_SET_VALUE          = 0x0002;
    public static final long KEY_CREATE_SUB_KEY     = 0x0004;
    public static final long KEY_ENUMERATE_SUB_KEYS = 0x0008;
    public static final long KEY_NOTIFY             = 0x0010;
    public static final long KEY_CREATE_LINK        = 0x0020;

    public static final long KEY_ALL_ACCESS =
                                (
                                     (~SYNCHRONIZE) &
                                     (
                                        STANDARD_RIGHTS_REQUIRED |
                                        KEY_QUERY_VALUE |
                                        KEY_SET_VALUE |
                                        KEY_CREATE_SUB_KEY |
                                        KEY_ENUMERATE_SUB_KEYS |
                                        KEY_NOTIFY |
                                        KEY_CREATE_LINK
                                     )
                                );

    public static final long KEY_READ =
                                (
                                     (~SYNCHRONIZE) &
                                     (
                                        STANDARD_RIGHTS_READ |
                                        KEY_QUERY_VALUE |
                                        KEY_ENUMERATE_SUB_KEYS |
                                        KEY_NOTIFY
                                     )
                                );


    public static final long KEY_WRITE =
                                (
                                    (~SYNCHRONIZE) &
                                    (
                                        STANDARD_RIGHTS_WRITE |
                                        KEY_SET_VALUE |
                                        KEY_CREATE_SUB_KEY
                                    )
                                );

    public static final long KEY_EXECUTE =
                                (
                                    (~SYNCHRONIZE) &
                                    (KEY_READ)
                                );
}
