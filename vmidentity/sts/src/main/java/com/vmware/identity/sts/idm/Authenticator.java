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
package com.vmware.identity.sts.idm;

import java.security.cert.X509Certificate;

import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.sts.NoSuchIdPException;

/**
 * Insert your comment for Authenticator here
 */
public interface Authenticator {

   /**
    *
    * @param userUPN
    *           name of the principal to authenticate
    * @param password
    * @return not null identifier of the authenticated principal
    * @throws PasswordExpiredException
    *            when the password of the user who is trying to authenticate is
    *            expired.
    * @throws LockedUserAccountException
    *            when the user account is locked.
    * @throws InvalidCredentialsException
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws SystemException
    */
   PrincipalId authenticate(String userUPN, String password)
      throws PasswordExpiredException, LockedUserAccountException,
      InvalidCredentialsException, NoSuchIdPException, SystemException;

   /**
    * Performs GSS authentication. Currently, Kerberos is assumed only.
    *
    * NB! This interface is intended to support only authentication mechanisms
    * that could finish in one client-server loop only.
    *
    * @param gssTicket
    *           raw bytes of the ticket. Note that they are not encoded, e.g.
    *           Byte64
    * @return not null GSS result. If might indicate for already established or
    *         not security context.
    * @throws InvalidCredentialsException
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws SystemException
    */
   GSSResult authenticate(String contextId, byte[] gssTicket) throws InvalidCredentialsException,
      NoSuchIdPException, SystemException;

   /**
    * Performs user certificate authentication.
    *
    * @param x509CertificateChain
    *           user certificate chain
    * @return not null identifier of the authenticated principal
    * @throws UserCertificateValidateException
    *           if user certificate validation fails
    * @throws InvalidCredentialsException
    *           if credential is invalid
    * @throws NoSuchIdPException
    *           if the IdP is missing
    * @throws SystemException
    */
   PrincipalId authenticate(X509Certificate[] x509CertificateChain)
      throws UserCertificateValidateException, InvalidCredentialsException,
      NoSuchIdPException, SystemException;

   /**
    * Performs RSA securID authentication.
    *
    * @param username
    *           username
    * @param sessionID
    *           session ID
    * @param passcode
    *           passcode
    * @return not null identifier of the authenticated principal
    * @throws IdmSecureIDNewPinException
    *           if SecurID asks for a new PIN
    * @throws InvalidCredentialsException
    *           if credential is invalid
    * @throws NoSuchIdPException
    *           if the IdP is missing
    * @throws SystemException
    */
   RSAAMResult authenticate(String username, String sessionID, String passcode)
      throws IdmSecureIDNewPinException, InvalidCredentialsException,
      NoSuchIdPException, SystemException;
}
