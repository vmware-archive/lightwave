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
package com.vmware.identity.sts.impl;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;

/**
 * Insert your comment for SamlTokenMock here
 */
final class SamlTokenMock implements ServerValidatableSamlToken {

   private final PrincipalId subjectId;
   private final List<SamlTokenDelegate> delChain = new ArrayList<SamlTokenDelegate>();
   private final Date expireOn;
   private final X509Certificate confCertificate;
   private final Set<String> audience;
   private final List<Advice> advice;
   private final NameId _issuer;
   private final String ISSUER = "issuer";
   private final String ISSUER_NAME_FORMAT = "urn:oasis:names:tc:SAML:2.0:nameid-format:entity";

   /**
    * Creates a not delegated token.
    *
    * @param subjectName
    * @param subjectDomain
    * @param confCertificate
    * @param expireOn
    */
   SamlTokenMock(String subjectName, String subjectDomain,
      X509Certificate confCertificate, Date expireOn) {

      this(subjectName, subjectDomain, confCertificate, expireOn, null, null,
         null, 0);
   }

   /**
    * Creates delegated token.
    *
    * @param subjectName
    * @param subjectDomain
    * @param confCertificate
    * @param expireOn
    * @param delegateName
    * @param delegateDomain
    * @param delegationDate
    * @param delChainSize
    */
   SamlTokenMock(String subjectName, String subjectDomain,
      X509Certificate confCertificate, Date expireOn,
      final String delegateName, final String delegateDomain,
      final Date delegationDate, int delChainSize) {

      this.subjectId = new PrincipalId(subjectName,
         subjectDomain);
      this.confCertificate = confCertificate;
      for (int i = 0; i < delChainSize; i++) {
          delChain.add(new SamlTokenDelegate() {

                @Override
                public Subject subject()
                {
                    return new ServerValidatableSamlToken.SubjectImpl(
                            new PrincipalId(delegateName, delegateDomain),
                            new NameIdImpl(
                                    delegateName + "@" + delegateDomain,
                                 "http://schemas.xmlsoap.org/claims/UPN"
                            )
                            ,
                            SubjectValidation.Regular
                    );
                }

                @Override
                public Date delegationInstant()
                {
                    return delegationDate;
                }
           });
      }
      this.expireOn = expireOn;
      this.audience = new HashSet<String>();
      this.advice = new ArrayList<Advice>();
      this._issuer = new NameIdImpl(ISSUER, ISSUER_NAME_FORMAT); // default format
   }

   @Override
   public List<Advice> getAdvice() {
      return advice;
   }

   @Override
   public Set<String> getAudience() {
      return audience;
   }

   @Override
   public X509Certificate getConfirmationCertificate() {
      return confCertificate;
   }

   @Override
   public ConfirmationType getConfirmationType() {
      return confCertificate == null ? ConfirmationType.BEARER
         : ConfirmationType.HOLDER_OF_KEY;
   }

   @Override
   public Date getExpirationTime() {
      return expireOn;
   }

   @Override
   public List<PrincipalId> getGroupList() {
      return null;
   }

   @Override
   public String getId() {
      return null;
   }

   @Override
   public Date getStartTime() {
      return null;
   }

   @Override
   public Subject getSubject() {
       return new ServerValidatableSamlToken.SubjectImpl(
           this.subjectId,
           new NameIdImpl(
               this.subjectId.getName() + "@" + this.subjectId.getDomain(),
               "http://schemas.xmlsoap.org/claims/UPN" ),
           SubjectValidation.Regular
       );
   }

   @Override
   public NameId getIssuerNameId()
   {
       return this._issuer;
   }

   @Override
   public boolean isDelegable() {
      return false;
   }

   @Override
   public boolean isRenewable() {
      return false;
   }

   @Override
   public boolean isSolution() {
      return false;
   }

   public void addAudience(Collection<String> audience) {
      this.audience.addAll(audience);
   }

   public void addAdvice(List<Advice> advice) {
      this.advice.addAll(advice);

   }

    @Override
    public Boolean isExternal()
    {
        return false;
    }

    @Override
    public List<SamlTokenDelegate> getDelegationChain()
    {
        return this.delChain;
    }

    @Override
    public void validate(X509Certificate[] trustedRootCertificates,
            long clockToleranceSec,
            SubjectValidatorExtractor subjectValidator) throws InvalidTokenException
    {
    }
}
