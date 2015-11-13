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
 * A mapping of a logical object (such as user or group)
 * to the actual representation in underlying identity store.
 *
 * @see IdentityStoreSchemaMapping
 */
public class IdentityStoreObjectMapping implements Serializable {

    private static final long serialVersionUID = -8343330753152883765L;

    private String _objectId;
    private String _objectClass;

    private Map<String, IdentityStoreAttributeMapping> _storeAttributes;

    private IdentityStoreObjectMapping(
        String objectId, String objectClass,
        Map<String, IdentityStoreAttributeMapping> attributesMapping )
    {
        this._objectId = objectId;
        this._objectClass = objectClass;
        this._storeAttributes = new HashMap<String, IdentityStoreAttributeMapping>();
        this._storeAttributes.putAll(attributesMapping);
        this._storeAttributes = Collections.unmodifiableMap(this._storeAttributes);
    }

    /**
    * Identifies the object the mapping is for. @see ObjectIds
    */
    public String getObjectId() { return this._objectId; }

   /**
    * The object class that is used within the underlying store to represent
    * this object. For example for the object with id equal to ObjectIds.ObjectIdUser
    * this could be a 'user' in AD or an 'inetOrgPerson' in OpenLdap.
    */
    public String getObjectClass() { return this._objectClass; }

    /**
     * Get an attribute mapping for the attribute with the given Id.
     * @param attributeId Id of an attribute whose mapping to lookup.
     *                    Not empty. @see AttributeIds
     * @return An attribute mapping for the attribute with the given Id.
     * Null if there is no mapping for the specified attribute.
     */
    public IdentityStoreAttributeMapping getAttributeMapping( String attributeId )
    {
        ValidateUtil.validateNotEmpty(attributeId, attributeId);
        return this._storeAttributes.get(attributeId);
    }

   /**
    * Attribute mappings for the object.
    */
    public Collection<IdentityStoreAttributeMapping> getAttributeMappings()
    {
        return this._storeAttributes.values();
    }

    @Override
    public int hashCode()
    {
        final int prime = 31;
        int result = 1;
        result = prime * result + ObjectUtils.hashCode(_objectId);
        result = prime * result + ObjectUtils.hashCode(_objectClass);
        result = prime * result + ObjectUtils.hashCode(_storeAttributes);
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

        IdentityStoreObjectMapping other = (IdentityStoreObjectMapping) obj;
        return ObjectUtils.equals(_objectId, other._objectId)
            && ObjectUtils.equals(_objectClass, other._objectClass)
            && ObjectUtils.equals(_storeAttributes, other._storeAttributes);
    }

    /**
     * A helper class, for constructing an object mapping.
     */
    public static final class Builder
    {
        private String _idOfObject;
        private String _objectClassOfObject;
        private Map<String, IdentityStoreAttributeMapping> _storeAttributes;

        /**
         * Constructor.
         * @param objectId Id of an object whose object mapping to construct. shall be one of ObjectIds.
         */
        public Builder(String idOfObject)
        {
            IdentityStoreAttributeMapping.validateId(idOfObject, "idOfObject");
            this._idOfObject = idOfObject;
            this._objectClassOfObject = null;
            this._storeAttributes = new HashMap<String, IdentityStoreAttributeMapping>();
        }

        /**
          * Set the object class that is used within the underlying store to represent
          * this object. For example for the object with id equal to ObjectIds.ObjectIdUser
          * this could be a 'user' in AD or an 'inetOrgPerson' in OpenLdap.
          * @param objectClass The object class. objectClass attribute (RFC 4512).
          *                    Null/empty is treated as un-customized/default object class.
          */
        public void setObjectClass(String objectClassOfObject)
        {
            if( ( objectClassOfObject != null ) && (objectClassOfObject.isEmpty() == false) )
            {
                IdentityStoreAttributeMapping.validateName( objectClassOfObject, "objectClassOfObject" );
            }
            this._objectClassOfObject = objectClassOfObject;
        }

        /**
         * add an attribute mapping for this object. Not null.
         * @param attributeMapping
         */
        public void addAttributeMapping( IdentityStoreAttributeMapping attributeMapping )
        {
            ValidateUtil.validateNotNull(attributeMapping, "attributeMapping");
            this._storeAttributes.put(attributeMapping.getAttributeId(), attributeMapping);
        }

        /**
         * Create an instance of IdentityStoreObjectMapping.
         * @return IdentityStoreObjectMapping
         */
        public IdentityStoreObjectMapping buildObjectMapping()
        {
            return new IdentityStoreObjectMapping( this._idOfObject, this._objectClassOfObject, this._storeAttributes );
        }
    }

    /**
     * Ids for objects in the identity store.
     */
    public final static class ObjectIds {

        /**
         * Object representing a user.
         * (for example, object class of this object could be 'user' in AD or 'inetOrgPerson' in OpenLdap).
         */
        public final static String ObjectIdUser = "ObjectIdUser";

        /**
         * Object representing a group.
         * (for example, object class of this object could be 'group' in AD or 'groupOfUniqueNames' in OpenLdap).
         */
        public final static String ObjectIdGroup = "ObjectIdGroup";

        /**
         * Object representing Password settings object for accounts.
         * (for example, object class of this object could be 'msDS-PasswordSettings' in AD).
         */
        public final static String ObjectIdPasswordSettings = "ObjectIdPasswordSettings";

        /**
         * Object representing a domain.
         * (for example, object class of this object could be 'domain' in AD).
         */
        public final static String ObjectIdDomain = "ObjectIdDomain";

        private ObjectIds() {}
     };
}
