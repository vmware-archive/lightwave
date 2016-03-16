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

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotNull;

import com.vmware.vim.sso.PrincipalId;

/**
 * State object representing group type principals
 */
public final class Group extends Principal {

   private final GroupDetails _details;

   /**
    * Constructs group principal with no alias if its domain is not configured
    * with such.
    * 
    * @param id
    *           group id; {@code not-null} value is required
    * @param details
    *           group details; {@code not-null} value is required
    */
   public Group(PrincipalId id, GroupDetails details) {

      this(id, null /* no alias */, details);
   }

   /**
    * Constructs group principal
    * 
    * @param id
    *           principal id; {@code not-null} value is required
    * @param alias
    *           principal alias; {@code null} value when alias is not known or
    *           the corresponding domain has no alias specified; note that the
    *           alias should not be equal to the ID
    * @param details
    *           group details; {@code not-null} value is required
    */
   public Group(PrincipalId id, PrincipalId alias, GroupDetails details) {

      super(id, alias);

      validateNotNull(details, "Group Details");
      _details = details;
   }

   @Override
   public GroupDetails getDetails() {
      return _details;
   }

   /**
    * Canonical name for groups is NetBIOS format, e.g.<br>
    * <i>&lt;shortDomainName&gt;&#92;&lt;account_name&gt;</i><br>
    * Domain part will always be upper case. When alias is not specified id will
    * be used. The character case of the account name depends on the particular
    * domain implementation.
    */
   @Override
   public final String getCanonicalName() {
      final PrincipalId alias = getAlias() != null ? getAlias() : getId();
      return String.format("%s\\%s", alias.getDomain().toUpperCase(), alias
         .getName());
   }

}
