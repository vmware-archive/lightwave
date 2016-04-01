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
package com.vmware.vim.idp;

import java.util.Set;

import com.vmware.vim.idp.exception.DuplicateIdpNameException;
import com.vmware.vim.idp.exception.IdpNotFoundException;
import com.vmware.vim.idp.exception.NoPermissionException;
import com.vmware.vim.idp.exception.NotAuthenticatedException;

/**
 * Identity provider ( IDP ) management interface.<br>
 * IDPs are identified by their names. When a new IDP is created, a set of
 * associated services are automatically spawned. These services are also
 * automatically dropped when IDP is deleted.
 * <p>
 * Important note: Implementations are not required to guarantee that endpoints
 * returned as result of creation or list of IDPs are accessible!
 *
 */
public interface IdpManagement {

   /**
    * Creates an IDP.
    * <p>
    * The administrative user account will be created at the IDP's system
    * domain.
    * <p>
    * The following services will be returned:
    * <ul>
    * <li>Security token service with service type <code>urn:sso:sts</code></li>
    * <li>SSO administrative service with service type
    * <code>urn:sso:admin</code></li>
    * <li>Groupcheck service with service type <code>urn:sso:groupcheck</code></li>
    * <li>SAML metadata endpoint with service type <code>urn:sso:metadata</code>
    * </li>
    * </ul>
    *
    * @param idpName
    *           IDP name; should not match any other existing IDP name;
    *           {@code not-null} and {@code not-empty} string value is required
    * @param adminUser
    *           administrative account name in UPN format; {@code not-null} and
    *           {@code not-empty} string value is required
    * @param adminPass
    *           administrative account password; {@code not-null} and non-empty array
    *           value is required
    *
    * @return {@code not-empty} set of all the services automatically spawned on
    *         IDP creation
    *
    * @throws DuplicateIdpNameException
    *            when there is already an IDP with the specified name
    * @throws NotAuthenticatedException
    *            there is no authenticated SSO user associated with this method
    *            call
    * @throws NoPermissionException
    *            the required privilege for calling this method is not held by
    *            the caller
    *
    */
   @Privilege(Role.Administrator)
   Set<IdpService> createIdp(String idpName, String adminUser, char[] adminPass)
      throws DuplicateIdpNameException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Lists all the services associated with the specified IDP.<br>
    * The result is the same as when {@link #createIdp} is called with the same
    * IDP name.
    *
    * @param idpName
    *           an existing IDP name; {@code not-null} and {@code not-empty}
    *           string value is required
    *
    * @return {@code not-empty} set of all the services automatically spawned on
    *         IDP creation
    *
    * @throws IdpNotFoundException
    *            there is no IDP with the specified name
    * @throws NotAuthenticatedException
    *            there is no authenticated SSO user associated with this method
    *            call
    * @throws NoPermissionException
    *            the required privilege for calling this method is not held by
    *            the caller
    */
   @Privilege(Role.Administrator)
   Set<IdpService> listServices(String idpName) throws IdpNotFoundException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Deletes the IDP with specific name.
    *
    * @param idpName
    *           the name of an existing IDP which to delete; {@code not-null}
    *           and {@code not-empty} string value is required
    *
    * @throws IdpNotFoundException
    *            there is no IDP with the specified name
    * @throws NotAuthenticatedException
    *            there is no authenticated SSO user associated with this method
    *            call
    * @throws NoPermissionException
    *            the required privilege for calling this method is not held by
    *            the caller
    */
   @Privilege(Role.Administrator)
   void deleteIdp(String idpName) throws IdpNotFoundException,
      NotAuthenticatedException, NoPermissionException;

}
