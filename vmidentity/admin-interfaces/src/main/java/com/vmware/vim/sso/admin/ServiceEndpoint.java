/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin;

import java.io.Serializable;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Representation of a service endpoint.
 */
public class ServiceEndpoint implements Serializable
{
    private static final long serialVersionUID = -6445916692718863817L;

    private final String name;
    private final String endpoint;
    private final String binding;

    /**
     * Creates a ServiceEndpoint with endpoint and binding. The name
     * will default to {@code endpoint}.
     *
     * @param endpoint
     *          the endpoint of the service.
     * @param binding
     *          the binding for the service.
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
     *          the endpoint of the service.
     * @param binding
     *          the binding for the service.
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
     * Fetch the binding for the service.
     *
     * @return the binding of the service.
     */
    public String getBinding()
    {
        return binding;
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
