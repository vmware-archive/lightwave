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

import com.vmware.identity.saml.ext.RenewRestrictionType;

/**
 * Insert your comment for RenewRestrictionTypeMarshaller here
 */
public final class RenewRestrictionTypeUnmarshaller extends AbstractSAMLObjectUnmarshaller {

   @Override
   protected void processAttribute(XMLObject samlObject, Attr attribute)
      throws UnmarshallingException {

      RenewRestrictionType renewRestriction = (RenewRestrictionType) samlObject;

      if (attribute.getLocalName().equals(
         RenewRestrictionType.COUNT_ATTRIB_NAME)) {
         renewRestriction.setCount(Integer.valueOf(attribute.getValue()));
      } else {
         super.processAttribute(samlObject, attribute);
      }
   }

}
