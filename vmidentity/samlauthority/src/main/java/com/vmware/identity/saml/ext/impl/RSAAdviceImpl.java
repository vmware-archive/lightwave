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

import java.util.ArrayList;
import java.util.List;

import org.opensaml.common.impl.AbstractSAMLObject;
import org.opensaml.saml2.core.Attribute;
import org.opensaml.xml.XMLObject;

import com.vmware.identity.saml.ext.RSAAdvice;

/**
 * OpenSAML implementation of {@link RSAAdvice}
 */
final class RSAAdviceImpl extends AbstractSAMLObject implements RSAAdvice {

   private String source;
   private List<Attribute> attributes = new ArrayList<Attribute>();

   RSAAdviceImpl(String namespaceURI, String elementLocalName,
      String namespacePrefix) {
      super(namespaceURI, elementLocalName, namespacePrefix);
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public List<XMLObject> getOrderedChildren() {
      return new ArrayList<XMLObject>(attributes);
   }

   @Override
   public String getSource() {
      return source;
   }

   @Override
   public void setSource(String newSource) {
      if (newSource == null) {
         throw new IllegalArgumentException("Source must be not null string");
      }
      source = prepareForAssignment(source, newSource);
   }

   @Override
   public List<Attribute> getAttributes() {
      return attributes;
   }

}
