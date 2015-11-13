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
import java.util.Collection;

/**
 * User: snambakam
 * Date: 12/25/11
 * Time: 1:16 AM
 */
public class AttributeConsumerService implements Serializable
{
	private static final long serialVersionUID = 7168956299468223935L;
	
	private String      name;
	private int         index;
    private Collection<Attribute> attributes;

	/**
	 * @param name the name to set
	 */
	public AttributeConsumerService(String name) 
	{
        ValidateUtil.validateNotEmpty(name, "name");
        this.name = name;
	}

	public int getIndex()
    {
        return index;
    }

    public void setIndex(int index)
    {
        this.index = index;
    }

    public Collection<Attribute> getAttributes()
    {
        return attributes;
    }

    public void setAttributes(Collection<Attribute> attributes)
    {
        this.attributes = attributes;
    }

	/**
	 * @return the name
	 */
	public String getName() 
	{
		return name;
	}
}
