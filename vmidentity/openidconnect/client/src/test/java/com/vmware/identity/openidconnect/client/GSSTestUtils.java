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

package com.vmware.identity.openidconnect.client;

import java.io.IOException;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.UnsupportedCallbackException;
import javax.security.auth.login.AppConfigurationEntry;
import javax.security.auth.login.Configuration;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;

import org.apache.commons.lang3.Validate;
import org.ietf.jgss.GSSContext;
import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSManager;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

/**
 * GSS Test Utils
 *
 * @author Jun Sun
 */
class GSSTestUtils {

    private static String spn;
    private static PrincipalId principal;
    private static char[] password;

    private static void getProperties() throws IOException {
        Properties properties = new Properties();
        properties.load(OIDCClientGssIT.class.getClassLoader().getResourceAsStream("resources/config.properties"));
        String adDomain = properties.getProperty("ad.domain");
        String adSpn = properties.getProperty("ad.spn");
        String adUsername = properties.getProperty("ad.admin.user");
        String adPassword = properties.getProperty("ad.admin.password");

        spn = adSpn;
        principal = new PrincipalId(adUsername, adDomain);
        password = adPassword.toCharArray();
    };

    static GSSNegotiationHandler getKerberosNegotiationHandler() throws GSSException, LoginException, IOException {
        getProperties();
        final javax.security.auth.Subject jaasSubject = new javax.security.auth.Subject();
        GSSContext context = createGSSContext(spn);
        GSSNegotiationHandler handler = new GssNegotiationHandler(jaasSubject, context);

        LoginContext login = getLoginCtx(principal, password, jaasSubject);
        login.login();

        return handler;
    }

    static LoginContext getLoginCtx(final PrincipalId validAdUser, final char[] userPass, javax.security.auth.Subject jaasSubject)
            throws LoginException {
        return new LoginContext("SampleLoginContext", jaasSubject, new CallbackHandler() {
            @Override
            public void handle(Callback[] callbacks) throws IOException, UnsupportedCallbackException {
                String userName = String.format("%s@%s", validAdUser.getName(), validAdUser.getDomain());
                for (Callback callback : callbacks) {
                    if (callback instanceof NameCallback) {
                        ((NameCallback) callback).setName(userName);
                    } else if (callback instanceof PasswordCallback) {
                        ((PasswordCallback) callback).setPassword(userPass);
                    }
                }
            }
        },

        new Configuration() {
            @Override
            public AppConfigurationEntry[] getAppConfigurationEntry(String name) {
                Map<String, String> config = new HashMap<String, String>();
                config.put("useTicketCache", "false");
                return new AppConfigurationEntry[] { new AppConfigurationEntry("com.sun.security.auth.module.Krb5LoginModule",
                        AppConfigurationEntry.LoginModuleControlFlag.REQUIRED, config) };
            }
        });
    }

    static GSSContext createGSSContext(String spn) throws GSSException {
        GSSManager manager = GSSManager.getInstance();
        Oid krb5PrincipalNameType = new Oid("1.2.840.113554.1.2.2.1"); // RFC 1964
        GSSName gssName = manager.createName(spn, krb5PrincipalNameType);
        Oid krb5Mech = new Oid("1.2.840.113554.1.2.2"); // RFC 1964
        return GSSManager.getInstance().createContext(gssName, krb5Mech, null, GSSContext.DEFAULT_LIFETIME);
    }

    static class GssNegotiationHandler implements GSSNegotiationHandler {
        private final javax.security.auth.Subject _jaasSubject;
        private final GSSContext _context;

        GssNegotiationHandler(javax.security.auth.Subject jaasSubject, GSSContext context) {
            this._jaasSubject = jaasSubject;
            this._context = context;
        }

        private final PrivilegedExceptionAction<byte[]> action = new PrivilegedExceptionAction<byte[]>() {
            @Override
            public byte[] run() throws Exception {
                byte[] token = new byte[0];
                return GssNegotiationHandler.this._context.initSecContext(token, 0, token.length);
            }
        };

        @Override
        public byte[] negotiate(byte[] leg) {
            if (leg == null) {
                try {
                    return javax.security.auth.Subject.doAs(this._jaasSubject, this.action);
                } catch (PrivilegedActionException e) {
                    //throw new OIDCClientException("Cannot retrieve initial leg", e);
                }
            }

            byte[] newLeg = null;
            try {
                newLeg = this._context.initSecContext(leg, 0, leg.length);
            } catch (GSSException e) {
                //throw new OIDCClientException("Cannot create GSS context", e);
            }

            return !this._context.isEstablished() ? newLeg : null;
        }
    }

    static final class PrincipalId {
        /** Principal name */
        private final String _name;
        /** Principal domain */
        private final String _domain;

        /**
         * Construct a principal identifier by domain name where he/she is
         * located and short name which should be unique in scope of the given
         * domain
         *
         * @param name
         *            principal short name (e.g. jdoe); requires
         *            {@code not-null} and not empty string value
         * @param domain
         *            domain name or alias (e.g. vmware.com); requires
         *            {@code not-null} and not empty string value;
         */
        public PrincipalId(String name, String domain) {
            Validate.notEmpty(name, "name");
            Validate.notEmpty(domain, "domain");

            this._name = name;
            this._domain = domain;
        }

        /**
         * @return the name; {@code not-null} and not empty string value
         */
        public String getName() {
            return this._name;
        }

        /**
         * @return the domain; {@code not-null} and not empty string value
         */
        public String getDomain() {
            return this._domain;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean equals(Object obj) {
            if (obj == this) {
                return true;
            }

            if (obj == null) {
                return false;
            }

            if (!obj.getClass().equals(PrincipalId.class)) {
                return false;
            }

            PrincipalId other = (PrincipalId) obj;
            return this._name.equals(other._name) && this._domain.equalsIgnoreCase(other._domain);

        }

        /**
         * {@inheritDoc}
         */
        @Override
        public int hashCode() {
            return Arrays.hashCode(new Object[] { this._name, this._domain.toLowerCase() });
        }

        @Override
        public String toString() {
            return String.format("{Name: %s, Domain: %s}", getName(), getDomain());
        }
    }
}
