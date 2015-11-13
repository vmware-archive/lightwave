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
 * Time: 1:12 AM
 */
public class AssertionConsumerService implements Serializable
{
	private static final long serialVersionUID = 1751657647344344729L;

	private String  name;
	private String  endpoint;
	private String  binding;
    public     int  index;

	/**
	 * @param name the name to set.
	 */
	public AssertionConsumerService(String name)
	{
	    this(name, "DummyBinding", "DummyEndpoint");
	}

	/**
	 * AssertionConsumerService ctr taking all required parameters
	 *
     * @param name the name to set.
     * @param binding  {@code non-null}     required
     * @param endpint   {@code non-null}    required
     *
     */
    public AssertionConsumerService(String name, String binding, String endpoint)
    {
        ValidateUtil.validateNotEmpty(binding, "binding");
        ValidateUtil.validateNotEmpty(endpoint, "endpoint");
        this.name = name;
        this.endpoint = endpoint;
        this.binding = binding;
    }

	public String getBinding()
    {
        return binding;
    }

    /**
     * @param binding the binding to set. required
     */
    public void setBinding(String binding)
    {
        ValidateUtil.validateNotEmpty(binding, "binding");
        this.binding = binding;
    }

    public int getIndex()
    {
        return index;
    }

    public void setIndex(int index)
    {
        this.index = index;
    }

	public String getEndpoint()
	{
		return endpoint;
	}

    /**
     * @param endpointy the url location to set. required
     */
	public void setEndpoint(String endpoint)
	{
        ValidateUtil.validateNotEmpty(endpoint, "endpoint");
		this.endpoint = endpoint;
	}

	/**
	 * @return the name
	 */
	public String getName()
	{
		return name;
	}
}
