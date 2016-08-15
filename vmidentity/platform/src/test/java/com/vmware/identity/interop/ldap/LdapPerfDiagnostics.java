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

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;

import junit.framework.Assert;

import org.junit.Test;

public class LdapPerfDiagnostics
{
    /**
     * LOTUS parameters
     */
    private final String hostname = "127.0.0.1";
    private final String bindDN = "CN=Administrator,CN=Users,DC=vsphere,DC=local";
    private final String password = "vmware";
    private final int port = LdapConstants.LDAP_PORT_LOTUS;

    /**
     * Exercise multiple rounds of init/open/bind connection.
     * Nominal range should be all within 1 ms.
     * @throws Exception
     */
    @Test
    public void diagConnectionAll() throws Exception
    {
       int rounds = 10;
       for (int i = 0; i < rounds; i++) {
           diagLdapConnectionURI();
           diagLdapConnectionHostNamePort();
           System.out.println();
       }
    }

    @Test
    public void diagLdapConnectionURI() throws Exception
    {
        ILdapConnectionEx connection = initByURI();
        try
        {
            setOptionAndBind(connection);
        } finally
        {
            connection.close();
        }
    }

    @Test
    public void diagLdapConnectionHostNamePort() throws Exception
    {
        ILdapConnectionEx connection = initByHostNamePort();
        try
        {
            setOptionAndBind(connection);
        } finally
        {
            connection.close();
        }
    }

    /**
     * Exercise various kinds of
     * @throws Exception
     */
    @Test
    public void diagLdapQueryPerf() throws Exception
    {
        final int tries = 5;
        for (int i = 0; i < tries; i++) {
            try {
                System.out.println("Executing diagLdapQueryPerf() test");

                ILdapConnectionEx connection = initByURI();
                setOptionAndBind(connection);

                // Find all the Users
                String baseDN = "CN=Users,DC=vsphere,DC=local";
                String dnAttrName = "dn";
                String objectClassAttrName = "objectClass";
                String[] attrs = { dnAttrName, objectClassAttrName, null };
                List<String> filters = Arrays.asList(
                                    "(objectClass=vmIdentity-User)",
                                    "(&(vmIdentity-Account=Administrator)(|(objectClass=vmIdentity-User)(objectClass=vmIdentity-ServicePrincipal)))",
                                    "(|(objectClass=vmIdentity-User)(objectClass=vmIdentity-ServicePrincipal))"
                                    );
                boolean getAttrsOnly = false;

                long batchStarted = System.nanoTime();
                for (String filter : filters) {
                    long searchStarted = System.nanoTime();
                    ILdapMessage message = connection.search(baseDN, LdapScope.SCOPE_SUBTREE,
                                    filter, Arrays.asList(attrs), getAttrsOnly
                                    );
                    System.out.println(String.format("search [%s] took [%d] ms",
                            filter,
                            TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - searchStarted)));
                    ILdapEntry[] entries = message.getEntries();
                    assert (entries.length > 0);
                }
                System.out.println(String.format("3 searches took [%d] ms\n", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - batchStarted)));
            }
            catch (Exception e) {
                e.printStackTrace();
                throw e;
            }
        }
    }


    private ILdapConnectionEx initByHostNamePort()
    {
        long startedAt = System.nanoTime();
        ILdapConnectionEx connection =
                new LdapConnection(hostname, port);
        long initDone = System.nanoTime();
        System.out.println(String.format("HostnamePort init connection takes [%d] ms", TimeUnit.NANOSECONDS.toMillis(initDone - startedAt)));
        Assert.assertNotNull(connection);
        return connection;
    }

    private ILdapConnectionEx initByURI()
    {
        long startedAt = System.nanoTime();
        ILdapConnectionEx connection =
                LdapConnectionFactoryEx.getInstance().getLdapConnection(hostname,
                        port);
        long initDone = System.nanoTime();
        System.out.println(String.format("URI init connection takes [%d] ms", TimeUnit.NANOSECONDS.toMillis(initDone - startedAt)));
        Assert.assertNotNull(connection);
        return connection;
    }

    private void setOptionAndBind(final ILdapConnectionEx connection)
    {
        long startedAt = System.nanoTime();
        connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                LdapConstants.LDAP_VERSION3);

        connection.bindConnection(bindDN, password,
                LdapBindMethod.LDAP_BIND_SIMPLE);
        System.out.println(String.format("bind connection takes [%d] ms", TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt)));
    }
}
