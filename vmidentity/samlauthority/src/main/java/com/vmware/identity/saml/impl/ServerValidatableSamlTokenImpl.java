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
import java.util.concurrent.atomic.AtomicBoolean;

import org.w3c.dom.Element;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.Advice;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.Advice.Attribute;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;
import com.vmware.identity.token.impl.ValidateUtil;
import com.vmware.vim.sso.client.ValidatableSamlTokenEx;
import com.vmware.vim.sso.client.Advice.AdviceAttribute;
import com.vmware.vim.sso.client.ValidatableSamlTokenEx.TokenDelegateEx;
import com.vmware.vim.sso.client.ValidatableTokenFactory;

/**
 * Implementation of the ServerValidatableSamlToken.
 */
public class ServerValidatableSamlTokenImpl implements
        ServerValidatableSamlToken
{
    private ValidatableSamlTokenEx _validatableSamlToken;
    private Boolean _isExternal;
    private NameId _issuerNameId;
    private Subject _subject;
    private List<SamlTokenDelegate> _delegates;
    private List<Advice> _advice;
    private List<PrincipalId> _groups;

    private final AtomicBoolean _tokenValidated = new AtomicBoolean(false);

    public ServerValidatableSamlTokenImpl(Element tokenRoot) throws InvalidTokenException
    {
       ValidatableTokenFactory factory = new ValidatableTokenFactory();
       try
       {
           this._validatableSamlToken = factory.parseValidatableTokenEx(tokenRoot);
       } catch (com.vmware.vim.sso.client.exception.InvalidTokenException e)
       {
           throw new InvalidTokenException(e.getMessage(), e);
       }
       this._isExternal = false;
    }

    @Override
    public Date getStartTime()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.getStartTime();
    }

    @Override
    public Date getExpirationTime()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.getExpirationTime();
    }

    @Override
    public boolean isRenewable()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.isRenewable();
    }

    @Override
    public boolean isDelegable()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.isDelegable();
    }

    @Override
    public ConfirmationType getConfirmationType()
    {
        this.checkIsValidated();
        if (this._validatableSamlToken.getConfirmationType() == com.vmware.vim.sso.client.ConfirmationType.BEARER)
        {
            return ConfirmationType.BEARER;
        }
        else //ConfirmationType.HOLDER_OF_KEY
        {
            return ConfirmationType.HOLDER_OF_KEY;
        }
    }

    @Override
    public Subject getSubject()
    {
        this.checkIsValidated();
        return this._subject;
    }

    @Override
    public Boolean isExternal()
    {
        this.checkIsValidated();
        return this._isExternal;
    }

    @Override
    public NameId getIssuerNameId()
    {
        this.checkIsValidated();
        return this._issuerNameId;
    }

    @Override
    public String getId()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.getId();
    }

    @Override
    public List<SamlTokenDelegate> getDelegationChain()
    {
        this.checkIsValidated();
        return this._delegates;
    }

    @Override
    public Set<String> getAudience()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.getAudience();
    }

    @Override
    public X509Certificate getConfirmationCertificate()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.getConfirmationCertificate();
    }

    @Override
    public List<Advice> getAdvice()
    {
        this.checkIsValidated();
        return this._advice;
    }

    @Override
    public List<PrincipalId> getGroupList()
    {
        this.checkIsValidated();
        return this._groups;
    }

    @Override
    public boolean isSolution()
    {
        this.checkIsValidated();
        return this._validatableSamlToken.isSolution();
    }

    @Override
    public void validate(X509Certificate[] trustedRootCertificates,
            long clockToleranceSec,
            SubjectValidatorExtractor subjectValidatorExtractor)
            throws InvalidTokenException
    {
        try
        {
            this._validatableSamlToken.validate(trustedRootCertificates, clockToleranceSec);
            this._issuerNameId =
                new NameIdImpl(
                    this._validatableSamlToken.getIssuerNameId().getValue(),
                    this._validatableSamlToken.getIssuerNameId().getFormat()
                );
            this._advice = toSamlAuthorityAdvice(this._validatableSamlToken.getAdvice());
            this._groups = toSamlAuthorityGroups(this._validatableSamlToken.getGroupList());

            SubjectValidator subjectValidator = subjectValidatorExtractor.getSubjectValidator(this._issuerNameId);
            this._isExternal = subjectValidator.IsIssuerExternal();

            this._subject = subjectValidator.validateSubject(
                    (this._validatableSamlToken.getSubject() != null)
                    ? toPrincipalId(this._validatableSamlToken.getSubject())
                    :
                    null,
                    new NameIdImpl(
                            this._validatableSamlToken.getSubjectNameId().getValue(),
                            this._validatableSamlToken.getSubjectNameId().getFormat()
                    )
            );

            this._delegates = new ArrayList<SamlTokenDelegate>(this._validatableSamlToken.getDelegationChainEx().size());

            for (TokenDelegateEx delegateEx : this._validatableSamlToken.getDelegationChainEx() )
            {
                Subject delegate = subjectValidator.validateSubject(
                        (delegateEx.getSubject() != null)
                        ? toPrincipalId(delegateEx.getSubject())
                        :
                        null,
                        new NameIdImpl(
                            delegateEx.getSubjectNameId().getValue(),
                            delegateEx.getSubjectNameId().getFormat()
                        )
                );
                this._delegates.add( new SamlTokenDelegateImpl(delegate, delegateEx.getDelegationDate()));
            }
            _tokenValidated.set(true);
        }
        catch (com.vmware.vim.sso.client.exception.InvalidSignatureException e)
        {
            throw new InvalidSignatureException(e.getMessage(), e);
        }
        catch (com.vmware.vim.sso.client.exception.InvalidTokenException e)
        {
            throw new InvalidTokenException(e.getMessage(), e);
        }
    }

    private void checkIsValidated()
    {
        if (!_tokenValidated.get()) {
            throw new IllegalStateException(
               "Until token signature is validated accessors cannot be used.");
         }
    }

    private static class SamlTokenDelegateImpl implements SamlTokenDelegate
    {
        private final Subject _subject;
        private final Date _delegationInstant;

        public SamlTokenDelegateImpl( Subject subject, Date delegationInstant )
        {
            ValidateUtil.validateNotNull(subject, "subject");
            ValidateUtil.validateNotNull(delegationInstant, "delegationInstant");
            this._subject = subject;
            this._delegationInstant = delegationInstant;
        }

        @Override
        public Subject subject()
        {
            return this._subject;
        }

        @Override
        public Date delegationInstant()
        {
            return this._delegationInstant;
        }
    }

    private PrincipalId toPrincipalId(com.vmware.vim.sso.PrincipalId source) {
        assert source != null;
        return new PrincipalId(source.getName(), source.getDomain());
    }

    private static List<Advice> toSamlAuthorityAdvice(
        List<com.vmware.vim.sso.client.Advice> advice) {
        assert advice != null;

        final List<Advice> result = new ArrayList<Advice>(advice.size());
        for (com.vmware.vim.sso.client.Advice adviceMember : advice) {
           result.add(new Advice(adviceMember.getSource(),
              tokenToSamlAuthorityAttributes(adviceMember.getAttributes())));
        }
        return result;
     }

    private static List<Attribute> tokenToSamlAuthorityAttributes(
        List<AdviceAttribute> attributes) {
        assert attributes != null;

        final List<Attribute> result = new ArrayList<Attribute>(attributes.size());
        for (AdviceAttribute attribute : attributes) {
           result.add(new Attribute(attribute.getName(), attribute.getFriendlyName(), attribute
              .getValue()));
        }
        return result;
     }

    private static List<PrincipalId> toSamlAuthorityGroups(List<com.vmware.vim.sso.PrincipalId> groupsList)
    {
        assert groupsList != null;
        List<PrincipalId> samlGroups = new ArrayList<PrincipalId>(groupsList.size());
        for( int i = 0; i < groupsList.size(); i++ )
        {
            samlGroups.add(
                new PrincipalId(
                    groupsList.get(i).getName(),
                    groupsList.get(i).getDomain()
                )
            );
        }
        return samlGroups;
    }
}
