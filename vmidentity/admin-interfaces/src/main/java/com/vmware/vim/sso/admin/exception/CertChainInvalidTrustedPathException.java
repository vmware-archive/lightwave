/*
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
 */

package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods to indicate that the certificate
 * chain is invalid.
 */
public class CertChainInvalidTrustedPathException
    extends ServiceException
{
    private static final long serialVersionUID = -7325876422889330524L;

    private final String name;

    /**
     * Creates a new exception to indicate that the certificate chain
     * is invalid.
     *
     * @param name
     *          Name of the problematic certificate chain.
     */
    public CertChainInvalidTrustedPathException(String name) {
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
        return String.format("Certpath for [%s] doesn't exist.", name);
    }
}
