/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved. 
 * *********************************************************************/
package com.vmware.identity.saml.ext.impl;

import java.util.Collections;
import java.util.List;

import org.opensaml.common.impl.AbstractSAMLObject;
import org.opensaml.xml.XMLObject;

import com.vmware.identity.saml.ext.DelegableType;

/**
 * DelegableType implementation
 *
 */
public final class DelegableTypeImpl extends AbstractSAMLObject implements
        DelegableType {

    DelegableTypeImpl(String namespaceURI, String elementLocalName,
            String namespacePrefix) {
            super(namespaceURI, elementLocalName, namespacePrefix);
         }

    /* (non-Javadoc)
     * @see org.opensaml.xml.XMLObject#getOrderedChildren()
     */
    public List<XMLObject> getOrderedChildren() {
        return Collections.emptyList();
    }

}
