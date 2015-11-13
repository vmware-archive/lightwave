/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.util.Set;

/**
 * Represents a generic identity source (IdS) which connects at least one domain to SSO.
 *
 * @see Domain
 */
public class GenericIdentitySource extends IdentitySource {

   /**
    * See {@link IdentitySource}
    */
   public GenericIdentitySource(String name, Set<Domain> domains) {
      super(name, domains);
   }

   // equals and hashcode of the parent are correct for this class as well
}
