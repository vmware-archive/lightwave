/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.endpoint;

import org.opensaml.Configuration;
import org.opensaml.DefaultBootstrap;
import org.opensaml.xml.ConfigurationException;
import org.springframework.stereotype.Component;

import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.saml.ext.impl.DelegableTypeBuilder;
import com.vmware.identity.saml.ext.impl.DelegableTypeMarshaller;
import com.vmware.identity.saml.ext.impl.DelegableTypeUnmarshaller;
import com.vmware.identity.saml.ext.impl.RenewableTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewableTypeMarshaller;
import com.vmware.identity.saml.ext.impl.RenewableTypeUnmarshaller;

/**
 * SharedComponent for common initializing task
 * 
 */
@Component
public class SharedComponent {
    public SharedComponent() throws ConfigurationException {
        bootstrap();
    }

    public static void bootstrap() throws ConfigurationException {
        // opensaml initialization
        DefaultBootstrap.bootstrap();
        Configuration.registerObjectProvider(RenewableType.TYPE_NAME,
                new RenewableTypeBuilder(), new RenewableTypeMarshaller(),
                new RenewableTypeUnmarshaller());
        Configuration.registerObjectProvider(DelegableType.TYPE_NAME,
                new DelegableTypeBuilder(), new DelegableTypeMarshaller(),
                new DelegableTypeUnmarshaller());
    }
}
