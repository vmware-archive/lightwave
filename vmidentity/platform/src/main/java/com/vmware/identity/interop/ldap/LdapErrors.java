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

import org.apache.commons.lang.SystemUtils;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 2/2/12
 * Time: 1:28 PM
 * To change this template use File | Settings | File Templates.
 */
enum LdapErrors {
    LDAP_SUCCESS (0x00),
    LDAP_OPERATIONS_ERROR (0x01),
    LDAP_PROTOCOL_ERROR(0x02),
    LDAP_TIMELIMIT_EXCEEDED(0x03),
    LDAP_SIZELIMIT_EXCEEDED(0x04),
    LDAP_COMPARE_FALSE(0x05),
    LDAP_COMPARE_TRUE(0x06),
    LDAP_AUTH_METHOD_NOT_SUPPORTED(0x07),
    LDAP_STRONG_AUTH_REQUIRED(0x08),
    LDAP_REFERRAL_V2(0x09),
    LDAP_PARTIAL_RESULTS(0x09),
    LDAP_REFERRAL(0x0a),
    LDAP_ADMIN_LIMIT_EXCEEDED(0x0b),
    LDAP_UNAVAILABLE_CRIT_EXTENSION(0x0c),
    LDAP_CONFIDENTIALITY_REQUIRED(0x0d),
    LDAP_SASL_BIND_IN_PROGRESS(0x0e),

    LDAP_NO_SUCH_ATTRIBUTE(0x10),
    LDAP_UNDEFINED_TYPE(0x11),
    LDAP_INAPPROPRIATE_MATCHING(0x12),
    LDAP_CONSTRAINT_VIOLATION(0x13),
    LDAP_ATTRIBUTE_OR_VALUE_EXISTS(0x14),
    LDAP_INVALID_SYNTAX(0x15),

    LDAP_NO_SUCH_OBJECT(0x20),
    LDAP_ALIAS_PROBLEM(0x21),
    LDAP_INVALID_DN_SYNTAX(0x22),
    LDAP_IS_LEAF(0x23),
    LDAP_ALIAS_DEREF_PROBLEM(0x24),

    LDAP_INAPPROPRIATE_AUTH(0x30),
    LDAP_INVALID_CREDENTIALS(0x31),
    LDAP_INSUFFICIENT_RIGHTS(0x32),
    LDAP_BUSY(0x33),
    LDAP_UNAVAILABLE(0x34),
    LDAP_UNWILLING_TO_PERFORM(0x35),
    LDAP_LOOP_DETECT(0x36),

    LDAP_NAMING_VIOLATION(0x40),
    LDAP_OBJECT_CLASS_VIOLATION(0x41),
    LDAP_NOT_ALLOWED_ON_NONLEAF(0x42),
    LDAP_NOT_ALLOWED_ON_RDN(0x43),
    LDAP_ALREADY_EXISTS(0x44),
    LDAP_NO_OBJECT_CLASS_MODS(0x45),
    LDAP_RESULTS_TOO_LARGE(0x46),
    LDAP_AFFECTS_MULTIPLE_DSAS(0x47),
    LDAP_OTHER(0x50),

    LDAP_WinLdap_SERVER_DOWN(0x51),
    LDAP_WinLdap_LOCAL_ERROR(0x52),
    LDAP_WinLdap_ENCODING_ERROR(0x53),
    LDAP_WinLdap_DECODING_ERROR(0x54),
    LDAP_WinLdap_TIMEOUT(0x55),
    LDAP_WinLdap_AUTH_UNKNOWN(0x56),
    LDAP_WinLdap_FILTER_ERROR(0x57),
    LDAP_WinLdap_USER_CANCELLED(0x58),
    LDAP_WinLdap_PARAM_ERROR(0x59),
    LDAP_WinLdap_NO_MEMORY(0x5a),
    LDAP_WinLdap_CONNECT_ERROR(0x5b),
    LDAP_WinLdap_NOT_SUPPORTED(0x5c),
    LDAP_WinLdap_CONTROL_NOT_FOUND(0x5d),
    LDAP_WinLdap_NO_RESULTS_RETURNED(0x5e),
    LDAP_WinLdap_MORE_RESULTS_TO_RETURN(0x5f),
    LDAP_WinLdap_CLIENT_LOOP(0x60),
    LDAP_WinLdap_REFERRAL_LIMIT_EXCEEDED(0x61),

    LDAP_OpenLdap_SERVER_DOWN(-1),
    LDAP_OpenLdap_LOCAL_ERROR(-2),
    LDAP_OpenLdap_ENCODING_ERROR(-3),
    LDAP_OpenLdap_DECODING_ERROR(-4),
    LDAP_OpenLdap_TIMEOUT(-5),
    LDAP_OpenLdap_AUTH_UNKNOWN(-6 ),
    LDAP_OpenLdap_FILTER_ERROR(-7),
    LDAP_OpenLdap_USER_CANCELLED(-8),
    LDAP_OpenLdap_PARAM_ERROR(-9),
    LDAP_OpenLdap_NO_MEMORY(-10),
    LDAP_OpenLdap_CONNECT_ERROR(-11),
    LDAP_OpenLdap_NOT_SUPPORTED(-12),
    LDAP_OpenLdap_CONTROL_NOT_FOUND(-13),
    LDAP_OpenLdap_NO_RESULTS_RETURNED(-14),
    LDAP_OpenLdap_MORE_RESULTS_TO_RETURN(-15),
    LDAP_OpenLdap_CLIENT_LOOP(-16),
    LDAP_OpenLdap_REFERRAL_LIMIT_EXCEEDED(-17);

    private int _code;

    private LdapErrors(int code)
    {
        _code = code;
    }

    public int getCode()
    {
        return _code;
    }
}
