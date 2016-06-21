/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/
package com.vmware.identity.saml.ext.impl;

import org.opensaml.common.impl.AbstractSAMLObjectBuilder;

import com.vmware.identity.saml.ext.DelegableType;

/**
 * Insert your comment for DelegableTypeBuilder here
 */
public final class DelegableTypeBuilder extends AbstractSAMLObjectBuilder<DelegableType> {

   @Override
   public DelegableType buildObject() {
      return buildObject(
              DelegableType.DEFAULT_ELEMENT_NAME.getNamespaceURI(),
              DelegableType.DEFAULT_ELEMENT_NAME.getLocalPart(),
              DelegableType.DEFAULT_ELEMENT_NAME.getPrefix(),
              DelegableType.TYPE_NAME);
   }

   @Override
   public DelegableType buildObject(String namespaceURI,
      String localName, String namespacePrefix) {

      return new DelegableTypeImpl(namespaceURI, localName,
         namespacePrefix);
   }

}
