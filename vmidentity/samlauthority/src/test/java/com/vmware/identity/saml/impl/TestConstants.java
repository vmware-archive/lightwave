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
package com.vmware.identity.saml.impl;

import java.util.Arrays;

import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.Advice.Attribute;

/**
 * Contain constants widely used in tests
 */
final class TestConstants {
   static final String SOURCE1 = "sourceURI1";
   static final String SOURCE2 = "sourceURI2";

   static final Attribute ATTR1 = new Attribute("nameURI", null,
      Arrays.asList("attrvalue1"));
   static final Attribute ATTR1_OTHER_VALUE = new Attribute(ATTR1.nameURI(),
      ATTR1.friendlyName(), Arrays.asList("attrvalue2", "attrvalue3"));
   static final Attribute ATTR2 = new Attribute("nameURI2",
      ATTR1.friendlyName(), ATTR1.values());

   static final Advice ADVICE = new Advice(SOURCE1, Arrays.asList(ATTR1));
   static final Advice ADVICE_OTHER_VALUE = new Advice(ADVICE.sourceURI(),
      Arrays.asList(ATTR1_OTHER_VALUE));

   static final String ISSUER = "issuer";
   static final String EXTERNAL_ISSUER = "externalIssuer";

}
