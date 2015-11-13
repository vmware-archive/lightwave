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

package com.vmware.identity.interop;

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/13/11
 * Time: 4:56 PM
 * To change this template use File | Settings | File Templates.
 */
public class NoMoreDataException extends NativeCallException
{
    /**
	 * 
	 */
	private static final long serialVersionUID = 2541551893154432215L;
	public static final int ERROR_NO_MORE_ITEMS = 259;

    public
    NoMoreDataException()
    {
        super(ERROR_NO_MORE_ITEMS);
    }
}
