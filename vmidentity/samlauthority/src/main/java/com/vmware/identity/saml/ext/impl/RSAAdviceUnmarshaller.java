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

import org.opensaml.common.impl.AbstractSAMLObjectUnmarshaller;
import org.opensaml.xml.XMLObject;
import org.opensaml.xml.io.UnmarshallingException;
import org.w3c.dom.Attr;

import com.vmware.identity.saml.ext.RSAAdvice;

/**
 * OpenSAML unmarshaller for {@link RSAAdvice}
 */
public final class RSAAdviceUnmarshaller extends AbstractSAMLObjectUnmarshaller {

   @Override
   protected void processAttribute(XMLObject samlObject, Attr attribute)
      throws UnmarshallingException {

      RSAAdvice advice = (RSAAdvice) samlObject;

      if (attribute.getLocalName().equals(RSAAdvice.SOURCE_ATTRIB_NAME)) {
         advice.setSource(attribute.getValue());
      } else {
         super.processAttribute(samlObject, attribute);
      }
   }

}
