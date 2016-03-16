/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.interop.ldap;

import java.util.HashMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class LdapErrorChecker
{
    private static final Log logger = LogFactory.getLog(LdapErrorChecker.class);
    
    static final HashMap<Integer, ILdapErrorRaiser> _errorsMap;

    private interface ILdapErrorRaiser
    {
        void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException ;
    }

    static
    {
        _errorsMap = new HashMap<Integer, ILdapErrorRaiser>();

        _errorsMap.put(
            LdapErrors.LDAP_OPERATIONS_ERROR.getCode(),
            new ILdapErrorRaiser() {
                @Override
                public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                {
                    throw new OperationsErrorLdapException(
                            errorCode,
                            iLdapClientLibrary.ldap_err2string(errorCode));

                }
            }
        );

        _errorsMap.put(
                LdapErrors.LDAP_PROTOCOL_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ProtocolErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_TIMELIMIT_EXCEEDED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new TimeLimitExceededLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_SIZELIMIT_EXCEEDED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new SizeLimitExceededLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_COMPARE_FALSE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new CompareFalseLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_COMPARE_TRUE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new CompareTrueLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_AUTH_METHOD_NOT_SUPPORTED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AuthMethodNotSupportedLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_STRONG_AUTH_REQUIRED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new StrongAuthRequiredLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_REFERRAL_V2.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ReferralV2LdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_PARTIAL_RESULTS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new PartialResultsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_REFERRAL.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ReferralLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_ADMIN_LIMIT_EXCEEDED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AdminLimitExceededLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_UNAVAILABLE_CRIT_EXTENSION.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new UnavailableCritExtensionLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_CONFIDENTIALITY_REQUIRED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ConfidentialityRequiredLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_SASL_BIND_IN_PROGRESS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new SaslBindInProgressLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NO_SUCH_ATTRIBUTE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NoSuchAttributeLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_UNDEFINED_TYPE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new UndefinedTypeLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INAPPROPRIATE_MATCHING.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InappropriateMatchingLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_CONSTRAINT_VIOLATION.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ConstraintViolationLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_ATTRIBUTE_OR_VALUE_EXISTS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AttributeOrValueExistsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INVALID_SYNTAX.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InvalidSyntaxLdapException(
                            errorCode,
                            iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NO_SUCH_OBJECT.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NoSuchObjectLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));

                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_ALIAS_PROBLEM.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AliasProblemLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INVALID_DN_SYNTAX.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InvalidDnSyntaxLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_IS_LEAF.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new IsLeafLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_ALIAS_DEREF_PROBLEM.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AliasDerefProblemLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INAPPROPRIATE_AUTH.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InappropriateAuthLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INVALID_CREDENTIALS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InvalidCredentialsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_INSUFFICIENT_RIGHTS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new InsufficientRightsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_BUSY.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new BusyLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_UNAVAILABLE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new UnavailableLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_UNWILLING_TO_PERFORM.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new UnwillingToPerformLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_LOOP_DETECT.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new LoopDetectLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NAMING_VIOLATION.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NamingViolationLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OBJECT_CLASS_VIOLATION.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ObjectClassViolationLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NOT_ALLOWED_ON_NONLEAF.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NotAllowedOnNonLeafLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NOT_ALLOWED_ON_RDN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NotAllowedOnRdnLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_ALREADY_EXISTS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AlreadyExistsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_NO_OBJECT_CLASS_MODS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new NoObjectClassModsLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_RESULTS_TOO_LARGE.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new ResultsTooLargeLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_AFFECTS_MULTIPLE_DSAS.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new AffectsMultipleDsasLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OTHER.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        throw new OtherLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_SERVER_DOWN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ServerDownLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_SERVER_DOWN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ServerDownLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_LOCAL_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new LocalErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_LOCAL_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new LocalErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_ENCODING_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new EncodingErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_ENCODING_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new EncodingErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_DECODING_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new DecodingErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_DECODING_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new DecodingErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_TIMEOUT.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new TimeoutLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_TIMEOUT.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new TimeoutLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_AUTH_UNKNOWN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new AuthUnknownLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_AUTH_UNKNOWN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new AuthUnknownLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_FILTER_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new FilterErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_FILTER_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new FilterErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_USER_CANCELLED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new UserCancelledLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_USER_CANCELLED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new UserCancelledLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_PARAM_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ParamErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_PARAM_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ParamErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_NO_MEMORY.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new NoMemoryLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_NO_MEMORY.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new NoMemoryLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_CONNECT_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ConnectErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_CONNECT_ERROR.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ConnectErrorLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_NOT_SUPPORTED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new NotSupportedLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_NOT_SUPPORTED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new NotSupportedLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_NO_RESULTS_RETURNED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new NoResultsReturnedLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_NO_RESULTS_RETURNED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new NoResultsReturnedLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_CONTROL_NOT_FOUND.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ControlNotFoundLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_CONTROL_NOT_FOUND.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ControlNotFoundLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_MORE_RESULTS_TO_RETURN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new MoreResultsToReturnLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_MORE_RESULTS_TO_RETURN.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new MoreResultsToReturnLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_CLIENT_LOOP.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ClientLoopLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_CLIENT_LOOP.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ClientLoopLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_WinLdap_REFERRAL_LIMIT_EXCEEDED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof WinLdapClientLibrary);
                        throw new ReferralLimitExceededLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

        _errorsMap.put(
                LdapErrors.LDAP_OpenLdap_REFERRAL_LIMIT_EXCEEDED.getCode(),
                new ILdapErrorRaiser() {
                    @Override
                    public void RaiseLdapError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
                    {
                        assert(iLdapClientLibrary instanceof OpenLdapClientLibrary);
                        throw new ReferralLimitExceededLdapException(
                                errorCode,
                                iLdapClientLibrary.ldap_err2string(errorCode));
                    }
                }
        );

    }

    static void CheckError(int errorCode, ILdapClientLibrary iLdapClientLibrary) throws LdapException
    {
        if( errorCode != LdapErrors.LDAP_SUCCESS.getCode() )
        {
            logger.warn(
                    String.format("Error received by LDAP client: %s, error code: %d", 
                    iLdapClientLibrary.getClass().getName(), errorCode));
            ILdapErrorRaiser errorRaiser = _errorsMap.get( errorCode );
            if( errorRaiser != null )
            {
                errorRaiser.RaiseLdapError( errorCode, iLdapClientLibrary );
            }
            else
            {
                throw new LdapException(
                        errorCode,
                        iLdapClientLibrary.ldap_err2string(errorCode));
            }
        }
    }
}
