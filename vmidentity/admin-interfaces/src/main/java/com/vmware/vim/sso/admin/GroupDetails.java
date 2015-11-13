/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
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
