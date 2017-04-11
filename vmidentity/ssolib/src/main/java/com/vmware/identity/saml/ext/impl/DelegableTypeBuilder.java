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
