/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.interop;

import java.net.URI;
import java.nio.ByteBuffer;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.junit.Assert;
import org.junit.Test;

import com.sun.jna.Platform;
import com.vmware.identity.interop.ldap.AlreadyExistsLdapException;
import com.vmware.identity.interop.ldap.BerValNative;
import com.vmware.identity.interop.ldap.ControlNotFoundLdapException;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.ldap.ILdapConnection;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.ILdapMessageEx;
import com.vmware.identity.interop.ldap.ILdapPagedSearchResult;
import com.vmware.identity.interop.ldap.ISslX509VerificationCallback;
import com.vmware.identity.interop.ldap.LdapBindMethod;
import com.vmware.identity.interop.ldap.LdapConnectionFactory;
import com.vmware.identity.interop.ldap.LdapConnectionFactoryEx;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.ldap.LdapControlNative;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapOption;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapSetting;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.ServerDownLdapException;
import com.vmware.identity.interop.ldap.SizeLimitExceededLdapException;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;

public class LdapTest
{
    String hostname = "127.0.0.1";
    String bindDN   = "CN=Administrator,CN=Users,DC=vsphere,DC=local";
    String password = "Admin!23";
    int    port     = LdapConstants.LDAP_PORT_LOTUS;
    String    adminUpn = "administrator@vsphere.local";

    // AD - ssolabs
    String adHostname = "dc-1.ssolabs.eng.vmware.com";
    String bindAdDN   = "CN=Administrator,CN=Users,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM";
    String adPassword = "ca$hc0w";
    int    adPort = LdapConstants.LDAP_PORT;

    // AD-topology lab for SASL bind (sasl bind only succeeds when we successfully set up krb5.conf
    /*String adSaslHostname = "dc1-c1-s2.child1.ssolabs2.com";
    String adSaslUserName = "administrator";
    String adSaslDomainname = "child1.ssolabs2.com";
    String adSaslPassword = "$so2014";*/
    String adSaslHostname = "dc-1.ssolabs.eng.vmware.com";
    String adSaslUserName = "administrator";
    String adSaslDomainname = "ssolabs.eng.vmware.com";
    String adSaslPassword = "ca$hc0w";

    String olHostname = "10.136.33.172";
    String bindOlDN   = "cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com";
    String olPassword = "123";
    int    olPort = LdapConstants.LDAP_PORT;

    private final long DEFAULT_LDAP_NETWORK_TIMEOUT = 30;
    private final URI ldapsURI =
          DirectoryStoreProtocol.LDAPS.getUri(adHostname, LdapConstants.LDAP_SSL_PORT);

    @Test
    public void testLdapConnection() throws Exception
    {
        System.out.println("Executing testLdapConnection() test");

        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx> ();
        connections.add(LdapConnectionFactoryEx.getInstance().getLdapConnection(hostname, port));
        if(Platform.isWindows()){
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);
            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindDN, password,
                        LdapBindMethod.LDAP_BIND_SIMPLE);
            } finally {
                connection.close();
            }
        }
    }

    /* prior running this test need krb5.conf configured as well as the krb tgt used to do the bind generated
     * and not expired in '/etc/krb5.conf'
     */
    @Test
    public void testLdapSaslConnectionToAD() throws Exception
    {
        System.out.println("Executing testLdapSaslConnectionToAD() test");

        ILdapConnectionEx connection = LdapConnectionFactoryEx.getInstance().getLdapConnection(adSaslHostname, adPort);
        Assert.assertNotNull(connection);

        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindSaslConnection(adSaslUserName, adSaslDomainname, adSaslPassword);
        }
        finally
        {
            connection.close();
        }
    }

    @Test
    public void testLdapSaslSrpConnection() throws Exception
    {
        System.out.println("Executing testLdapSaslSrpConnection() test");

        URI uri = PlatformUtils.getConnectionUriDefaultScheme("localhost", port);
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnectionEx connection = (ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        Assert.assertNotNull(connection);

        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            ((ILdapConnectionExWithGetConnectionString)connection).bindSaslSrpConnection("administrator@vsphere.local", "Admin!23");
        }
        finally
        {
            connection.close();
        }
    }

    @Test
    public void testLegacyLdapQuery() throws Exception{
        System.out.println("Executing testLegacyLdapQuery() test");
        ILdapConnectionEx connection = LdapConnectionFactoryEx.getInstance().getLdapConnection(hostname, port);
        Assert.assertNotNull(connection);
        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindConnection(
                            bindDN,
                            password,
                            LdapBindMethod.LDAP_BIND_SIMPLE);
            testLdapQuery(connection);
        }
        finally
        {
            connection.close();
        }
    }

    @Test
    public void testOpenLdapQueryWithSrpBind() throws Exception{
        System.out.println("Executing testOpenLdapQueryWithSrpBind() test");
        URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnection connection = LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        Assert.assertNotNull(connection);
        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindSaslSrpConnection(adminUpn, password);
            testLdapQuery((ILdapConnectionEx)connection);
        }
        finally
        {
            connection.close();
        }
    }

    private void testLdapQuery(ILdapConnectionEx connection) throws Exception
    {
        ILdapMessage message = null;
        try
        {
            // Find all the Users
            String   baseDN = "CN=Users,DC=vsphere,DC=local";
            String   dnAttrName = "entryDn";
            String   objectClassAttrName = "objectClass";
            String[] attrs = { dnAttrName, objectClassAttrName, null };
            String   filter = "(objectClass=*)";
            boolean  getAttrsOnly = false;

            message = connection.search(
                                    baseDN,
                                    LdapScope.SCOPE_ONE_LEVEL,
                                    filter,
                                    attrs,
                                    getAttrsOnly);

            ILdapEntry[] entries = message.getEntries();

            Assert.assertNotNull(entries);
            Assert.assertTrue(entries.length > 0);

            for (ILdapEntry entry : entries)
            {

                String dn = entry.getDN();
                Assert.assertNotNull(dn);

                String[] attrNames = entry.getAttributeNames();

                Assert.assertNotNull(attrNames);
                Assert.assertTrue(attrNames.length == 2);

                System.out.println(String.format("Entry (DN: %s)", entry.getDN()));

                for (String attrName : attrNames)
                {
                    System.out.println(String.format("\tAttribute (Name: %s)", attrName));

                    LdapValue[] values = entry.getAttributeValues(attrName);

                    // handle values that are not ASCII
                    for (LdapValue value : values)
                    {
                        byte[] bytes = value.getValue();

                        ByteBuffer bb = ByteBuffer.allocate(bytes.length);
                        bb.put(bytes);

                        String val = new String(bb.array());

                        System.out.println(String.format("\t\tValue : %s", val));
                    }

                    Assert.assertNotNull(values);
                    Assert.assertTrue(values.length >= 1);
                }
            }
        }
        finally
        {
            if (message != null)
            {
                message.close();
            }
        }
    }

    @Test
    public void testLdapQueryWithSaslBind() throws Exception
    {
        System.out.println("Executing testLdapQueryWithSaslBind() test");

        ILdapConnectionEx connection = LdapConnectionFactoryEx.getInstance().getLdapConnection(adSaslHostname, adPort);
        Assert.assertNotNull(connection);

        ILdapMessage message = null;

        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindSaslConnection(adSaslUserName, adSaslDomainname, adSaslPassword);

            // Find all the Users
            String   baseDN = "CN=Users,DC=ssolabs,DC=eng,DC=vmware,DC=com";
            //String   baseDN = "CN=Users,DC=CHILD1,DC=ssolabs2,DC=com";
            String   samAccountName = "sAMAccountName";
            String   objectClassAttrName = "objectClass";
            String[] attrs = { samAccountName, objectClassAttrName, null };
            String   filter = "(objectClass=user)";
            boolean  getAttrsOnly = false;

            message = connection.search(
                                    baseDN,
                                    LdapScope.SCOPE_ONE_LEVEL,
                                    filter,
                                    attrs,
                                    getAttrsOnly);

            ILdapEntry[] entries = message.getEntries();

            Assert.assertNotNull(entries);
            Assert.assertTrue(entries.length > 0);

            for (ILdapEntry entry : entries)
            {

                String dn = entry.getDN();
                Assert.assertNotNull(dn);

                String[] attrNames = entry.getAttributeNames();

                Assert.assertNotNull(attrNames);
                Assert.assertTrue(attrNames.length == 2);

                System.out.println(String.format("Entry (DN: %s)", entry.getDN()));

                for (String attrName : attrNames)
                {
                    System.out.println(String.format("\tAttribute (Name: %s)", attrName));

                    LdapValue[] values = entry.getAttributeValues(attrName);

                    // handle values that are not ASCII
                    for (LdapValue value : values)
                    {
                        byte[] bytes = value.getValue();

                        ByteBuffer bb = ByteBuffer.allocate(bytes.length);
                        bb.put(bytes);

                        String val = new String(bb.array());

                        System.out.println(String.format("\t\tValue : %s", val));
                    }

                    Assert.assertNotNull(values);
                    Assert.assertTrue(values.length >= 1);
                }
            }
        }
        finally
        {
            if (message != null)
            {
                message.close();
            }

            if (connection != null)
            {
                connection.close();
            }
        }
    }

    @Test
    public void testLegacyLdapExtQuery() throws Exception{
        System.out.println("Executing testLegacyLdapQuery() test");
        ILdapConnectionEx connection = LdapConnectionFactoryEx.getInstance().getLdapConnection(hostname, port);
        Assert.assertNotNull(connection);
        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindConnection(
                            bindDN,
                            password,
                            LdapBindMethod.LDAP_BIND_SIMPLE);
            testLdapExtQuery(connection);
        }
        finally
        {
            connection.close();
        }
    }

    @Test
    public void testOpenLdapLdapExtQuery() throws Exception{
        System.out.println("Executing testOpenLdapQueryWithSrpBind() test");
        URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnection connection = LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        Assert.assertNotNull(connection);
        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindConnection(
                            bindDN,
                            password,
                            LdapBindMethod.LDAP_BIND_SIMPLE);
            testLdapExtQuery((ILdapConnectionEx)connection);
        }
        finally
        {
            connection.close();
        }
    }

    private void testLdapExtQuery(ILdapConnectionEx connection) throws Exception
    {
        System.out.println("Executing testLdapExtQuery() test");
        ILdapMessage message = null;

        try
        {
            connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.bindConnection(
                            bindDN,
                            password,
                            LdapBindMethod.LDAP_BIND_SIMPLE);

            // Find all the Users
            String   baseDN = "DC=local";
            String   dnAttrName = "entryDn";
            String   objectClassAttrName = "objectClass";
            String   isDeletedAttrName = "isDeleted";
            String[] attrs = { dnAttrName, objectClassAttrName, isDeletedAttrName, null };
            String   filter = "(objectClass=*)";
            boolean  getAttrsOnly = false;
            LdapControlNative delObjCtrl = new LdapControlNative( "1.2.840.113556.1.4.417", new BerValNative(), '1', null);
            LdapControlNative[] sctrls = { delObjCtrl };

            message = connection.search_ext(
                                    baseDN,
                                    LdapScope.SCOPE_SUBTREE,
                                    filter,
                                    Arrays.asList(attrs),
                                    getAttrsOnly,
                                    Arrays.asList(sctrls),
                                    null,
                                    null,
                                    -1);

            ILdapEntry[] entries = message.getEntries();

            Assert.assertNotNull(entries);
            Assert.assertTrue(entries.length > 0);

            for (ILdapEntry entry : entries)
            {
                String dn = entry.getDN();
                Assert.assertNotNull(dn);

                String[] attrNames = entry.getAttributeNames();

                Assert.assertNotNull(attrNames);

                System.out.println(String.format("Entry (DN: %s)", entry.getDN()));

                for (String attrName : attrNames)
                {
                    System.out.println(String.format("\tAttribute (Name: %s)", attrName));

                    LdapValue[] values = entry.getAttributeValues(attrName);

                    // handle values that are not ASCII
                    for (LdapValue value : values)
                    {
                        byte[] bytes = value.getValue();

                        ByteBuffer bb = ByteBuffer.allocate(bytes.length);
                        bb.put(bytes);

                        String val = new String(bb.array());

                        System.out.println(String.format("\t\tValue : %s", val));
                    }

                    Assert.assertNotNull(values);
                    Assert.assertTrue(values.length >= 1);
                }
            }
        }
        finally
        {
            if (message != null)
            {
                message.close();
            }

            connection.close();
        }
    }

    // return how many paged search has done
    private int performPagedSearch_with_one_page(
        ILdapConnectionEx connection,
        String baseDN,
        String filter,
        String[] attrs,
        int pageSize
        )
    {
        int entryCountInOnePage = 0;
        int pageCount = 0;
        ILdapPagedSearchResult prev_pagedResult = null;
        ILdapPagedSearchResult pagedResult = null;

        while (!(prev_pagedResult == null ? false : prev_pagedResult.isSearchFinished()))
        {
            try
            {
                pagedResult = connection.search_one_page(baseDN,
                                                         LdapScope.SCOPE_SUBTREE,
                                                         filter,
                                                         Arrays.asList(attrs),
                                                         pageSize,
                                                         prev_pagedResult);
                if (pagedResult != null)
                {
                    pageCount++;

                    ILdapEntry[] entries = pagedResult.getEntries();

                    Assert.assertNotNull(entries);
                    Assert.assertTrue(entries.length > 0);

                    for (ILdapEntry entry : entries)
                    {
                        entryCountInOnePage++;
                        String[] attrNames = entry.getAttributeNames();

                        Assert.assertNotNull(attrNames);
                        System.out.println(String.format("Entry (DN: %s)", entry.getDN()));

                        for (String attrName : attrNames)
                        {
                            System.out.println(String.format("\tAttribute (Name: %s)", attrName));

                            LdapValue[] values = entry.getAttributeValues(attrName);

                            // handle values that are not ASCII
                            for (LdapValue value : values)
                            {
                                byte[] bytes = value.getValue();

                                ByteBuffer bb = ByteBuffer.allocate(bytes.length);
                                bb.put(bytes);

                                String val = new String(bb.array());

                                System.out.println(String.format("\t\tValue : %s", val));
                             }

                             Assert.assertNotNull(values);
                             Assert.assertTrue(values.length >= 1);
                         }
                     }
                     System.out.println(String.format("One page contains %d objects", entryCountInOnePage));
                     entryCountInOnePage = 0;
                 }
            }
            finally
            {
                if (prev_pagedResult != null)
                {
                    prev_pagedResult.close();
                }
                prev_pagedResult = pagedResult;
                pagedResult = null;
            }
        }

        if (prev_pagedResult != null)
        {
            prev_pagedResult.close();
        }

        return pageCount;
    }

    // return how many paged search has done
    private int performPagedSearch(
        ILdapConnectionEx connection,
        String baseDN,
        String filter,
        String[] attrs,
        int pageSize
        )
    {
        int entryCountInOnePage = 0;

        Collection<ILdapMessage> messages = connection.paged_search(
                                                           baseDN,
                                                           LdapScope.SCOPE_SUBTREE,
                                                           filter,
                                                           Arrays.asList(attrs),
                                                           pageSize,
                                                           -1);
        if (messages == null || messages.size() == 0)
            return 0;

        for (ILdapMessage message : messages)
        {
            try
            {
                ILdapEntry[] entries = message.getEntries();

                Assert.assertNotNull(entries);
                Assert.assertTrue(entries.length > 0);

                for (ILdapEntry entry : entries)
                {
                    entryCountInOnePage++;
                    String[] attrNames = entry.getAttributeNames();

                    Assert.assertNotNull(attrNames);
                    System.out.println(String.format("Entry (DN: %s)", entry.getDN()));

                    for (String attrName : attrNames)
                    {
                        System.out.println(String.format("\tAttribute (Name: %s)", attrName));

                        LdapValue[] values = entry.getAttributeValues(attrName);

                        // handle values that are not ASCII
                        for (LdapValue value : values)
                        {
                            byte[] bytes = value.getValue();

                            ByteBuffer bb = ByteBuffer.allocate(bytes.length);
                            bb.put(bytes);

                            String val = new String(bb.array());

                            System.out.println(String.format("\t\tValue : %s", val));
                         }

                         Assert.assertNotNull(values);
                         Assert.assertTrue(values.length >= 1);
                     }
                 }
                 System.out.println(String.format("One page contains %d objects", entryCountInOnePage));
                 entryCountInOnePage = 0;
             }
             finally
             {
                 message.close();
             }
         }

         return messages.size();
    }

    @Test
    public void testLdapPagedSearchAgainstAD() throws Exception
    {
        System.out.println("Executing testLdapPagedSearchAgainstAD() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx> ();
        connections.add(LdapConnectionFactoryEx.getInstance().getLdapConnection(adHostname, adPort));
        if(Platform.isWindows()){
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(adHostname, adPort);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);

            ILdapMessage message = null;
            // Find all the Users (there are 2200 users)
            String baseDN = "CN=USERS,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM";
            String[] attrs = { "sAMAccountName", "userPrincipalName",
                    "description", "givenName", "sn", "mail",
                    "userAccountControl", null };
            String filter = "(&(objectClass=user)(cn=*John*))";

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                boolean getAttrsOnly = false;
                message = connection.search(baseDN, LdapScope.SCOPE_SUBTREE,
                        filter, attrs, getAttrsOnly);
            } catch (SizeLimitExceededLdapException e) {
                // Do paged search
                int pageCount = performPagedSearch(connection, baseDN, filter,
                        attrs, 1000);
                Assert.assertEquals(3, pageCount);
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();
            }
        }
    }

    @Test
    public void testLdapPagedSearchOneByOneAgainstAD() throws Exception
    {
        System.out.println("Executing testLdapPagedSearchAgainstAD() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx> ();
        connections.add(LdapConnectionFactoryEx.getInstance().getLdapConnection(adSaslHostname, adPort));
        if(Platform.isWindows()){
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(adSaslHostname, adPort);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);

            ILdapMessage message = null;
            // Find all the Users (there are 2200 users)
            String baseDN = "CN=USERS,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM";
            String[] attrs = { "sAMAccountName", "userPrincipalName",
                    "description", "givenName", "sn", "mail",
                    "userAccountControl", null };
            String filter = "(&(objectClass=user)(cn=*John*))";

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                boolean getAttrsOnly = false;
                message = connection.search(baseDN, LdapScope.SCOPE_SUBTREE,
                        filter, attrs, getAttrsOnly);
            } catch (SizeLimitExceededLdapException e) {
                // Do paged search
                int pageCount = performPagedSearch_with_one_page(connection,
                        baseDN, filter, attrs, 1000);
                Assert.assertEquals(3, pageCount);
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();
            }
        }
    }

    @Test
    public void testLdapPagedSearchAgainstOpenLdap() throws Exception
    {
        System.out.println("Executing testLdapPagedSearchAgainstOpenLdap() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx>();
        connections.add(LdapConnectionFactoryEx.getInstance()
                .getLdapConnection(olHostname, olPort));
        if (Platform.isWindows()) {
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(olHostname, olPort);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);

            ILdapMessage message = null;

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindOlDN, olPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                // Find all the Users
                String baseDN = "DC=SSOLABS-OPENLDAP,DC=ENG,DC=VMWARE,DC=COM";
                String objectClassAttrName = "objectClass";
                String isDeletedAttrName = "isDeleted";
                String[] attrs = { objectClassAttrName, isDeletedAttrName, null };
                String filter = "(objectClass=inetOrgPerson)";

                // In openldap provider there are over 2214 inetOrgPerson
                // objects one-level under
                // "DC=SSOLABS-OPENLDAP,DC=ENG,DC=VMWARE,DC=COM"
                int pageCount = performPagedSearch(connection, baseDN, filter,
                        attrs, 3);
                Assert.assertEquals(2214 / 3 + 1, pageCount);
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();
            }
        }
    }

    @Test
    public void testLdapPagedSearchAgainstLotus() throws Exception
    {
        System.out.println("Executing testLdapPagedSearchAgainstLotus() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx>();
        connections.add(LdapConnectionFactoryEx.getInstance()
                .getLdapConnection(hostname, port));
        if (Platform.isWindows()) {
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);
            ILdapMessage message = null;
            boolean bExceptionThrownAsExpected = false;

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindDN, password,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                // Find all the Users
                String baseDN = "CN=Users,DC=vsphere,DC=local";
                String objectClassAttrName = "objectClass";
                String isDeletedAttrName = "isDeleted";
                String[] attrs = { objectClassAttrName, isDeletedAttrName, null };
                String filter = "(objectClass=*)";

                performPagedSearch(connection, baseDN, filter, attrs, 3);

            } catch (ControlNotFoundLdapException e) {
                bExceptionThrownAsExpected = true;
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();

                Assert.assertFalse(bExceptionThrownAsExpected);
            }
        }
    }

    @Test
    public void testLdapSizeLimitedExtSearchAgainstAD() throws Exception
    {
        System.out.println("Executing testLdapSizeLimitedExtSearchAgainstAD() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx>();
        connections.add(LdapConnectionFactoryEx.getInstance()
                .getLdapConnection(adHostname, adPort));
        if (Platform.isWindows()) {
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(adHostname, adPort);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);

            ILdapMessage message = null;
            // Find all the Users (there are 2200 users)
            String baseDN = "CN=USERS,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM";
            String[] attrs = { "sAMAccountName", "userPrincipalName",
                    "description", "givenName", "sn", "mail",
                    "userAccountControl", null };
            String filter = "(&(objectClass=user)(cn=*John*))";

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                message = connection.search_ext(baseDN,
                        LdapScope.SCOPE_SUBTREE, filter, Arrays.asList(attrs),
                        false, null, null, null, 100);
                Assert.assertNotNull("We should be able to retrieve message.",
                        message);
                ILdapMessageEx mex = (ILdapMessageEx) message;
                Assert.assertEquals("Number of entries should be as expected.",
                        100, mex.getEntriesCount());
                ILdapEntry[] entries = mex.getEntries();
                Assert.assertNotNull("We should be able to retrieve entries.",
                        entries);
                Assert.assertEquals("Number of entries should be as expected.",
                        100, entries.length);
                for (ILdapEntry entry : entries) {
                    String[] attr = entry.getAttributeNames();
                    for (int i = 0; i < attr.length; i++) {
                        LdapValue[] vals = entry.getAttributeValues(attr[i]);
                    }
                }

                try {
                    message = connection.search_ext(baseDN,
                            LdapScope.SCOPE_SUBTREE, filter,
                            Arrays.asList(attrs), false, null, null, null,
                            1000000);
                    Assert.fail("Retrieving 1000000 entries should fail with server sending SizeLimitExceeded exception.");
                } catch (SizeLimitExceededLdapException ex) {
                    // expected
                }
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();
            }
        }
    }

    @Test
    public void testLdapSizeLimitedExtSearchAgainstOpenLdap() throws Exception
    {
        System.out.println("Executing testLdapSizeLimitedExtSearchAgainstOpenLdap() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx>();
        connections.add(LdapConnectionFactoryEx.getInstance()
                .getLdapConnection(olHostname, olPort));
        if (Platform.isWindows()) {
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(olHostname, olPort);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }

        for (ILdapConnectionEx connection : connections) {
            Assert.assertNotNull(connection);

            ILdapMessage message = null;

            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.bindConnection(bindOlDN, olPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);

                // Find all the Users
                String baseDN = "DC=SSOLABS-OPENLDAP,DC=ENG,DC=VMWARE,DC=COM";
                String objectClassAttrName = "objectClass";
                String[] attrs = { objectClassAttrName, "uid", "cn",
                        "displayName", null };
                String filter = "(objectClass=inetOrgPerson)";

                // In openldap provider there are over 2214 inetOrgPerson
                // objects one-level under
                // "DC=SSOLABS-OPENLDAP,DC=ENG,DC=VMWARE,DC=COM"
                message = connection.search_ext(baseDN,
                        LdapScope.SCOPE_SUBTREE, filter, Arrays.asList(attrs),
                        false, null, null, null, 100);
                Assert.assertNotNull("We should be able to retrieve message.",
                        message);
                ILdapMessageEx mex = (ILdapMessageEx) message;
                Assert.assertEquals("Number of entries should be as expected.",
                        100, mex.getEntriesCount());
                ILdapEntry[] entries = mex.getEntries();
                Assert.assertNotNull("We should be able to retrieve entries.",
                        entries);
                Assert.assertEquals("Number of entries should be as expected.",
                        100, entries.length);
                for (ILdapEntry entry : entries) {
                    String[] attr = entry.getAttributeNames();
                    for (int i = 0; i < attr.length; i++) {
                        LdapValue[] vals = entry.getAttributeValues(attr[i]);
                    }
                }
            } finally {
                if (message != null) {
                    message.close();
                }

                connection.close();
            }
        }
    }

    @Test
    public void testLdapsConnection() throws Exception
    {
       List<LdapSetting> connOptionsBase = Arrays.asList(
             new LdapSetting(LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3),
             new LdapSetting(LdapOption.LDAP_OPT_REFERRALS, Boolean.FALSE),
             new LdapSetting(LdapOption.LDAP_OPT_NETWORK_TIMEOUT, DEFAULT_LDAP_NETWORK_TIMEOUT)
             );
       String   baseDN = "CN=USERS,DC=SSOLABS,DC=ENG,DC=VMWARE,DC=COM";
       String[] attrs = { "sAMAccountName", "userPrincipalName", null };
       String   filter = "(&(objectClass=user)(cn=Administrator))";

       ILdapMessage message = null;

       List<LdapSetting> connOptionTLSNever = new ArrayList<LdapSetting>(connOptionsBase);
       connOptionTLSNever.add(
             new LdapSetting(LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT, LdapConstants.LDAP_OPT_X_TLS_NEVER));

       List<ILdapConnectionEx> connections1 = new ArrayList<ILdapConnectionEx>();
       connections1.add(LdapConnectionFactoryEx.getInstance()
               .getLdapConnection(hostname, port));
       if (Platform.isWindows()) {
           URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
           List<LdapSetting> connOptions = Collections.emptyList();
           connections1.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
       }

       for (ILdapConnectionEx conn : connections1) {
            try {
                conn = LdapConnectionFactoryEx.getInstance().getLdapConnection(
                        ldapsURI, connOptionTLSNever);
                conn.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);
                message = conn.search(baseDN, LdapScope.SCOPE_SUBTREE, filter,
                        attrs, false);
                Assert.assertTrue("search result cannot be null",
                        message != null);
            } finally {
                if (message != null) {
                    message.close();
                }
                conn.close();
            }
       }

       List<LdapSetting> connOptionFpCheckMatched = new ArrayList<LdapSetting>(connOptionsBase);
       connOptionFpCheckMatched.add(
             new LdapSetting(LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT, LdapConstants.LDAP_OPT_X_TLS_DEMAND));
       connOptionFpCheckMatched.add(
             new LdapSetting(LdapOption.LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK,
                   new ISslX509VerificationCallback() {
                    @Override
                    public boolean isTrustedCertificate(X509Certificate cert) {
                        return true;
                    }
             }));
       List<ILdapConnectionEx> connections2 = new ArrayList<ILdapConnectionEx>();
       connections2.add(LdapConnectionFactoryEx.getInstance()
               .getLdapConnection(hostname, port));
       if (Platform.isWindows()) {
           URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
           List<LdapSetting> connOptions = Collections.emptyList();
           connections2.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
       }

        for (ILdapConnectionEx conn : connections2) {
            try {
                conn = LdapConnectionFactoryEx.getInstance().getLdapConnection(
                        ldapsURI,
                        Collections.unmodifiableList(connOptionFpCheckMatched));
                conn.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);
                message = conn.search(baseDN, LdapScope.SCOPE_SUBTREE, filter,
                        attrs, false);
                Assert.assertTrue("search result cannot be null",
                        message != null);
            } finally {
                if (message != null) {
                    message.close();
                }
                conn.close();
            }
        }

       List<LdapSetting> connOptionFpCheckMismatched = new ArrayList<LdapSetting>(connOptionsBase);
       connOptionFpCheckMismatched.add(
             new LdapSetting(LdapOption.LDAP_OPT_X_TLS_REQUIRE_CERT, LdapConstants.LDAP_OPT_X_TLS_DEMAND));
       connOptionFpCheckMismatched.add(
             new LdapSetting(LdapOption.LDAP_OPT_X_CLIENT_TRUSTED_FP_CALLBACK,
                   new ISslX509VerificationCallback() {
                     @Override
                     public boolean isTrustedCertificate(X509Certificate cert) {
                        return false;
                     }
                   })
             );
       List<ILdapConnectionEx> connections3 = new ArrayList<ILdapConnectionEx>();
       connections3.add(LdapConnectionFactoryEx.getInstance()
               .getLdapConnection(hostname, port));
       if (Platform.isWindows()) {
           URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
           List<LdapSetting> connOptions = Collections.emptyList();
           connections3.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
       }

        for (ILdapConnectionEx conn : connections3) {
            try {
                conn = LdapConnectionFactoryEx.getInstance().getLdapConnection(
                        ldapsURI,
                        Collections
                                .unmodifiableList(connOptionFpCheckMismatched));
                conn.bindConnection(bindAdDN, adPassword,
                        LdapBindMethod.LDAP_BIND_SIMPLE);
                Assert.fail("should not be able to establish connection when finger print is mismatching");
            } catch (Exception e) {
                Assert.assertTrue(e instanceof ServerDownLdapException);
            } finally {
                conn.close();
            }
        }
    }

    private String addObject(ILdapConnectionEx connection, String groupName,
            String desc) {
        ArrayList<LdapMod> attributeList = new ArrayList<LdapMod>();
        String dn = String.format("CN=%s,%s", groupName, bindDN);

        LdapMod objectClass = new LdapMod(LdapModOperation.ADD, "objectclass",
                new LdapValue[] { LdapValue.fromString("group"), });
        attributeList.add(objectClass);

        LdapMod attrName = new LdapMod(LdapModOperation.ADD, "name",
                new LdapValue[] { LdapValue.fromString(groupName) });
        attributeList.add(attrName);

        LdapMod attrCN = new LdapMod(LdapModOperation.ADD, "cn",
                new LdapValue[] { LdapValue.fromString(groupName) });
        attributeList.add(attrCN);

        LdapMod attrAccount = new LdapMod(LdapModOperation.ADD,
                "sAMAccountName",
                new LdapValue[] { LdapValue.fromString(groupName) });
        attributeList.add(attrAccount);

        LdapMod attrDescription = new LdapMod(LdapModOperation.ADD,
                "description", new LdapValue[] { LdapValue.fromString(desc) });
        attributeList.add(attrDescription);

        try {
            connection.addObject(dn,
                    attributeList.toArray(new LdapMod[attributeList.size()]));
        } catch (AlreadyExistsLdapException e) {

        }

        return dn;
    }

    private void modifyObject(ILdapConnectionEx connection, String groupName,
            String desc) {
        String dn = String.format("CN=%s,%s", groupName, bindDN);
        connection.modifyObject(dn, new LdapMod(LdapModOperation.ADD,
                "description", new LdapValue[] { LdapValue.fromString(desc) }));
        return;
    }

    @Test
    public void testUpdateLdapObject() throws Exception {
        System.out.println("Executing testUpdateLdapObject() test");
        List<ILdapConnectionEx> connections = new ArrayList<ILdapConnectionEx>();
        connections.add(LdapConnectionFactoryEx.getInstance()
                .getLdapConnection(hostname, port));
        if (Platform.isWindows()) {
            URI uri = PlatformUtils.getConnectionUriDefaultScheme(hostname, port);
            List<LdapSetting> connOptions = Collections.emptyList();
            connections.add((ILdapConnectionEx)LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true));
        }
        for (ILdapConnectionEx connection : connections) {
            String dn = null;
            try {
                connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION,
                        LdapConstants.LDAP_VERSION3);
                connection.setOption(LdapOption.LDAP_OPT_REFERRALS, false);
                connection.bindConnection(bindDN, password,
                        LdapBindMethod.LDAP_BIND_SIMPLE);
                dn = addObject(connection, "Test", "1");
                modifyObject(connection, "Test", "2");
            } finally {
                if (dn != null) {
                    connection.deleteObject(dn);
                }
                connection.close();
            }
        }
    }

    public static void main(String args[]) {
      org.junit.runner.JUnitCore.main("com.vmware.identity.interop.LdapTest");
    }
}
