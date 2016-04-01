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

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.ResourceServer.Builder;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.interop.ldap.LdapValue;

/**
 * @author Yehia Zayour
 */
public final class ResourceServerLdapObject extends BaseLdapObjectBase<ResourceServer, ResourceServer.Builder> {

    private static ResourceServerLdapObject _instance = new ResourceServerLdapObject();

    private static final String OBJECT_CLASS = "vmwOidcResourceServer";
    private static final String PROPERTY_NAME = CN;
    private static final String PROPERTY_RESOURCE_SERVER_NAME = "vmwOidcResourceServerName";
    private static final String PROPERTY_RESOURCE_SERVER_GROUP_FILTER = "vmwOidcResourceServerGroupFilter";

    public static ResourceServerLdapObject getInstance() {
        return _instance;
    }

    @SuppressWarnings("unchecked")
    private ResourceServerLdapObject() {
        super(OBJECT_CLASS, new PropertyMapperMetaInfoBase[] {
                new PropertyMapperMetaInfoBase<ResourceServer, ResourceServer.Builder>(
                        PROPERTY_NAME,
                        0 /* ctorOrder */,
                        true /* isSettableOnObject */,
                        new IPropertyGetterSetterBase<ResourceServer, ResourceServer.Builder>() {
                            @Override
                            public void SetLdapValue(ResourceServer.Builder builder, LdapValue[] value) {
                                throw new IllegalStateException("property is not settable.");
                            }

                            @Override
                            public LdapValue[] GetLdapValue(ResourceServer resourceServer) {
                                ValidateUtil.validateNotNull(resourceServer, "resourceServer");
                                return ServerUtils.getLdapValue(resourceServer.getName());
                            }
                        },
                        false /* isUpdateableOnLdapServer */
                ),
                new PropertyMapperMetaInfoBase<ResourceServer, ResourceServer.Builder>(
                        PROPERTY_RESOURCE_SERVER_NAME,
                        -1 /* ctorOrder */,
                        true /* isSettableOnObject */,
                        new IPropertyGetterSetterBase<ResourceServer, ResourceServer.Builder>() {
                            @Override
                            public void SetLdapValue(ResourceServer.Builder builder, LdapValue[] value) {
                                // no-op
                            }

                            @Override
                            public LdapValue[] GetLdapValue(ResourceServer resourceServer) {
                                ValidateUtil.validateNotNull(resourceServer, "resourceServer");
                                return ServerUtils.getLdapValue(resourceServer.getName());
                            }
                        },
                        false /* isUpdateableOnLdapServer */
                ),
                new PropertyMapperMetaInfoBase<ResourceServer, ResourceServer.Builder>(
                        PROPERTY_RESOURCE_SERVER_GROUP_FILTER,
                        -1 /* ctorOrder */,
                        true /* isSettableOnObject */,
                        new IPropertyGetterSetterBase<ResourceServer, ResourceServer.Builder>() {
                            @Override
                            public void SetLdapValue(ResourceServer.Builder builder, LdapValue[] value) {
                                ValidateUtil.validateNotNull(builder, "builder");
                                String[] groupFilterArray = ServerUtils.getMultiStringValue(value);
                                Set<String> groupFilter = (groupFilterArray != null) ?
                                        new HashSet<String>(Arrays.asList(groupFilterArray)) :
                                        Collections.<String>emptySet();
                                builder.groupFilter(groupFilter);
                            }

                            @Override
                            public LdapValue[] GetLdapValue(ResourceServer resourceServer) {
                                ValidateUtil.validateNotNull(resourceServer, "resourceServer");
                                return ServerUtils.getLdapValue(resourceServer.getGroupFilter());
                            }
                        },
                        true /* isUpdateableOnLdapServer */
                )});
    }

    @Override
    protected Builder createObject(List<LdapValue[]> ctorParams) {
        if ((ctorParams == null) || (ctorParams.size() != 1)) {
            throw new IllegalArgumentException("ctorParams");
        }
        String resourceServerName = ServerUtils.getStringValue(ctorParams.get(0));
        return new Builder(resourceServerName);
    }

    @Override
    protected ResourceServer createFinalObject(Builder builder) {
        Validate.notNull(builder, "builder");
        return builder.build();
    }
}
