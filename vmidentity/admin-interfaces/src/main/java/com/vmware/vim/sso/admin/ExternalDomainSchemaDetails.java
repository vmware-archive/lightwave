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
package com.vmware.vim.sso.admin;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.vmware.vim.sso.admin.ExternalDomain;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents schema mapping details for an external domain.
 *
 * Allows to specify a set of object and attribute mappings
 * for external domain. Only those objects/attributes that need
 * customizations should be specified.
 *
 * @see ExternalDomain
 */
public final class ExternalDomainSchemaDetails {

    private Map<String, ExternalDomainObjectMapping> _mappedObjects;

    private ExternalDomainSchemaDetails(HashMap<String, ExternalDomainObjectMapping> schemaMappings)
    {
        this._mappedObjects = new HashMap<String, ExternalDomainObjectMapping>();
        this._mappedObjects.putAll(schemaMappings);
        this._mappedObjects = Collections.unmodifiableMap(this._mappedObjects);
    }

    /**
     * Object mappings.
     *
     * @see ExternalDomainObjectMapping
     */
    public Collection<ExternalDomainObjectMapping> getMappedObjects()
    {
        return this._mappedObjects.values();
    }

    /**
     * Get an object mapping for the object with specified id.
     * @param objectId Id of an object to lookup an object mapping for. Not empty.
     * @return ExternalDomainObjectMapping or null if no object mapping exists.
     */
    public ExternalDomainObjectMapping getObjectMapping(String objectId)
    {
        ValidateUtil.validateNotEmpty(objectId, objectId);

        return this._mappedObjects.get(objectId);
    }

    /**
     * A helper class to construct instances of ExternalDomainObjectMapping.
     */
    public static final class Builder
    {
        private HashMap<String, ExternalDomainObjectMapping> _objectMappings;

        /**
         * Constructor.
         */
        public Builder()
        {
            this._objectMappings = new HashMap<String, ExternalDomainObjectMapping>();
        }

        /**
         * Add an object mapping.
         * @param objectMapping ExternalDomainObjectMapping, not null.
         */
        public void addObjectMapping( ExternalDomainObjectMapping objectMapping )
        {
            ValidateUtil.validateNotNull(objectMapping, "objectMapping");

            this._objectMappings.put(objectMapping.getObjectId(), objectMapping);
        }

        /**
         * Create an instance of ExternalDomainSchemaDetails.
         * @return ExternalDomainSchemaDetails
         */
        public ExternalDomainSchemaDetails buildSchemaDetails()
        {
            return new ExternalDomainSchemaDetails(this._objectMappings);
        }
    }

    @Override
    public String toString() {
        StringBuilder objString = new StringBuilder(200);
        objString.append(super.toString());

        objString.append(" MappedObjects={ ");
        for(ExternalDomainObjectMapping mapping : this._mappedObjects.values())
        {
            objString.append( mapping );
            objString.append( ", " );
        }
        objString.append(" }");

        return objString.toString();
    }
}
