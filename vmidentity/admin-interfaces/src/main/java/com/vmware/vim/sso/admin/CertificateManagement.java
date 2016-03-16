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

import java.security.cert.X509Certificate;
import java.util.Set;

import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * Allows for listing, adding and deleting certificates from a single
 * certificate store in the SSO Server.
 * <p>
 * The certificates are uniquely identified by their SHA-1 fingerprints. The
 * fingerprints are expected to be formatted like "XX:XX:...:XX" where "XX" are
 * the hexadecimal representations of the fingerprint's bytes. The fingerprints
 * are case-insensitive.
 */
public interface CertificateManagement {

   /**
    * Add a certificate to the managed trust store.
    *
    * @param certificate
    *           the X.509 certificate to add; requires {@code not-null} value
    *
    * @return {@code true} if the certificate was added successfully; {@code
    *         false} if it is already present in the managed store.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @Deprecated LDAP certificates shoudn't be managed individually. See {@link IdentitySourceManagement#updateLdap}
    */
   @Deprecated
   @Privilege(Role.Administrator)
   boolean addCertificate(X509Certificate certificate)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Returns all certificates present in the managed trust store.
    *
    * @return possibly empty set of all certificates present in the managed
    *         trust store.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   Set<X509Certificate> getAllCertificates() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Find a certificate in the managed trust store by the certificate's SHA-1
    * fingerprint.
    *
    * @param fingerprint
    *           The fingerprint of the certificate to search for. Must not be
    *           {@code null} or empty string. Refer to the class' description
    *           for the expected format of the fingerprint.
    *
    * @return certificate found or {@code null}
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   X509Certificate findCertificate(String fingerprint)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * From the managed trust store, delete the certificate identified by the
    * given SHA-1 fingerprint.
    *
    * @return {@code true} if the certificate was successfully delete; {@code
    *         false} if certificate with the specified finger print was not
    *         present in the store.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean deleteCertificate(String fingerprint)
      throws NotAuthenticatedException, NoPermissionException;
}