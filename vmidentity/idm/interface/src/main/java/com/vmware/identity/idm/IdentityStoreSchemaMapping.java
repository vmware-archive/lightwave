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
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.ObjectUtils;

/**
 * Represents schema mapping for an identity store.
 *
 * Allows to specify a set of object and attribute mappings
 * for identity store. Only those objects/attributes that need
 * customizations should be specified.
 *
 * @see IdentityStoreDataEx
 */
public class IdentityStoreSchemaMapping implements Serializable
{
    private static final long serialVersionUID = -5995905025951190843L;

    private Map<String, IdentityStoreObjectMapping> _storeObjects;

    private IdentityStoreSchemaMapping(Map<String, IdentityStoreObjectMapping> storeObjects)
    {
        this._storeObjects = new HashMap<String, IdentityStoreObjectMapping>();
        this._storeObjects.putAll(storeObjects);
        this._storeObjects = Collections.unmodifiableMap(this._storeObjects);
    }

    /**
     * Object mappings.
     *
     * @see IdentityStoreObjectMapping
     */
    public Collection<IdentityStoreObjectMapping> getObjectMappings()
    {
        return this._storeObjects.values();
    }

    /**
     * Get an object mapping for the object with specified id.
     * @param objectId Id of an object to lookup an object mapping for. Not empty.
     * @return IdentityStoreObjectMapping or null if no object mapping exists.
     */
    public IdentityStoreObjectMapping getObjectMapping( String objectId )
    {
        ValidateUtil.validateNotEmpty(objectId, "objectId");
        return this._storeObjects.get(objectId);
    }

    @Override
    public int hashCode()
    {
        return ObjectUtils.hashCode(_storeObjects);
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

        IdentityStoreSchemaMapping other = (IdentityStoreSchemaMapping) obj;
        return ObjectUtils.equals(_storeObjects, other._storeObjects);
    }

    /**
     * A helper class to construct instances of ExternalDomainObjectMapping.
     */
    public static final class Builder
    {
        private Map<String, IdentityStoreObjectMapping> _storeObjects;

        /**
         * Constructor.
         */
        public Builder()
        {
            this._storeObjects = new HashMap<String, IdentityStoreObjectMapping>();
        }

        /**
         * Add an object mapping.
         * @param objectMapping IdentityStoreObjectMapping, not null.
         */
        public void addObjectMappings(IdentityStoreObjectMapping objectMapping)
        {
            ValidateUtil.validateNotNull(objectMapping, "objectMapping");
            this._storeObjects.put(objectMapping.getObjectId(), objectMapping);
        }

        /**
         * Create an instance of IdentoityStoreSchemaMapping.
         * @return IdentoityStoreSchemaMapping
         */
        public IdentityStoreSchemaMapping buildSchemaMapping()
        {
            return new IdentityStoreSchemaMapping( this._storeObjects );
        }
    }
}
