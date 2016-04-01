/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved. 
 * *********************************************************************/
package com.vmware.identity.saml.ext.impl;

import org.opensaml.common.impl.AbstractSAMLObjectUnmarshaller;
import org.opensaml.xml.XMLObject;
import org.opensaml.xml.io.UnmarshallingException;
import org.w3c.dom.Attr;

/**
 * Unmarshaller for RenewableType
 *
 */
public final class RenewableTypeUnmarshaller extends
        AbstractSAMLObjectUnmarshaller {

    @Override
    protected void processAttribute(XMLObject samlObject, Attr attribute)
       throws UnmarshallingException {

       super.processAttribute(samlObject, attribute);
    }

}
