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
 * Group specific details. Used as an aggregate of {@link Group} object. This
 * class is immutable.
 */
public final class GroupDetails extends PrincipalDetails {

   /**
    * Creates a {@link GroupDetails} instance
    */
   public GroupDetails() {
      super(null);
   }

   /**
    * Creates a {@link GroupDetails} instance
    *
    * @param description
    *           the description to set. cannot be <code>null</code>
    */
   public GroupDetails(String description) {
      super(description);
      ValidateUtil.validateNotNull(description, "Description");
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected Object[] getIdentityState() {
      return new Object[] { getDescription() };
   }

}
