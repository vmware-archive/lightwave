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

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotEmpty;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Represents all domains in the SSO server, i.e.
 * <ul>
 * <li>The <b>system</b> domain: represented by non-empty name string.
 * <li>Zero or more <b>external</b> domains: a set of non-{@code null}
 * ExternalDomain objects.</li>
 * </ul>
 *
 * @see ExternalDomain
 */
public class Domains {
   private final Set<ExternalDomain> _externalDomains;
   private final String _systemDomainName;
   private final String _localOSDomainName;
   private final Set<String> _systemDomainUpnSuffixes;

   /**
    * Populate a Domains instance.
    */
   public Domains(Set<ExternalDomain> externalDomains, String systemDomainName) {

      validateNotEmpty(systemDomainName, "systemDomainName");

      _externalDomains = externalDomains;
      _systemDomainName = systemDomainName;
      _localOSDomainName = null;
      _systemDomainUpnSuffixes = Collections.<String> emptySet();
   }

   /**
    * Populate a Domains instance without local OS domain name
    * @param externalDomains   optional external domains
    * @param systemDomainName  cannot be null or empty
    * @param systemDomainUpnSuffixes  optional UPN suffixes for system domain accounts, can be null or empty
    */
   public Domains(Set<ExternalDomain> externalDomains, String systemDomainName,
         Set<String> systemDomainUpnSuffixes) {

      validateNotEmpty(systemDomainName, "systemDomainName");

      _externalDomains = externalDomains;
      _systemDomainName = systemDomainName;
      _localOSDomainName = null;
      _systemDomainUpnSuffixes = new HashSet<String>();
      if (systemDomainUpnSuffixes != null) {
         _systemDomainUpnSuffixes.addAll(systemDomainUpnSuffixes);
      }
   }

   /**
    * Populate a Domains instance with local OS domain name
    */
   public Domains(Set<ExternalDomain> externalDomains, String systemDomainName,
      String localOSDomainName) {

      this(externalDomains, systemDomainName, localOSDomainName, Collections.<String> emptySet());
   }

   /**
    * Populate a Domains instance with all parameters
    * @param externalDomains   optional external domains
    * @param systemDomainName  cannot be null or empty
    * @param localOSDomainName cannot be null or empty
    * @param systemDomainUpnSuffixes  optional UPN suffixes for system domain accounts, can be null or empty
    *
    * @see ExternalDomain
    */
   public Domains(Set<ExternalDomain> externalDomains, String systemDomainName,
         String localOSDomainName, Set<String> systemDomainUpnSuffixes) {

      validateNotEmpty(systemDomainName, "systemDomainName");
      validateNotEmpty(localOSDomainName, "localOSDomainName");

      _externalDomains = externalDomains;
      _systemDomainName = systemDomainName;
      _localOSDomainName = localOSDomainName;

      _systemDomainUpnSuffixes = new HashSet<String>();
      if (systemDomainUpnSuffixes != null) {
         _systemDomainUpnSuffixes.addAll(systemDomainUpnSuffixes);
      }
   }

   /**
    * @return The external domains or empty set when no external domains have
    *         been registered
    */
   public Set<ExternalDomain> getExternalDomains() {
      return _externalDomains;
   }

   /**
    * @return The name of the system domain. {@code not-null} value
    */
   public String getSystemDomainName() {
      return _systemDomainName;
   }

   /**
    * The name of local OS domain
    *
    * @return domain name or {@code null} if the local OS domain has not been
    *         registered; not-empty value
    */
   public String getLocalOSDomainName() {
      return _localOSDomainName;
   }

   /**
    *
    * @return the unmodifiable set of UPN suffixes for system domain accounts, can be empty but not null
    */
   public Set<String> getSystemDomainUpnSuffixes() {
      return Collections.unmodifiableSet(_systemDomainUpnSuffixes);
   }
}
