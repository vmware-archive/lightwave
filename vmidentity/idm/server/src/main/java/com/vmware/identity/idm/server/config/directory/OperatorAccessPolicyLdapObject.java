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

package com.vmware.identity.idm.server.config.directory;

import java.util.List;

import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.OperatorAccessPolicy;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.interop.ldap.LdapValue;

/**
 */
public final class OperatorAccessPolicyLdapObject extends BaseLdapObjectBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder> {

    private static OperatorAccessPolicyLdapObject _instance = new OperatorAccessPolicyLdapObject();

    private static final String OBJECT_NAME = "OperatorAccessPolicy";
    private static final String OBJECT_CLASS = "vmwSTSOperatorAccessPolicy";
    private static final String PROPERTY_NAME = CN;
    private static final String PROPERTY_ENABLED = "vmwSTSEnableOperatorAccess";
    private static final String PROPERTY_USER_BASE_DN = "vmwSTSUserBaseDN";
    private static final String PROPERTY_GROUP_BASE_DN = "vmwSTSGroupBaseDN";

    public static OperatorAccessPolicyLdapObject getInstance() {
        return _instance;
    }

    @SuppressWarnings("unchecked")
    private OperatorAccessPolicyLdapObject() {
        super(OBJECT_CLASS, new PropertyMapperMetaInfoBase[] {
            new PropertyMapperMetaInfoBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>(
                PROPERTY_NAME,
                -1 /* ctorOrder */,
                true /* isSettableOnObject */,
                new IPropertyGetterSetterBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>() {
                    @Override
                    public void SetLdapValue(OperatorAccessPolicy.Builder builder, LdapValue[] value) {
                        // no op
                    }

                    @Override
                    public LdapValue[] GetLdapValue(OperatorAccessPolicy policy) {
                        return ServerUtils.getLdapValue(OBJECT_NAME);
                    }
                },
                false /* isUpdateableOnLdapServer */
            ),
            new PropertyMapperMetaInfoBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>(
                PROPERTY_ENABLED,
                -1 /* ctorOrder */,
                true /* isSettableOnObject */,
                new IPropertyGetterSetterBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>() {
                    @Override
                    public void SetLdapValue(OperatorAccessPolicy.Builder builder, LdapValue[] value) {
                        ValidateUtil.validateNotNull(builder, "builder");
                        builder.withEnabled(ServerUtils.getBooleanValue(value));
                    }

                    @Override
                    public LdapValue[] GetLdapValue(OperatorAccessPolicy policy) {
                        ValidateUtil.validateNotNull(policy, "policy");
                        return ServerUtils.getLdapValue(policy.enabled());
                    }
                },
                true /* isUpdateableOnLdapServer */
            ),
            new PropertyMapperMetaInfoBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>(
                PROPERTY_USER_BASE_DN,
                -1 /* ctorOrder */,
                true /* isSettableOnObject */,
                new IPropertyGetterSetterBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>() {
                    @Override
                    public void SetLdapValue(OperatorAccessPolicy.Builder builder, LdapValue[] value) {
                        ValidateUtil.validateNotNull(builder, "builder");
                        builder.withUserBaseDn(ServerUtils.getStringValue(value));
                    }

                    @Override
                    public LdapValue[] GetLdapValue(OperatorAccessPolicy policy) {
                        ValidateUtil.validateNotNull(policy, "policy");
                        return ServerUtils.getLdapValue(policy.userBaseDn());
                    }
                },
                true /* isUpdateableOnLdapServer */
            ),
            new PropertyMapperMetaInfoBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>(
                PROPERTY_GROUP_BASE_DN,
                -1 /* ctorOrder */,
                true /* isSettableOnObject */,
                new IPropertyGetterSetterBase<OperatorAccessPolicy, OperatorAccessPolicy.Builder>() {
                    @Override
                    public void SetLdapValue(OperatorAccessPolicy.Builder builder, LdapValue[] value) {
                        ValidateUtil.validateNotNull(builder, "builder");
                        builder.withGroupBaseDn(ServerUtils.getStringValue(value));
                    }

                    @Override
                    public LdapValue[] GetLdapValue(OperatorAccessPolicy policy) {
                        ValidateUtil.validateNotNull(policy, "policy");
                        return ServerUtils.getLdapValue(policy.groupBaseDn());
                    }
                },
                true /* isUpdateableOnLdapServer */
            )});
    }

    @Override
    protected OperatorAccessPolicy.Builder createObject(List<LdapValue[]> ctorParams) {
        if ((ctorParams != null) && (ctorParams.size() != 0)) {
            throw new IllegalArgumentException("ctorParams");
        }
        return new OperatorAccessPolicy.Builder();
    }

    @Override
    protected OperatorAccessPolicy createFinalObject(OperatorAccessPolicy.Builder builder) {
        Validate.notNull(builder, "builder");
        return builder.build();
    }
}
