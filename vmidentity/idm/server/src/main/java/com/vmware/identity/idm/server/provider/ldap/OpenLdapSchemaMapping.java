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
package com.vmware.identity.idm.server.provider.ldap;

import java.util.HashMap;
import java.util.Map;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.provider.BaseLdapSchemaMapping;

public class OpenLdapSchemaMapping extends BaseLdapSchemaMapping
{
    public OpenLdapSchemaMapping(IdentityStoreSchemaMapping schemaMapping)
    {
        super(schemaMapping, defaultAttributesMap);
    }

    @Override
    protected String buildUserQueryByAccountNameOrUpn()
    {
        // there is no userPrincipalName attribute in Open Ldap
        return null;
    }

    @Override
    protected String buildUserQueryByUpn()
    {
        // there is no userPrincipalName in open ldap
        return null;
    }

    @Override
    protected String buildUserQueryByAccountName()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            userObjectClass,
            userAccountNameAttribute,
            "%s"
        );
    }

    @Override
    protected String buildUserQueryByObjectUniqueId()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userObjectIdAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userObjectIdAttribute, "userObjectIdAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            userObjectClass,
            userObjectIdAttribute,
            "%s"
        );
    }

    @Override
    protected String buildUserQueryByCriteria()
    {
        //    "(&(objectClass=inetOrgPerson)(|(cn=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(displayName=*%1$s*)(description=*%1$s*)))";
        final String USER_PRINC_QUERY_BY_CRITERIA =
            "(&(objectClass=%2$s)(|(%3$s=*%1$s*)(%4$s=*%1$s*)(%5$s=*%1$s*)(%6$s=*%1$s*)(%7$s=*%1$s*)))";

        final String userObjectClass = this.getUserObjectClassValue();
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");
        final String userLastNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
        ValidateUtil.validateNotNull(userLastNameAttribute, "userLastNameAttribute");
        final String userFirstNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
        ValidateUtil.validateNotNull(userFirstNameAttribute, "userFirstNameAttribute");
        final String userdisplayNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName);
        ValidateUtil.validateNotNull(userdisplayNameAttribute, "userdisplayNameAttribute");
        final String userDescriptionAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription);
        ValidateUtil.validateNotNull(userDescriptionAttribute, "userDescriptionAttribute");

        return String.format(
            USER_PRINC_QUERY_BY_CRITERIA,
            "%1$s",
            userObjectClass,
            userAccountNameAttribute,
            userLastNameAttribute,
            userFirstNameAttribute,
            userdisplayNameAttribute,
            userDescriptionAttribute
        );
    }

    @Override
    protected String buildUserQueryByCriteriaForName()
    {
       //     "(&(objectClass=inetOrgPerson)(|(uid=%1$s*)(mail=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=$1$s*)(displayName=%1$s*)))";
       final String USER_PRINC_QUERY_BY_CRITERIA =
           "(&(objectClass=%2$s)(|(%3$s=%1$s*)(%4$s=%1$s*)(%5$s=%1$s*)(%6$s=%1$s*)(%7$s=%1$s*)(%8$s=%1$s*)))";


       final String userObjectClass = this.getUserObjectClassValue();
       ValidateUtil.validateNotNull(userObjectClass, "userObjectClass"); // inetOrgPerson
       final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
       ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute"); // uid
       final String userEmailAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail);
       ValidateUtil.validateNotNull(userEmailAttribute, "userEmailAttribute"); // mail
       final String userLastNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
       ValidateUtil.validateNotNull(userLastNameAttribute, "userLastNameAttribute"); // sn
       final String userFirstNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
       ValidateUtil.validateNotNull(userFirstNameAttribute, "userFirstNameAttribute"); // givenName
       final String userCommonNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName);
       ValidateUtil.validateNotNull(userCommonNameAttribute, "userCommonNameAttribute"); // cn
       final String userDisplayNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName);
       ValidateUtil.validateNotNull(userDisplayNameAttribute, "userDisplayNameAttribute"); // displayName

       return String.format(
             USER_PRINC_QUERY_BY_CRITERIA,
             "%1$s",
             userObjectClass,
             userAccountNameAttribute,
             userEmailAttribute,
             userLastNameAttribute,
             userFirstNameAttribute,
             userCommonNameAttribute,
             userDisplayNameAttribute
       );
    }

    @Override
    protected String buildAllUsersQuery()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");

        return String.format(
            "(objectClass=%s)",
            userObjectClass
        );
    }

    @Override
    protected String buildAllDisabledUsersQuery()
    {
        return this.getAllUsersQuery();
    }

    @Override
    protected String buildGroupQueryByCriteria()
    {
        // (&(objectClass=groupOfUniqueNames)(|(cn=*%1$s*)(description=*%1$s*)(entryUUID=*%1$s*)))
        final String GROUP_PRINC_QUERY_BY_CRITERIA =
            "(&(objectClass=%2$s)(|(%3$s=*%1$s*)(%4$s=*%1$s*)(%5$s=*%1$s*)))";

        final String groupObjectClass = this.getGroupObjectClassValue();
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");
        final String groupDescriptionAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
        ValidateUtil.validateNotNull(groupDescriptionAttribute, "groupDescriptionAttribute");
        final String groupEntryUuidAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
        ValidateUtil.validateNotNull(groupEntryUuidAttribute, "groupEntryUuidAttribute");

        return String.format(
            GROUP_PRINC_QUERY_BY_CRITERIA,
            "%1$s",
            groupObjectClass,
            groupAccountNameAttribute,
            groupDescriptionAttribute,
            groupEntryUuidAttribute
        );
    }

    @Override
    protected String buildGroupQueryByCriteriaForName()
    {
        //    "(&(objectClass=groupOfUniqueNames)(cn=%1$s*));"
        final String GROUP_PRINC_QUERY_BY_CRITERIA_FOR_NAME =
            "(&(objectClass=%2$s)(%3$s=%1$s*))";

        final String groupObjectClass = this.getGroupObjectClassValue();
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass"); // groupOfUniqueNames
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute"); // cn

        return String.format(
            GROUP_PRINC_QUERY_BY_CRITERIA_FOR_NAME,
            "%1$s",
            groupObjectClass,
            groupAccountNameAttribute
        );
    }

    @Override
    protected String buildAllGroupsQuery()
    {

        final String groupObjectClass = this.getGroupObjectClassValue();
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");

        return String.format(
            "(objectClass=%s)",
            groupObjectClass
        );
    }

    @Override
    protected String buildDirectParentGroupsQuery() {

        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupMembersListAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupMembersListAttribute, "groupMembersListAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            groupObjectClass,
            groupMembersListAttribute,
            "%s"
        );
    }

    @Override
    protected String buildNestedParentGroupsQuery()
    {

        // no notion of matching rule in chain for OpenLdap,
        // so no single query exists to fetch all nested parent groups
        return null;
    }

    @Override
    protected String buildGroupQueryByObjectUniqueId()
    {
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupObjectIdAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupObjectIdAttribute, "groupAccountNameAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            groupObjectClass,
            groupObjectIdAttribute,
            "%s"
        );
    }

    @Override
    protected String buildGroupQueryByAccountName()
    {
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            groupObjectClass,
            groupAccountNameAttribute,
            "%s"
        );
    }

    @Override
    protected String buildUserOrGroupQueryByAccountNameOrUpn()
    {
        // there is no notion of the userPrincipalName in the open ldap
        return null;
    }

    @Override
    protected String buildUserOrGroupQueryByAccountName()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");

        if ( userAccountNameAttribute.equals(groupAccountNameAttribute) )
        {
            return String.format(
                    "(&(|(objectClass=%2$s)(objectClass=%3$s))(%4$s=%1$s))",
                    "%1$s",
                    userObjectClass,
                    groupObjectClass,
                    groupAccountNameAttribute
                );
        }
        else
        {
            return String.format(
                    "(|(&(objectClass=%2$s)(%3$s=%1$s))(&(objectClass=%4$s)(%5$s=%1$s)))",
                    "%1$s",
                    userObjectClass,
                    userAccountNameAttribute,
                    groupObjectClass,
                    groupAccountNameAttribute
                );
        }
    }

    @Override
    protected String buildPasswordSettingsQuery()
    {
       final String pwdPolicyObjectClass =
             this.getPasswordSettingsObjectClassValue();
       ValidateUtil.validateNotNull(pwdPolicyObjectClass, "pwdPolicyObjectClass");

       return String.format("(objectClass=%s)", pwdPolicyObjectClass);
    }

    @Override
    protected String buildDomainObjectQuery()
    {
        // un-used for open ldap at the moment
        return null;
    }

    private static Map<String, String> defaultAttributesMap = null;
    static
    {
        defaultAttributesMap = new HashMap<String,String>(27);
        defaultAttributesMap.put(ObjectIds.ObjectIdUser, "inetOrgPerson");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "uid");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "cn");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "pwdAccountLockedTime");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "entryUUID");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, DN_ATTRIBUTE);

        defaultAttributesMap.put(ObjectIds.ObjectIdGroup, "groupOfUniqueNames");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "cn");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "uniqueMember");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "entryUUID");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, DN_ATTRIBUTE);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupType, "groupType");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeTokenGroups, "tokenGroups");

        defaultAttributesMap.put(ObjectIds.ObjectIdPasswordSettings, "pwdPolicy");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.PwdPolicyAttributePwdLockoutDuration, "pwdLockoutDuration");

        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, null);

        defaultAttributesMap.put(ObjectIds.ObjectIdDomain, null);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, null);
    }
    @Override
    public String getUserQueryByAttribute() {
        final String userObjectClass = this.getUserObjectClassValue();
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            userObjectClass,
            "%s",
            "%s"
        );
    }
}
