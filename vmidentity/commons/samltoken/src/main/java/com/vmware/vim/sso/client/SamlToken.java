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
package com.vmware.vim.sso.client;

import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.List;
import java.util.Set;

import com.vmware.vim.sso.PrincipalId;

/**
 * Class representing an instance of SAML token. Various data and the xml token
 * itself can be obtained using this class.
 */
public interface SamlToken extends XmlPresentable {

   /**
    * @return the startTime
    */
   public Date getStartTime();

   /**
    * @return the expiration time
    */
   public Date getExpirationTime();

   /**
    * @return the isRenewable
    */
   public boolean isRenewable();

   /**
    * @return the isDelegable
    */
   public boolean isDelegable();

   /**
    * @return the confirmationType
    */
   public ConfirmationType getConfirmationType();

   /**
    * A subject of a token is the principal to which the token is issued.
    * Although the token can be delegated this field will not change - it will
    * contain the {@link PrincipalId} of the initial requester.
    *
    * This method should be used only to get the subject if it is in UPN format.
    * Otherwise the more general method {@link #getSubjectNameId()} should be
    * used.
    *
    * @return the subject of the token if it is in UPN format, null otherwise.
    */
   public PrincipalId getSubject();

   /**
    * A subject of a token is the principal to which the token is issued. It
    * will be returned together with its format.
    *
    * @return the subject of the token.
    */
   public SubjectNameId getSubjectNameId();

   /**
    * @return the token ID; Cannot be <code>null</code>
    */
   public String getId();

   /**
    * The delegation chain reflects the path of the assertion through one or
    * more intermediaries that act on behalf of the subject of the assertion.
    * </br> It will be ordered from least to most recent.</br> Recommended use:
    * A relying party MUST evaluate the list of delegates, and SHOULD NOT accept
    * the assertion unless it wishes to permit each delegate to act on behalf of
    * the subject of the containing assertion.
    *
    * @return the delegation chain for this token or empty list if the token has
    *         never been delegated.
    */
   public List<TokenDelegate> getDelegationChain();

   /**
    * If the assertion is addressed to one or more specific audiences they will
    * be in this list.</br> Although a SAML relying party that is outside the
    * audiences specified is capable of drawing conclusions from an assertion,
    * the SAML asserting party explicitly makes no representation as to accuracy
    * or trustworthiness to such a party.</br> Recommended use: The assertion
    * evaluates to Valid if and only if the SAML relying party is a member of
    * one or more of the audiences specified. Empty set means no restrictions.
    *
    * @return the audience restriction set. Cannot be <code>null</code>
    */
   public Set<String> getAudience();

   /**
    * If the confirmation type of the token is holder-of-key then this method
    * will return the user's certificate.
    *
    * @return the user certificate if HoK token, <code>null</code> otherwise
    */
   public X509Certificate getConfirmationCertificate();

   /**
    * SAML assertions may have custom data that is embedded in them. The token
    * service does not give any guarantees about it and just put it there on
    * request. This data is called advice.
    *
    * @return the advice list. Cannot be <code>null</code>
    */
   public List<Advice> getAdvice();

   /**
    * Returns the group IDs the token subject belongs to. These groups could be
    * both AD groups and local groups. If the subject does not belong to any
    * group empty list will be returned.
    *
    * @return the list of groups the principal belongs to. Cannot be
    *         <code>null</code>
    * @see {@link SamlToken#getSubject()}
    */
   public List<PrincipalId> getGroupList();

   /**
    * @return <code>true</code> if this token represents a solution user. In
    *         other words - if principal of the subject to which the token is
    *         issued is a solution type of principal then this method will
    *         return <code>true</code>
    */
   public boolean isSolution();

   /**
    * For any non-null reference values x and y, this method returns
    * <code>true</code> if and only if the argument value implements this
    * interface and the corresponding token IDs match (
    * x.getId().equals(y.getId() == true )
    */
   @Override
   public boolean equals(Object other);

   /**
    * {@inheritDoc}
    */
   @Override
   public int hashCode();

   /**
    * Implementations of this class will act as containers for a single
    * intermediary/delegate represented by the assertion.
    */
   public interface TokenDelegate {

      /**
       * Identifies the delegate
       *
       * @return the delegate
       */
      public PrincipalId getSubject();

      /**
       * Supplies additional information about the act of delegation.
       *
       * @return the delegation date
       */
      public Date getDelegationDate();
   }
}