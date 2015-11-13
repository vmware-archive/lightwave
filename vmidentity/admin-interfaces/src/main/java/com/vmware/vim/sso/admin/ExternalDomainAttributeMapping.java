/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin;

import java.util.regex.Pattern;

import com.vmware.vim.sso.admin.ExternalDomainSchemaDetails;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Represents an attribute mapping.
 *
 * @see ExternalDomainSchemaDetails
 */
public final class ExternalDomainAttributeMapping {

    private String _attributeId;
    private String _attributeName;

    /**
     * Constructs ExternalDomainAttributeMapping object.
     * @param attributeId Specifies the attribute whose mapped name is configured.
     *                    Shall be one of AttributeIds
     *                    constants. @see AttributeIds
     * @param attributeName The name of the attribute used within the underlying store
     *                      to represent this attribute. For example for an attribute
     *                      with Id equal to AttributeIds.UserAttributeFirstName
     *                      this could be 'givenName'.
     *                      Attribute description (RFC 4512).
     */
    public ExternalDomainAttributeMapping( String attributeId, String attributeName )
    {
        validateId(attributeId, "attributeId");
        validateName(attributeName, "attributeName");
        this._attributeId = attributeId;
        this._attributeName = attributeName;
    }

    /**
     * Specifies the attribute whose mapped name is configured.
     */
    public String getAttributeId() { return this._attributeId; }

   /**
    * The name of the attribute used within the underlying store to represent
    * this attribute. For example for an attribute with Id equal to
    * AttributeIds.UserAttributeFirstName this could be 'givenName'.
    */
    public String getAttributeName() { return this._attributeName; }

    @Override
    public String toString() {
        StringBuilder objString = new StringBuilder(100);
        objString.append(super.toString());

        objString.append(" [AttributeId=");
        objString.append(this._attributeId);
        objString.append(", AttributeName=");
        objString.append(this._attributeName);
        objString.append("]");

        return objString.toString();
    }

    static void validateId(String name, String fieldName)
    {
        // for Id we will follow keystring from RFC 4512
        ValidateUtil.validateNotEmpty(name, fieldName);
        if(keystringPattern.matcher(name).matches() == false)
        {
            throw new IllegalArgumentException(String.format("[%s] value of [%s] is in unexpected format.", name, fieldName));
        }
    }

    static void validateName(String name, String fieldName)
    {
        // for the name we are following the Oid from RFC4512
        ValidateUtil.validateNotEmpty(name, fieldName);
        if( (keystringPattern.matcher(name).matches() == false)
            &&
            (numericoidPattern.matcher(name).matches() == false) )
        {
            throw new IllegalArgumentException(String.format("[%s] value of [%s] is in unexpected format.", name, fieldName));
        }
    }

    // RFC 4512: keystring(descr), numericoid
    private static Pattern keystringPattern = Pattern.compile("[a-zA-Z]([a-zA-Z0-9\\x2D])*");
    private static Pattern numericoidPattern = Pattern.compile("([0-9]|([1-9]([0-9])+))(\\x2E([0-9]|([1-9]([0-9])+)))+");

    /**
     * Symbolic names for the attributes.
     */
    public static final class AttributeIds {

        /**
         * User object's attribute containing the logon name used.
         * (for example, 'sAMAccountName' in AD or 'cn' in OpenLdap)
         */
        public final static String UserAttributeAccountName = "UserAttributeAccountName";

       /**
        * User object's attribute containing the family or last name for a user.
        * (for example, 'sn' in AD or OpenLdap)
        */
        public final static String UserAttributeLastName = "UserAttributeLastName";

       /**
        * User object's attribute containing the given name (first name) of the user.
        * (for example, 'givenName' in AD or OpenLdap)
        */
        public final static String UserAttributeFirstName = "UserAttributeFirstName";

        /**
         * User object's attribute containing the description for an object.
         * (for example, 'description' in AD or OpenLdap)
         */
        public final static String UserAttributeDescription = "UserAttributeDescription";

        /**
         * User object's attribute containing the display name for an object.
         * (for example, 'displayName' in AD or OpenLdap)
         */
        public final static String UserAttributeDisplayName = "UserAttributeDisplayName";

        /**
         * User object's attribute containing email address.
         * (for example, this could be 'mail' in AD OpenLdap)
         */
        public final static String UserAttributeEmail = "UserAttributeEmail";

        /**
         * User object's attribute containing unique value used to identify a user.
         * (for example, 'objectSid' in AD or 'entryUUID' in OpenLdap)
         */
        public final static String UserAttributeObjectId = "UserAttributeObjectId";

        /**
         * User object's attribute containing the UPN that is an
         * Internet-style login name for a user based on the Internet standard RFC 822.
         * (for example, 'userPrincipalName' in AD or OpenLdap)
         */
        public final static String UserAttributePrincipalName = "UserAttributePrincipalName";

        /**
         * User object's attribute containing flags that control the behavior of the user account.
         * (for example, 'userAccountControl' in AD or OpenLdap)
         */
        public final static String UserAttributeAcountControl = "UserAttributeAcountControl";

        /**
         * User object's attribute containing the distinguished name of the groups to which this object belongs.
         * (for example, 'memberOf' in AD or OpenLdap)
         */
        public final static String UserAttributeMemberOf = "UserAttributeMemberOf";

        /**
         * User object's attribute containing the relative identifier (RID) for the primary group of the user.
         * (for example, 'primaryGroupID' in AD)
         */
        public final static String UserAttributePrimaryGroupId = "UserAttributePrimaryGroupId";

        /**
         * User object's attribute containing the date and time (UTC) that this account was locked out.
         * (for example, 'lockoutTime' in AD)
         */
        public final static String UserAttributeLockoutTime = "UserAttributeLockoutTime";

        /**
         * User object's attribute containing Resultant password settings object applied to this object.
         * (for example, 'msDS-ResultantPSO' in AD)
         */
        public final static String UserAttributePasswordSettingsObject = "UserAttributePasswordSettingsObject";

        /**
         * User object's attribute containing the date and time that the password for this account was last changed.
         * (for example, 'pwdLastSet' in AD)
         */
        public final static String UserAttributePwdLastSet = "UserAttributePwdLastSet";

        /**
         * Group object's attribute containing the logon name used.
         * (for example, 'sAMAccountName' in AD or 'cn' in OpenLdap)
         */
        public final static String GroupAttributeAccountName = "GroupAttributeAccountName";

        /**
         * Group object's attribute containing the description for an object.
         * (for example, 'description' in AD or OpenLdap)
         */
        public final static String GroupAttributeDescription = "GroupAttributeDescription";

        /**
         * Group object's attribute containing unique value used to identify a group.
         * (for example, 'objectSid' in AD or 'entryUUID' in OpenLdap)
         */
        public final static String GroupAttributeObjectId = "GroupAttributeObjectId";

        /**
         * Group object's attribute containing the distinguished name of the groups to which this object belongs.
         * (for example, this could be 'memberof' in AD)
         */
        public final static String GroupAttributeMemberOf = "GroupAttributeMemberOf";

        /**
         * Group object's attribute containing the list of users that belong to the group.
         * (for example, 'member' in AD or 'uniqueMember' in OpenLdap)
         */
        public final static String GroupAttributeMembersList = "GroupAttributeMembersList";

        /**
         * Password settings object's attribute, containing the maximum age for user account passwords.
         * (for example, 'msDS-MaximumPasswordAge')
         */
        public final static String PasswordSettingsAttributeMaximumPwdAge = "PasswordSettingsAttributeMaximumPwdAge";

        /**
         * Domain object's attribute, containing the maximum amount of time a password is valid.
         * (for example, 'maxPwdAge' in AD)
         */
        public final static String DomainAttributeMaxPwdAge = "DomainAttributeMaxPwdAge";

        private AttributeIds() {}
    };
}
