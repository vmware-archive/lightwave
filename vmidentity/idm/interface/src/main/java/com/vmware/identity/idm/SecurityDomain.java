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

import org.apache.commons.lang.ObjectUtils;

/**
 * This class represents a security domain using its fully qualified domain name
 * and corresponding alias.
 *
 */
public class SecurityDomain implements Serializable {

    private static final long serialVersionUID = 442882988232214807L;

    private final String _name;
    private final String _alias;

    public SecurityDomain(String name, String alias)
    {
        ValidateUtil.validateNotEmpty(name, "name");
        ValidateUtil.validateNotEmpty(alias, "alias");

        _name = name;
        _alias = alias;
    }

    public String getName()
    {
        return _name;
    }

    public String getAlias()
    {
        return _alias;
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result = prime * result + ObjectUtils.hashCode(_name);
        result = prime * result + ObjectUtils.hashCode(_alias);
        return result;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (this == obj)
        {
            return true;
        }
        if (obj == null || this.getClass() != obj.getClass())
        {
            return false;
        }

        SecurityDomain other = (SecurityDomain) obj;

        return ObjectUtils.equals(_name, other._name)
            && ObjectUtils.equals(_alias, other._alias);
    }
}
