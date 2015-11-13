/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;

public class AttributeConfig implements Serializable
{

   private static final long serialVersionUID = 751016747297157026L;

   private final String tokenSubjectFormat;
   private final String storeAttribute;

   public AttributeConfig(String tokenSubjectFormat, String storeAttribute)
   {
      ValidateUtil.validateNotEmpty(tokenSubjectFormat, "tokenSubjectFormat");
      ValidateUtil.validateNotEmpty(storeAttribute, "storeAttribute");

      this.tokenSubjectFormat = tokenSubjectFormat;
      this.storeAttribute = storeAttribute;
   }

   public String getTokenSubjectFormat()
   {
      return tokenSubjectFormat;
   }

   public String getStoreAttribute()
   {
      return storeAttribute;
   }
}
