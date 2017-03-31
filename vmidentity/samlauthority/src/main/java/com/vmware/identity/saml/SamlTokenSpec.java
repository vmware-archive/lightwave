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
package com.vmware.identity.saml;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.Validate;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.util.TimePeriod;

/**
 * Represents an immutable specification which is used for building a SAML
 * token.
 */
public final class SamlTokenSpec {

   private final TimePeriod lifespan;
   private final Confirmation confirmationData;
   private final AuthenticationData authnData;
   private final DelegationSpec delegationSpec;
   private final RenewSpec renewSpec;
   private final Set<String> audience;
   private final List<Advice> adviceRequested;
   private final List<Advice> advicePresent;
   private final Collection<String> attributeNames;
   private final SignatureAlgorithm signatureAlgorithm;

   private SamlTokenSpec(TimePeriod lifespan, Confirmation confirmation,
      AuthenticationData authnData, DelegationSpec delegationSpec,
      RenewSpec renewSpec, Set<String> audience, List<Advice> adviceRequested,
      List<Advice> advicePresent, Collection<String> attributeNames,
      SignatureAlgorithm signatureAlgorithm) {
      Validate.notNull(lifespan);
      Validate.notNull(confirmation);
      Validate.notNull(authnData);
      Validate.notNull(delegationSpec);
      Validate.notNull(renewSpec);
      Validate.notNull(audience);
      Validate.notNull(adviceRequested);
      Validate.notNull(advicePresent);
      Validate.notNull(attributeNames);

      this.lifespan = lifespan;
      this.confirmationData = confirmation;
      this.authnData = authnData;
      this.delegationSpec = delegationSpec;
      this.renewSpec = renewSpec;
      // no need to copy the next four collections since all clients of this
      // ctor are internal and they already have copied them
      this.audience = audience;
      this.adviceRequested = adviceRequested;
      this.advicePresent = advicePresent;
      this.attributeNames = attributeNames;
      this.signatureAlgorithm = signatureAlgorithm;
   }

   /**
    * @return authentication data. Cannot be null.
    */
   public AuthenticationData getAuthenticationData() {
      return authnData;
   }

   /**
    * @return desired token lifespan. Cannot be null.
    */
   public TimePeriod getLifespan() {
      return lifespan;
   }

   /**
    * @return token confirmation data. Cannot be null.
    */
   public Confirmation getConfirmationData() {
      return confirmationData;
   }

   /**
    * @return the delegation specification. Cannot be null;
    */
   public DelegationSpec getDelegationSpec() {
      return delegationSpec;
   }

   /**
    * @return the renew specification. Cannot be null;
    */
   public RenewSpec getRenewSpec() {
      return renewSpec;
   }

   /**
    * @return the audience. Empty set means no audience. Cannot be null.
    */
   public Set<String> getAudience() {
      return new HashSet<String>(audience);
   }

   /**
    * @return the newly requested advice. Empty list means no advice. Cannot be
    *         null.
    */
   public List<Advice> getRequestedAdvice() {
      return new ArrayList<Advice>(adviceRequested);
   }

   /**
    * @return the advice already present in the present token. Empty list means
    *         no advice. Cannot be null.
    */
   public List<Advice> getPresentAdvice() {
      return new ArrayList<Advice>(advicePresent);
   }

   /**
    * @return attributes to be included in the token. Cannot be null.
    */
   public Collection<String> getAttributeNames() {
      return new HashSet<String>(attributeNames);
   }

   /**
    * @return signature algorithm which should be used for signing the token.
    *         Can be null.
    */
   public SignatureAlgorithm getSignatureAlgorithm() {
      return signatureAlgorithm;
   }

   @Override
   public String toString() {
      return String
         .format(
            "SamlTokenSpec [lifespan=%s, confirmation=%s, "
               + "authentication=%s, delegationSpec=%s, renewSpec=%s, audience=%s, adviceReq=%s, advicePresent=%s, attributeNames=%s, signatureAlgorithm=%s]",
            lifespan, confirmationData, authnData, delegationSpec, renewSpec,
            audience, adviceRequested, advicePresent, attributeNames,
            signatureAlgorithm);
   }

@Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + authnData.hashCode();
      result = prime * result + confirmationData.hashCode();
      result = prime * result + lifespan.hashCode();
      result = prime * result + delegationSpec.hashCode();
      result = prime * result + renewSpec.hashCode();
      result = prime * result + audience.hashCode();
      result = prime * result + adviceRequested.hashCode();
      result = prime * result + advicePresent.hashCode();
      result = prime * result + attributeNames.hashCode();
      result = prime * result + ObjectUtils.hashCode(signatureAlgorithm);
      return result;
   }

   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }
      if (obj == null || this.getClass() != obj.getClass()) {
         return false;
      }

      SamlTokenSpec other = (SamlTokenSpec) obj;
      return authnData.equals(other.authnData)
         && confirmationData.equals(other.confirmationData)
         && lifespan.equals(other.lifespan)
         && delegationSpec.equals(other.delegationSpec)
         && renewSpec.equals(other.renewSpec)
         && audience.equals(other.audience)
         && adviceRequested.equals(other.adviceRequested)
         && advicePresent.equals(other.advicePresent)
         && attributeNames.equals(other.attributeNames)
         && ObjectUtils.equals(signatureAlgorithm, other.signatureAlgorithm);
   }

   public boolean requesterIsTokenOwner() {
      return (delegationSpec.getDelegationHistory() == null)
         || authnData.getPrincipalId().equals(
            delegationSpec.getDelegationHistory().getTokenSubject());
   }

   /**
    * Represents the token confirmation data.
    */
   public static class Confirmation {

      private final ConfirmationType type;
      private final String inResponseTo;
      private final String recipient;
      private final X509Certificate certificate;

      /**
       * Creates BEARER type confirmation.
       */
      public Confirmation() {
         this(null, null);
      }

      /**
       * Creates BEARER type confirmation.
       *
       * @param inResponseTo
       *           optional
       * @param recipient
       *           optional
       */
      public Confirmation(String inResponseTo, String recipient) {
         this.type = ConfirmationType.BEARER;
         this.inResponseTo = inResponseTo;
         this.recipient = recipient;
         this.certificate = null;
      }

      /**
       * Creates HOK type confirmation.
       *
       * @param certificate
       *           the HolderOfKey certificate. Cannot be null.
       */
      public Confirmation(X509Certificate certificate) {
         Validate.notNull(certificate);

         this.type = ConfirmationType.HOLDER_OF_KEY;
         this.inResponseTo = null;
         this.recipient = null;
         this.certificate = certificate;
      }

      /**
       * @return token confirmation type. Cannot be null.
       */
      public ConfirmationType getType() {
         return type;
      }

      /**
       * @return 'in-response-to' attribute. Can be null.
       */
      public String getInResponseTo() {
         return inResponseTo;
      }

      /**
       * @return recipient. Can be null.
       */
      public String getRecipient() {
         return recipient;
      }

      /**
       * @return client certificate. Cannot be null if Confirmation type is
       *         HolderOfKey.
       */
      public X509Certificate getCertificate() {
         return certificate;
      }

      @Override
      public String toString() {
         return "Confirmation [type=" + type + ", inResponseTo=" + inResponseTo
            + ", recipient=" + recipient + ", certificate=" + certificate + "]";
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + type.hashCode();
         result = prime * result + ObjectUtils.hashCode(inResponseTo);
         result = prime * result + ObjectUtils.hashCode(recipient);
         result = prime * result + ObjectUtils.hashCode(certificate);
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (this == obj) {
            return true;
         }
         if (obj == null || this.getClass() != obj.getClass()) {
            return false;
         }

         Confirmation other = (Confirmation) obj;
         return type == other.type
            && ObjectUtils.equals(inResponseTo, other.inResponseTo)
            && ObjectUtils.equals(recipient, other.recipient)
            && ObjectUtils.equals(certificate, other.certificate);
      }

   }

   /**
    * Describes the token's confirmation type.
    */
   public static enum ConfirmationType {
      /**
       * Confirmation type indicating that the token does not contain
       * requester-specific confirmation information.
       */
      BEARER("http://docs.oasis-open.org/ws-sx/ws-trust/200512/Bearer"),

      /**
       * Confirmation type indicating that the requester's certificate is
       * embedded into the token. This allows the requester to prove its right
       * to use the token by signing a message containing the token using the
       * private key corresponding to the certificate.
       */
      HOLDER_OF_KEY(
         "http://docs.oasis-open.org/ws-sx/ws-trust/200512/PublicKey");

      private final String uri;

      private ConfirmationType(String uri) {
         this.uri = uri;
      }

      @Override
      public String toString() {
         return uri;
      }
   }

   /**
    * Represents delegation-related information that is to be used to determine
    * the ProxyRestriction and delegates list of the issued token
    */
   public static final class DelegationSpec {

      private final PrincipalId delegate;
      private final boolean delegable;
      private final DelegationHistory history;

      /**
       * This should be used when there is no template token that token issuance
       * should be based on.
       *
       * @param delegate
       *           the principal to which the resulting token should be
       *           delegated. Not required.
       * @param delegable
       *           true if the token should be delegable
       */
      public DelegationSpec(PrincipalId delegate, boolean delegable) {
         this.delegate = delegate;
         this.delegable = delegable;
         this.history = null;
      }

      /**
       * This should be used when there is a template token that token issuance
       * should be based on.
       *
       * @param delegate
       *           the principal to which the resulting token should be
       *           delegated. Not required.
       * @param delegable
       *           true if the token should be delegable
       * @param delegationHistory
       *           the template token that should be used as a base for
       *           delegation. It could be either the authentication token when
       *           'delegateTo' or act as token when 'actAs', required
       */
      public DelegationSpec(PrincipalId delegate, boolean delegable,
         DelegationHistory delegationHistory) {

         Validate.notNull(delegationHistory);
         this.delegate = delegate;
         this.delegable = delegable;
         this.history = delegationHistory;
      }

      /**
       * @return the principal to which the resulting token should be delegated.
       *         Can be null.
       */
      public PrincipalId getDelegate() {
         return delegate;
      }

      /**
       * @return true if the token should be delegable
       */
      public boolean isDelegable() {
         return delegable;
      }

      /**
       * @return the template token that should be used as a base for
       *         delegation. It could be either the authentication token when
       *         'delegateTo' or act as token when 'actAs', could be null
       */
      public DelegationHistory getDelegationHistory() {
         return history;
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + ObjectUtils.hashCode(delegate);
         result = prime * result + (delegable ? 1231 : 1237);
         result = prime * result + ObjectUtils.hashCode(history);
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (this == obj) {
            return true;
         }
         if (obj == null || this.getClass() != obj.getClass()) {
            return false;
         }

         DelegationSpec other = (DelegationSpec) obj;
         return ObjectUtils.equals(delegate, other.delegate)
            && delegable == other.delegable
            && ObjectUtils.equals(history, other.history);
      }

      @Override
      public String toString() {
         return String.format(
            "DelegationSpec [delegate=%s, delegable=%s, history=%s]", delegate,
            delegable, history);
      }

      /**
       * This class represents the history with respect to delegation. This info
       * is conveyed in tokens that serve as a template for delegation.
       */
      public static final class DelegationHistory {

         private final PrincipalId subject;
         private final int remainingDelegations;
         private final List<TokenDelegate> currentDelegateList;
         private final Date delegatedTokenExpires;

         /**
          * @param subject
          *           subject of the template token
          * @param currentDelegateList
          *           required but possibly empty.
          * @param remainingDelegations
          * @param tokenExpires
          *           required
          */
         public DelegationHistory(PrincipalId subject,
            List<TokenDelegate> currentDelegateList, int remainingDelegations,
            Date tokenExpires) {
            Validate.notNull(subject);
            Validate.notNull(currentDelegateList);
            Validate.isTrue(remainingDelegations >= 0);
            Validate.notNull(tokenExpires);

            this.subject = subject;
            this.currentDelegateList = currentDelegateList;
            this.remainingDelegations = remainingDelegations;
            this.delegatedTokenExpires = tokenExpires;
         }

         /**
          * @return the remainingDelegations
          */
         public int getRemainingDelegations() {
            return remainingDelegations;
         }

         /**
          * @return the currentDelegateList. not null but possibly empty.
          */
         public List<TokenDelegate> getCurrentDelegateList() {
            return currentDelegateList;
         }

         /**
          * @return the delegatedTokenExpires not null
          */
         public Date getDelegatedTokenExpires() {
            return delegatedTokenExpires;
         }

         /**
          * @return subject of the template token, nut null
          */
         public PrincipalId getTokenSubject() {
            return subject;
         }

         @Override
         public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + subject.hashCode();
            result = prime * result + currentDelegateList.hashCode();
            result = prime * result + remainingDelegations;
            result = prime * result + delegatedTokenExpires.hashCode();

            return result;
         }

         @Override
         public boolean equals(Object obj) {
            if (this == obj) {
               return true;
            }
            if (obj == null || this.getClass() != obj.getClass()) {
               return false;
            }

            DelegationHistory other = (DelegationHistory) obj;
            return subject.equals(other.subject)
               && currentDelegateList.equals(other.currentDelegateList)
               && remainingDelegations == other.remainingDelegations
               && delegatedTokenExpires.equals(other.delegatedTokenExpires);
         }

         @Override
         public String toString() {
            return String
               .format(
                  "DelegationHistory [subject=%s, remainingDelegations=%s, currentDelegateList=%s, delegatedTokenExpires=%s]",
                  subject, remainingDelegations, currentDelegateList,
                  delegatedTokenExpires);
         }

      }
   }

   /**
    * Represents a token delegate
    */
   public static final class TokenDelegate {

      private final PrincipalId subject;
      private final Date delegationDate;

      /**
       * @param subject
       *           required
       * @param delegationDate
       *           required
       */
      public TokenDelegate(PrincipalId subject, Date delegationDate) {
         Validate.notNull(subject);
         Validate.notNull(delegationDate);

         this.subject = subject;
         this.delegationDate = delegationDate;
      }

      /**
       * @return the delegation subject. not null.
       */
      public PrincipalId getSubject() {
         return subject;
      }

      /**
       * @return the date when the delegation took place. not null.
       */
      public Date getDelegationDate() {
         return delegationDate;
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + delegationDate.hashCode();
         result = prime * result + subject.hashCode();
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (this == obj) {
            return true;
         }
         if (obj == null || this.getClass() != obj.getClass()) {
            return false;
         }

         TokenDelegate other = (TokenDelegate) obj;
         return delegationDate.equals(other.delegationDate)
            && subject.equals(other.subject);
      }

      @Override
      public String toString() {
         return String.format("TokenDelegate [subject=%s, delegationDate=%s]",
            subject, delegationDate);
      }

   }

   /**
    * Represents renew-related information that is to be used to determine the
    * RenewRestriction of the issued token
    */
   public static final class RenewSpec {
      private final boolean renewable;
      private final boolean renew;
      private final int remainingRenewables;

      /**
       * Creates a new specification suitable when issuing a completely new
       * token, i.e. no token exchange nor renew
       *
       * @param renewable
       *           whether the resulting token is requested to be renewable
       */
      public RenewSpec(boolean renewable) {
         this(renewable, false, 0);
      }

      /**
       * Creates a new specification
       *
       * @param renewable
       *           whether the resulting token is requested to be renewable
       * @param renew
       *           whether a renewing is being requested
       * @param remainingRenewables
       *           number of remaining renews
       *
       */
      public RenewSpec(boolean renewable, boolean renew, int remainingRenewables) {
         Validate.isTrue(remainingRenewables >= 0);
         this.remainingRenewables = remainingRenewables;
         this.renewable = renewable;
         this.renew = renew;
      }

      /**
       * @return whether the resulting token is requested to be renewable
       */
      public boolean isRenewable() {
         return renewable;
      }

      /**
       * @return whether a renewing is being requested
       */
      public boolean isRenew() {
         return renew;
      }

      /**
       * @return number of remaining renews
       */
      public int getRemainingRenewables() {
         return remainingRenewables;
      }

      @Override
      public String toString() {
         return String.format(
            "RenewSpec [renewable=%s, renew=%s, remaining=%s]", renewable,
            renew, remainingRenewables);
      }

      @Override
      public int hashCode() {
         final int prime = 31;
         int result = renewable ? 1 : 0;
         result = prime * result + (renew ? 1 : 0);
         result = prime * result + remainingRenewables;
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (obj == this) {
            return true;
         }

         if (!(obj instanceof RenewSpec)) {
            return false;
         }

         RenewSpec other = (RenewSpec) obj;
         return renewable == other.renewable && renew == other.renew
            && remainingRenewables == other.remainingRenewables;
      }
   }

   /**
    * This class represents the authentication data needed for constructing a
    * SAML token. Note that the authenticated principal may differ from
    * principal for which token is being issued. This happens when delegate user
    * is the token presenter.
    */
   public static final class AuthenticationData {

      private final PrincipalId principalId;
      private final Date authnTime;
      private final AuthnMethod authnMethod;
      private final String identityAttrName;
      private final String sessionIndex;
      private final Date sessionExpireDate;

      public static enum AuthnMethod {
         PASSWORD, KERBEROS, XMLDSIG, NTLM, ASSERTION, TLSCLIENT, TIMESYNCTOKEN, SMARTCARD
      }

      /**
       * @param principalId
       *           Identifier of a principal. This will be used to query
       *           attributes etc. Required.
       * @param authenticationTime
       *           time at which the authentication happened. Required.
       * @param authnMethod
       *           by what means the principal was authenticated. Required.
       * @param identityAttrName
       *           the value of which attribute to use when constructing token
       *           subject. Required.
       */
      public AuthenticationData(PrincipalId principalId, Date authnTime,
         AuthnMethod authnMethod, String identityAttrName) {

         this(principalId, authnTime, authnMethod, identityAttrName, null, null);
      }

      /**
       * @param principalId
       *           Identifier of a principal. This will be used to query
       *           attributes etc. Required.
       * @param authenticationTime
       *           time at which the authentication happened. Required.
       * @param authnMethod
       *           by what means the principal was authenticated. Required.
       * @param identityAttrName
       *           the value of which attribute to use when constructing token
       *           subject. Required.
       * @param sessionIndex
       *           specifies the index of a particular session between the
       *           principal identified by the subject and the authenticating
       *           authority. Optional.
       * @param sessionExpiredate
       *           the value after which session is not valid. Optional
       */
      public AuthenticationData(PrincipalId principalId, Date authnTime,
         AuthnMethod authnMethod, String identityAttrName, String sessionIndex, Date sessionExpireDate) {
         Validate.notNull(principalId);
         Validate.notNull(authnTime);
         Validate.notNull(authnMethod);
         Validate.notNull(identityAttrName);

         this.principalId = principalId;
         this.authnTime = authnTime;
         this.authnMethod = authnMethod;
         this.identityAttrName = identityAttrName;
         this.sessionIndex = sessionIndex;
         this.sessionExpireDate = sessionExpireDate;
      }

      /**
       * @return the principal. not null.
       */
      public PrincipalId getPrincipalId() {
         return principalId;
      }

      /**
       * @return the time at which the authentication happened.
       */
      public Date getAuthnTime() {
         return authnTime;
      }

      /**
       * @return the authnMethod. not null.
       */
      public AuthnMethod getAuthnMethod() {
         return authnMethod;
      }

      /**
       * @return the name of identity attribute. not null.
       */
      public String getIdentityAttrName() {
         return identityAttrName;
      }

      /**
       * @return the index of a particular session between the principal
       *         identified by the subject and the authenticating authority. May
       *         be null.
       */
      public String getSessionIndex() {
         return sessionIndex;
      }

      /**
       * returns the date time after which session is no more valid.
       * @return
       */
      public Date getSessionExpireDate() {
		return sessionExpireDate;
      }

	@Override
      public int hashCode() {
         final int prime = 31;
         int result = 1;
         result = prime * result + authnMethod.hashCode();
         result = prime * result + authnTime.hashCode();
         result = prime * result + principalId.hashCode();
         result = prime * result + identityAttrName.hashCode();
         result = (sessionIndex == null) ? result : prime * result
            + sessionIndex.hashCode();
         return result;
      }

      @Override
      public boolean equals(Object obj) {
         if (this == obj) {
            return true;
         }
         if (obj == null || this.getClass() != obj.getClass()) {
            return false;
         }

         AuthenticationData other = (AuthenticationData) obj;
         return authnMethod == other.authnMethod
            && authnTime.equals(other.authnTime)
            && principalId.equals(other.principalId)
            && identityAttrName.equals(other.identityAttrName)
            && ObjectUtils.equals(sessionIndex, other.sessionIndex);
      }

      @Override
      public String toString() {
         return String
            .format(
               "AuthenticationData [principalId=%s, authnTime=%s, authnMethod=%s, identityAttr=%s, sessionExpireDate=%s]",
               principalId, authnTime, authnMethod, identityAttrName, sessionExpireDate);
      }

   }

   /**
    * This class helps building SamlToken specifications
    */
   public static final class Builder {

      private final TimePeriod lifespan;
      private final Confirmation confirmationData;
      private final AuthenticationData authnData;
      private final Collection<String> attributeNames;
      private DelegationSpec delegationSpec;
      private RenewSpec renewSpec;
      private final Set<String> audience;
      private final List<Advice> adviceReq;
      private final List<Advice> advicePresent;
      private SignatureAlgorithm signatureAlgorithm;

      /**
       * @param lifespan
       *           duration of the token. Can be null, which means no desired
       *           lifespan in request.
       * @param confirmation
       *           token confirmation data. Required.
       * @param authnData
       *           authentication information. Required.
       * @param attributeNames
       *           attributes to be included in the issued token. Required.
       */
      public Builder(TimePeriod lifespan, Confirmation confirmation,
         AuthenticationData authnData, Collection<String> attributeNames) {
         Validate.notNull(confirmation);
         Validate.notNull(authnData);

         this.lifespan = lifespan != null ? lifespan : new TimePeriod(null,
            null);
         this.confirmationData = confirmation;
         this.authnData = authnData;
         this.attributeNames = new HashSet<String>(attributeNames);
         this.audience = new HashSet<String>();
         this.adviceReq = new ArrayList<Advice>();
         this.advicePresent = new ArrayList<Advice>();
      }

      /**
       * @param delegationSpec
       *           required
       * @return the same builder
       */
      public Builder setDelegationSpec(DelegationSpec delegationSpec) {
         Validate.notNull(delegationSpec);
         this.delegationSpec = delegationSpec;

         return this;
      }

      /**
       * @param renewSpec
       *           required
       * @return the same builder
       */
      public Builder setRenewSpec(RenewSpec renewSpec) {
         Validate.notNull(renewSpec);
         this.renewSpec = renewSpec;

         return this;
      }

      /**
       * @param audience
       *           required
       * @return the same builder
       */
      public Builder addAudience(String audience) {
         Validate.notNull(audience);
         this.audience.add(audience);

         return this;
      }

      /**
       * @param advice
       *           requested advice, required
       * @return the same builder
       */
      public Builder addRequestedAdvice(Advice advice) {
         return addAdviceInt(this.adviceReq, advice);
      }

      /**
       * @param advice
       *           advice already present in the authentication token, required
       * @return the same builder
       */
      public Builder addPresentAdvice(Advice advice) {
         return addAdviceInt(this.advicePresent, advice);
      }

      /**
       * @param desiredSigningAlgorithmInRequest
       *           signature algorithm to be used when signing token. can be
       *           null which means there is no desired algorithm in the
       *           request.
       * @return the same builder
       */
      public Builder setSignatureAlgorithm(
         SignatureAlgorithm desiredSigningAlgorithmInRequest) {
         Validate.notNull(desiredSigningAlgorithmInRequest);
         this.signatureAlgorithm = desiredSigningAlgorithmInRequest;

         return this;
      }

      /**
       * @return the created {@link SamlTokenSpec}. Cannot be null.
       */
      public SamlTokenSpec createSpec() {
         if (delegationSpec == null) {
            delegationSpec = new DelegationSpec(null, false);
         }

         if (renewSpec == null) {
            renewSpec = new RenewSpec(false);
         }

         return new SamlTokenSpec(lifespan, confirmationData, authnData,
            delegationSpec, renewSpec, audience, adviceReq, advicePresent,
            attributeNames, signatureAlgorithm);
      }

      private Builder addAdviceInt(List<Advice> advice, Advice newPiece) {
         Validate.notNull(newPiece);
         advice.add(newPiece);

         return this;
      }

   }
}
