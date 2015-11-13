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
import java.util.Date;
import java.util.List;
import java.util.Set;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.saml.SamlTokenSpec.ConfirmationType;

public interface ServerValidatableSamlToken
{
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
     * Although the token can be delegated this field will not change.
     *
     * @return the subject of the token.
     */
    public Subject getSubject();

    /**
     * @return Whether the token was issued by the external registered ipd.
     */
    public Boolean isExternal();

    /**
     * The issuer element of a token provides information about the issuer of a
     * SAML assertion.
     * @return Assertion's issuer.
     */
    public NameId getIssuerNameId();

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
    public List<SamlTokenDelegate> getDelegationChain();

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
     * Validates that the token is signed using a trusted certificate and is
     * within the lifetime range
     * In addition subject verification is performed.
     *
     * @param trustedRootCertificates
     *           List of trusted root STS certificates that ValidatableSamlToken
     *           will use when validating the token's signature. Required.
     * @param clockToleranceSec
     *           Tolerate that many seconds of discrepancy between the token's
     *           sender clock and the local system clock when validating the
     *           token's start and expiration time. This effectively "expands"
     *           the token's validity period with the given number of seconds.
     * @return
     * @throws InvalidSignatureException
     *            when the signature cannot be verified.
     * @throws InvalidTokenException
     *            if the token or some of its elements is invalid or malformed
     */
    public void validate(X509Certificate[] trustedRootCertificates,
       long clockToleranceSec, SubjectValidatorExtractor subjectValidatorExtractor)
    throws InvalidTokenException, InvalidSignatureException;

    /**
     * Subject lookup state.
     *     None - subject was not found/validated.
     *     Regular - subject was confirmed as an active user for the tenant.
     */
    public enum SubjectValidation {None, Regular} // FUTURE: possibly FSP

    /**
     * Defines a name together with it's format.
     *   Such as token's issuer or subject.
     */
    public interface NameId
    {
        String getName();
        String getNameFormat();
    }

    /**
     * Token's subject information. This includes subject name id, ValidationResult,
     * and subjectUpn if any.
     */
    public interface Subject
    {
        PrincipalId subjectUpn();
        NameId subjectNameId();
        SubjectValidation subjectValidation();
    }

    /**
     * Represents a token delegate, specified by Subject and the delegation instant.
     */
    public interface SamlTokenDelegate
    {
        Subject subject();
        Date delegationInstant();
    }

    /**
     * Extracts the SubjectValidator for the specified issuer.
     */
    public interface SubjectValidatorExtractor
    {
        public SubjectValidator getSubjectValidator(NameId issuer);
    }

    /**
     * Allows for the Subject validation.
     */
    public interface SubjectValidator
    {
        /**
         * @return Whether this validator is for the external token's issuer.
         */
        public Boolean IsIssuerExternal();

        /**
         * Validate the specified subject.
         * @param upnSubject subject in the upn format. Can be null.
         * @param subject Token subject, cannot be null.
         * @return a validated Subject.
         * @throws InvalidTokenException
         */
        public Subject validateSubject( PrincipalId upnSubject, NameId subject )
            throws InvalidTokenException;
    }

    /**
     * Convenience class giving the default Subject interface implementation.
     */
    public static final class SubjectImpl implements Subject
    {
        private final PrincipalId _subjectUpn;
        private final NameId _subjectNameId;
        private final SubjectValidation _subjectValidation;

        public SubjectImpl(PrincipalId subjectUpn, NameId subjectNameId, SubjectValidation subjectValidation)
        {
            ValidateUtil.validateNotNull(subjectNameId, "subjectNameId");
            ValidateUtil.validateNotNull(subjectValidation, "subjectValidation");
            if ( subjectValidation == SubjectValidation.Regular )
            {
                ValidateUtil.validateNotNull(subjectUpn, "subjectUpn");
            }
            this._subjectUpn = subjectUpn;
            this._subjectNameId = subjectNameId;
            this._subjectValidation = subjectValidation;
        }

        @Override
        public PrincipalId subjectUpn()
        {
            return this._subjectUpn;
        }

        @Override
        public NameId subjectNameId()
        {
            return this._subjectNameId;
        }

        @Override
        public SubjectValidation subjectValidation()
        {
            return this._subjectValidation;
        }

        @Override
        public String toString()
        {
            return String.format(
                "{PrincipalId=[%s], NameId=[%s], SubjectValidation[%s]}",
                (this._subjectUpn != null) ? this._subjectUpn : "(null)",
                this._subjectNameId,
                this._subjectValidation
            );
        }
    }

    /**
     * Convenience class giving the default NameId interface implementation.
     */
    public static final class NameIdImpl implements NameId
    {
        private final String _name;
        private final String _nameFormat;

        public NameIdImpl(String name, String nameFormat)
        {
            ValidateUtil.validateNotNull(name, "name");
            ValidateUtil.validateNotNull(nameFormat, "nameFormat");
            this._name = name;
            this._nameFormat = nameFormat;
        }

        @Override
        public String getName()
        {
            return this._name;
        }

        @Override
        public String getNameFormat()
        {
            return this._nameFormat;
        }

        @Override
        public String toString()
        {
            return String.format("{Name=[%s], NameFormat=[%s]}", this._name,this._nameFormat);
        }
    }
}
