/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
 */
package com.vmware.identity.idm.server.provider;

import java.util.Set;

import org.apache.commons.lang.ObjectUtils;

import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.SecurityDomain;

public class PrincipalGroupLookupInfo
{
    private String _objectId;
    private Set<Group> _groups;
    public PrincipalGroupLookupInfo( Set<Group> groups, String objectId )
    {
        // both could be null/empty
        this._groups = groups;
        this._objectId = objectId;
    }

    public Set<Group> getGroups()
    {
        return this._groups;
    }

    public String getPrincipalObjectId()
    {
        return this._objectId;
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result = prime * result + ObjectUtils.hashCode(this._groups);
        result = prime * result + ObjectUtils.hashCode(this._objectId);
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

        PrincipalGroupLookupInfo other = (PrincipalGroupLookupInfo) obj;

        return ObjectUtils.equals(this._objectId, other._objectId)
            && ObjectUtils.equals(this._groups, other._groups);
    }
}
