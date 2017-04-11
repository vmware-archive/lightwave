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
package com.vmware.identity.sts.idm.impl;

import java.security.cert.X509Certificate;
import java.util.concurrent.TimeUnit;

import javax.security.auth.login.LoginException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CertificateRevocationCheckException;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.IdmCertificateRevokedException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.idm.Authenticator;
import com.vmware.identity.sts.idm.IdmSecureIDNewPinException;
import com.vmware.identity.sts.idm.InvalidCredentialsException;
import com.vmware.identity.sts.idm.InvalidPrincipalException;
import com.vmware.identity.sts.idm.LockedUserAccountException;
import com.vmware.identity.sts.idm.PasswordExpiredException;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.STSConfigExtractor;
import com.vmware.identity.sts.idm.STSConfiguration;
import com.vmware.identity.sts.idm.SsoStatisticsService;
import com.vmware.identity.sts.idm.SystemException;
import com.vmware.identity.sts.idm.UserCertificateValidateException;
import com.vmware.identity.util.PerfConstants;

/**
 * Insert your comment for AuthenticatorImpl here
 *
 * Thread safety: not thread safe. Intended for use within single thread
 * context.
 */
final class AuthenticatorImpl implements Authenticator, STSConfigExtractor,
   PrincipalDiscovery, SsoStatisticsService {

   private final static IDiagnosticsLogger perfLog = DiagnosticsLoggerFactory
      .getLogger(PerfConstants.PERF_LOGGER.getClass());
   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
		      .getLogger(AuthenticatorImpl.class);

   private final CasIdmClient idmClient;
   private final String tenantName;

   /**
    * @param idmClient
    * @param tenantName
    *
    */
   AuthenticatorImpl(CasIdmClient idmClient, String tenantName)
      throws SystemException {
      assert idmClient != null;
      assert tenantName != null;

      this.idmClient = idmClient;
      this.tenantName = tenantName;
   }

   @Override
   public PrincipalId authenticate(String userUPN, String password)
      throws InvalidCredentialsException, NoSuchIdPException,
      LockedUserAccountException, PasswordExpiredException, SystemException {

      final PrincipalId result;
      try {
         final long startedAt = now();
         result = idmClient.authenticate(tenantName, userUPN, password);
         perfLog.trace("'idm.authenticateByUserPass' took {} ms.",
            now() - startedAt);
      } catch (UserAccountLockedException e) {
         throw new LockedUserAccountException(
            "The account of the user trying to authenticate is locked.", e);
      } catch (com.vmware.identity.idm.PasswordExpiredException e) {
         throw new PasswordExpiredException(
            "Password of the user logging on is expired.", e);
      } catch (IDMLoginException e) {
         throw new InvalidCredentialsException(e);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
      checkNotNull(result);
      return result;
   }

   @Override
   public GSSResult authenticate(String contextId, byte[] gssTicket)
      throws InvalidCredentialsException, NoSuchIdPException, SystemException {

      final GSSResult result;
      try {
         final long startedAt = now();
         result = idmClient.authenticate(tenantName, contextId, gssTicket);
         perfLog.trace("'idm.authenticateByGSS' took {} ms.", now()
            - startedAt);
      } catch (LoginException e) {
         throw new InvalidCredentialsException(e);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
      checkNotNull(result);
      return result;
   }

   @Override
   public PrincipalId authenticate(X509Certificate[] x509CertificateChain)
      throws UserCertificateValidateException, InvalidCredentialsException, NoSuchIdPException, SystemException {

      final PrincipalId result;
      try {
         final long startedAt = now();
         result = idmClient.authenticate(tenantName, x509CertificateChain, null); // PR 1747182
         perfLog.trace("'idm.authenticateByUserCertificate' took {} ms.", now() - startedAt);
      } catch (CertificateRevocationCheckException e) {
         throw new UserCertificateValidateException("Revocation check fails to determine the certificate status.", e);
      } catch (IdmCertificateRevokedException e) {
         throw new UserCertificateValidateException("Certificate is revoked.", e);
      } catch (InvalidArgumentException e) {
         throw new UserCertificateValidateException("Certificate check parameter was incorrectly set.", e);
      } catch (IDMLoginException e) {
         throw new InvalidCredentialsException(e);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
      checkNotNull(result);
      return result;
   }

   @Override
   public RSAAMResult authenticate(String username, String sessionID, String passcode)
      throws IdmSecureIDNewPinException, InvalidCredentialsException, NoSuchIdPException, SystemException {

      final RSAAMResult result;
      try {
         final long startedAt = now();
         result = idmClient.authenticateRsaSecurId(tenantName, sessionID, username, passcode);
         perfLog.trace("'idm.authenticateByRSASecurId' took {} ms.", now() - startedAt);
      } catch (IDMSecureIDNewPinException e) {
         throw new IdmSecureIDNewPinException("SecurID asks for a new PIN.", e);
      } catch (IDMLoginException e) {
         throw new InvalidCredentialsException(e);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
      checkNotNull(result);
      return result;
   }

   @Override
   public STSConfiguration getConfig() throws NoSuchIdPException,
      SystemException {

      final long clockTolerance;
      try {
         final long startedAt = now();
         clockTolerance = idmClient.getClockTolerance(tenantName);
         perfLog.trace("'idm.getClockTolerance' took {} ms.", now()
            - startedAt);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }

      return new STSConfiguration(clockTolerance);
   }

   @Override
   public SolutionUser findSolutionUser(String subjectDN)
      throws NoSuchIdPException, SystemException {
      try {
         final long startedAt = now();
         final SolutionUser solutionUser = idmClient.findSolutionUserByCertDn(
            tenantName, subjectDN);
         perfLog.trace(
            "'idm.findSolutionUserByCertDn' took {} ms.", now()
               - startedAt);
         return solutionUser;
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
   }

   @Override
   public SolutionUser findSolutionUserByName(String name)
      throws NoSuchIdPException, SystemException {

      try {
         final long startedAt = now();
         final SolutionUser solutionUser = idmClient.findSolutionUser(
            tenantName, name);
         perfLog.trace("'idm.findSolution' took {} ms.", now()
            - startedAt);
         return solutionUser;
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (RuntimeException e) {
         throw new SystemException(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
   }

   @Override
   public boolean isMemberOfSystemGroup(PrincipalId principalId, String groupName)
      throws InvalidPrincipalException, SystemException {

      final boolean isMemberOfSystemGroup;
      try {
         final long startedAt = now();
         isMemberOfSystemGroup = idmClient.isMemberOfSystemGroup(tenantName,
            principalId, groupName);
         perfLog.trace("'idm.isMemberOfSystemGroup' took {} ms.",
            now() - startedAt);
      } catch (com.vmware.identity.idm.InvalidPrincipalException e) {
         throw new InvalidPrincipalException(e);
      } catch (NoSuchTenantException e) {
         throw noSuchIdPExc(e);
      } catch (Exception e) {
         throw new SystemException(e);
      }
      return isMemberOfSystemGroup;
    }

    @Override
    public void incrementGeneratedTokens() {
        try {
            final long startedAt = now();
            idmClient.incrementGeneratedTokens(tenantName);
            log.debug("'idm.incrementGeneratedTokens' took {} ms.", now()
                    - startedAt);
        } catch (Exception e) {
            log.error("Caught exception while incrementing generated tokens. {}", e);
        }
    }

    @Override
    public void incrementRenewedTokens() {
        try {
            final long startedAt = now();
            idmClient.incrementRenewedTokens(tenantName);
            log.debug("'idm.incrementRenewedTokens' took {} ms.", now()
                    - startedAt);
        } catch (Exception e) {
            log.error("Caught exception while incrementing renewed tokens. {}", e);
        }
    }

   private void checkNotNull(Object result) {
      if (result == null) {
         throw new SystemException("Bug: Non respected contract!");
      }
   }

   private NoSuchIdPException noSuchIdPExc(NoSuchTenantException e) {
      return new NoSuchIdPException("IdP '" + tenantName + "' not found.", e);
   }

   private long now() {
      return TimeUnit.NANOSECONDS.toMillis(System.nanoTime());
   }
}
