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
package com.vmware.identity.saml.ext;

import java.util.List;

import javax.xml.namespace.QName;

import org.opensaml.common.SAMLObject;
import org.opensaml.saml2.core.Attribute;

/**
 * Represents an OpenSAML abstraction of RSA extension to the standard SAML
 * advice.
 */
public interface RSAAdvice extends SAMLObject {

   public static final String ELEMENT_LOCAL_NAME = "RSAAdvice";
   public static final QName ELEMENT_NAME = new QName(AssertionExtNames.TNS,
      ELEMENT_LOCAL_NAME, AssertionExtNames.NS_PREFIX);

   public static final String TYPE_LOCAL_NAME = "RSAAdviceType";
   public static final QName TYPE_NAME = new QName(AssertionExtNames.TNS,
      TYPE_LOCAL_NAME, AssertionExtNames.NS_PREFIX);

   public static final String SOURCE_ATTRIB_NAME = "AdviceSource";

   /**
    * @return advice source URI, not null
    */
   public String getSource();

   public void setSource(String source);

   public List<Attribute> getAttributes();
}
