/* **********************************************************************
 * Copyright 2010-2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin;

import java.security.PrivateKey;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.List;
import java.util.Set;

import org.w3c.dom.Document;

import com.vmware.vim.sso.admin.RoleManagement.NoPrivilege;
import com.vmware.vim.sso.admin.RoleManagement.Privilege;
import com.vmware.vim.sso.admin.RoleManagement.Role;
import com.vmware.vim.sso.admin.exception.CertificateDeletionException;
import com.vmware.vim.sso.admin.exception.CertChainInvalidTrustedPathException;
import com.vmware.vim.sso.admin.exception.ExternalSTSCertChainInvalidTrustedPathException;
import com.vmware.vim.sso.admin.exception.ExternalSTSExtraneousCertsInCertChainException;
import com.vmware.vim.sso.admin.exception.ExtraneousCertsInCertChainException;
import com.vmware.vim.sso.admin.exception.NoPermissionException;
import com.vmware.vim.sso.admin.exception.NoSuchConfigException;
import com.vmware.vim.sso.admin.exception.NoSuchExternalSTSConfigException;
import com.vmware.vim.sso.admin.exception.NoSuchRelyingPartyException;
import com.vmware.vim.sso.admin.exception.NotAuthenticatedException;

/**
 * SSO server configuration
 */
public interface ServerConfigurator {

   /**
    * Returns all valid signing X.509 certificate chains, known to the server.
    * This method will always return at least one chain and each chain will
    * contain at least one certificate.
    *
    * @return the set of all known X.509 certificates chains
    */
   @NoPrivilege
   Set<CertPath> getKnownCertificateChains();

   /**
    * Returns all trusted root certificates. This method will always return at
    * least one trusted certificate.
    *
    * @return the non-empty set of all X.509 certificates which are trusted
    *         roots
    * @deprecated use {@link #getIssuerCertificates}
    */
   @Deprecated
   @NoPrivilege
   Set<X509Certificate> getTrustedCertificates();

   /**
    * Returns all leaf signing certificates being used or have been used by issuers
    * (STS in single-node and multiple-node configuration). Expired signing
    * certificates are not included.
    *
    * @return non-empty set of all valid X.509 certificates being used or was
    *         used by STS for signing
    */
   @NoPrivilege
   Set<X509Certificate> getIssuersCertificates();

   /**
    * import trusted STS configuration
    *
    * @param stsConfig
    *           cannot be null
    * @throws ExternalSTSCertChainInvalidTrustedPathException
    *            when the certificate chain is invalid
    * @throws ExternalSTSExtraneousCertsInCertChainException
    *            when there are extra certificates outside of the certificate
    *            chain
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @deprecated Replaced by {@link #importExternalIDPConfiguration(Document)}.
    */
   @Privilege(Role.Administrator)
   @Deprecated
   void importTrustedSTSConfiguration(TrustedSTSConfig stsConfig)
         throws NotAuthenticatedException, NoPermissionException,
         ExternalSTSCertChainInvalidTrustedPathException,
         ExternalSTSExtraneousCertsInCertChainException;

   /**
    * remove trusted STS configuration
    *
    * @param issuerName
    *           unique ID of the trusted STS configuration to be removed
    * @throws NoSuchExternalSTSConfigException
    *            specified STS configuration is not found
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    * @deprecated Replaced by {@link #deleteExternalIDPConfiguration(String)}.
    */
   @Privilege(Role.Administrator)
   @Deprecated
   void removeTrustedSTSConfiguration(String issuerName)
         throws NotAuthenticatedException, NoPermissionException,
         NoSuchExternalSTSConfigException;

   /**
    * Import a trusted external IDP configuration.
    *
    * @param configDoc
    *           Valid configuration document for the external IDP.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws ExternalSTSCertChainInvalidTrustedPathException
    *           When the certificate chain is invalid.
    * @throws ExternalSTSExtraneousCertsInCertChainException
    *           When there are extra certificates outside of the certificate
    *           chain.
    */
   @Privilege(Role.Administrator)
   void importExternalIDPConfiguration(Document configDoc)
         throws NotAuthenticatedException, NoPermissionException,
         ExternalSTSExtraneousCertsInCertChainException,
         ExternalSTSCertChainInvalidTrustedPathException;

   /**
    * Create a trusted external IDP configuration.
    *
    * @param config
    *           Valid configuration to be created for the external IDP.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws CertChainInvalidTrustedPathException
    *           When the certificate chain is invalid.
    * @throws ExtraneousCertsInCertChainException
    *           When there are extra certificates outside of the certificate
    *           chain.
    */
   @Privilege(Role.Administrator)
   void createExternalIDPConfiguration(IDPConfiguration config)
       throws NotAuthenticatedException, NoPermissionException,
       CertChainInvalidTrustedPathException,
       ExtraneousCertsInCertChainException;

   /**
    * Fetch a trusted external IDP configuration.
    *
    * @param entityID
    *           EntityID for the external IDP configuration to be retrieved.
    * @return The external IDP's configuration object.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws NoSuchConfigException
    *           When no config can be found for {@code entityID}.
    */
   @Privilege(Role.Administrator)
   IDPConfiguration getExternalIDPConfiguration(String entityID)
       throws NotAuthenticatedException, NoPermissionException,
       NoSuchConfigException;

   /**
    * Create or update a trusted external IDP configuration.
    *
    * @param config
    *           Valid configuration to be set for the external IDP.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws CertChainInvalidTrustedPathException
    *           When the certificate chain is invalid.
    * @throws ExtraneousCertsInCertChainException
    *           When there are extra certificates outside of the certificate
    *           chain.
    */
   @Privilege(Role.Administrator)
   void setExternalIDPConfiguration(IDPConfiguration config)
       throws NotAuthenticatedException, NoPermissionException,
       CertChainInvalidTrustedPathException,
       ExtraneousCertsInCertChainException;

   /**
    * Delete a trusted external IDP configuration.
    *
    * @param entityID
    *           EntityID for the external IDP configuration to be deleted.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws NoSuchConfigException
    *           When no config can be found for {@code entityID}.
    */
   @Privilege(Role.Administrator)
   void deleteExternalIDPConfiguration(String entityID)
       throws NotAuthenticatedException, NoPermissionException,
       NoSuchConfigException;

   /**
    * Delete a trusted external IDP configuration and its JIT users.
    *
    * @param entityID
    *           EntityID for the external IDP configuration and JIT users to be deleted.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws NoSuchConfigException
    *           When no config can be found for {@code entityID}.
    */
   @Privilege(Role.Administrator)
   void deleteExternalIDPConfigurationAndUsers(String entityID)
       throws NotAuthenticatedException, NoPermissionException,
       NoSuchConfigException;

   /**
    * Check if JIT is enabled for a trusted external IDP.
    *
    * @param entityID
    *           EntityID for the external IDP configuration to be retrieved.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    * @throws NoSuchConfigException
    *           When no config can be found for {@code entityID}.
    */
   @Privilege(Role.Administrator)
   boolean isExternalIDPJitEnabled(String entityID)
       throws NotAuthenticatedException, NoPermissionException, NoSuchConfigException;

   /**
    * Set Jit attribute for a trusted external IDP configuration.
    *
    * @param entityID
    *           EntityID for the external IDP configuration to be retrieved.
    * @param enableJit
    *           Boolean value to be set for the external IDP.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    */
   @Privilege(Role.Administrator)
   void setExternalIDPJitAttribute(String entityID, boolean enableJit)
       throws NotAuthenticatedException, NoPermissionException;

   /**
    * Fetch a list of all external IDP entity IDs.
    *
    * @return set of all external IDP entity IDs, null if no external IDPs are configured.
    */
   @Privilege(Role.Administrator)
   List<String> enumerateExternalIDPEntityIDs()
       throws NotAuthenticatedException, NoPermissionException;

   /**
    * Return list of external IDPs' certificate chains configured.
    *
    * @return non-null set of all X.509 certificates. Empty list if none
    *         external IDPs are configured.
    * @throws NotAuthenticatedException
    *           When there is no authenticated SSO user associated with this
    *           method call.
    * @throws NoPermissionException
    *           When the required privilege for calling this method is not held
    *           by the caller.
    */
   @NoPrivilege
   Set<CertPath> getExternalIdpTrustedCertificateChains();

   /**
    * Return the certificate chain for external IDP specified by entityId in the configuration setup.
    * @return non-null set of all X.509 certificates. Empty list if none external
    * IDPs are configured.
    *
    * @param entityId the issuer cannot be null
    * @return certPath of the externalIDP, null if not found
    */
   @NoPrivilege
   CertPath getExternalIdpTrustedCertificateChain(String entityId);

   /**
    * Delete trusted root certificate.
    *
    * @param fingerprint
    *           SHA-1 fingerprint of the certificate to delete; case
    *           insensitive; {@code not-null} and {@code not-empty} string value
    *           is required
    * @return {@code false} when there is no certificate with given fingerprint,
    *         otherwise delete that certificate and return {@code true}
    *
    * @throws CertificateDeletionException
    *            on attempt to delete root certificate of current signing chain
    *            on current or replicated SSO nodes
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   boolean deleteTrustedCertificate(String fingerprint)
      throws CertificateDeletionException, NotAuthenticatedException,
      NoPermissionException;

   /**
    * Sets the server's token signing key and X.509 certificate chain. After
    * successful call, the Security Token Service will start signing all issued
    * tokens with the provided key. The provided certificate chain will be
    * embedded into the tokens signature's &lt;ds:KeyInfo&gt; element.<br>
    * The certificate corresponding to the signingKey must be the first
    * certificate in the signing chain.
    *
    * @param signingKey
    *           The private key to use to sign tokens with; should be RSA
    *           private key. Must not be null.
    * @param signingCertificateChain
    *           The certificate chain to embed in the token signature's KeyInfo.
    *           If should be ordered with the signing certificate (i.e. the
    *           certificate with public key corresponding to the signing key)
    *           first, optionally followed by any number of certificate
    *           authority certificates.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    *
    * @see X509Certificate
    */
   @Privilege(Role.Administrator)
   void setNewSignerIdentity(PrivateKey signingKey,
      CertPath signingCertificateChain) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Returns the maximum allowed clock discrepancy between the client and the
    * server machine when acquiring a token.
    *
    * @return The maximum clock discrepancy in milliseconds.
    */
   @NoPrivilege
   long getClockTolerance();

   /**
    * Changes the maximum allowed clock discrepancy between the client and the
    * server machines when acquiring a token. If a token request is older than
    * the server's current time minus the allowed tolerance, it is discarded.
    *
    * @param milliseconds
    *           The maximum accepted discrepancy in milliseconds.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setClockTolerance(long milliseconds) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Return how many times a token can be delegated
    *
    * @return a positive integer for token delegation count
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   int getDelegationCount() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Set how many times a token can be delegated
    *
    * @param delegationCount
    *           token delegation count; a positive integer value is required
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setDelegationCount(int delegationCount)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Return how many times a token could be renewed
    *
    * @return a positive integer for token renew count
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   int getRenewCount() throws NotAuthenticatedException, NoPermissionException;

   /**
    * Set how many times a token can be renewed
    *
    * @param renewCount
    *           token renew count; a positive integer value is required
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setRenewCount(int renewCount) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Return the maximum lifetime (in seconds) for bearer tokens
    *
    * @return a positive integer for token lifetime
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   long getMaximumBearerTokenLifetime() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Set the maximum lifetime for bearer tokens
    *
    * @param maxLifetime
    *           max token lifetime (in seconds); a positive integer value is
    *           required
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setMaximumBearerTokenLifetime(long maxLifetime)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Return the maximum lifetime (in seconds) for holder-of-key tokens
    *
    * @return a positive integer for token lifetime
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   long getMaximumHoKTokenLifetime() throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Set the maximum lifetime for holder-of-key tokens
    *
    * @param maxLifetime
    *           max token lifetime (in seconds); a positive integer value is
    *           required
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void setMaximumHoKTokenLifetime(long maxLifetime)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Returns the server's current password expiration configuration. Not
    * <code>null</code>
    *
    * @return The configuration.
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.RegularUser)
   public PasswordExpirationConfig getPasswordExpirationConfiguration()
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Replaces the server's current password expiration configuration with the
    * new one
    *
    * @param config
    *           The new password expiration configuration. <code>null</code>
    *           values are not acceptable
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   public void updatePasswordExpirationConfiguration(
      PasswordExpirationConfig config) throws NotAuthenticatedException,
      NoPermissionException;

   /**
    * Import service provider (SP) and/or identity provider (IDP) configuration.
    *
    * @param samlConfigDoc
    *           DOM document containing the IDP configuration; {@code not-null}
    *           value is required
    *
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   public void importSAMLMetadata(Document samlConfigDoc)
      throws NotAuthenticatedException, NoPermissionException;

   /**
    * Removes a relying party with specific name.
    *
    * @param rpName
    *           name of relying party; case-insensitive; {@code not-null} and
    *           {@code not-empty} value is required
    * @throws NoSuchRelyingPartyException
    *            if relying party with given name does not exist
    * @throws NotAuthenticatedException
    *            when there is no authenticated SSO user associated with this
    *            method call
    * @throws NoPermissionException
    *            when the required privilege for calling this method is not held
    *            by the caller
    */
   @Privilege(Role.Administrator)
   void deleteRelyingParty(String rpName) throws NoSuchRelyingPartyException,
      NotAuthenticatedException, NoPermissionException;

   /**
    * Returns the issuer identity.
    *
    * @return the issuer identity of the STS instance
    */
   @NoPrivilege
   String getIssuerName();

    /**
     * Set the AuthnPolicy for the tenant.
     *
     * @param anthnPolicy
     * @throws NotAuthenticatedException
     *             when there is no authenticated SSO user associated with this
     *             method call
     * @throws NoPermissionException
     *             when the required privilege for calling this method is not
     *             held by the caller
     */
    @Privilege(Role.Administrator)
    public void setAuthnPolicy(AuthnPolicy anthnPolicy)
            throws NotAuthenticatedException, NoPermissionException;

    /**
     * Get the AuthnPolicy for the tenant.
     *
     * @return the AuthnPolicy instance
     * @throws NotAuthenticatedException
     *             when there is no authenticated SSO user associated with this
     *             method call
     * @throws NoPermissionException
     *             when the required privilege for calling this method is not
     *             held by the caller
     */
    @Privilege(Role.RegularUser)
    public AuthnPolicy getAuthnPolicy()
            throws NotAuthenticatedException, NoPermissionException;
}
