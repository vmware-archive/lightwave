/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * A mapping of a logical object (such as user or group)
 * to the actual representation in External Domain.
 *
 * @see ExternalDomain
 */
public final class ExternalDomainObjectMapping {

    private String _objectId;
    private String _objectClass;
    private Map<String, ExternalDomainAttributeMapping> _attributeMappings;

    private ExternalDomainObjectMapping(
        String objectId, String objectClass,
        Map<String, ExternalDomainAttributeMapping> attributeMappings
    )
    {
        this._objectId = objectId;
        this._objectClass = objectClass;
        this._attributeMappings = new HashMap<String, ExternalDomainAttributeMapping>();
        this._attributeMappings.putAll(attributeMappings);
        this._attributeMappings = Collections.unmodifiableMap(this._attributeMappings);
    }

    /**
    * Identifies the object the mapping is for.
    * Shall be one of the ObjectIds constants.
    * @see ObjectIds
    */
    public String getObjectId() { return this._objectId; }

   /**
    * The object class that is used within the underlying store to represent
    * this object. For example for the object with id equal to ObjectIds.ObjectIdUser
    * this could be a 'user' in AD or an 'inetOrgPerson' in OpenLdap.
    */
    public String getObjectClass() { return this._objectClass; }

   /**
    * Attribute mappings for the object.
    */
    public Collection<ExternalDomainAttributeMapping> getMappedAttributes()
    {
        return this._attributeMappings.values();
    }

    /**
     * Get an attribute mapping for the attribute with the given Id.
     *
     * @param attributeId Identifies the object the mapping is for.
     *                    Not empty. @see AttributeIds
     * @return An attribute mapping for the attribute with the given Id.
     *         Null if there is no mapping for the specified attribute.
     */
    public ExternalDomainAttributeMapping getAttributeMapping(String attributeId)
    {
        ValidateUtil.validateNotEmpty(attributeId, attributeId);

        return this._attributeMappings.get(attributeId);
    }

    @Override
    public String toString() {
        StringBuilder objString = new StringBuilder(200);
        objString.append(super.toString());

        objString.append(" [ObjectId=");
        objString.append(this._objectId);
        objString.append(", ObjectClass=");
        objString.append((this._objectClass!= null)?this._objectClass : "(null)");
        objString.append(" MappedAttributes={ ");
        for(ExternalDomainAttributeMapping mapping : this._attributeMappings.values())
        {
            objString.append( mapping );
            objString.append( ", " );
        }
        objString.append(" }");
        objString.append("]");

        return objString.toString();
    }

    /**
     * A helper class, for constructing an object mapping.
     */
    public static final class Builder
    {
        Map<String, ExternalDomainAttributeMapping> _attributeMappings;
        private String _idOfObject;
        private String _objectClassOfObject;

        /**
         * Constructor.
         * @param objectId Id of an object whose object mapping to construct.
         *                 Not empty. Shall be one of ObjectIds.
         */
        public Builder(String objectId)
        {
            ExternalDomainAttributeMapping.validateId(objectId, "objectId");
            this._attributeMappings = new HashMap<String, ExternalDomainAttributeMapping>();
            this._idOfObject = objectId;
            this._objectClassOfObject = null;
        }

        /**
         * Set the object class that is used within the underlying store to represent
         * this object. For example for the object with id equal to ObjectIds.ObjectIdUser
         * this could be a 'user' in AD or an 'inetOrgPerson' in OpenLdap.
         * @param objectClass The object class. objectClass attribute (RFC 4512).
         *                    Null/empty means non-customized default
         *                    object class to be used.
         */
        public void setObjectClass(String objectClass)
        {
            if ( ( objectClass != null ) && (objectClass.isEmpty() == false) )
            {
                ExternalDomainAttributeMapping.validateName(objectClass, "objectClass");
            }
            this._objectClassOfObject = objectClass;
        }

        /**
         * add an attribute mapping for this object.
         * @param attributeMapping Not null. @see ExternalDomainAttributeMapping
         */
        public void addAttributeMapping( ExternalDomainAttributeMapping attributeMapping )
        {
            ValidateUtil.validateNotNull(attributeMapping, "attributeMapping");

            this._attributeMappings.put(attributeMapping.getAttributeId(), attributeMapping);
        }

        /**
         * Create an instance of ExternalDomainObjectMapping.
         * @return ExternalDomainObjectMapping
         */
        public ExternalDomainObjectMapping buildObjectMapping()
        {
            return new ExternalDomainObjectMapping( this._idOfObject, this._objectClassOfObject, this._attributeMappings );
        }
    }

    /**
     * Symbolic names for objects in the external domain.
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
