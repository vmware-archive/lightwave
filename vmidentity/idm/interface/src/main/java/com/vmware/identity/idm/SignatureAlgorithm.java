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

package com.vmware.identity.idm;

import java.io.Serializable;

/**
 * User: snambakam
 * Date: 12/25/11
 * Time: 1:06 AM
 */
public class SignatureAlgorithm implements Serializable
{
	private static final long serialVersionUID = -2285461699463851391L;
	
	private int maximumKeySize;
    private int minimumKeySize;
    private int priority;

    public int getMaximumKeySize()
    {
        return maximumKeySize;
    }

    public void setMaximumKeySize(int maximumKeySize)
    {
        this.maximumKeySize = maximumKeySize;
    }

    public int getMinimumKeySize()
    {
        return minimumKeySize;
    }

    public void setMinimumKeySize(int minimumKeySize)
    {
        this.minimumKeySize = minimumKeySize;
    }

    public int getPriority()
    {
        return priority;
    }

    public void setPriority(int priority)
    {
        this.priority = priority;
    }

    public SignatureAlgorithm()
    {
        maximumKeySize = 0;
        minimumKeySize = 0;
        priority = 0;
    }
}
