/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/
package com.vmware.identity.saml.ext.impl;

import org.opensaml.common.impl.AbstractSAMLObjectBuilder;

import com.vmware.identity.saml.ext.RenewableType;

/**
 * Insert your comment for RenewableTypeBuilder here
 */
public final class RenewableTypeBuilder extends AbstractSAMLObjectBuilder<RenewableType> {

   @Override
   public RenewableType buildObject() {
      return buildObject(
              RenewableType.DEFAULT_ELEMENT_NAME.getNamespaceURI(),
              RenewableType.DEFAULT_ELEMENT_NAME.getLocalPart(),
              RenewableType.DEFAULT_ELEMENT_NAME.getPrefix(),
              RenewableType.TYPE_NAME);
   }

   @Override
   public RenewableType buildObject(String namespaceURI,
      String localName, String namespacePrefix) {

      return new RenewableTypeImpl(namespaceURI, localName,
         namespacePrefix);
   }

}
