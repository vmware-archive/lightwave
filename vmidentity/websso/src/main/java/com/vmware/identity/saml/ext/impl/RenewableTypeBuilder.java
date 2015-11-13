/*
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
 */
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
