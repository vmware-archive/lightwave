/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. VMware Confidential
 **********************************************************************************/
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
