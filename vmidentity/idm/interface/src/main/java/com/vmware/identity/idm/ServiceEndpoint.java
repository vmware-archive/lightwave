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

public class ServiceEndpoint implements Serializable
{
    private static final long serialVersionUID = 243822623432354432L;

    private final String name;
    private String endpoint;
    private String responseEndpoint;
    private String binding;

    /**
     * Creates a ServiceEndpoint with name only.
     *
     * @param name
     *          the name of the service. Cannot be null or empty.
     */
    public ServiceEndpoint(String name)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        this.name = name;
    }

    /**
     * Creates a ServiceEndpoint with endpoint and binding. The name
     * will default to {@code endpoint}.
     *
     * @param endpoint
     *          the endpoint of the service. Cannot be null or empty.
     * @param binding
     *          the binding for the service. Cannot be null or empty.
     */
    public ServiceEndpoint(String endpoint, String binding)
    {
        this(endpoint, endpoint, binding);
    }

    /**
     * Creates a ServiceEndpoint with name, endpoint, and binding.
     *
     * @param name
     *          the name of the service. If null or empty, defaults to
     *          the {@code endpoint}.
     * @param endpoint
     *          the endpoint of the service. Cannot be null or empty.
     * @param binding
     *          the binding for the service. Cannot be null or empty.
     */
    public ServiceEndpoint(String name, String endpoint, String binding)
    {
        ValidateUtil.validateNotEmpty(endpoint, "endpoint");
        ValidateUtil.validateNotEmpty(binding, "binding");

        if (name == null || name.isEmpty()) {
            name = endpoint;
        }

        this.name = name;
        this.endpoint = endpoint;
        this.binding = binding;
    }

    /**
     * Creates a ServiceEndpoint with name, endpoint, and binding.
     *
     * @param name
     *          the name of the service. If null or empty, defaults to
     *          the {@code endpoint}.
     * @param endpoint
     *          the endpoint of the service. Cannot be null or empty.
     * @param responseEndpoint
     *          the response endpoint of the service. Optional.
     * @param binding
     *          the binding for the service. Cannot be null or empty.
     */
    public ServiceEndpoint(String name, String endpoint, String responseEndpoint, String binding)
    {
        ValidateUtil.validateNotEmpty(endpoint, "endpoint");
        ValidateUtil.validateNotEmpty(binding, "binding");

        if (name == null || name.isEmpty()) {
            name = endpoint;
        }

        this.name = name;
        this.endpoint = endpoint;
        this.responseEndpoint = responseEndpoint;
        this.binding = binding;
    }

    /**
     * Fetch the name of the service.
     *
     * @return the name of the service.
     */
    public String getName()
    {
        return name;
    }

    /**
     * Fetch the endpoint for the service.
     *
     * @return the endpoint of the service.
     */
    public String getEndpoint()
    {
        return endpoint;
    }

    /**
     * Set the endpoint for the service.
     *
     * @param endpoint
     *          the new endpoint of the service.
     */
    public void setEndpoint(String endpoint)
    {
        this.endpoint = endpoint;
    }

    /**
     * Fetch the response endpoint for the service.
     *
     * @return the response endpoint of the service.
     */
    public String getResponseEndpoint()
    {
        return this.responseEndpoint;
    }

    /**
     * Set the response endpoint for the service.
     *
     * @param responseEndpoint
     *          the new endpoint of the service.
     */
    public void setResponseEndpoint(String responseEndpoint)
    {
        this.responseEndpoint = responseEndpoint;
    }

    /**
     * Fetch the binding for the service.
     *
     * @return the binding of the service.
     */
    public String getBinding()
    {
        return binding;
    }

    /**
     * Set the binding for the service.
     *
     * @param binding
     *          the new binding of the service.
     */
    public void setBinding(String binding)
    {
        this.binding = binding;
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result = prime * result + binding.hashCode();
        result = prime * result + endpoint.hashCode();
        result = prime * result + name.hashCode();
        return result;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        ServiceEndpoint other = (ServiceEndpoint) obj;
        if (!binding.equals(other.binding))
            return false;
        if (!endpoint.equals(other.endpoint))
            return false;
        if (!name.equals(other.name))
            return false;
        return true;
    }

    @Override
    public String toString()
    {
        return "ServiceEndpoint [name=" + name + ", endpoint=" + endpoint
                + ", binding=" + binding + "]";
    }
}
