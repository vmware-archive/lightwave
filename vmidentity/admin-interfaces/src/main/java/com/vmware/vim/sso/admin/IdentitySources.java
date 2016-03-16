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

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Represents all Identity Sources configured in the SSO server.
 *
 * @see IdentitySource
 */
public final class IdentitySources {

   private final Set<LdapIdentitySource> _ldaps;
   private final IdentitySource _system;
   private final IdentitySource _localOS;
   private final IdentitySource _nativeAD;
   private final Set<IdentitySource> _all;

   /**
    * Populate a IdentitySources instance.
    *
    * TODO Not sure that this constructor will be needed
    */
   public IdentitySources(Set<LdapIdentitySource> ldaps,
      GenericIdentitySource system, GenericIdentitySource localOS, GenericIdentitySource nativeAD) {
      this(ldaps, system, localOS, nativeAD,
         collectAll(ldaps, system, localOS, nativeAD));
   }

   /**
    * Populate a IdentitySources instance.
    *
    * <p>The all parameter must contains at least all other provided Identity Sources.
    */
   public IdentitySources(Set<LdapIdentitySource> ldaps,
      GenericIdentitySource system, GenericIdentitySource localOS, GenericIdentitySource nativeAD,
      Set<IdentitySource> all) {

      validateNotNull(system, "system");
      validateNotNull(ldaps, "ldaps");
      validateNotNull(all, "all");

      _ldaps = Collections.unmodifiableSet(ldaps);
      _system = system;
      _localOS = localOS;
      _nativeAD = nativeAD;
      _all = all;
   }

   /**
    * @return The LDAP identity source or empty set when none have
    *         been registered
    */
   public Set<LdapIdentitySource> getLdaps() {
      return _ldaps;
   }

   /**
    * @return All identity sources in unspecified order or empty set
    *         when none have been registered
    *
    * <p>If the SSO Server is newer than the one this interface was written for
    * this set may contain more identity source than the union of all other
    * fields.
    */
   public Set<IdentitySource> getAll() {
      return _all;
   }

   /**
    * return The AD to which the SSO server is joined, can be null
    *
    * Native AD Identity Source cannot be configured if there is already an LDAP
    * Identity Sources of type {@link LdapIdentitySource.Type} configured.
    */
   public IdentitySource getNativeAD() {
      return _nativeAD;
   }

   /**
    * @return the local OS identity source, can be null
    */
   public IdentitySource getLocalOS() {
      return _localOS;
   }

   /**
    * @return the system IdS, {@code not-null} value
    */
   public IdentitySource getSystem() {
      return _system;
   }

   /**
    * @return The name of the system domain. Not empty.
    */
   public String getSystemDomainName() {
      return _system.getDomains().iterator().next().getName();
   }

   /**
    * The name of local OS domain
    *
    * @return domain name or {@code null} if the local OS domain has not been
    *         registered. Not empty.
    */
   public String getLocalOSDomainName() {
      if (_localOS == null) {
         return null;
      }
      return _localOS.getDomains().iterator().next().getName();
   }

   private static Set<IdentitySource> collectAll(Set<LdapIdentitySource> ldaps,
         GenericIdentitySource system, GenericIdentitySource localOs,
         GenericIdentitySource nativeAd) {
      Set<IdentitySource> all = new HashSet<IdentitySource>(ldaps);
      all.add(system);
      if (localOs != null) {
         all.add(localOs);
      }
      if (nativeAd != null) {
         all.add(nativeAd);
      }
      return Collections.unmodifiableSet(all);
   }
}
