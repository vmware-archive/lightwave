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
package com.vmware.identity.idm.server.provider.activedirectory;

import java.util.HashMap;
import java.util.Map;

import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping.ObjectIds;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.provider.BaseLdapSchemaMapping;

public class ADSchemaMapping extends BaseLdapSchemaMapping
{
    public ADSchemaMapping( IdentityStoreSchemaMapping schemaMapping )
    {
        super(schemaMapping, defaultAttributesMap);
    }

    @Override
    protected String buildUserQueryByAccountNameOrUpn()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        final String userPrincipalNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");
        ValidateUtil.validateNotNull(userPrincipalNameAttribute, "userPrincipalNameAttribute");

        return String.format(
            "(&(|(%1$s=%3$s)(%2$s=%4$s))(objectClass=%5$s))",
            userAccountNameAttribute,
            userPrincipalNameAttribute,
            "%1$s",
            "%2$s",
            userObjectClass
        );
    }

    @Override
    protected String buildUserQueryByUpn()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userUpnAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userUpnAttribute, "userPrincipalNameAttribute");

        return String.format(
            "(&(%1$s=%2$s)(objectClass=%3$s))",
            userUpnAttribute,
            "%s",
            userObjectClass
        );
    }

    @Override
    protected String buildUserQueryByAccountName()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");

        return String.format(
            "(&(%1$s=%2$s)(objectClass=%3$s))",
            userAccountNameAttribute,
            "%s",
            userObjectClass
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
        //"(&(objectClass=user)(|(sAMAccountName=*%1$s*)(sn=*%1$s*)(givenName=*%1$s*)(cn=*%1$s*)(name=*%1$s*)(displayname=*%1$s*)))";
        final String USER_PRINC_QUERY_BY_CRITERIA =
          "(&(objectClass=%2$s)(|(%3$s=*%1$s*)(%4$s=*%1$s*)(%5$s=*%1$s*)(cn=*%1$s*)(name=*%1$s*)(%6$s=*%1$s*)))";

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

        return String.format(
            USER_PRINC_QUERY_BY_CRITERIA,
            "%1$s",
            userObjectClass,
            userAccountNameAttribute,
            userLastNameAttribute,
            userFirstNameAttribute,
            userdisplayNameAttribute
        );

    }

    @Override
    protected String buildUserQueryByCriteriaForName()
    {
       // Use 'starts with' queries to utilize the index
       //"(&(objectClass=user)(|(sAMAccountName=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=%1$s*)(displayname=%1$s*)(userPrincipalName=%1$s*)))";
       final String USER_PRINC_QUERY_BY_CRITERIA =
         "(&(objectClass=%2$s)(|(%3$s=%1$s*)(%4$s=%1$s*)(%5$s=%1$s*)(%6$s=%1$s*)(%7$s=%1$s*)(%8$s=%1$s*)))";

       final String userObjectClass = this.getUserObjectClassValue();
       ValidateUtil.validateNotNull(userObjectClass, "userObjectClass"); // user
       final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
       ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute"); // sAMAccountName
       final String userLastNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName);
       ValidateUtil.validateNotNull(userLastNameAttribute, "userLastNameAttribute"); // sn
       final String userFirstNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName);
       ValidateUtil.validateNotNull(userFirstNameAttribute, "userFirstNameAttribute"); // givenName
       final String userCommonNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName);
       ValidateUtil.validateNotNull(userCommonNameAttribute, "userCommonNameAttribute"); // cn
       final String userdisplayNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName);
       ValidateUtil.validateNotNull(userdisplayNameAttribute, "userdisplayNameAttribute"); //displayName
       final String userPrincipalNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
       ValidateUtil.validateNotNull(userPrincipalNameAttribute, "userPrincipalNameAttribute"); // userPrincipalName

       return String.format(
           USER_PRINC_QUERY_BY_CRITERIA,
           "%1$s",
           userObjectClass,
           userAccountNameAttribute,
           userLastNameAttribute,
           userFirstNameAttribute,
           userCommonNameAttribute,
           userdisplayNameAttribute,
           userPrincipalNameAttribute
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
        final String userObjectClass = this.getUserObjectClassValue();
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        final String userAccountControlAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl);
        ValidateUtil.validateNotNull(userAccountControlAttribute, "userAccountControlAttribute");

        return String.format(
            "(&(objectclass=%s)(!(%s=0)))",
            userObjectClass,
            userAccountControlAttribute
        );
    }

    @Override
    protected String buildGroupQueryByCriteria()
    {
        //"(&(objectClass=group)(|(sAMAccountName=*%1$s*)(description=*%1$s*)(objectSid=*%1$s*)))";
        final String GROUP_PRINC_QUERY_BY_CRITERIA =
          "(&(objectClass=%2$s)(|(%3$s=*%1$s*)(%4$s=*%1$s*)(%5$s=*%1$s*)))";

        final String groupObjectClass = this.getGroupObjectClassValue();
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");
        final String groupDescriptionAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription);
        ValidateUtil.validateNotNull(groupDescriptionAttribute, "groupDescriptionAttribute");
        final String groupObjectIdAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
        ValidateUtil.validateNotNull(groupObjectIdAttribute, "groupObjectIdAttribute");

        return String.format(
            GROUP_PRINC_QUERY_BY_CRITERIA,
            "%1$s",
            groupObjectClass,
            groupAccountNameAttribute,
            groupDescriptionAttribute,
            groupObjectIdAttribute
        );
    }

    @Override
    protected String buildGroupQueryByCriteriaForName()
    {
        //"(&(objectClass=group)(|(sAMAccountName=%1$s*)(cn=%1$s*))";  want to utilize index (by not doing a contain)
        final String GROUP_PRINC_QUERY_BY_CRITERIA_FOR_NAME =
          "(&(objectClass=%2$s)(|(%3$s=%1$s*)(cn=%1$s*)))";

        final String groupObjectClass = this.getGroupObjectClassValue();
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");

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
    protected String buildDirectParentGroupsQuery()
    {
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
        //"(&(objectClass=group)(member:1.2.840.113556.1.4.1941:=%s))"
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupMembersListAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupMembersListAttribute, "groupMembersListAttribute");

        return String.format(
            "(&(objectClass=%s)(%s:1.2.840.113556.1.4.1941:=%s))", // using matching_rule_in_chain operator
            groupObjectClass,
            groupMembersListAttribute,
            "%s"
        );
    }

    @Override
    protected String buildGroupQueryByObjectUniqueId() {
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupObjectIdAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupObjectIdAttribute, "groupObjectIdAttribute");

        return String.format(
            "(&(objectClass=%s)(%s=%s))",
            groupObjectClass,
            groupObjectIdAttribute,
            "%s"
        );
    }

    @Override
    protected String buildGroupQueryByAccountName() {
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");

        return String.format(
            "(&(%2$s=%1$s)(objectClass=%3$s))",
            "%s",
            groupAccountNameAttribute,
            groupObjectClass
        );
    }

    @Override
    protected String buildUserOrGroupQueryByAccountNameOrUpn()
    {
        final String userObjectClass = this.getUserObjectClassValue();
        final String userAccountNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName);
        final String userPrincipalNameAttribute = this.getUserAttribute(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName);
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");
        ValidateUtil.validateNotNull(userAccountNameAttribute, "userAccountNameAttribute");
        ValidateUtil.validateNotNull(userPrincipalNameAttribute, "userPrincipalNameAttribute");
        final String groupObjectClass = this.getGroupObjectClassValue();
        final String groupAccountNameAttribute = this.getGroupAttribute(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName);
        ValidateUtil.validateNotNull(groupObjectClass, "groupObjectClass");
        ValidateUtil.validateNotNull(groupAccountNameAttribute, "groupAccountNameAttribute");

        if ( userAccountNameAttribute.equals(groupAccountNameAttribute) )
        {
            return String.format(
                "(|(&(%2$s=%1$s)(|(objectClass=%3$s)(objectClass=%4$s)))(&(%5$s=%6$s)(objectClass=%3$s)))",
                "%1$s",
                userAccountNameAttribute,
                userObjectClass,
                groupObjectClass,
                userPrincipalNameAttribute,
                "%2$s"
            );
        }
        else
        {
            return String.format(
                "(|(&(|(%3$s=%1$s)(%6$s=%7$s))(objectClass=%2$s))(&(%4$s=%1$s)(objectClass=%5$s)))",
                "%1$s",
                userObjectClass,
                userAccountNameAttribute,
                groupAccountNameAttribute,
                groupObjectClass,
                userPrincipalNameAttribute,
                "%2$s"
            );
        }
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
                "(&(%2$s=%1$s)(|(objectClass=%3$s)(objectClass=%4$s)))",
                "%1$s",
                userAccountNameAttribute,
                userObjectClass,
                groupObjectClass
            );
        }
        else
        {
            return String.format(
                "(|(&(%3$s=%1$s)(objectClass=%2$s))(&(%4$s=%1$s)(objectClass=%5$s)))",
                "%1$s",
                userObjectClass,
                userAccountNameAttribute,
                groupAccountNameAttribute,
                groupObjectClass
            );
        }
    }

    @Override
    protected String buildPasswordSettingsQuery()
    {
        final String passwordSettingsObjectClass = this.getPasswordSettingsObjectClassValue();
        ValidateUtil.validateNotNull(passwordSettingsObjectClass, "passwordSettingsObjectClass");

        return String.format(
            "(objectclass=%s)",
            passwordSettingsObjectClass
        );
    }

    @Override
    protected String buildDomainObjectQuery()
    {
        final String domainObjectClass = this.getDomainObjectClassValue();
        ValidateUtil.validateNotNull(domainObjectClass, "domainObjectClass");

        return String.format(
            "(objectclass=%s)",
            domainObjectClass
        );
    }

    private static Map<String, String> defaultAttributesMap = null;
    static
    {
        defaultAttributesMap = new HashMap<String,String>(27);
        defaultAttributesMap.put(ObjectIds.ObjectIdUser, "user");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAcountControl, "userAccountControl");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "sAMAccountName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeCommonName, "cn");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "description");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "displayname");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeEmail, "mail");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeFirstName, "givenName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLastName, "sn");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeLockoutTime, "lockoutTime");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeMemberOf, "memberof");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeObjectId, "objectSid");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePasswordSettingsObject, "msDS-ResultantPSO");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrimaryGroupId, "primaryGroupID");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePrincipalName, "userPrincipalName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributePwdLastSet, "pwdLastSet");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.UserAttributeGroupMembersListLink, DN_ATTRIBUTE);

        defaultAttributesMap.put(ObjectIds.ObjectIdGroup, "group");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "sAMAccountName");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "description");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMemberOf, "memberof");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeMembersList, "member");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeObjectId, "objectSid");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupMembersListLink, DN_ATTRIBUTE);
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeGroupType, "groupType");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.GroupAttributeTokenGroups, "tokenGroups");

        defaultAttributesMap.put(ObjectIds.ObjectIdPasswordSettings, "msDS-PasswordSettings");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.PasswordSettingsAttributeMaximumPwdAge, "msDS-MaximumPasswordAge");

        defaultAttributesMap.put(ObjectIds.ObjectIdDomain, "domain");
        defaultAttributesMap.put(IdentityStoreAttributeMapping.AttributeIds.DomainAttributeMaxPwdAge, "maxPwdAge");
    }
    @Override
    public String getUserQueryByAttribute() {
        final String userObjectClass = this.getUserObjectClassValue();
        ValidateUtil.validateNotNull(userObjectClass, "userObjectClass");

        return String.format(
            "(&(%1$s=%2$s)(objectClass=%3$s))",
            "%s",
            "%s",
            userObjectClass
        );

    }
}
