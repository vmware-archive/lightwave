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

package com.vmware.vim.sso.admin;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Immutable data type for configuration of mappings between token attribute and
 * data store attribute.
 *
 * This class is a generic definition so that it can be used to configure
 * various mappings (M*N) from token attributes to store attributes.
 *
 */
public final class AttributeConfig {

   private final String tokenAttribute;
   private final String storeAttribute;

   /**
    * C'tor
    * @param tokenAttribute
    *           attribute name in the token, cannot be null or empty
    * @param storeAttribute
    *           attribute name in the data store, cannot be null or empty
    */
   public AttributeConfig(String tokenAttribute, String storeAttribute) {
      ValidateUtil.validateNotEmpty(tokenAttribute, "tokenAttribute");
      ValidateUtil.validateNotEmpty(storeAttribute, "storeAttribute");

      this.tokenAttribute = tokenAttribute;
      this.storeAttribute = storeAttribute;
   }

   /**
    *
    * @return the mapping tokenAttribute
    */
   public String getTokenAttribute() {
      return tokenAttribute;
   }

   /**
    *
    * @return the mapped storeAttribute
    */
   public String getStoreAttribute() {
      return storeAttribute;
   }
}
