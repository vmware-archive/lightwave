/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using System.ComponentModel;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes
{
    public enum UserAttributeId
    {
        [Description("Account Name")]
        UserAttributeAccountName,

        [Description("Common Name")]
        UserAttributeCommonName,

        [Description("Last Name")]
        UserAttributeLastName,

        [Description("First Name")]
        UserAttributeFirstName,

        [Description("Description")]
        UserAttributeDescription,

        [Description("Display Name")]
        UserAttributeDisplayName,

        [Description("Email")]
        UserAttributeEmail,

        [Description("Object Id")]
        UserAttributeObjectId,

        [Description("Principal Name")]
        UserAttributePrincipalName,

        [Description("Account Control")]
        UserAttributeAcountControl,

        [Description("Member Of")]
        UserAttributeMemberOf,

        [Description("Lockout Time")]
        UserAttributeLockoutTime,

        [Description("Primary Group Id")]
        UserAttributePrimaryGroupId,

        [Description("Password Settings Object")]
        UserAttributePasswordSettingsObject,

        [Description("Password Last Set")]
        UserAttributePwdLastSet,

        [Description("Group Members List Link")]
        UserAttributeGroupMembersListLink,
    }
}
