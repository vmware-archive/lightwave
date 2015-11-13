/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods to indicate that there
 * are extra certificates outside of the certificate chain.
 */
public class ExtraneousCertsInCertChainException
    extends ServiceException
{
    private static final long serialVersionUID = 8381881870767058305L;

    private final String name;

    /**
     * Creates a new exception to indicate that there are extra certificates
     * outside of the certificate chain.
     *
     * @param name
     *            Name of the problematic certificate chain.
     */
    public ExtraneousCertsInCertChainException(String name) {
        super(getDefaultMessage(name));
        assert name != null && !name.trim().isEmpty();
        this.name = name;
    }

    public String getName()
    {
        return name;
    }

    private static String getDefaultMessage(String name)
    {
        return String.format("Certpath for [%s] should contain only one chain.", name);
    }
}
