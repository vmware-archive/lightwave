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
package com.vmware.identity.saml.impl;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.UUID;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;

/**
 * Mock implementation of {@link ValidatableSamlToken} for test purposes
 */
public class ValidatableTokenMock implements ServerValidatableSamlToken {

   private final String id = UUID.randomUUID().toString();
   private final PrincipalId subject;
   private Subject tokenSubject;
   private final boolean isValid;
   private final List<PrincipalId> groupList;
   private final List<SamlTokenDelegate> delegates;
   private final NameId _issuer;

   private boolean isExternal;

   private final boolean invalidSignature;

   public ValidatableTokenMock(boolean isValid, List<PrincipalId> groupList) {
       this(isValid, groupList, TestConstants.ISSUER);
   }
   public ValidatableTokenMock(boolean isValid, List<PrincipalId> groupList, String issuer) {
      this(new PrincipalId("admin", "vmware.com"), isValid, groupList, issuer);
   }

   public ValidatableTokenMock(PrincipalId subject, boolean isValid,
           List<PrincipalId> groupList)
   {
       this( subject, isValid, groupList, TestConstants.ISSUER );
   }
   public ValidatableTokenMock(PrincipalId subject, boolean isValid,
      List<PrincipalId> groupList, String issuer) {
      this.subject = subject;
      this.isValid = isValid;
      this.invalidSignature = false;
      this.groupList = groupList;
      this.delegates = new ArrayList<SamlTokenDelegate>();
      this._issuer = new NameIdImpl(issuer, org.opensaml.saml2.core.Issuer.ENTITY);
   }

   /**
    * Use this when you want to create a token mock which has invalid signature
    */
   public ValidatableTokenMock(boolean invalidSignature) {
      this(invalidSignature, TestConstants.ISSUER);
   }
   /**
    * Use this when you want to create a token mock which has invalid signature
    */
   public ValidatableTokenMock(boolean invalidSignature, String issuer) {
      this.invalidSignature = invalidSignature;
      this.isValid = invalidSignature;
      this.groupList = null;
      this.delegates = null;
      this.subject = null;
      this._issuer = new NameIdImpl(issuer, org.opensaml.saml2.core.Issuer.ENTITY);
   }

   @Override
   public List<Advice> getAdvice() {
      return null;
   }

   @Override
   public Set<String> getAudience() {
      return null;
   }

   @Override
   public X509Certificate getConfirmationCertificate() {
      return null;
   }

   @Override
   public ConfirmationType getConfirmationType() {
      return null;
   }

   @Override
   public Date getExpirationTime() {
      return null;
   }

   @Override
   public List<PrincipalId> getGroupList() {
      return groupList;
   }

   @Override
   public String getId() {
      return id;
   }

   @Override
   public Date getStartTime() {
      return null;
   }

   @Override
   public Subject getSubject() {
      return this.tokenSubject;
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

   void addDelegate(final PrincipalId delegate) {
      delegates.add(new SamlTokenDelegate() {
          private final Date delegationInstant = new Date();

            @Override
            public Subject subject()
            {
                return new ServerValidatableSamlToken.SubjectImpl(
                        delegate,
                        new NameIdImpl(
                             delegate.getName() + "@" + delegate.getDomain(),
                             "http://schemas.xmlsoap.org/claims/UPN"
                        )
                        ,
                        SubjectValidation.None
                );
            }

            @Override
            public Date delegationInstant()
            {
                return delegationInstant;
            }
       });
   }

    @Override
    public List<SamlTokenDelegate> getDelegationChain()
    {
        return this.delegates;
    }
    @Override
	public void validate(X509Certificate[] trustedRootCertificates,
            long clockToleranceSec,
            SubjectValidatorExtractor subjectValidator) throws InvalidTokenException
    {
        if (invalidSignature) {
            throw new InvalidSignatureException("invalid signature");
         }

         if (!isValid) {
            throw new InvalidTokenException("invalid");
         }

         SubjectValidator validator = subjectValidator.getSubjectValidator(this._issuer);
         this.isExternal = validator.IsIssuerExternal();
         this.tokenSubject = validator.validateSubject(
             this.subject,
             new NameIdImpl(
                     this.subject.getName() + "@" + this.subject.getDomain(),
                     "http://schemas.xmlsoap.org/claims/UPN"
                )
             );
         for(int i = 0; i < this.delegates.size(); i++)
         {
             final Subject delValidate = validator.validateSubject(
                  this.delegates.get(i).subject().subjectUpn(),
                  this.delegates.get(i).subject().subjectNameId());
             final Date delInstant = this.delegates.get(i).delegationInstant();

             this.delegates.set(
                 i,
                 new SamlTokenDelegate() {
                       @Override
                       public Subject subject()
                       {
                           return delValidate;
                       }

                       @Override
                       public Date delegationInstant()
                       {
                           return delInstant;
                       }
                });
         }
    }
    @Override
    public Boolean isExternal()
    {
        return this.isExternal;
    }
}
