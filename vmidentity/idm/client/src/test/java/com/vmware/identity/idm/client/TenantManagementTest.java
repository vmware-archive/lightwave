/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
*/
/**
 * VMware Identity Service
 *
 * Tenant Manager Tests
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-9
 *
 */

package com.vmware.identity.idm.client;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Callable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import junit.framework.Assert;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.SystemUtils;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.CertificateInUseException;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.DuplicateProviderException;
import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.HashingAlgorithmType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.NoSuchCertificateException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchRelyingPartyException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.Principal;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.performanceSupport.IIdmAuthStat;

@RunWith(OrderedRunner.class)
public final class TenantManagementTest
{
    // built-in group which is created by default
    private static final String ADMIN_GROUP_NAME = "Administrators";

    private final String CFG_KEY_IDM_SYSTEM_TENANT_NAME =
            "idm.server.service-provider-name";
    private final String CFG_KEY_IDM_HOSTNAME = "idm.server.hostname";
    private final String CFG_KEY_IDM_TENANT_1_NAME = "idm.tenant-1.name";
    private final String CFG_KEY_IDM_TENANT_1_ADMIN_ACCOUNT_NAME = "idm.tenant-1.adminAccountName";
    private final String CFG_KEY_IDM_TENANT_1_ADMIN_PWD = "idm.tenant-1.adminPwd";
    private final String CFG_KEY_IDM_TENANT_1_LONG_NAME =
            "idm.tenant-1.long-name";
    // keystore to feed information to 'setTenantCredentials'
    private final String CFG_KEY_STS_KEY_ALIAS = "idm.server.stskey-alias";
    private final String CFG_KEY_STS_KEY_ALIAS1 = "idm.server.stskey-alias1";
    private final String CFG_KEY_STS_KEYSTORE = "idm.server.stskey-store";
    private final String CFG_KEY_STS_KEYSTORE_PASSWORD =
            "idm.server.stskey-store-password";
    // keystore to feed information to 'setTrustedTenantCredentials'
    private final String CFG_KEY_TRUSTED_STS_KEY_ALIAS =
            "idm.server.trusted-stskey-alias";
    private final String CFG_KEY_TRUSTED_STS_KEY_ALIAS1 =
            "idm.server.trusted-stskey-alias1";
    private final String CFG_KEY_TRUSTED_STS_KEYSTORE =
            "idm.server.trusted-stskey-store";
    private final String CFG_KEY_TRUSTED_STS_KEYSTORE_PASSWORD =
            "idm.server.trusted-stskey-store-password";

    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME =
            "idm.tenant-1.ad-provider-1.domain-name";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS =
            "idm.tenant-1.ad-provider-1.alias";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_UPN =
            "idm.tenant-1.ad-provider-1.bind-upn";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN =
            "idm.tenant-1.ad-provider-1.bind-dn";
    private final String CFG_KEY_IDM_TENANT_1_KDC =
            "idm.tenant-1.ad-provider-1.kdc-1";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD =
            "idm.tenant-1.ad-provider-1.password";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_USER_BASE_DN =
            "idm.tenant-1.ad-provider-1.user-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_GROUP_BASE_DN =
            "idm.tenant-1.ad-provider-1.group-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID =
            "idm.tenant-1.ad-provider-1.user-1-id";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_PASSWORD =
            "idm.tenant-1.ad-provider-1.user-1-password";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_NONAME_ID =
            "idm.tenant-1.ad-provider-1.user-2-id";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_DEFAULT_DOMAIN_POLICY_TEST_USER =
            "idm.tenant-1.ad-provider-1.defDomPlyTestUser";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST_ID =
            "idm.tenant-1.ad-provider-1.user-3-id";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST1_ID =
            "idm.tenant-1.ad-provider-1.user-4-id";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_FSPTEST1_ID =
            "idm.tenant-1.ad-provider-1.subgroup-1-id";
    private final String CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_FSPTEST2_ID =
            "idm.tenant-1.ad-provider-1.subgroup-2-id";

    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.domain-name";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.alias";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.bind-dn";
    private final String CFG_KEY_IDM_TENANT_1_KDC_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.kdc-1";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.password";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_USER_BASE_DN_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.user-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_GROUP_BASE_DN_LDAPS =
          "idm.tenant-1.ad-ldaps-provider-1.group-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_AD_PROVIDER_CERT_ALIAS_LDAPS =
        "idm.tenant-1.ad-ldaps-provider-1.cert.alias";

    private final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME =
            "idm.tenant-1.ol-provider-1.domain-name";
    private final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS =
            "idm.tenant-1.ol-provider-1.alias";
    private final String CFG_KEY_IDM_TENANT_1_OL_HOST_NAME =
            "idm.tenant-1.ol-provider-1.host-name";
    private final String CFG_KEY_IDM_TENANT_1_OL_SSL_HOST_NAME =
            "idm.tenant-1.ol-provider-1.host-name-1";
    private final String CFG_KEY_IDM_TENANT_1_OL_SSL_CERT_ALIAS =
	    "idm.tenant-1.ol-provider-1.cert-alias";
    private final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN =
            "idm.tenant-1.ol-provider-1.bind-dn";
    private final String CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD =
            "idm.tenant-1.ol-provider-1.password";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_BASE_DN =
            "idm.tenant-1.ol-provider-1.user-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_BASE_DN =
            "idm.tenant-1.ol-provider-1.group-search-base-dn";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST_ID =
            "idm.tenant-1.ol-provider-1.user-1-id";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST1_ID =
            "idm.tenant-1.ol-provider-1.user-2-id";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST1_ID =
            "idm.tenant-1.ol-provider-1.subgroup-1-id";
    private final String CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST2_ID =
            "idm.tenant-1.ol-provider-1.subgroup-2-id";

    private final String CFG_KEY_IDM_TENANT_2_NAME = "idm.tenant-2.name";

    // tenant that will not be added (for tenant negative tests)
    private final String CFG_KEY_IDM_TENANT_3_NAME = "idm.tenant-3.name";

    private final String CFG_KEY_IDM_PROBECONNECTIVITY_TENANTNAME =
            "idm.probeConnectivity.tenantName";
    private final String CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI =
            "idm.probeConnectivity.providerUri";
    private final String CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME =
            "idm.probeConnectivity.userName";
    private final String CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD =
            "idm.probeConnectivity.password";
    private final String CFG_KEY_IDM_PROBECONNECTIVITY_BADPASSWORD =
            "idm.probeConnectivity.badPassword";

    private final String CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME =
            "idm.AcctFlags.domainName";
    private final String CFG_KEY_IDM_ACCTFLAGS_USERNAME =
            "idm.AcctFlags.userName";
    private final String CFG_KEY_IDM_ACCTFLAGS_DETAILS_FIRSTNAME =
            "idm.AcctFlags.details.firstName";
    private final String CFG_KEY_IDM_ACCTFLAGS_DETAILS_LASTNAME =
            "idm.AcctFlags.details.lastName";
    private final String CFG_KEY_IDM_ACCTFLAGS_DETAILS_EMAILADDR =
            "idm.AcctFlags.details.emailAddr";
    private final String CFG_KEY_IDM_ACCTFLAGS_DETAILS_DESCRIPTION =
            "idm.AcctFlags.details.description";
    private final String CFG_KEY_IDM_ACCTFLAGS_PASSWORD =
            "idm.AcctFlags.password";

    private final String CFG_KEY_IDM_ACCTFLAGS_PWDEXPIRED_USERNAME =
            "idm.AcctFlags.pwdExpired.userName";
    private final String CFG_KEY_IDM_ACCTFLAGS_PWDEXPIRED_PASSWORD =
            "idm.AcctFlags.pwdExpired.password";

    private final String CFG_KEY_IDM_ACCTFLAGS_LOCKEDUSER_USERNAME =
            "idm.AcctFlags.lockedUser.userName";
    private final String CFG_KEY_IDM_ACCTFLAGS_LOCKEDUSER_PASSWORD =
            "idm.AcctFlags.lockedUser.password";

    // Test admin user credentials
    private static final String DEFAULT_TENANT_ADMIN_NAME     = "Administrator";
    private static final String DEFAULT_TENANT_ADMIN_PASSWORD = "defaultPwd#1";

    private Properties _testProps;
    private CasIdmClient _idmClient;

    private static final String ATTR_NAME_IS_SOLUTION =
            "http://vmware.com/schemas/attr-names/2011/07/isSolution";
    private static final String ATTR_NAME_UPN =
            "http://schemas.xmlsoap.org/claims/UPN";
    private static final String ATTRIBUTE_GROUPS =
            "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";

    private final String _exportedConfigFile = "export-config.xml";
    private final String _expTenantName = "xyz";
    private final String _impExternalIDPConfigFile = "/openam.xml";
    private final String _impExternalIDPNoSLOConfigFile = "/openam-noSLO.xml";
    private final String _expCastleAsSPProfileFile = "export-SPProfile.xml";
    private final String _expCastleAsSPProfileFileNoOptionalExternalIDPData =
            "export-SPProfile-NoExternalIDP.xml";
    private final String _impTenantName = "xyz";
    private final String _impTenantConfigFile = "/saml-metadata-test.xml";

    private final String _solution_user_name_non_sp = "invsvc";
    private final String _solution_user_name_non_sp_no_cert = "invsvc-no-cert";
    private final String _solution_user_name_sp = "invsvc-sp-in-sys-domain";
    private final String _solution_user_name_sp_no_cert =
            "invsvc-sp-in-sys-domain-no-cert";

    private final String CFG_KEY_I18N_USERNAME_1 = "idm.I18N.Lotus.userName.1";
    private final String CFG_KEY_I18N_PASSWORD_1 = "idm.I18N.Lotus.password.1";
    private final String CFG_KEY_I18N_FIRSTNAME_1 =
            "idm.I18N.Lotus.givenName.1";
    private final String CFG_KEY_I18N_LASTNAME_1 = "idm.I18N.Lotus.sn.1";

    private final String CFG_KEY_I18N_DESCRIPTION_COMMON =
            "idm.I18N.Lotus.desc";

    private final String CFG_KEY_I18N_USERNAME_2 = "idm.I18N.Lotus.userName.2";
    private final String CFG_KEY_I18N_PASSWORD_2 = "idm.I18N.Lotus.password.2";
    private final String CFG_KEY_I18N_FIRSTNAME_2 =
            "idm.I18N.Lotus.givenName.2";
    private final String CFG_KEY_I18N_LASTNAME_2 = "idm.I18N.Lotus.sn.2";

    private final String CFG_KEY_ALL_TENANTS_NON_EXISTENT_PRINCIPAL_NAME =
            "idm.AllTenants.nonExistentPrincipalName";

    private final String SAMLATTR_GIVEN_NAME =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
    private final String SAMLATTR_SUR_NAME =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
    private final String SAMLATTR_GROUP_IDENTITY =
            "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";

    private final String SAMLATTR_EMAIL_ADDRESS =
            "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";
    private final String SAMLATTR_UPN = "http://schemas.xmlsoap.org/claims/UPN";

    private final String CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID =
            "idm.external-idp.1.entity-id";
    private final String CFG_KEY_EXTERNAL_IDP_1_NAME_ID_FORMATS =
            "idm.external-idp.1.name-id-formats";

    private final String CFG_KEY_EXTERNAL_IDP_1_SSO_NAME_1 =
            "idm.external-idp.1.sso.name.1";
    private final String CFG_KEY_EXTERNAL_IDP_1_SSO_LOCATION_1 =
            "idm.external-idp.1.sso.location.1";
    private final String CFG_KEY_EXTERNAL_IDP_1_SSO_BINDING_1 =
            "idm.external-idp.1.sso.binding.1";

    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_NAME_1 =
            "idm.external-idp.1.slo.name.1";
    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_LOCATION_1 =
            "idm.external-idp.1.slo.location.1";
    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_BINDING_1 =
            "idm.external-idp.1.slo.binding.1";

    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_NAME_2 =
            "idm.external-idp.1.slo.name.2";
    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_LOCATION_2 =
            "idm.external-idp.1.slo.location.2";
    private final String CFG_KEY_EXTERNAL_IDP_1_SLO_BINDING_2 =
            "idm.external-idp.1.slo.binding.2";

    private final String CFG_KEY_EXTERNAL_IDP_2_ENTITY_ID =
            "idm.external-idp.2.entity-id";
    private final String CFG_KEY_EXTERNAL_IDP_2_NAME_ID_FORMATS =
            "idm.external-idp.2.name-id-formats";

    private final String CFG_KEY_EXTERNAL_IDP_2_SSO_NAME_1 =
            "idm.external-idp.2.sso.name.1";
    private final String CFG_KEY_EXTERNAL_IDP_2_SSO_LOCATION_1 =
            "idm.external-idp.2.sso.location.1";
    private final String CFG_KEY_EXTERNAL_IDP_2_SSO_BINDING_1 =
            "idm.external-idp.2.sso.binding.1";

    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_NAME_1 =
            "idm.external-idp.2.slo.name.1";
    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_LOCATION_1 =
            "idm.external-idp.2.slo.location.1";
    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_BINDING_1 =
            "idm.external-idp.2.slo.binding.1";

    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_NAME_2 =
            "idm.external-idp.2.slo.name.2";
    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_LOCATION_2 =
            "idm.external-idp.2.slo.location.2";
    private final String CFG_KEY_EXTERNAL_IDP_2_SLO_BINDING_2 =
            "idm.external-idp.2.slo.binding.2";

    private final String CFG_IDENTITY_STORE_HINT_ATTR_NAME = "givenName";
    private final boolean CFG_IDENTITY_STORE_LINK_USE_UPN = false;

    private static final String TENANT_PSC_SITE_1 = "psc-site-1";
    private static final String TENANT_PSC_SITE_2 = "psc-site-2";

    private final String CFG_KEY_EXTERNAL_IDP_STS_STORE = "idm.external-idp.sts.store";
    private final String CFG_KEY_EXTERNAL_IDP_STS_STORE_PASS = "idm.external-idp.sts.store.pass";
    private final String CFG_KEY_EXTERNAL_IDP_STS_ALIAS_ROOT_CERT = "idm.external-idp.sts.alias.rootCert";
    private final String CFG_KEY_EXTERNAL_IDP_STS_ALIAS_LEAF_CERT = "idm.external-idp.sts.alias.leafCert";
    private final String USERS_GROUP = "Users";

    @Test
    @TestOrderAnnotation(order = 0)
    public void setupExternalIdpConfig() throws Exception
    {
        //setup externalIDP configuration for vsphere.local
        final String ISSUER_ID =
                "https://etco-vm-vlan1301-dhcp-140-90.eng.vmware.com/websso/SAML2/Metadata/vsphere.local";
        CasIdmClient client = getIdmClient();
        Properties props = getTestProperties();
        String tenantName = "vsphere.local";
        IdmClientTestUtil.ensureTenantExists(client, tenantName);
        List<X509Certificate> x509Certs = buildExternalIdpSTSKeyCertificates();
        IDPConfig idpConfig = new IDPConfig(ISSUER_ID);
        idpConfig.setNameIDFormats(Arrays.asList(props.getProperty(
                CFG_KEY_EXTERNAL_IDP_1_NAME_ID_FORMATS).split(",")));
        idpConfig.setSsoServices(buildSSOServicesIdp1(props));
        idpConfig.setSloServices(buildSLOServicesIdp1(props));
        idpConfig.setSigningCertificateChain(x509Certs);

        client.setExternalIdpConfig(tenantName, idpConfig);
    }

    @TestOrderAnnotation(order = 1)
    @Test
    public void testAddTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        Tenant tenantToCreate = new Tenant(tenantName);

        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);

        idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());

        try
        {
            Tenant tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
            Assert.assertEquals(tenantName, tenant.getName());
        } catch (NoSuchTenantException ex)
        {
            Assert.fail("should not reach here");
        }

        try
        {
            idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
        } catch (DuplicateTenantException ex)
        {
            // cleanup
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
            return;
        }

        Assert.fail("Should not reach here, proper exception is thrown and quit");
    }

    @TestOrderAnnotation(order = 2)
    @Test
    public void testGetSystemTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String systemTenantName = idmClient.getSystemTenant();

        Assert.assertNotNull(systemTenantName);

        String realSystemTenantName =
                props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        Assert.assertNotNull(realSystemTenantName);

        Assert.assertEquals(systemTenantName.toLowerCase(),
                realSystemTenantName.toLowerCase());
    }

    @TestOrderAnnotation(order = 3)
    @Test
    public void testImportExportExternalIDPConfiguration() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        DocumentBuilderFactory builderFactory =
                DocumentBuilderFactory.newInstance();

        builderFactory.setNamespaceAware(true);

        DocumentBuilder builder = builderFactory.newDocumentBuilder();

        builder.setErrorHandler(new SamlParserErrorHandler());

        Document externalIDPDoc =
                builder.parse(getClass().getResourceAsStream(
                        _impExternalIDPConfigFile));

        Document externalIDPNoSLODoc =
                builder.parse(getClass().getResourceAsStream(
                        _impExternalIDPNoSLOConfigFile));

        IdmClientTestUtil.ensureTenantExists(idmClient, _impTenantName);

        //get the certificates in order and key to setup the tenant's credentials
        String password = props.getProperty(CFG_KEY_STS_KEYSTORE_PASSWORD);

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);
        Certificate certForPrivKeyEntry =
                ks.getCertificate(props.getProperty(CFG_KEY_STS_KEY_ALIAS));
        Certificate certAlias1 =
                ks.getCertificate(props.getProperty(CFG_KEY_STS_KEY_ALIAS1));

        PrivateKey key =
                (PrivateKey) ks.getKey(
                        props.getProperty(CFG_KEY_STS_KEY_ALIAS),
                        password.toCharArray());

        idmClient.setTenantCredentials(_impTenantName,
                Arrays.asList(certForPrivKeyEntry, certAlias1), key);


        String importedEntityId = null;
        try
        {
            //import
            importedEntityId =
                    idmClient.importExternalIDPConfiguration(_impTenantName,
                            externalIDPNoSLODoc);
            importedEntityId =
                    idmClient.importExternalIDPConfiguration(_impTenantName,
                            externalIDPDoc);
            Collection<IDPConfig> idpConfigs =
                    idmClient.getAllExternalIdpConfig(_impTenantName);
            Assert.assertEquals(idpConfigs.size(), 1);

            //export
            // include optional data for external IDPs
            Document castleAsSPProfileDoc =
                    idmClient.exportExternalIDPFederation(_impTenantName, true);
            persistDoc(castleAsSPProfileDoc, _expCastleAsSPProfileFile);
            loadFileAndvalidate(idmClient, _expCastleAsSPProfileFile);

            // w/o optional data
            castleAsSPProfileDoc =
                    idmClient.exportExternalIDPFederation(_impTenantName, false);
            persistDoc(castleAsSPProfileDoc,
                    _expCastleAsSPProfileFileNoOptionalExternalIDPData);
            loadFileAndvalidate(idmClient,
                    _expCastleAsSPProfileFileNoOptionalExternalIDPData);
        } finally
        {
            //cleanup, note that any partial import has been clean up by the import API
            if (null != importedEntityId)
            {
                idmClient.removeExternalIdpConfig(_impTenantName,
                        importedEntityId);
            }
        }
    }

    @TestOrderAnnotation(order = 4)
    @Test
    public void testQueryANotAddedTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_3_NAME);
        Assert.assertNotNull(tenantName);

        boolean bLoginFailure = false;

        // make sure 'authenticate' only throws 'login failed exception'
        try
        {
            idmClient.authenticate(tenantName, "dummy", "dummypassword");
        } catch (IDMLoginException ex)
        {
            bLoginFailure = true;
        }
        Assert.assertTrue(bLoginFailure);

        try
        {
            idmClient.getClockTolerance(tenantName);
        } catch (NoSuchTenantException ex)
        {
            // test one more API
            try
            {
                idmClient.getMaximumBearerTokenLifetime(tenantName);
            } catch (NoSuchTenantException subEx)
            {
                return;
            }

            Assert.fail("tenant does not exists, should not reach here");
        }

        Assert.fail("tenant does not exists, should not reach here");
    }

    @Ignore("bugzilla#1173915 - this seems to require native ad ...also importTenantConfiguration is nolonger support backing up idp configuration via xml")
    @TestOrderAnnotation(order = 5)
    @Test
    public void testImportTenantConfiguration() throws Exception, IDMException
    {
        // NOTE: this test reads xml metadata from saml-metadata-test.xml file
        // within client/test/resources
        // This file has a ValidUntil setting and if this test fails, one of the
        // possibilities is that this file has expired. Checking ValidUntil element
        // within that file might be a good idea.
        CasIdmClient idmClient = getIdmClient();

        boolean newTenant = false;

        try
        {
            DocumentBuilderFactory builderFactory =
                    DocumentBuilderFactory.newInstance();

            builderFactory.setNamespaceAware(true);
            DocumentBuilder builder = builderFactory.newDocumentBuilder();
            builder.setErrorHandler(new SamlParserErrorHandler());

            Document tenantDoc =
                    builder.parse(getClass().getResourceAsStream(
                            _impTenantConfigFile));

            //Test importing to a non existing tenant.
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, _impTenantName);

            try
            {
                idmClient.importTenantConfiguration(_impTenantName, tenantDoc);
            } catch (NoSuchTenantException e)
            {
                //expected
            }

            //Test setting a existing tenant.
            newTenant = true;
            Tenant newtenant = new Tenant(_impTenantName);
            idmClient.addTenant(newtenant,DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());

            idmClient.importTenantConfiguration(_impTenantName, tenantDoc);
            try
            {
                Tenant tenant = idmClient.getTenant(_impTenantName);
                Assert.assertNotNull(tenant);
            } catch (NoSuchTenantException ex)
            {
                Assert.fail("tenant should exists, should not reach here");
            }
        } catch (Exception e)
        {
            if (newTenant)
            {
                //clean up partial importing.
                IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, _impTenantName);
            }
            throw new AssertionError(e);
        }
    }

    @TestOrderAnnotation(order = 6)
    @Test
    public void testImportTenantSPConfiguration() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        try
        {
            DocumentBuilderFactory builderFactory =
                    DocumentBuilderFactory.newInstance();

            builderFactory.setNamespaceAware(true);

            DocumentBuilder builder = builderFactory.newDocumentBuilder();

            builder.setErrorHandler(new SamlParserErrorHandler());

            Document tenantNoSLODoc =
                    builder.parse(getClass().getResourceAsStream("/vcd-noSLO.xml"));
            Document tenantDoc =
                    builder.parse(getClass().getResourceAsStream("/vcd.xml"));
            IdmClientTestUtil.ensureTenantExists(idmClient, _impTenantName);
            idmClient.importTenantConfiguration(_impTenantName, tenantNoSLODoc);
            idmClient.importTenantConfiguration(_impTenantName, tenantDoc);
        } catch (Exception e)
        {
            throw new AssertionError(e);
        }
    }

    @TestOrderAnnotation(order = 7)
    @Test
    public void testSetTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        String longName = props.getProperty(CFG_KEY_IDM_TENANT_1_LONG_NAME);

        Assert.assertNotNull(longName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        tenant._issuerName = longName;

        idmClient.setTenant(tenant);

        try
        {
            tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
        } catch (NoSuchTenantException ex)
        {
            Assert.fail("should not reach here");
        }

        Assert.assertEquals(longName, tenant._issuerName);
    }

    @TestOrderAnnotation(order = 8)
    @Test
    public void testAuthenticatedUserAccountLockedExceptionVmdirProvider()
            throws Exception
            {
        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        String userName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_USERNAME);
        String firstName =
                props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_FIRSTNAME);
        String lastName =
                props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_LASTNAME);
        String emailAddr =
                props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_EMAILADDR);
        String description =
                props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_DESCRIPTION);
        String passWord = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_PASSWORD);
        char[] pass = passWord.toCharArray();

        PersonDetail detail =
                new PersonDetail.Builder().firstName(firstName)
                .lastName(lastName).emailAddress(emailAddr)
                .description(description).build();

        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        if (null != client.findPersonUser(tenantName, new PrincipalId(userName,
                domainName)))
        {
            client.deletePrincipal(tenantName, userName);
        }
        client.addPersonUser(tenantName, userName, detail, pass);

        boolean isInitDone = false;
        PrincipalId principalId = null;
        String principal = userName + "@" + domainName;
        while (!isInitDone)
        {
            try
            {
                principalId =
                        client.authenticate(tenantName, principal, passWord);
                isInitDone = (null != principalId);
            } catch (UserAccountLockedException e)
            {// unlock if needed
                client.unlockUserAccount(tenantName, new PrincipalId(userName,
                        domainName));
            } catch (Exception e)
            {
                Assert.fail("Unexpected exception: " + e.getMessage());
            }
        }

        Assert.assertNotNull(principalId);

        String wrongPass = "wrongPass" + pass.toString();


        int maxAttempts =
                client.getLockoutPolicy(tenantName).getMaxFailedAttempts();

        try
        {
            for (int i = 0; i < maxAttempts - 1; i++)
            {
                try
                {
                    client.authenticate(tenantName, principal, wrongPass);
                } catch (UserAccountLockedException e)
                {
                    Assert.fail(String
                            .format("Unexpected exception before max attempts [%d] is reached: [%d] ",
                                    maxAttempts, i, e.getMessage()));
                } catch (IDMLoginException e)
                { // bad password
                    Assert.assertFalse(e.getMessage().contains(
                            "UserAccountLockedException: User account locked"));
                    Assert.assertTrue(e.getMessage().contains("Login failed"));
                    continue;
                }
            }
        } catch (Exception e)
        {
            Assert.fail(String.format(
                    "should not encounter exception within %d attempt",
                    maxAttempts));
        }

        try
        {
            client.authenticate(tenantName, principal, wrongPass);
        } catch (UserAccountLockedException e)
        {// expected exception
            Assert.assertTrue(e.getMessage().contains("User account locked"));
            Assert.assertFalse(e.getMessage().contains("Login failed"));

            client.unlockUserAccount(tenantName, new PrincipalId(userName,
                    domainName));
            client.deletePrincipal(tenantName, userName);
            return;
        } catch (Exception e)
        {
            Assert.fail(String
                    .format("should have gotten an UserAccountLockedException after max attempts = %d",
                            maxAttempts));
        }
        Assert.fail(String
                .format("should have gotten an UserAccountLockedException after max attempts = %d",
                        maxAttempts));
            }

    @TestOrderAnnotation(order = 9)
    @Test
    @Ignore("Bugzilla#1324293")
    public void testAuthenticateUserPasswordExpirationVmdirProvider()
            throws Exception
            {
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        String userName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_USERNAME);
        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        PasswordPolicy origPolicy = client.getPasswordPolicy(tenantName);
        try
        {


            PasswordPolicy testPolicy =
                    new PasswordPolicy(
                            "password policy with zero password life time",
                            origPolicy.getProhibitedPreviousPasswordsCount(),
                            origPolicy.getMinimumLength(),
                            origPolicy.getMaximumLength(),
                            origPolicy.getMinimumAlphabetCount(),
                            origPolicy.getMinimumUppercaseCount(),
                            origPolicy.getMinimumLowercaseCount(),
                            origPolicy.getMinimumNumericCount(),
                            origPolicy.getMinimumSpecialCharacterCount(),
                            origPolicy
                            .getMaximumAdjacentIdenticalCharacterCount(),
                            0 /* passwordLifetimeDays set to 0 */);

            String firstName =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_FIRSTNAME);
            String lastName =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_LASTNAME);
            String emailAddr =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_EMAILADDR);
            String description =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DETAILS_DESCRIPTION);
            String password = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_PASSWORD);
            PersonDetail detail =
                    new PersonDetail.Builder().firstName(firstName)
                    .lastName(lastName).emailAddress(emailAddr)
                    .description(description).build();

            String principal = userName + "@" + domainName;

            client.addPersonUser(tenantName, userName, detail,
                    password.toCharArray());
            client.setPasswordPolicy(tenantName, testPolicy);

            // 1 second sleep is needed here in order to be sure that the
            // expiration of the account will work. If this call goes in the
            // same second when setting password policy happens then the
            // authentication will be successful because password expiration
            // should happen in the following second.
            final int oneSecond = 1000;
            Thread.sleep(oneSecond);

            client.authenticate(tenantName, principal, password);
            Assert.fail("should not reach due to passwordExpiredException");
        } catch (PasswordExpiredException e)
        {
            Assert.assertTrue(e.getMessage().contains("User account expired"));
        } catch (Exception e)
        {
            Assert.fail(String.format("Unexpected exception: %s",
                    e.getMessage()));
        } finally
        {
            client.setPasswordPolicy(tenantName, origPolicy);
            client.deletePrincipal(tenantName, userName);
        }
            }

    @TestOrderAnnotation(order = 10)
    @Test
    public void testAuthenticateNonExistentUserVmdirProvider() throws Exception
    {
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        String userName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_USERNAME);
        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        String principal = userName + "_NonExistent@" + domainName;
        TestNonExistentUserAuthenticate(client, tenantName, principal, "my pwd");
    }

    @TestOrderAnnotation(order = 11)
    @Test
    public void testAddGroupToSelf() throws Exception
    {
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        PrincipalId Group = new PrincipalId(ADMIN_GROUP_NAME, domainName);
        try
        {
            client.addGroupToGroup(tenantName, Group, Group.getName());
            Assert.fail("Adding group to self should fail.");
        } catch (InvalidArgumentException ex)
        {
            // expected
        }
    }

    @TestOrderAnnotation(order = 12)
    @Test
    public void testAddGroup() throws Exception
    {
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        final String groupName = "group_test";
        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        PrincipalId Group = new PrincipalId(groupName, domainName);

        // add group with null description
        client.addGroup(tenantName, groupName, new GroupDetail());
        Group group_test = client.findGroup(tenantName, Group);
        Assert.assertNotNull(group_test);

        client.deletePrincipal(tenantName, groupName);
        try
        {
            group_test = client.findGroup(tenantName, Group);
        } catch (InvalidPrincipalException ex)
        {
            // add group with empty description
            client.addGroup(tenantName, groupName, new GroupDetail(""));
            group_test = client.findGroup(tenantName, Group);
            Assert.assertNotNull(group_test);

            client.deletePrincipal(tenantName, groupName);
            try
            {
                group_test = client.findGroup(tenantName, Group);
            } catch (InvalidPrincipalException ex1)
            {
                // ignore
                return;
            }
        }

        Assert.fail(String.format("%s group does not exist, "
                + "should expect InvalidPrincipalException", groupName));
    }

    @TestOrderAnnotation(order = 13)
    @Test
    public void testSetTenantCredentials() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        List<Certificate> certList = new ArrayList<Certificate>();

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        String alias = props.getProperty(CFG_KEY_STS_KEY_ALIAS);

        Assert.assertNotNull(alias);

        certList.add(ks.getCertificate(alias));

        String alias1 = props.getProperty(CFG_KEY_STS_KEY_ALIAS1);

        Assert.assertNotNull(alias1);

        Certificate trustedRootCert = ks.getCertificate(alias1);
        certList.add(trustedRootCert);

        String password = props.getProperty(CFG_KEY_STS_KEYSTORE_PASSWORD);

        PrivateKey key = (PrivateKey) ks.getKey(alias, password.toCharArray());

        idmClient.setTenantCredentials(tenantName, certList, key);

        List<Certificate> certList2 =
                idmClient.getTenantCertificate(tenantName);

        Assert.assertNotNull(certList2);
        Assert.assertEquals(2, certList2.size());

        PrivateKey key2 = idmClient.getTenantPrivateKey(tenantName);

        Assert.assertNotNull(key2);

        // Attempt to delete trusted Root certificate that is the active signerIdentity
        try
        {
            idmClient.deleteCertificate(tenantName, CertificateUtil
                    .generateFingerprint((X509Certificate) trustedRootCert),
                    CertificateType.STS_TRUST_CERT);
        } catch (CertificateInUseException e)
        {
            //Expect to reach here
            try
            {
                idmClient
                .deleteCertificate(
                        tenantName,
                        CertificateUtil
                        .generateFingerprint((X509Certificate) trustedRootCert),
                        CertificateType.LDAP_TRUSTED_CERT);
            } catch (NoSuchCertificateException e1)
            {
                //Expect to reach here
               return;
            }

            Assert.fail("Should not reach here, "
                    + "attempting to remove an in-existing trusted Root Certificate should fail.");
        }

        Assert.fail("Should not reach here, "
                + "attempting to remove a trusted Root Certificate that is the root of active signerIdentity should be denied.");
    }

    @TestOrderAnnotation(order = 14)
    @Test
    public void testSetTenantTrustedCertChain() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        // Set trustedCertChain for non-system tenant
        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        List<Certificate> certList = new ArrayList<Certificate>();

        KeyStore ks =
                loadKeyStore(CFG_KEY_TRUSTED_STS_KEYSTORE,
                        CFG_KEY_TRUSTED_STS_KEYSTORE_PASSWORD);

        String alias = props.getProperty(CFG_KEY_TRUSTED_STS_KEY_ALIAS);
        Assert.assertNotNull(alias);
        certList.add(ks.getCertificate(alias));

        String alias1 = props.getProperty(CFG_KEY_TRUSTED_STS_KEY_ALIAS1);
        Assert.assertNotNull(alias1);
        certList.add(ks.getCertificate(alias1));

        Collection<List<Certificate>> certList_prev =
                idmClient.getTenantCertificates(tenantName);
        idmClient.setTenantTrustedCertificateChain(tenantName, certList);
        Collection<List<Certificate>> certList_after =
                idmClient.getTenantCertificates(tenantName);

        Assert.assertTrue(certList_after.contains(certList));
        Assert.assertTrue(certList_prev.size() + 1 == certList_after.size());

        // Set trustedCertChain for system tenant
        List<Certificate> certList1 = new ArrayList<Certificate>();

        ks = loadKeyStore(CFG_KEY_STS_KEYSTORE, CFG_KEY_STS_KEYSTORE_PASSWORD);

        alias = props.getProperty(CFG_KEY_STS_KEY_ALIAS);
        Assert.assertNotNull(alias);
        Certificate trusted_leaf_cert = ks.getCertificate(alias);
        certList1.add(trusted_leaf_cert);

        alias1 = props.getProperty(CFG_KEY_STS_KEY_ALIAS1);
        Assert.assertNotNull(alias1);
        Certificate trusted_root_cert = ks.getCertificate(alias1);
        certList1.add(trusted_root_cert);

        String system_tenantName =
                props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        Assert.assertNotNull(system_tenantName);
        String obtainedSysTenant = idmClient.getSystemTenant();
        Assert.assertTrue(system_tenantName.equalsIgnoreCase(obtainedSysTenant));

        Collection<List<Certificate>> certList_prev_sp =
                idmClient.getTenantCertificates(system_tenantName);
        idmClient
        .setTenantTrustedCertificateChain(system_tenantName, certList1);
        Collection<List<Certificate>> certList_after_sp =
                idmClient.getTenantCertificates(system_tenantName);

        Assert.assertTrue(certList_after_sp.contains(certList1));
        Assert.assertTrue(certList_prev_sp.size() + 1 == certList_after_sp
                .size());

        Collection<Certificate> allLdapCerts = idmClient.getAllCertificates(system_tenantName, CertificateType.LDAP_TRUSTED_CERT);
        Assert.assertTrue(allLdapCerts == null || allLdapCerts.size() == 0);
        Collection<Certificate> allStsCerts = idmClient.getAllCertificates(system_tenantName, CertificateType.STS_TRUST_CERT);

        Collection<Certificate> rootCerts = idmClient.getTrustedCertificates(system_tenantName);
        Assert.assertTrue(rootCerts.contains(trusted_root_cert));

        Collection<Certificate> leafCerts = idmClient.getStsIssuersCertificates(system_tenantName);
        Assert.assertTrue(leafCerts.contains(trusted_leaf_cert));

        // clean up added TrustedCertificateChain
        idmClient.deleteCertificate(system_tenantName, CertificateUtil
                .generateFingerprint((X509Certificate) trusted_root_cert),
                CertificateType.STS_TRUST_CERT);

        // Make sure 'trusted_root_cert' is removed successfully
        allStsCerts =
                idmClient.getAllCertificates(system_tenantName,
                        CertificateType.STS_TRUST_CERT);
        Assert.assertFalse(allStsCerts.contains(trusted_root_cert));

        rootCerts = idmClient.getTrustedCertificates(system_tenantName);
        Assert.assertTrue(rootCerts.size() == 1);

        leafCerts = idmClient.getStsIssuersCertificates(system_tenantName);
        Assert.assertTrue(leafCerts.size() == 1);

        // Make sure 'trustedCertChain is removed (triggered by trustedRoot Cert removal)
        Collection<List<Certificate>> certChains =
                idmClient.getTenantCertificates(system_tenantName);
        // certChain used in 'TenantTest' has two elements (sorted by leaf followed by root)
        for (List<Certificate> certChain : certChains)
        {
            boolean bcertChainExist =
                    certChain.get(0).equals(trusted_leaf_cert)
                    && certChain.get(1).equals(trusted_root_cert);
            Assert.assertFalse(bcertChainExist);
        }

        return;
    }

    @TestOrderAnnotation(order = 15)
    @Test
    public void testAddIdentityStoreNullAlias() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        // Add Active Directory IDP with null alias
        final String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }

        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, false);
        // check before create
        if (store == null)
        {
            idmClient.addProvider(tenantName, adStore);
        }

        Set<Group> groups = idmClient.findGroupsInGroup(tenantName, new PrincipalId("Group1", "ssolabs.eng.vmware.com"), "", -1);
        Assert.assertEquals(1, groups.size());


        Set<PersonUser> users = idmClient.findPersonUsersInGroup(tenantName, new PrincipalId("grpFgPwdPolicy_A", "ssolabs.eng.vmware.com"), "", -1);
        Assert.assertEquals(2, users.size());

        users = idmClient.findPersonUsersInGroup(tenantName, new PrincipalId("grpFgPwdPolicy_A", "ssolabs.eng.vmware.com"), "", 1);
        Assert.assertEquals(1, users.size());

        testAddRemoveADFsptestUserInternal(); // exercise AD provider API when alias is null for AD IDP
        testAddRemoveADFsptestGroupInternal(); // exercise AD provider API when alias is null for AD IDP

        // clean up AD provider
        store = idmClient.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }

        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);

        store = idmClient.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, olProviderName);
            store = idmClient.getProvider(tenantName, olProviderName);
            Assert.assertNull(store);
        }

        ensureOpenLdapIdentityStoreExistForTenant(tenantName, true);

        testAddRemoveOLFspTestUserInternal(); // exercise OL provider API when alias is null for OL IDP
        testAddRemoveOLFstTestGroupInternal(); // exercise OL provider API when alias is null for OL IDP

        // clean up OL provider
        store = idmClient.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, olProviderName);
            store = idmClient.getProvider(tenantName, olProviderName);
            Assert.assertNull(store);
        }

        // clean up Tenant
        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
    }

    @TestOrderAnnotation(order = 16)
    @Test
    public void testAddRelyingParty() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        String rpName = "coke";
        String assertionServiceName = "Assertion0";
        String attributeServiceName = "Attribute0";
        String sampleUrl = "http://localhost:8080";
        List<SignatureAlgorithm> algorithmList =
                new ArrayList<SignatureAlgorithm>();
        List<AssertionConsumerService> assertSvcList =
                new ArrayList<AssertionConsumerService>();
        List<AttributeConsumerService> attrSvcList =
                new ArrayList<AttributeConsumerService>();
        List<Attribute> attrList = new ArrayList<Attribute>();

        RelyingParty rp = new RelyingParty(rpName);
        rp.setUrl(sampleUrl);

        SignatureAlgorithm algorithm = new SignatureAlgorithm();

        algorithm.setMaximumKeySize(256);
        algorithm.setMinimumKeySize(10);

        algorithmList.add(algorithm);

        rp.setSignatureAlgorithms(algorithmList);

        AssertionConsumerService assertSvc =
                new AssertionConsumerService(assertionServiceName);

        String binding = "urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST";

        assertSvc.setBinding(binding);
        assertSvc.setEndpoint(sampleUrl);

        assertSvcList.add(assertSvc);

        rp.setDefaultAssertionConsumerService(assertionServiceName);
        rp.setAssertionConsumerServices(assertSvcList);

        Attribute attr = new Attribute("Group");
        attr.setFriendlyName("Group");
        attr.setNameFormat("urn:oasis:names:tc:SAML:20.blah");

        attrList.add(attr);

        attr = new Attribute("FirstName");
        attr.setFriendlyName("First Name");
        attr.setNameFormat("urn:oasis:names:tc:SAML:20.blah");

        attrList.add(attr);
        attr = new Attribute("LastName");
        attr.setFriendlyName("Last Name");
        attr.setNameFormat("urn:oasis:names:tc:SAML:20.blah");

        attrList.add(attr);

        AttributeConsumerService attrSvc =
                new AttributeConsumerService(attributeServiceName);

        attrSvc.setAttributes(attrList);

        attrSvcList.add(attrSvc);

        rp.setDefaultAttributeConsumerService(attributeServiceName);

        rp.setAttributeConsumerServices(attrSvcList);

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        rp.setCertificate(ks.getCertificate(props
                .getProperty(CFG_KEY_STS_KEY_ALIAS)));

        idmClient.addRelyingParty(tenantName, rp);

        RelyingParty rp2 = idmClient.getRelyingParty(tenantName, rpName);
        Assert.assertNotNull(rp2 != null);

        RelyingParty rp3 =
                idmClient.getRelyingPartyByUrl(tenantName, sampleUrl);
        Assert.assertNotNull(rp3 != null);

        // Check parameters
    }

    @TestOrderAnnotation(order = 17)
    @Test
    public void testAddADIdentityStoreAgain() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String adProviderName =

                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        // Prepare adStore with alias
        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);

        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);

        if (store != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);
        }

        try
        {
            idmClient.addProvider(tenantName, adStore);
            idmClient.addProvider(tenantName, adStore);
        } catch (DuplicateProviderException ex)
        {
            idmClient.deleteProvider(tenantName, adProviderName);

            adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, false);

            store = idmClient.getProvider(tenantName, adProviderName);
            Assert.assertNull(store);

            try
            {
                idmClient.addProvider(tenantName, adStore);
                idmClient.addProvider(tenantName, adStore);
            } catch (DuplicateProviderException subEx)
            {
                idmClient.deleteProvider(tenantName, adProviderName);
            }
        }
    }

    @TestOrderAnnotation(order = 18)
    @Test
    public void testAddProviderWithInvalidAuth() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        // Add invalid ldap IDP, probe will fail
        final String ldapProviderName = "bar.com";
        final String ldapProviderAlias = "foo.com";
        final String ldapHost = "INVALIDLDAP://bar.com";
        final String ldapUserName = "CN=Administrator,DC=BAR,DC=COM";
        final String ldapPwd = "password";
        final ArrayList<String> ldapHosts = new ArrayList<String>();
        ldapHosts.add(ldapHost);

        IdentityStoreData ldapStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        ldapProviderName, ldapProviderAlias,
                        IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                        AuthenticationType.PASSWORD, null, 0, ldapUserName,
                        ldapPwd, "dc=bar,dc=com", "dc=bar,dc=com", ldapHosts, null);
        try
        {
            idmClient.addProvider(tenantName, ldapStore);
            Assert.fail("failed probeConnectivity() provider successfully finished addProvider()!");
        } catch (com.vmware.identity.idm.InvalidArgumentException ex)
        {
            Assert.assertNull(idmClient.getProvider(tenantName,
                    ldapProviderName));
        }
    }

    @TestOrderAnnotation(order = 19)
    @Test
    public void testAddIdentityStore() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        // Add Active Directory IDP with LDAPS connection
        final String adLdapsProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME_LDAPS);
        Assert.assertNotNull(adLdapsProviderName);

        final String adLdapsAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS_LDAPS);
        Assert.assertNotNull(adLdapsAlias);

        final String ldapskdc = props.getProperty(CFG_KEY_IDM_TENANT_1_KDC_LDAPS);
        Assert.assertNotNull(ldapskdc);

        final ArrayList<String> ldapskdcList = new ArrayList<String>();
        ldapskdcList.add(ldapskdc);

        final String adLdapsUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN_LDAPS);
        Assert.assertNotNull(adLdapsUserName);

        final String adLdapsPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD_LDAPS);

        Assert.assertNotNull(adLdapsPwd);

        final Map<String, String> attrMap = new HashMap<String, String>();

        attrMap.put(SAMLATTR_GIVEN_NAME, "givenName");
        attrMap.put(SAMLATTR_SUR_NAME, "sn");
        attrMap.put(SAMLATTR_GROUP_IDENTITY, "memberof");
        attrMap.put(SAMLATTR_EMAIL_ADDRESS, "mail");
        attrMap.put(SAMLATTR_UPN, "userPrincipalName");

        List<X509Certificate> certs = null;
        X509Certificate certificate =  getCertificate(props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_CERT_ALIAS_LDAPS));
        if (certificate != null)
           certs = java.util.Collections.singletonList(certificate);
        final String userSearchBaseDn =
              props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_USER_BASE_DN_LDAPS);

        final String groupSearchBaseDn =
              props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_GROUP_BASE_DN_LDAPS);
        IdentityStoreData adLdapsStore =
              IdentityStoreData.CreateExternalIdentityStoreData(
                    adLdapsProviderName, adLdapsAlias,
                    IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING,
                    AuthenticationType.PASSWORD, null, 0, adLdapsUserName, false, null,
                    adLdapsPwd, userSearchBaseDn, groupSearchBaseDn, ldapskdcList,
                    attrMap, null, null, 0, certs, null,
                    CFG_IDENTITY_STORE_HINT_ATTR_NAME, CFG_IDENTITY_STORE_LINK_USE_UPN);

        IIdentityStoreData ldapsStore =
                idmClient.getProvider(tenantName, adLdapsProviderName);

        // check before create
        if (ldapsStore == null)
        {
            idmClient.addProvider(tenantName, adLdapsStore);
        }

        ldapsStore = idmClient.getProvider(tenantName, adLdapsProviderName);
        Assert.assertNotNull(ldapsStore);
        Assert.assertEquals(adLdapsProviderName, ldapsStore.getName());
        Assert.assertEquals(DomainType.EXTERNAL_DOMAIN, ldapsStore.getDomainType());
        Assert.assertNotNull(ldapsStore.getExtendedIdentityStoreData());
        Assert.assertEquals(
                IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING, ldapsStore
                .getExtendedIdentityStoreData().getProviderType());
        Assert.assertEquals(adLdapsUserName.toLowerCase(), ldapsStore.getExtendedIdentityStoreData()
                .getUserName().toLowerCase());
        Assert.assertEquals(adLdapsPwd, ldapsStore.getExtendedIdentityStoreData()
                .getPassword());
        Assert.assertNotNull(ldapsStore.getExtendedIdentityStoreData()
                .getConnectionStrings());
        Assert.assertEquals(ldapsStore.getExtendedIdentityStoreData()
                .getCertUserHintAttributeName(), CFG_IDENTITY_STORE_HINT_ATTR_NAME);
        Assert.assertTrue(ldapsStore.getExtendedIdentityStoreData()
                .getCertLinkingUseUPN() == CFG_IDENTITY_STORE_LINK_USE_UPN);
        Assert.assertEquals(ldapskdcList.size(), ldapsStore
                .getExtendedIdentityStoreData().getConnectionStrings().size());

        int i = 0;
        for (String str : ldapsStore.getExtendedIdentityStoreData()
                .getConnectionStrings())
        {
            Assert.assertEquals(ldapskdcList.get(i), str);
            i++;
        }
        i = 0;
        if (certs != null)
        {
            Assert.assertNotNull(ldapsStore.getExtendedIdentityStoreData().getCertificates());
            for (X509Certificate cert : ldapsStore.getExtendedIdentityStoreData()
                    .getCertificates())
            {
                Assert.assertEquals(certs.get(i), cert);
                i++;
            }
        }

        //clean up ldaps store
        idmClient.deleteProvider(tenantName, adLdapsProviderName);

        // Add Active Directory IDP
        final String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        final String alias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        Assert.assertNotNull(alias);

        final String kdc = props.getProperty(CFG_KEY_IDM_TENANT_1_KDC);
        Assert.assertNotNull(kdc);

        final ArrayList<String> kdcList = new ArrayList<String>();
        kdcList.add(kdc);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_BIND_DN);
        Assert.assertNotNull(adUserName);

        final String adPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_PASSWORD);

        Assert.assertNotNull(adPwd);

        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);

        // check before create
        if (store == null)
        {
            idmClient.addProvider(tenantName, adStore);
        }

        store = idmClient.getProvider(tenantName, adProviderName);
        Assert.assertNotNull(store);
        Assert.assertEquals(adProviderName, store.getName());
        Assert.assertEquals(DomainType.EXTERNAL_DOMAIN, store.getDomainType());
        Assert.assertNotNull(store.getExtendedIdentityStoreData());
        Assert.assertEquals(
                IdentityStoreType.IDENTITY_STORE_TYPE_LDAP_WITH_AD_MAPPING, store
                .getExtendedIdentityStoreData().getProviderType());
        Assert.assertEquals(adUserName.toLowerCase(), store.getExtendedIdentityStoreData()
                .getUserName().toLowerCase());
        Assert.assertEquals(adPwd, store.getExtendedIdentityStoreData()
                .getPassword());
        Assert.assertNotNull(store.getExtendedIdentityStoreData()
                .getConnectionStrings());
        Assert.assertEquals(kdcList.size(), store
                .getExtendedIdentityStoreData().getConnectionStrings().size());
        Assert.assertTrue(ldapsStore.getExtendedIdentityStoreData()
                .getCertLinkingUseUPN());
        Assert.assertEquals(ldapskdcList.size(), ldapsStore
                .getExtendedIdentityStoreData().getConnectionStrings().size());

        //check the default cert mapping options
        Assert.assertNull(ldapsStore.getExtendedIdentityStoreData()
                .getCertUserHintAttributeName());
        Assert.assertTrue(ldapsStore.getExtendedIdentityStoreData()
                .getCertLinkingUseUPN());
        i = 0;
        for (String str : store.getExtendedIdentityStoreData()
                .getConnectionStrings())
        {
            Assert.assertEquals(kdcList.get(i), str);
            i++;
        }

        // Cleanup, Add OpenLdap IDP with ldaps and verify
        final String oldapsProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(oldapsProviderName);

        if (null != idmClient.getProvider(tenantName, oldapsProviderName))
        {
            idmClient.deleteProvider(tenantName, oldapsProviderName);
        }

        final String oldapsProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
        Assert.assertNotNull(oldapsProviderAlias);

        final String oldapsHost =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_SSL_HOST_NAME);
        Assert.assertNotNull(oldapsHost);

        final String oldapsUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN);
        Assert.assertNotNull(oldapsUserName);
        final String oldapsPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD);
        Assert.assertNotNull(oldapsPwd);

        final ArrayList<String> oldapsHosts = new ArrayList<String>();
        oldapsHosts.add(oldapsHost);

        List<X509Certificate> openldapsCerts = null;
        X509Certificate openLdapCertificate =  getCertificate(props.getProperty(CFG_KEY_IDM_TENANT_1_OL_SSL_CERT_ALIAS));
        if (openLdapCertificate != null)
            openldapsCerts = java.util.Collections.singletonList(openLdapCertificate);
        ensureOpenLdapIdentityStoreExistForTenant(
                props.getProperty(CFG_KEY_IDM_TENANT_1_NAME), true, CFG_KEY_IDM_TENANT_1_OL_SSL_HOST_NAME, openldapsCerts);

        store = idmClient.getProvider(tenantName, oldapsProviderName);

        Assert.assertNotNull(store);
        Assert.assertEquals(oldapsProviderName, store.getName());
        Assert.assertEquals(DomainType.EXTERNAL_DOMAIN, store.getDomainType());
        Assert.assertNotNull(store.getExtendedIdentityStoreData());
        Assert.assertEquals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP, store
                .getExtendedIdentityStoreData().getProviderType());
        Assert.assertEquals(oldapsUserName, store.getExtendedIdentityStoreData()
                .getUserName());
        Assert.assertEquals(oldapsPwd, store.getExtendedIdentityStoreData()
                .getPassword());
        Assert.assertEquals(oldapsProviderAlias, store
                .getExtendedIdentityStoreData().getAlias());
        i = 0;
        for (String str : store.getExtendedIdentityStoreData()
                .getConnectionStrings())
        {
            Assert.assertEquals(oldapsHosts.get(i), str);
            i++;
        }
        i = 0;
        if (openldapsCerts != null)
        {
            Assert.assertNotNull(store.getExtendedIdentityStoreData().getCertificates());
            for (X509Certificate cert : store.getExtendedIdentityStoreData()
                    .getCertificates())
            {
                Assert.assertEquals(openldapsCerts.get(i), cert);
                i++;
            }
        }

        store = idmClient.getProvider(tenantName, oldapsProviderAlias);

        Assert.assertNotNull(store);
        Assert.assertEquals(oldapsProviderName, store.getName());
        Assert.assertEquals(oldapsProviderAlias, store
                .getExtendedIdentityStoreData().getAlias());

        // Cleanup, Add OpenLdap IDP and verify
        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);

        if (null != idmClient.getProvider(tenantName, olProviderName))
        {
            idmClient.deleteProvider(tenantName, olProviderName);
        }

        final String olProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
        Assert.assertNotNull(olProviderAlias);

        final String olHost =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_HOST_NAME);
        Assert.assertNotNull(olHost);

        final String olUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN);
        Assert.assertNotNull(olUserName);
        final String olPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD);
        Assert.assertNotNull(olPwd);

        final ArrayList<String> olHosts = new ArrayList<String>();
        olHosts.add(olHost);

        ensureOpenLdapIdentityStoreExistForTenant(
                props.getProperty(CFG_KEY_IDM_TENANT_1_NAME), true);

        store = idmClient.getProvider(tenantName, olProviderName);

        Assert.assertNotNull(store);
        Assert.assertEquals(olProviderName, store.getName());
        Assert.assertEquals(DomainType.EXTERNAL_DOMAIN, store.getDomainType());
        Assert.assertNotNull(store.getExtendedIdentityStoreData());
        Assert.assertEquals(IdentityStoreType.IDENTITY_STORE_TYPE_LDAP, store
                .getExtendedIdentityStoreData().getProviderType());
        Assert.assertEquals(olUserName, store.getExtendedIdentityStoreData()
                .getUserName());
        Assert.assertEquals(olPwd, store.getExtendedIdentityStoreData()
                .getPassword());
        Assert.assertEquals(olProviderAlias, store
                .getExtendedIdentityStoreData().getAlias());
        i = 0;
        for (String str : store.getExtendedIdentityStoreData()
                .getConnectionStrings())
        {
            Assert.assertEquals(olHosts.get(i), str);
            i++;
        }

        store = idmClient.getProvider(tenantName, olProviderAlias);

        Assert.assertNotNull(store);
        Assert.assertEquals(olProviderName, store.getName());
        Assert.assertEquals(olProviderAlias, store
                .getExtendedIdentityStoreData().getAlias());

        // Add ldap IDP (inactive IDP, probe will fail and be removed)
        final String ldapProviderName = "bar.com";
        final String ldapProviderAlias = "foo.com";
        final String ldapHost = "LDAP://bar.com";
        final String ldapUserName = "CN=Administrator,DC=BAR,DC=COM";
        final String ldapPwd = "password";
        final ArrayList<String> ldapHosts = new ArrayList<String>();
        ldapHosts.add(ldapHost);

        IdentityStoreData ldapStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        ldapProviderName, ldapProviderAlias,
                        IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                        AuthenticationType.PASSWORD, null, 0, ldapUserName,
                        ldapPwd, "dc=bar,dc=com", "dc=bar,dc=com", ldapHosts, null);

        try
        {
            idmClient.addProvider(tenantName, ldapStore);
        } catch (Exception ex)
        {
            // ignore (addProvder validates IDP before adding it now)
            Assert.assertNull(idmClient.getProvider(tenantName,
                    ldapProviderName));
        }

        // Add VM directory IDP (inactive IDP, probe will fail and be removed)
        final String vmStorename = "baz.com";
        final String vmHostname = "LDAP://bar.com";
        final String vmUsername = "CN=Administrator,DC=BAR,DC=COM";
        final String vmPassword = "password";
        final ArrayList<String> vmHosts = new ArrayList<String>();
        vmHosts.add(vmHostname);

        IdentityStoreData vmwstore =
                IdentityStoreData.CreateExternalIdentityStoreData(vmStorename,
                        null,
                        IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                        AuthenticationType.PASSWORD, null, 0, vmUsername,
                        vmPassword, "dc=bar,dc=com", "dc=bar,dc=com", vmHosts, null);

        try
        {
            idmClient.addProvider(tenantName, vmwstore);
        } catch (Exception ex)
        {
            // ignore (addProvder validates IDP before adding it now)
            Assert.assertNull(idmClient.getProvider(tenantName, vmStorename));
        }

        //test illegal userName detection

        final String badVMUsername = "Administrator@BAR.COM";
        try
        {
            IdentityStoreData badvmwstore =
                    IdentityStoreData
                    .CreateExternalIdentityStoreData(
                            vmStorename,
                            null,
                            IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                            AuthenticationType.PASSWORD, null, 0,
                            badVMUsername, vmPassword, "dc=bar,dc=com", "dc=bar,dc=com",
                            vmHosts, null);

            idmClient.addProvider(tenantName, badvmwstore);
            Assert.fail("should not reach here");
        } catch (Exception e)
        {
        }

        //test adding local OS provider
        final String localOsDomainName = "localOsDomain";
        IIdentityStoreData localOsProvider =
                IdentityStoreData
                .CreateLocalOSIdentityStoreData(localOsDomainName);
        String defaultTenant = idmClient.getDefaultTenant();
        if ((defaultTenant == null) || (defaultTenant.isEmpty() == true)
                || (tenantName.equalsIgnoreCase(defaultTenant) == false))
        {
            try
            {
                idmClient.addProvider(tenantName, localOsProvider);
                Assert.fail("Expected to not be able to add local OS provider to non-default tenant.");
            } catch (Exception ex)
            {
            }
        }

        idmClient.setDefaultTenant(tenantName);
        idmClient.addProvider(tenantName, localOsProvider);
        store = idmClient.getProvider(tenantName, localOsDomainName);
        Assert.assertNotNull(store);
        Assert.assertEquals(localOsDomainName, store.getName());
        Assert.assertEquals(DomainType.LOCAL_OS_DOMAIN, store.getDomainType());
        Assert.assertNull(store.getExtendedIdentityStoreData());

        // authenticate non-existent user
        TestNonExistentUserAuthenticate(idmClient, tenantName,
                "non-existentuser@" + localOsDomainName, "my pwd");

        Collection<IIdentityStoreData> stores =
                idmClient.getProviders(tenantName,
                        EnumSet.of(DomainType.EXTERNAL_DOMAIN));

        Assert.assertNotNull(stores);
        Assert.assertEquals(2, stores.size());
        for (IIdentityStoreData data : stores)
        {
            Assert.assertNotNull(data);
            Assert.assertTrue(data.getDomainType() == DomainType.EXTERNAL_DOMAIN);
            Assert.assertNotNull(data.getExtendedIdentityStoreData());
        }

        stores =
                idmClient.getProviders(tenantName,
                        EnumSet.of(DomainType.SYSTEM_DOMAIN));
        Assert.assertNotNull(stores);
        Assert.assertEquals(1, stores.size());
        for (IIdentityStoreData data : stores)
        {
            Assert.assertNotNull(data);
            Assert.assertTrue(data.getDomainType() == DomainType.SYSTEM_DOMAIN);
            Assert.assertNotNull(data.getExtendedIdentityStoreData());
            Assert.assertNull(data.getExtendedIdentityStoreData().getAlias());
            Assert.assertNull(data.getExtendedIdentityStoreData().getFriendlyName());
            Assert.assertNull(data.getExtendedIdentityStoreData().getGroupBaseDn());
            Assert.assertNull(data.getExtendedIdentityStoreData().getPassword());
            Assert.assertNull(data.getExtendedIdentityStoreData().getServicePrincipalName());
            Assert.assertNull(data.getExtendedIdentityStoreData().getUserBaseDn());
            Assert.assertNull(data.getExtendedIdentityStoreData().getUserName());
            Assert.assertNull(data.getExtendedIdentityStoreData().getAttributeMap());
            Assert.assertTrue(data.getExtendedIdentityStoreData().getConnectionStrings().isEmpty());
            Assert.assertNull(data.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping());
            Assert.assertEquals(-1, data.getExtendedIdentityStoreData().getSearchTimeoutSeconds());
            Assert.assertNull(data.getExtendedIdentityStoreData().getProviderType());
        }

        stores = idmClient.getProviders(tenantName);

        Assert.assertNotNull(stores);
        Assert.assertEquals(4, stores.size());

        idmClient.deleteProvider(tenantName, localOsDomainName);
        // re-set default tenant to original value
        if ((defaultTenant != null) && (defaultTenant.isEmpty() == false))
        {
            idmClient.setDefaultTenant(defaultTenant);
        }
    }

    @TestOrderAnnotation(order = 20)
    @Test
    public void testAddSchemaMappedIdentityProvider() throws Exception
    {
        CasIdmClient idmClient = getIdmClient();

        String tenantName = UUID.randomUUID().toString();

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        try
        {
            Assert.assertNotNull(tenant);

            // create an external provider with schema mappings
            // Add Active Directory IDP
            IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
            final String schemaMappedProviderName = adStore.getName();

            IdentityStoreData schemaMappedData =
                    IdentityStoreData.CreateExternalIdentityStoreData(
                            schemaMappedProviderName, null,
                            adStore.getExtendedIdentityStoreData().getProviderType(),
                            adStore.getExtendedIdentityStoreData().getAuthenticationType(),
                            null,
                            adStore.getExtendedIdentityStoreData().getSearchTimeoutSeconds(),
                            adStore.getExtendedIdentityStoreData().getUserName(),
                            adStore.getExtendedIdentityStoreData().getPassword(),
                            adStore.getExtendedIdentityStoreData().getUserBaseDn(),
                            adStore.getExtendedIdentityStoreData().getGroupBaseDn(),
                            adStore.getExtendedIdentityStoreData().getConnectionStrings(),
                            adStore.getExtendedIdentityStoreData().getAttributeMap(), getSchemaMapping());
            IIdentityStoreData schemaMapped = idmClient.getProvider(tenantName, schemaMappedProviderName);

            // check before create
            if (schemaMapped != null) { idmClient.deleteProvider(tenantName, schemaMappedProviderName); }

            idmClient.addProvider(tenantName, schemaMappedData);

            schemaMapped = idmClient.getProvider(tenantName, schemaMappedProviderName);

            Assert.assertNotNull("Schema mapped provider should be returned.", schemaMapped);
            Assert.assertNotNull("Schema mapped provider should be returned.", schemaMapped.getExtendedIdentityStoreData());

            validateSchemaMapping(schemaMappedData.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping(), schemaMapped.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping());

            idmClient.deleteProvider( tenantName, schemaMappedProviderName );
        }
        finally
        {
            idmClient.deleteTenant( tenantName );
        }
    }

    private static IdentityStoreSchemaMapping getSchemaMapping()
    {
        IdentityStoreSchemaMapping.Builder schemaMappingBuilder = new IdentityStoreSchemaMapping.Builder();
        IdentityStoreObjectMapping.Builder userObjectMappingBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdUser);
        userObjectMappingBuilder.setObjectClass("myUser");
        userObjectMappingBuilder.addAttributeMapping(new IdentityStoreAttributeMapping( IdentityStoreAttributeMapping.AttributeIds.UserAttributeAccountName, "mySamAccount"));
        userObjectMappingBuilder.addAttributeMapping(new IdentityStoreAttributeMapping( IdentityStoreAttributeMapping.AttributeIds.UserAttributeDescription, "myDescription"));
        userObjectMappingBuilder.addAttributeMapping(new IdentityStoreAttributeMapping( IdentityStoreAttributeMapping.AttributeIds.UserAttributeDisplayName, "myDisplayName"));
        IdentityStoreObjectMapping.Builder groupObjectMappingBuilder = new IdentityStoreObjectMapping.Builder(IdentityStoreObjectMapping.ObjectIds.ObjectIdGroup);
        groupObjectMappingBuilder.addAttributeMapping(new IdentityStoreAttributeMapping( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeAccountName, "mySamAccount"));
        groupObjectMappingBuilder.addAttributeMapping(new IdentityStoreAttributeMapping( IdentityStoreAttributeMapping.AttributeIds.GroupAttributeDescription, "myDescription"));

        schemaMappingBuilder.addObjectMappings(userObjectMappingBuilder.buildObjectMapping());
        schemaMappingBuilder.addObjectMappings(groupObjectMappingBuilder.buildObjectMapping());

        return schemaMappingBuilder.buildSchemaMapping();
    }

    private static void validateSchemaMapping(IdentityStoreSchemaMapping expected, IdentityStoreSchemaMapping actual)
    {
        if( expected != null )
        {
            Assert.assertNotNull("SchemaDetails should not be null.", actual);
            Assert.assertEquals( "number of matched objects should be the same", expected.getObjectMappings().size(), actual.getObjectMappings().size() );

            for( IdentityStoreObjectMapping objectMapping : expected.getObjectMappings() )
            {
                IdentityStoreObjectMapping actualObjectMapping = actual.getObjectMapping(objectMapping.getObjectId());
                Assert.assertNotNull("Object Mapping should exist.", actualObjectMapping);
                Assert.assertEquals( "number of matched attributes should be the same",
                        objectMapping.getAttributeMappings().size(), actualObjectMapping.getAttributeMappings().size() );
                Assert.assertEquals( "mapped object class should match",
                        objectMapping.getObjectClass(), actualObjectMapping.getObjectClass() );

                for( IdentityStoreAttributeMapping attributeMapping : objectMapping.getAttributeMappings() )
                {
                    IdentityStoreAttributeMapping actualAttributeMapping = actualObjectMapping.getAttributeMapping(attributeMapping.getAttributeId());
                    Assert.assertNotNull("Attribute Mapping should exist.", actualAttributeMapping);
                    Assert.assertEquals( "Mapped attributes should be the same",
                            attributeMapping.getAttributeName(), actualAttributeMapping.getAttributeName() );
                }
            }
        }
    }

    private void ensureADIdentityStoreExistForTenant(String tenantName)
            throws Exception
            {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();
        String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        IIdentityStoreData store =
                idmClient.getProvider(tenantName, adProviderName);
        if (store != null)
        {
            return;
        } else
        {
            IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
            idmClient.addProvider(tenantName, adStore);
        }
            }

    private void ensureOpenLdapIdentityStoreExistForTenant(String tenantName,
            boolean bWithAlias) throws Exception
            {
	ensureOpenLdapIdentityStoreExistForTenant(tenantName, bWithAlias, CFG_KEY_IDM_TENANT_1_OL_HOST_NAME, null);
            }

    private void ensureOpenLdapIdentityStoreExistForTenant(String tenantName,
            boolean bWithAlias, String hostnamePropertyName, Collection<X509Certificate> certs) throws Exception
            {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();
        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);

        IIdentityStoreData olStore =
                idmClient.getProvider(tenantName, olProviderName);
        if (olStore != null)
        {
            return;
        }

        final String olProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
        Assert.assertNotNull(olProviderAlias);

        final String olHost =
                props.getProperty(hostnamePropertyName);
        Assert.assertNotNull(olHost);

        final String olUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN);
        Assert.assertNotNull(olUserName);
        final String olPwd =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD);
        Assert.assertNotNull(olPwd);

        final ArrayList<String> olldapHosts = new ArrayList<String>();
        olldapHosts.add(olHost);

        final String userOlSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_BASE_DN);

        final String groupOlSearchBaseDn =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_BASE_DN);

        final Map<String, String> attrMap = new HashMap<String, String>();
        attrMap.put(SAMLATTR_GIVEN_NAME, "givenName");
        attrMap.put(SAMLATTR_SUR_NAME, "sn");
        attrMap.put(SAMLATTR_GROUP_IDENTITY, "memberof");
        //No subjectType attribute for nonSystemProvider
        attrMap.put(SAMLATTR_EMAIL_ADDRESS, "mail");
        attrMap.put(SAMLATTR_UPN, "userPrincipalName");

        olStore =
                IdentityStoreData.CreateExternalIdentityStoreData(
                        olProviderName, bWithAlias ? olProviderAlias : null,
                                IdentityStoreType.IDENTITY_STORE_TYPE_LDAP,
                                AuthenticationType.PASSWORD, null, 0, olUserName, false, null,
                                olPwd, userOlSearchBaseDn, groupOlSearchBaseDn,
                                olldapHosts, attrMap, null, null, certs, null);
        idmClient.addProvider(tenantName, olStore);
            }

    @TestOrderAnnotation(order = 21)
    @Test
    @Ignore("Bugzilla#1324293")
    public void testGetPasswordExpirationInfo() throws Exception, IDMException
    {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        // For AD provider as external domain
        String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);
        //save the original store before deleting, will be used for restoration later
        IIdentityStoreData origStore =
                idmClient.getProvider(tenantName, adProviderName);

        if (origStore != null)
        {
            idmClient.deleteProvider(tenantName, adProviderName);
        }

        // Add Active Directory IDP from config
        IdentityStoreData newADStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
        Assert.assertNull(idmClient.getProvider(tenantName, adProviderName));
        idmClient.addProvider(tenantName, newADStore);

        //  Verify pwdLastSet and pwdLifeTime is retrieved correctly
        // For user with FGPP setup
        String adUserId =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);
        PersonUser adUser =
                idmClient.findPersonUser(tenantName, new PrincipalId(adUserId,
                        adProviderName));
        Assert.assertTrue(adUser.getDetail().getPwdLastSet() > 0);
        Assert.assertTrue(adUser.getDetail().getPwdLifeTime() > 0);

        // For user with just Default Domain Policy setup
        adUserId =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_DEFAULT_DOMAIN_POLICY_TEST_USER);
        adUser =
                idmClient.findPersonUser(tenantName, new PrincipalId(adUserId,
                        adProviderName));
        Assert.assertTrue(adUser.getDetail().getPwdLastSet() > 0);
        Assert.assertTrue(adUser.getDetail().getPwdLifeTime() > 0);

        // cleanup and restore the original provider
        idmClient.deleteProvider(tenantName, newADStore.getName());
        if (origStore != null)
        {
            idmClient.addProvider(tenantName, origStore);
        }

        // For VmDir provider as system domain
        String pwdExpirationTestUser = "__test_user_pwd_expiration";
        // Add the new user to system domain
        PrincipalId principalPwdExpirationTestUser =
                idmClient
                .addPersonUser(
                        tenantName,
                        pwdExpirationTestUser,
                        new PersonDetail.Builder()
                        .firstName("firstname")
                        .lastName("lastname")
                        .emailAddress("example@vmware.com")
                        .description(
                                "Person created to test pwd expiration information")
                                .build(), "myPasword#123".toCharArray());

        PersonUser personUser =
                idmClient.findPersonUser(tenantName,
                        principalPwdExpirationTestUser);

        Assert.assertTrue(personUser.getDetail().getPwdLastSet() > 0);
        Assert.assertTrue(personUser.getDetail().getPwdLifeTime() > 0);

        // clean up the new user added
        idmClient.deletePrincipal(tenantName, pwdExpirationTestUser);
    }

    @Ignore("bugzilla#1173915 - looks like requires native Ad?")
    @TestOrderAnnotation(order = 22)
    @Test
    public void testAuthenticateUserAccountControlExceptionsADProvider()
            throws Exception, IDMException
            {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        // For AD provider as external domain
        String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);
        //save the original store before deleting, will be used for restoration later
        IIdentityStoreData origStore =
                idmClient.getProvider(tenantName, adProviderName);

        IdentityStoreData newADStore = null;

        try
        {
            if (origStore != null)
            {
                idmClient.deleteProvider(tenantName, adProviderName);
            }

            // Add Active Directory IDP from config
            newADStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
            Assert.assertNull(idmClient.getProvider(tenantName, adProviderName));
            idmClient.addProvider(tenantName, newADStore);

            // Verify PasswordExpiredException is returned if user password is expired
            String adExpiredPassUser =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_PWDEXPIRED_USERNAME);
            String adExpiredPassUserPid =
                    String.format("%s@%s", adExpiredPassUser, adProviderName);
            String adExpiredPassUserPwd =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_PWDEXPIRED_PASSWORD);
            try
            {
                idmClient.authenticate(tenantName, adExpiredPassUserPid,
                        adExpiredPassUserPwd);
            } catch (PasswordExpiredException ex)
            {
                Assert.assertTrue("unexpected message received", ex
                        .getMessage().contains("(23)")); //KDC_ERR_KEY_EXPIRED
            } catch (Exception ex)
            {
                Assert.fail(String.format("Unexpected exception: %s",
                        ex.getMessage()));
            }

            // Verify Non-ExistentUser authenticate fails with expected error(s)
            String adNonExistentUserPid =
                    String.format("%s_NonExistent@%s", adExpiredPassUser,
                            adProviderName);
            TestNonExistentUserAuthenticate(idmClient, tenantName,
                    adNonExistentUserPid, adExpiredPassUserPwd);

            // Verify UserAccountLockedException is returned if user account is disabled / locked / expired
            String adLockedUserId =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_LOCKEDUSER_USERNAME);
            String adLockedUserPid =
                    String.format("%s@%s", adLockedUserId, adProviderName);
            String adLockedUserPwd =
                    props.getProperty(CFG_KEY_IDM_ACCTFLAGS_LOCKEDUSER_PASSWORD);
            String adLockedUserWrongPwd = "wrong" + adLockedUserPwd;

            int i = 0;
            final int lockThreshold = 10; // lock threshold set for the ssolabs domain PSO settings
            boolean accountLocked = false;
            //setup locked state by bad logins up to the threshold.
            //verify that Kerberos response to locked account authentication is mapped correctly
            while (i++ <= lockThreshold)
            {
                try
                {
                    idmClient.authenticate(tenantName, adLockedUserPid,
                            adLockedUserWrongPwd);
                } catch (UserAccountLockedException ex)
                { //account now is in locked state.
                    accountLocked = true;
                    break;
                } catch (IDMLoginException ex)
                { // expected response for bad login attempt before account is locked
                    Assert.assertTrue("", i <= lockThreshold);
                }
            }
            Assert.assertTrue("account should be locked but isn't", accountLocked);
        } finally
        {
            // cleanup and restore the original provider
            idmClient.deleteProvider(tenantName, newADStore.getName());
            if (origStore != null)
            {
                idmClient.addProvider(tenantName, origStore);
            }
        }
            }

    @TestOrderAnnotation(order = 23)
    @Test
    public void testAddUserDetail() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String userTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(userTenantName);

        String password = "myPasword#123";

        // Add user w/o lastname should be allowed
        PrincipalId principalNoLastName =
                idmClient
                .addPersonUser(
                        userTenantName,
                        "usernolastname",
                        new PersonDetail.Builder()
                        .firstName("firstname")
                        .lastName(null)
                        .emailAddress(
                                "usernolastname@vmware.com")
                                .description(
                                        "Person created to test user without lastname can be added to sp")
                                        .build(), password.toCharArray());

        PersonUser userWithoutLastname =
                idmClient.findPersonUser(userTenantName, principalNoLastName);
        Assert.assertNotNull(userWithoutLastname);
        Assert.assertEquals(userWithoutLastname.getDetail().getFirstName(),
                "firstname");
        Assert.assertNull(userWithoutLastname.getDetail().getLastName());


        idmClient.deletePrincipal(userTenantName, "usernolastname");
        userWithoutLastname =
                idmClient.findPersonUser(userTenantName, principalNoLastName);
        Assert.assertNull(userWithoutLastname);

        // add user w/o firstname should be allowed
        PrincipalId principalNoFirstName =
                idmClient
                .addPersonUser(
                        userTenantName,
                        "usernofirstname",
                        new PersonDetail.Builder()
                        .firstName(null)
                        .lastName("lastname")
                        .emailAddress(
                                "usernofirstname@vmware.com")
                                .description(
                                        "Person created to test user without firstname can be added to sp")
                                        .build(), password.toCharArray());

        PersonUser userWithoutFirstname =
                idmClient.findPersonUser(userTenantName, principalNoFirstName);
        Assert.assertNotNull(userWithoutFirstname);
        Assert.assertEquals(userWithoutFirstname.getDetail().getLastName(),
                "lastname");
        Assert.assertNull(userWithoutFirstname.getDetail().getFirstName());

        idmClient.deletePrincipal(userTenantName, "usernofirstname");
        userWithoutFirstname =
                idmClient.findPersonUser(userTenantName, principalNoFirstName);
        Assert.assertNull(userWithoutFirstname);

        // add user with empty user detail should be allowed
        PrincipalId principalEmptydetails =
                idmClient.addPersonUser(userTenantName, "useremptydetail",
                        new PersonDetail.Builder().firstName("").lastName("")
                        .emailAddress("").description("").build(),
                        password.toCharArray());

        PersonUser userWithEmptydetails =
                idmClient.findPersonUser(userTenantName, principalEmptydetails);
        Assert.assertNotNull(userWithEmptydetails);
        Assert.assertNull(userWithEmptydetails.getDetail().getFirstName());

        idmClient.deletePrincipal(userTenantName, "useremptydetail");
        userWithEmptydetails =
                idmClient.findPersonUser(userTenantName, principalEmptydetails);
        Assert.assertNull(userWithEmptydetails);

        // Modify a user originally with firstname and lastname to both null firstname and lastname
        PrincipalId principal =
                idmClient
                .addPersonUser(
                        userTenantName,
                        "usermod",
                        new PersonDetail.Builder()
                        .firstName("firstname")
                        .lastName("lastname")
                        .emailAddress("usermod@vmware.com")
                        .description(
                                "Person created to test modify user firstname/lastname to null")
                                .build(), password.toCharArray());

        PersonUser user = idmClient.findPersonUser(userTenantName, principal);
        Assert.assertNotNull(user);
        Assert.assertEquals(user.getDetail().getFirstName(), "firstname");
        Assert.assertEquals(user.getDetail().getLastName(), "lastname");

        idmClient
        .updatePersonUserDetail(
                userTenantName,
                "usermod",
                new PersonDetail.Builder()
                .firstName(null)
                .lastName(null)
                .emailAddress("usermodified@vmware.com")
                .description(
                        "Person created to test modify user firstname/lastname to null")
                        .build());
        user = idmClient.findPersonUser(userTenantName, principal);

        Assert.assertNull(user.getDetail().getLastName());
        Assert.assertNull(user.getDetail().getFirstName());
        Assert.assertEquals(user.getDetail().getEmailAddress(),
                "usermodified@vmware.com");

        idmClient.deletePrincipal(userTenantName, "usermod");
        user = idmClient.findPersonUser(userTenantName, principal);
        Assert.assertNull(user);
    }

    private void testIsActive(String domainName) throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        Properties props = getTestProperties();

        String userTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(userTenantName);

        IdmClientTestUtil.ensureTenantExists(idmClient, userTenantName);

        // Test isActive throw proper exception in case such principal does not exist
        try
        {
            final String olDomainName = props.getProperty(domainName);
            Assert.assertNotNull(olDomainName);
            idmClient.isActive(userTenantName, new PrincipalId(
                    "userdoesnotexist", olDomainName));
            Assert.fail();
        } catch (InvalidPrincipalException ex)
        {
            // ignore
            System.out.println("Got expected exception");
        }
    }

    @TestOrderAnnotation(order = 24)
    @Test
    public void testNonExistingPrincipalIsActive() throws Exception,
    IDMException
    {
        // non-existing user in openldap provider
        testIsActive(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        // non-existing user in AD provider
        testIsActive(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        // non-existing user in system vmdir provider in tenant 1 'abc.com'
        testIsActive(CFG_KEY_IDM_TENANT_1_NAME);
    }

    @TestOrderAnnotation(order = 25)
    @Test
    public void testAddRemoveLocalOsFsptestUser() throws Exception, IDMException
    {
        if (SystemUtils.IS_OS_LINUX)
        {
            CasIdmClient idmClient = getIdmClient();

            String tenantName = idmClient.getDefaultTenant();
            Assert.assertNotNull(tenantName);

            Collection<IIdentityStoreData> stores =
                    idmClient.getProviders(tenantName,
                            EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
            if (stores == null || stores.isEmpty())
            {
                // add the localOs
                final String localOsDomainName = "localOsDomain";
                IIdentityStoreData localOsProvider =
                        IdentityStoreData
                        .CreateLocalOSIdentityStoreData(localOsDomainName);
                idmClient.addProvider(tenantName, localOsProvider);

                stores =
                        idmClient.getProviders(tenantName,
                                EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
            }
            Assert.assertNotNull(stores);
            Assert.assertTrue(stores.size() == 1);

            IIdentityStoreData store = stores.iterator().next();
            PrincipalId principal = new PrincipalId("root", store.getName());

            PersonUser person = idmClient.findPersonUser(tenantName, principal);
            Assert.assertNotNull(person);

            boolean bUserAdded = idmClient.addUserToGroup(tenantName, principal,
                    "Administrators");
            Assert.assertTrue(bUserAdded);

            try
            {
                bUserAdded = false;
                bUserAdded = idmClient.addUserToGroup(tenantName, principal,
                        "Administrators");
                Assert.assertTrue(bUserAdded);
            }
            catch(MemberAlreadyExistException e)
            {
                Assert.assertTrue(!bUserAdded);
            }

            Set<PersonUser> members =
                    idmClient.findPersonUsersInGroup(tenantName,
                            new PrincipalId("Administrators", tenantName),
                            "", -1);
            Assert.assertTrue(members.contains(person));

            boolean bIsRemoved = idmClient.removeFromLocalGroup(tenantName, principal, "Administrators");
            Assert.assertTrue(bIsRemoved);

            members =
                    idmClient.findPersonUsersInGroup(tenantName,
                            new PrincipalId("Administrators", tenantName),                                                     "", -1);
            Assert.assertTrue(members == null || members.size() == 0 || !members.contains(person));
        }
    }

    @TestOrderAnnotation(order = 26)
    @Test
    public void testAddRemoveLocalOsFsptestGroup() throws Exception, IDMException
    {
        if (SystemUtils.IS_OS_LINUX)
        {
            CasIdmClient idmClient = getIdmClient();

            String tenantName = idmClient.getDefaultTenant();
            Assert.assertNotNull(tenantName);

            Collection<IIdentityStoreData> stores =
                    idmClient.getProviders(tenantName,
                            EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
            if (stores == null || stores.isEmpty())
            {
                // add the localOs
                final String localOsDomainName = "localOsDomain";
                IIdentityStoreData localOsProvider =
                        IdentityStoreData
                        .CreateLocalOSIdentityStoreData(localOsDomainName);
                idmClient.addProvider(tenantName, localOsProvider);

                stores =
                        idmClient.getProviders(tenantName,
                                EnumSet.of(DomainType.LOCAL_OS_DOMAIN));
            }
            Assert.assertNotNull(stores);
            Assert.assertTrue(stores.size() == 1);

            IIdentityStoreData store = stores.iterator().next();
            PrincipalId principal = new PrincipalId("root", store.getName());

            Group group = idmClient.findGroup(tenantName, principal);
            Assert.assertNotNull(group);

            boolean bGroupAdded = idmClient.addGroupToGroup(tenantName, principal,
                    "Administrators");
            Assert.assertTrue(bGroupAdded);

            try
            {
                bGroupAdded = false;
                bGroupAdded = idmClient.addGroupToGroup(tenantName, principal,
                        "Administrators");
                Assert.assertTrue(bGroupAdded);
            }
            catch(MemberAlreadyExistException e)
            {
                Assert.assertTrue(!bGroupAdded);
            }

            Set<Group> members =
                    idmClient.findGroupsInGroup(tenantName,
                            new PrincipalId("Administrators", tenantName),
                            "", -1);
            Assert.assertTrue(members.contains(group));

            boolean bIsRemoved = idmClient.removeFromLocalGroup(tenantName, principal, "Administrators");
            Assert.assertTrue(bIsRemoved);

            members =
                    idmClient.findGroupsInGroup(tenantName,
                            new PrincipalId("Administrators", tenantName),                                                     "", -1);
            Assert.assertTrue(members == null || members.size() == 0 || !members.contains(group));
        }
    }

    @TestOrderAnnotation(order = 27)
    @Test
    public void testAddRemoveOLFsptestUser() throws Exception, IDMException
    {
        testAddRemoveOLFspTestUserInternal();
    }

    private void testAddRemoveOLFspTestUserInternal() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String userTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(userTenantName);

        Tenant userTenant = IdmClientTestUtil.ensureTenantExists(idmClient, userTenantName);
        Assert.assertNotNull(userTenant);

        final String olUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST_ID);
        Assert.assertNotNull(olUserName);

        final String olDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olDomainName);

        PrincipalId principal1 = new PrincipalId(olUserName, olDomainName);

        PersonUser olUser =
                idmClient.findPersonUser(userTenantName, principal1);
        Assert.assertNotNull(olUser);

        final String olUserName1 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_USER_FSPTEST1_ID);
        Assert.assertNotNull(olUserName1);

        PrincipalId principal2 = new PrincipalId(olUserName1, olDomainName);

        PersonUser olUser1 =
                idmClient.findPersonUser(userTenantName, principal2);
        Assert.assertNotNull(olUser1);

        testUserFSP(idmClient, userTenantName, principal1, principal2);
    }

    @TestOrderAnnotation(order = 28)
    @Test
    public void testAddRemoveOLFsptestGroup() throws Exception, IDMException
    {
        testAddRemoveOLFstTestGroupInternal();
    }

    private void testAddRemoveOLFstTestGroupInternal() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String groupTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(groupTenantName);

        Tenant groupTenant = IdmClientTestUtil.ensureTenantExists(idmClient, groupTenantName);
        Assert.assertNotNull(groupTenant);

        final String olGroupName1 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST1_ID);
        Assert.assertNotNull(olGroupName1);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adDomainName);

        PrincipalId principal1 = new PrincipalId(olGroupName1, adDomainName);

        Group olGroup1 = idmClient.findGroup(groupTenantName, principal1);
        Assert.assertNotNull(olGroup1);

        final String olGroupName2 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OLPROVIDER_GROUP_FSPTEST2_ID);
        Assert.assertNotNull(olGroupName2);

        PrincipalId principal2 = new PrincipalId(olGroupName2, adDomainName);

        Group olGroup2 = idmClient.findGroup(groupTenantName, principal2);
        Assert.assertNotNull(olGroup2);

        testGroupFSP(idmClient, groupTenantName, principal1, principal2);
    }

    @TestOrderAnnotation(order = 29)
    @Test
    public void testAddRemoveADFsptestUser() throws Exception, IDMException
    {
        testAddRemoveADFsptestUserInternal();
    }

    private void testAddRemoveADFsptestUserInternal() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String userTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(userTenantName);

        Tenant userTenant = IdmClientTestUtil.ensureTenantExists(idmClient, userTenantName);
        Assert.assertNotNull(userTenant);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST_ID);
        Assert.assertNotNull(adUserName);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adDomainName);

        PrincipalId principal1 = new PrincipalId(adUserName, adDomainName);

        PersonUser adUser = idmClient.findPersonUser(userTenantName, principal1);
        Assert.assertNotNull(adUser);

        final String adUserName1 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_FSPTEST1_ID);
        Assert.assertNotNull(adUserName1);

        PrincipalId principal2 = new PrincipalId(adUserName1, adDomainName);

        PersonUser adUser1 =
                idmClient.findPersonUser(userTenantName, principal2);
        Assert.assertNotNull(adUser1);

        testUserFSP(idmClient, userTenantName, principal1, principal2);
    }

    @TestOrderAnnotation(order = 30)
    @Test
    public void testAddRemoveADFsptestGroup() throws Exception, IDMException
    {
        testAddRemoveADFsptestGroupInternal();
    }

    private void testAddRemoveADFsptestGroupInternal() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String groupTenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(groupTenantName);

        Tenant groupTenant = IdmClientTestUtil.ensureTenantExists(idmClient, groupTenantName);
        Assert.assertNotNull(groupTenant);

        final String adGroupName1 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_FSPTEST1_ID);
        Assert.assertNotNull(adGroupName1);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adDomainName);

        PrincipalId principal1 = new PrincipalId(adGroupName1, adDomainName);

        Group adGroup1 = idmClient.findGroup(groupTenantName, principal1);
        Assert.assertNotNull(adGroup1);

        final String adGroupName2 =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_GROUP_FSPTEST2_ID);
        Assert.assertNotNull(adGroupName2);

        PrincipalId principal2 = new PrincipalId(adGroupName2, adDomainName);

        Group adGroup2 = idmClient.findGroup(groupTenantName, principal2);
        Assert.assertNotNull(adGroup2);

        testGroupFSP(idmClient, groupTenantName, principal1, principal2);
    }

    /**
     * Place this after set testings so the properties are available
     *
     * @throws Exception
     * @throws IDMException
     */
    @TestOrderAnnotation(order = 31)
    @Test
    public void testExportTenantConfiguration() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = _expTenantName;

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        List<Certificate> certList = new ArrayList<Certificate>();

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        String alias = props.getProperty(CFG_KEY_STS_KEY_ALIAS);

        Assert.assertNotNull(alias);

        certList.add(ks.getCertificate(alias));

        String alias1 = props.getProperty(CFG_KEY_STS_KEY_ALIAS1);

        Assert.assertNotNull(alias1);

        certList.add(ks.getCertificate(alias1));

        String password = props.getProperty(CFG_KEY_STS_KEYSTORE_PASSWORD);

        PrivateKey key = (PrivateKey) ks.getKey(alias, password.toCharArray());

        idmClient.setTenantCredentials(tenantName, certList, key);

        try
        {
            exportTest(idmClient, true);
            exportTest(idmClient, false);

        } catch (Exception e)
        {
            throw new AssertionError(e);
        }

    }

    /**
     * Run export test
     *
     * @param idmClient
     * @param exportPrivateData
     * @throws Exception
     * @throws FileNotFoundException
     * @throws TransformerConfigurationException
     * @throws TransformerFactoryConfigurationError
     * @throws TransformerException
     * @throws ParserConfigurationException
     * @throws SAXException
     * @throws IOException
     */
    private void exportTest(CasIdmClient idmClient, boolean exportPrivateData)
            throws Exception, FileNotFoundException,
            TransformerConfigurationException,
            TransformerFactoryConfigurationError, TransformerException,
            ParserConfigurationException, SAXException, IOException
            {
        Document tenantDoc =
                idmClient.exportTenantConfiguration(_expTenantName,
                        exportPrivateData);

        persistDoc(tenantDoc, _exportedConfigFile);
        loadFileAndvalidate(idmClient, _exportedConfigFile);
            }

    private void persistDoc(Document doc, String outputFileName)
            throws Exception
            {
        // Prepare the DOM document for writing
        Source source = new DOMSource(doc);

        // Prepare the output file. file->FileoutputStream->Result
        Result result = new StreamResult(new FileOutputStream(outputFileName));

        // Write the DOM to file
        Transformer xformer = TransformerFactory.newInstance().newTransformer();
        xformer.setOutputProperty(OutputKeys.INDENT, "yes");
        xformer.transform(source, result);
            }

    private void loadFileAndvalidate(CasIdmClient idmClient, String fileName)
            throws Exception
            {
        //Parse the output file and validate
        DocumentBuilderFactory builderFactory =
                DocumentBuilderFactory.newInstance();
        builderFactory.setNamespaceAware(true);
        DocumentBuilder builder = builderFactory.newDocumentBuilder();
        builder.setErrorHandler(new SamlParserErrorHandler());
        Document outputReadDoc = builder.parse(new FileInputStream(fileName));
        idmClient.samlValidate(outputReadDoc);
            }

    @TestOrderAnnotation(order = 32)
    @Test
    public void testFindUserByString() throws Exception
    {
        CasIdmClient idmClient = getIdmClient();
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        final String accountName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);
        Assert.assertNotNull(accountName);
        final String domainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(domainName);

        Principal user1 = null;
        Principal user2 = null;

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        // find user <name>@<domain>
        String principal = accountName + "@" + domainName;

        user1 = idmClient.findUser(tenantName, principal);

        Assert.assertNotNull(user1);
        Assert.assertNotNull(user1.getId());
        Assert.assertEquals(user1.getId().getName().toLowerCase(),
                accountName.toLowerCase());
        Assert.assertEquals(user1.getId().getDomain().toUpperCase(),
                domainName.toUpperCase());

        // find user <name><domain>, i.e. invalid
        principal = accountName + domainName;

        try
        {
            user1 = idmClient.findUser(tenantName, principal);
            Assert.fail("should not be able to find principal in invalid format.");
        } catch (InvalidPrincipalException ex)
        { /* expected */
        }

        // find user <name_upper>@<domain>
        principal = accountName.toUpperCase() + "@" + domainName;

        user2 = idmClient.findUser(tenantName, principal);
        Assert.assertNotNull(user2);
        Assert.assertNotNull(user2.getId());
        Assert.assertEquals(user1.getId().getName(), user2.getId().getName());
        Assert.assertEquals(user1.getId().getDomain(), user2.getId()
                .getDomain());

        final String alias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        Assert.assertNotNull(alias);

        // find user alias\\name
        principal = alias + "\\" + accountName;

        user2 = idmClient.findUser(tenantName, principal);

        Assert.assertNotNull(user2);
        Assert.assertNotNull(user2.getId());
        Assert.assertEquals(user1.getId().getName(), user2.getId().getName());
        Assert.assertEquals(user1.getId().getDomain(), user2.getId()
                .getDomain());

        Collection<String> defaultProviders =
                idmClient.getDefaultProviders(tenantName);

        idmClient.setDefaultProviders(tenantName,
                Arrays.asList(new String[] { domainName }));

        try
        {
            // find user <name>
            principal = accountName;

            user2 = idmClient.findUser(tenantName, principal);

            Assert.assertNotNull(user2);
            Assert.assertNotNull(user2.getId());
            Assert.assertEquals(user1.getId().getName(), user2.getId()
                    .getName());
            Assert.assertEquals(user1.getId().getDomain(), user2.getId()
                    .getDomain());

            // find user \\<name>
            principal = "\\" + accountName;

            user2 = idmClient.findUser(tenantName, principal);

            Assert.assertNotNull(user2);
            Assert.assertNotNull(user2.getId());
            Assert.assertEquals(user1.getId().getName(), user2.getId()
                    .getName());
            Assert.assertEquals(user1.getId().getDomain(), user2.getId()
                    .getDomain());

            // find user <name>@
            principal = accountName + "@";

            user2 = idmClient.findUser(tenantName, principal);

            Assert.assertNotNull(user2);
            Assert.assertNotNull(user2.getId());
            Assert.assertEquals(user1.getId().getName(), user2.getId()
                    .getName());
            Assert.assertEquals(user1.getId().getDomain(), user2.getId()
                    .getDomain());

            idmClient.setDefaultProviders(tenantName, null);

            principal = accountName;
            try
            {
                user2 = idmClient.findUser(tenantName, principal);

                Assert.fail("Should not be able to find user with just name, when no default provider is set.");
            } catch (InvalidPrincipalException ex)
            {
            }

        } finally
        {
            // restore original default providers
            idmClient.setDefaultProviders(tenantName, defaultProviders);
        }

        // find user <name>@bad domain
        // Disable this portion of test, we have logic in place if no idp found, it may be AD's trusted domains
        /*principal = accountName + "@" + domainName + "_bad"; // bad domain
        try
        {
            user2 = idmClient.findUser(tenantName, principal);
            Assert.fail("Should not be able to find user in unknown domain.");
        } catch (NoSuchIdpException ex)
        {
        }*/
    }

    @TestOrderAnnotation(order = 33)
    @Test
    public void testFindGroupByString() throws Exception
    {
        CasIdmClient idmClient = getIdmClient();
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String domainName = props.getProperty(CFG_KEY_IDM_ACCTFLAGS_DOMAINNAME);
        CasIdmClient client = getIdmClient();
        Assert.assertNotNull(IdmClientTestUtil.ensureTenantExists(client, tenantName));

        Group group1 = null;
        Principal group2 = null;

        // find group <name>@<domain
        String principal = ADMIN_GROUP_NAME + "@" + domainName;

        group1 = idmClient.findGroup(tenantName, principal);

        Assert.assertNotNull(group1);
        Assert.assertNotNull(group1.getId());
        Assert.assertEquals(group1.getId().getName().toLowerCase(),
                ADMIN_GROUP_NAME.toLowerCase());
        Assert.assertEquals(group1.getId().getDomain().toUpperCase(),
                domainName.toUpperCase());

        // find group <name><domain>, i.e. invalid
        principal = ADMIN_GROUP_NAME + domainName;

        try
        {
            group1 = idmClient.findGroup(tenantName, principal);
            Assert.fail("should not be able to find principal in invalid format");
        } catch (InvalidPrincipalException ex)
        { /* expected */
        }

        // find group <name_upper>@<domain>
        principal = ADMIN_GROUP_NAME.toUpperCase() + "@" + domainName;

        group2 = idmClient.findGroup(tenantName, principal);
        Assert.assertNotNull(group2);
        Assert.assertNotNull(group2.getId());
        Assert.assertEquals(group1.getId().getName(), group2.getId().getName());
        Assert.assertEquals(group1.getId().getDomain(), group2.getId()
                .getDomain());

        Collection<String> defaultProviders =
                idmClient.getDefaultProviders(tenantName);

        idmClient.setDefaultProviders(tenantName,
                Arrays.asList(new String[] { domainName }));

        try
        {
            // find group <name>
            principal = ADMIN_GROUP_NAME;

            group2 = idmClient.findGroup(tenantName, principal);
            Assert.assertNotNull(group2);
            Assert.assertNotNull(group2.getId());
            Assert.assertEquals(group1.getId().getName(), group2.getId()
                    .getName());
            Assert.assertEquals(group1.getId().getDomain(), group2.getId()
                    .getDomain());

            // find group \\<name>
            principal = "\\" + ADMIN_GROUP_NAME;

            group2 = idmClient.findGroup(tenantName, principal);
            Assert.assertNotNull(group2);
            Assert.assertNotNull(group2.getId());
            Assert.assertEquals(group1.getId().getName(), group2.getId()
                    .getName());
            Assert.assertEquals(group1.getId().getDomain(), group2.getId()
                    .getDomain());

            // find group @<name>
            principal = ADMIN_GROUP_NAME + "@";

            group2 = idmClient.findGroup(tenantName, principal);
            Assert.assertNotNull(group2);
            Assert.assertNotNull(group2.getId());
            Assert.assertEquals(group1.getId().getName(), group2.getId()
                    .getName());
            Assert.assertEquals(group1.getId().getDomain(), group2.getId()
                    .getDomain());

            idmClient.setDefaultProviders(tenantName, null);

            principal = ADMIN_GROUP_NAME;
            try
            {
                group2 = idmClient.findGroup(tenantName, principal);

                Assert.fail("Should not be able to find user with just name, when no default provider is set.");
            } catch (InvalidPrincipalException ex)
            {
            }

        } finally
        {
            // restore original default providers
            idmClient.setDefaultProviders(tenantName, defaultProviders);
        }

        // find group <name>@bad domain
        // Disable this portion of test, we have logic in place if no idp found, it may be AD's trusted domains
        /*principal = adminGroup + "@" + domainName + "_bad"; // bad domain
        try
        {
            group2 = idmClient.findGroup(tenantName, principal);
            Assert.fail("Should not be able to find group in unknown domain.");
        } catch (NoSuchIdpException ex)
        {
        }*/
    }

    @TestOrderAnnotation(order = 34)
    @Test
    public void testAuthenticate() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String accountName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);

        Assert.assertNotNull(accountName);

        final String domainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(domainName);

        String principal = accountName + "@" + domainName;

        String password =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_PASSWORD);

        Assert.assertNotNull(password);

        PrincipalId id =
                idmClient.authenticate(tenantName, principal, password);

        Assert.assertNotNull(id);
        Assert.assertEquals(id.getName(), accountName);
        Assert.assertEquals(id.getDomain().toUpperCase(),
                domainName.toUpperCase());

        principal = accountName.toUpperCase() + "@" + domainName;

        final String alias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        Assert.assertNotNull(alias);

        principal = alias + "\\" + accountName;

        id = idmClient.authenticate(tenantName, principal, password);

        Assert.assertNotNull(id);
        Assert.assertEquals(id.getName(), accountName);
        Assert.assertEquals(id.getDomain().toUpperCase(),
                domainName.toUpperCase());

        Collection<String> defaultProviders =
                idmClient.getDefaultProviders(tenantName);

        idmClient.setDefaultProviders(tenantName,
                Arrays.asList(new String[] { domainName }));

        try
        {
            principal = accountName;

            id = idmClient.authenticate(tenantName, principal, password);

            Assert.assertNotNull(id);
            Assert.assertEquals(id.getName(), accountName);
            Assert.assertEquals(id.getDomain().toUpperCase(),
                    domainName.toUpperCase());

            principal = "\\" + accountName;

            id = idmClient.authenticate(tenantName, principal, password);

            Assert.assertNotNull(id);
            Assert.assertEquals(id.getName(), accountName);
            Assert.assertEquals(id.getDomain().toUpperCase(),
                    domainName.toUpperCase());

            principal = accountName + "@";

            id = idmClient.authenticate(tenantName, principal, password);

            Assert.assertNotNull(id);
            Assert.assertEquals(id.getName(), accountName);
            Assert.assertEquals(id.getDomain().toUpperCase(),
                    domainName.toUpperCase());

            idmClient.setDefaultProviders(tenantName, null);

            principal = accountName;
            try
            {
                id = idmClient.authenticate(tenantName, principal, password);

                Assert.fail("Should not be able to authenticate account name, when no default provider is set.");
            } catch (IDMLoginException ex)
            {
            }

        } finally
        {
            // restore original default providers
            idmClient.setDefaultProviders(tenantName, defaultProviders);
        }

        principal = accountName + "@" + domainName;
        password = "password"; // bad password
        try
        {
            id = idmClient.authenticate(tenantName, principal, password);
        } catch (IDMLoginException ex)
        {
        }

        principal = accountName + "@" + domainName + "_bad"; // bad domain
        password = "bad_password";
        try
        {
            id = idmClient.authenticate(tenantName, principal, password);
        } catch (IDMLoginException ex)
        {
        }

        // Authenticate 'administrator@abc.com'
        // add new user to 'abc.com' and authenticate that user
        String spDomainName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        try
        {
            principal = "administrator" + "@" + spDomainName;
            //this needs to be aligned to IdmServerConfig.DEFAULT_TENANT_ADMIN_PASSWORD
            password = "defaultPwd#1";

            id = idmClient.authenticate(tenantName, principal, password);
        } catch (IDMLoginException ex)
        {
            Assert.fail("Should not reach here, 'administrator@abc.com should be authenticated");
        }

        String spUserPassword = "myPasword#123";

        PrincipalId newprincipalinSp =
                idmClient
                .addPersonUser(
                        tenantName,
                        "newuserInSp",
                        new PersonDetail.Builder()
                        .firstName("firstname")
                        .lastName(null)
                        .emailAddress("newuserInSp@vmware.com")
                        .description(
                                "Person created to test whether user in SP can be authenticated")
                                .build(), spUserPassword.toCharArray());
        try
        {
            PersonUser newUserinSp =
                    idmClient.findPersonUser(tenantName, newprincipalinSp);
            Assert.assertNotNull(newUserinSp);

            try
            {
                principal = "newuserInSp" + "@" + spDomainName;

                id =
                        idmClient.authenticate(tenantName, principal,
                                spUserPassword);
            } catch (IDMLoginException ex)
            {
                Assert.fail("Should not reach here, 'newuserInSp@abc.com should be authenticated");
            }
        } finally
        {
            idmClient.deletePrincipal(tenantName, "newuserInSp");
        }
    }

    @TestOrderAnnotation(order = 35)
    @Test
    public void testGetAttributeValues() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);

        Assert.assertNotNull(adUserName);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(adDomainName);

        PrincipalId principal = new PrincipalId(adUserName, adDomainName);

        PersonUser user = idmClient.findPersonUser(tenantName, principal);
        Assert.assertNotNull(user);

        // cannot retrieve user password hash from non-lotus provider
        // expected behavior: if such user exists, userPasswordHash is null
        // if such user does exist, return null as password
        byte[] userPasswordHash =
                idmClient.getUserHashedPassword(tenantName, principal);
        Assert.assertNull(userPasswordHash);

        // if such user does not exist, throw exception saying no such user
        try
        {
            userPasswordHash =
                    idmClient
                    .getUserHashedPassword(tenantName, new PrincipalId(
                            "nosuchuser", principal.getDomain()));
        } catch (InvalidPrincipalException ex)
        {
            // expected
        }

        Collection<Attribute> supportedAttributes =
                idmClient.getAttributeDefinitions(tenantName);

        Collection<AttributeValuePair> attributes =
                idmClient.getAttributeValues(tenantName, principal,
                        supportedAttributes);

        Assert.assertNotNull(attributes);

        AttributeValuePair isSolutionAttr =
                IdmClientTestUtil.findAttribute(attributes, ATTR_NAME_IS_SOLUTION);

        Assert.assertNotNull(isSolutionAttr);

        List<String> values = isSolutionAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertEquals("false", values.get(0));

        // The following user has no first name or last name

        final String adNonameUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_NONAME_ID);

        Assert.assertNotNull(adUserName);

        principal = new PrincipalId(adNonameUserName, adDomainName);

        user = idmClient.findPersonUser(tenantName, principal);
        Assert.assertNotNull(user);

        attributes =
                idmClient.getAttributeValues(tenantName, principal,
                        supportedAttributes);

        Assert.assertNotNull(attributes);

        AttributeValuePair upnAttr = IdmClientTestUtil.findAttribute(attributes, ATTR_NAME_UPN);

        Assert.assertNotNull(upnAttr);

        values = upnAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertTrue(values.get(0).length() > 0);

        AttributeValuePair groups = IdmClientTestUtil.findAttribute(attributes, ATTRIBUTE_GROUPS);

        Assert.assertNotNull(groups);

        values = groups.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() > 0);

        Pattern pattern = Pattern.compile("^([^\\\\])+\\\\([^\\\\])+$");

        for (String value : values)
        {
            Matcher matcher = pattern.matcher(value);

            Assert.assertTrue(matcher.matches());
        }
    }

    @TestOrderAnnotation(order = 36)
    @Test
    public void testADProviderGetNestedParentGroups() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);

        Assert.assertNotNull(adUserName);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(adDomainName);

        PrincipalId principal = new PrincipalId(adUserName, adDomainName);

        Set<Group> nestedGroups =
                idmClient.findNestedParentGroups(tenantName, principal);
        Assert.assertNotNull("nestedGroups must not be null", nestedGroups);
        Assert.assertFalse("nestedGroups must not be empty",
                nestedGroups.isEmpty());

        final String candidateGroupName = "Group1";
        boolean foundCandidateGroup = false;
        for (Group g : nestedGroups)
        {
            if (candidateGroupName.compareToIgnoreCase(g.getName()) == 0)
            {
                foundCandidateGroup = true;
                break;
            }
        }
        Assert.assertTrue(String.format("nestedGroups must include '%s'",
                candidateGroupName), foundCandidateGroup);
    }

    @TestOrderAnnotation(order = 37)
    @Test
    public void testPagedSearchAgainstAD() throws Exception
    {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adDomainName);

        SearchCriteria allUserInCisContainer =
                new SearchCriteria("John", adDomainName);
        Set<PersonUser> allUsers =
                idmClient.findPersonUsers(tenantName, allUserInCisContainer, -1);
        Assert.assertEquals(2200, allUsers.size());

        Set<PersonUser> limitedUsers =
                idmClient.findPersonUsers(tenantName, allUserInCisContainer, 800);
        Assert.assertEquals(800, limitedUsers.size());

        limitedUsers =
                idmClient.findPersonUsers(tenantName, allUserInCisContainer, 1600);
        Assert.assertEquals(1600, limitedUsers.size());

        SearchCriteria allFspUsers =
                new SearchCriteria("fsptest", adDomainName);
        allUsers =
                idmClient.findPersonUsersByName(tenantName, allFspUsers, -1);
        Assert.assertEquals(2, allUsers.size());

        limitedUsers =
                idmClient.findPersonUsersByName(tenantName, allFspUsers, 1);
        Assert.assertEquals(1, limitedUsers.size());

        SearchCriteria allGroupInCisContainer =
                new SearchCriteria("JohnGroup", adDomainName);
        Set<Group> allGroups =
                idmClient.findGroups(tenantName, allGroupInCisContainer, -1);
        Assert.assertEquals(2200, allGroups.size());

        Set<Group> limitedGroups =
                idmClient.findGroups(tenantName, allGroupInCisContainer, 800);
        Assert.assertEquals(800, limitedGroups.size());

        limitedGroups =
                idmClient.findGroups(tenantName, allGroupInCisContainer, 1600);
        Assert.assertEquals(1600, limitedGroups.size());

        allGroups =
                idmClient.findGroupsByName(tenantName, allGroupInCisContainer, -1);
        Assert.assertEquals(2200, allGroups.size());

        limitedGroups =
                idmClient.findGroupsByName(tenantName, allGroupInCisContainer, 800);
        Assert.assertEquals(800, limitedGroups.size());

        // test search is case-insensitive
        allGroupInCisContainer =
                new SearchCriteria("johngroup", adDomainName);
        allGroups =
                idmClient.findGroups(tenantName, allGroupInCisContainer, -1);
        Assert.assertEquals(2200, allGroups.size());

        // This exercise 'AD.findDisabledPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findDisabledPersonUsers(tenantName, "John", -1);

        // This exercise 'AD.findLockedPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findLockedUsers(tenantName, "John", -1);

    }

    @TestOrderAnnotation(order = 38)
    @Test
    public void testPagedSearchAgainstOpenLdap() throws Exception
    {
        Properties props = getTestProperties();
        CasIdmClient idmClient = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        Assert.assertNotNull(tenant);

        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);

        IIdentityStoreData store = idmClient.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, olProviderName);
            store = idmClient.getProvider(tenantName, olProviderName);
            Assert.assertNull(store);
        }

        ensureOpenLdapIdentityStoreExistForTenant(tenantName, true);

        final String olDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olDomainName);

        SearchCriteria allUserInPrefixedCis =
                new SearchCriteria("Cis", olDomainName);
        Set<PersonUser> allUsers =
                idmClient.findPersonUsers(tenantName, allUserInPrefixedCis, -1);
        Assert.assertEquals(2200, allUsers.size());

        allUsers =
                idmClient.findPersonUsersByName(tenantName, allUserInPrefixedCis, -1);
        Assert.assertEquals(2200, allUsers.size());

        SearchCriteria allGroupPrefexedCis =
                new SearchCriteria("CisGrp", olDomainName);
        Set<Group> allGroups =
                idmClient.findGroups(tenantName, allGroupPrefexedCis, -1);
        Assert.assertEquals(2200, allGroups.size());

        allGroups =
                idmClient.findGroupsByName(tenantName, allGroupPrefexedCis, -1);
        Assert.assertEquals(2200, allGroups.size());

        // test findGroups search is case-insensitive
        allGroupPrefexedCis =
                new SearchCriteria("cisgrp", olDomainName);
        allGroups =
                idmClient.findGroups(tenantName, allGroupPrefexedCis, -1);
        Assert.assertEquals(2200, allGroups.size());

        // This exercise 'OL.findDisabledPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findDisabledPersonUsers(tenantName, "Cis", -1);

        // This exercise 'OL.findLockedPersonUsers' with paged_search (it grabs all possibly users first)
        idmClient.findLockedUsers(tenantName, "Cis", -1);

        // clean up OL provider
        store = idmClient.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, olProviderName);
            store = idmClient.getProvider(tenantName, olProviderName);
            Assert.assertNull(store);
        }
    }

    @TestOrderAnnotation(order = 39)
    @Test
    public void addPersonUserWithHashedPassword() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        // SSO-v1-1
        String userName = "userWithHashedPassword";
        String userPrincipal = userName + "@" + tenant.getName();
        String clearPasswd = "password123#";
        String hashSchema = HashingAlgorithmType.SHA256_5.getHashAlgorithm();
        byte hashedPasswdBlob[] =
            { -104, 3, 91, 126, 53, -35, 117, -49, -95, 59, 43, -62, 13,
                47, 63, -96, 1, -68, 64, 108, 95, -1, -25, -96, -48,
                -117, 82, 114, 5, -93, 15, -53, 22, -26, 12, 60, 64 };
        // lotus appends hash password schema (the first one byte) to the original password blob and store it
        byte[] storedUserPassword = new byte[hashedPasswdBlob.length + 1];
        storedUserPassword[0] = 2; // this is the schema identifier
        for (int i = 0; i < hashedPasswdBlob.length; i++)
        {
            storedUserPassword[i + 1] = hashedPasswdBlob[i];
        }

        // Retrieve password for user lives in system tenant
        String systemTenantName =
                props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        byte[] retrievedSystemAdminPasswordHash =
                idmClient.getUserHashedPassword(systemTenantName,
                        new PrincipalId("administrator", systemTenantName));
        Assert.assertNotNull(retrievedSystemAdminPasswordHash);

        // Add user with hashedPassword SSO-v1-1
        PrincipalId principal =
                idmClient
                .addPersonUser(
                        tenant.getName(),
                        userName,
                        new PersonDetail.Builder()
                        .firstName("userWith")
                        .lastName("HashedPassword")
                        .emailAddress(
                                "userWithHashedPassword@vmware.com")
                                .description(
                                        "Person created to test importing user with hashedPassword")
                                        .build(), hashedPasswdBlob, hashSchema);
        Assert.assertNotNull(principal);

        PrincipalId authenticatedId =
                idmClient.authenticate(tenant.getName(), userPrincipal,
                        clearPasswd);
        Assert.assertNotNull(authenticatedId);

        // 'getUserHashedPassword' test
        byte[] retrievedUserPasswordHash =
                idmClient.getUserHashedPassword(tenant.getName(),
                        authenticatedId);
        Assert.assertTrue(Arrays.equals(storedUserPassword,
                retrievedUserPasswordHash));

        // delete this user
        idmClient.deletePrincipal(tenant.getName(), userName);

        // mimic upgrade and store this user back with the retrieved user password hash
        PrincipalId principalUpgrade =
                idmClient
                .addPersonUser(
                        tenant.getName(),
                        userName,
                        new PersonDetail.Builder()
                        .firstName("userWith")
                        .lastName("HashedPassword")
                        .emailAddress(
                                "userWithHashedPassword@vmware.com")
                                .description(
                                        "Person created to test importing user with hashedPassword")
                                        .build(), retrievedUserPasswordHash,
                                        HashingAlgorithmType.SHA512.getHashAlgorithm());
        Assert.assertNotNull(principalUpgrade);

        authenticatedId =
                idmClient.authenticate(tenant.getName(), userPrincipal,
                        clearPasswd);
        Assert.assertNotNull(authenticatedId);

        // retrieve user password one more time
        retrievedUserPasswordHash =
                idmClient.getUserHashedPassword(tenant.getName(),
                        authenticatedId);
        Assert.assertTrue(Arrays.equals(storedUserPassword,
                retrievedUserPasswordHash));

        // cleanup
        idmClient.deletePrincipal(tenant.getName(), userName);


        // Add user with hashedPassword SSO-v1-2
        userName = "userWithHashedPasswordNoSalt";
        userPrincipal = userName + "@" + tenant.getName();

        byte hashedPasswdBlobWithoutSalt[] =
            { 52, -101, -127, -18, 93, 37, 10, 27, 114, 4, -83, -82, -57,
                117, -97, 88, -27, -69, 20, -68, -78, 4, 55, 35, -7,
                -28, 53, -12, 9, 20, -87, -91 };

        principal =
                idmClient
                .addPersonUser(
                        tenant.getName(),
                        userName,
                        new PersonDetail.Builder()
                        .firstName("userWith")
                        .lastName("HashedPasswordNoSalt")
                        .emailAddress(
                                "userWithHashedPasswordNoSalt@vmware.com")
                                .description(
                                        "Person created to test importing user with hashedPassword SSO-v1-2")
                                        .build(), hashedPasswdBlobWithoutSalt,
                                        HashingAlgorithmType.SHA256_0
                                        .getHashAlgorithm());
        Assert.assertNotNull(principal);

        authenticatedId =
                idmClient.authenticate(tenant.getName(), userPrincipal,
                        clearPasswd);
        Assert.assertNotNull(authenticatedId);

        // cleanup
        idmClient.deletePrincipal(tenant.getName(), userName);
    }

    @TestOrderAnnotation(order = 40)
    @Test
    public void addSolutionUserInNonSystemTenant() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        X509Certificate cert =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_STS_KEY_ALIAS));

        SolutionDetail detail = new SolutionDetail(cert);

        PrincipalId principal =
                idmClient.addSolutionUser(tenant.getName(),
                        _solution_user_name_non_sp, detail);
        Assert.assertNotNull(principal);
        ValidateMemberOfSolutionUsersGroup(idmClient, tenant.getName(),
                _solution_user_name_non_sp);

        // add a solution user w/o cert
        try
        {
            principal =
                    idmClient.addSolutionUser(tenant.getName(),
                            _solution_user_name_non_sp_no_cert,
                            new SolutionDetail(null));
        } catch (InvalidArgumentException e)
        {
            // succeed
            Assert.assertNotNull(e);
            return;
        }

        Assert.fail("addSolution user failed to throw InvalidArgumentException "
                + "adding a solution user without userCertificate");
    }

    private void disableEnableSolutionUser(CasIdmClient idmClient,
            Tenant tenant, PrincipalId principal, String userName)
                    throws Exception, IDMException
                    {
        // disable solution user in tenant
        boolean changed =
                idmClient.disableUserAccount(tenant.getName(), principal);
        Assert.assertEquals(true, changed);

        Set<SolutionUser> solutionusers =
                idmClient.findDisabledSolutionUsers(tenant.getName(), userName);
        Assert.assertNotNull(solutionusers);
        Assert.assertEquals(1, solutionusers.size());
        Assert.assertEquals(userName, solutionusers.iterator().next().getId()
                .getName());

        // enable solution user in tenant
        changed = idmClient.enableUserAccount(tenant.getName(), principal);
        Assert.assertEquals(true, changed);

        solutionusers =
                idmClient.findDisabledSolutionUsers(tenant.getName(), userName);
        Assert.assertEquals(true, solutionusers == null
                || solutionusers.size() == 0);

        solutionusers = idmClient.findSolutionUsers(tenant.getName(), userName, -1);
        Assert.assertNotNull(solutionusers);
        Assert.assertEquals(1, solutionusers.size());
        Assert.assertEquals(userName, solutionusers.iterator().next().getId()
                .getName());
                    }

    @TestOrderAnnotation(order = 41)
    @Test
    public void findSolutionUserInNonSystemTenant() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        String userName = _solution_user_name_non_sp;

        // find solution user in non-system tenant
        SolutionUser user =
                idmClient.findSolutionUser(tenant.getName(), userName);

        Assert.assertNotNull(user);

        // findUser by name, for solution user
        Principal solUser =
                idmClient.findUser(tenant.getName(), userName + "@"
                        + user.getId().getDomain());
        Assert.assertNotNull("expect to be able to find solution user", solUser);
        Assert.assertTrue(
                "solution user should be an instance of SolutionUser",
                solUser instanceof SolutionUser);

        PrincipalId principal = user.getId();

        Assert.assertNotNull(principal);

        Assert.assertEquals(principal.getName(), userName);

        // disable/enable solution user in non-system tenant
        disableEnableSolutionUser(idmClient, tenant, principal,
                _solution_user_name_non_sp);
    }

    @TestOrderAnnotation(order = 42)
    @Test
    public void addSolutionUserInSystemTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String systemTenantName =
                props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);

        Assert.assertNotNull(systemTenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, systemTenantName);

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        X509Certificate cert =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_STS_KEY_ALIAS));

        SolutionDetail detail = new SolutionDetail(cert);

        PrincipalId principal =
                idmClient.addSolutionUser(tenant.getName(),
                        _solution_user_name_sp, detail);
        Assert.assertNotNull(principal);
        ValidateMemberOfSolutionUsersGroup(idmClient, tenant.getName(),
                _solution_user_name_sp);

        // add a solution user w/o cert
        try
        {
            principal =
                    idmClient.addSolutionUser(tenant.getName(),
                            _solution_user_name_sp_no_cert, new SolutionDetail(
                                    null));
        } catch (InvalidArgumentException e)
        {
            // succeed
            Assert.assertNotNull(e);

            return;

        } catch (Exception e)
        {
            Assert.fail("addSolution user failed to throw InvalidArgumentException "
                    + "adding a solution user without userCertificate");
        }

        Assert.fail("should not be able to addSolutionUser");
    }

    @TestOrderAnnotation(order = 43)
    @Test
    public void findSystemTenantSolutionUser() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        // solution user in system tenant
        String userName = _solution_user_name_sp;

        SolutionUser user =
                idmClient.findSolutionUser(tenant.getName(), userName);

        Assert.assertNotNull(user);

        PrincipalId principal = user.getId();
        Assert.assertNotNull(principal);

        Assert.assertEquals(principal.getName(), userName);

        // disable/enable solution user in system tenant
        disableEnableSolutionUser(idmClient, tenant, principal,
                _solution_user_name_sp);
    }

    @TestOrderAnnotation(order = 44)
    @Test
    public void findSolutionUserByCertDN() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);

        X509Certificate cert =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_STS_KEY_ALIAS));

        SolutionUser user =
                idmClient.findSolutionUserByCertDn(tenant.getName(), cert
                        .getSubjectX500Principal().getName());

        Assert.assertNotNull(user);
    }


    @TestOrderAnnotation(order = 45)
    @Test
    public void testGetPersonAttributes() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        Collection<IIdentityStoreData> stores =
                idmClient.getProviders(tenantName,
                        EnumSet.of(DomainType.SYSTEM_DOMAIN));
        Assert.assertNotNull(stores);
        Assert.assertEquals(1, stores.size());

        PrincipalId principal =
                new PrincipalId("administrator", stores.iterator().next()
                        .getName());

        PersonUser user = idmClient.findPersonUser(tenant.getName(), principal);
        Assert.assertNotNull(user);

        Collection<Attribute> supportedAttributes =
                idmClient.getAttributeDefinitions(tenantName);


        Collection<AttributeValuePair> attributes =
                idmClient.getAttributeValues(tenantName, principal,
                        supportedAttributes);

        Assert.assertNotNull(attributes);

        AttributeValuePair isSolutionAttr = null;

        for (AttributeValuePair candidate : attributes)
        {
            Attribute attr = candidate.getAttrDefinition();

            if (attr.getName().equals(ATTR_NAME_IS_SOLUTION))
            {
                isSolutionAttr = candidate;
                break;
            }
        }

        Assert.assertNotNull(isSolutionAttr);

        List<String> values = isSolutionAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertEquals("false", values.get(0));
    }

    @TestOrderAnnotation(order = 46)
    @Ignore("LIKEWISE SAMDB should be setup for this to work")
    @Test
    public void testAuthenticateRoot() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        String tenantName = idmClient.getDefaultTenant();
        Assert.assertNotNull(tenantName);
        Assert.assertFalse(tenantName.length() == 0);

        idmClient.authenticate(tenantName, "root", "vmware");
    }

    @TestOrderAnnotation(order = 47)
    @Test
    public void testGetSolutionAttributes() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        String userName = _solution_user_name_non_sp;

        SolutionUser user =
                idmClient.findSolutionUser(tenant.getName(), userName);

        Assert.assertNotNull(user);

        PrincipalId principal = user.getId();

        Assert.assertNotNull(principal);

        Collection<Attribute> supportedAttributes =
                idmClient.getAttributeDefinitions(tenantName);

        Collection<AttributeValuePair> attributes =
                idmClient.getAttributeValues(tenantName, principal,
                        supportedAttributes);

        Assert.assertNotNull(attributes);

        AttributeValuePair isSolutionAttr = null;

        for (AttributeValuePair candidate : attributes)
        {
            Attribute attr = candidate.getAttrDefinition();

            if (attr.getName().equals(ATTR_NAME_IS_SOLUTION))
            {
                isSolutionAttr = candidate;
                break;
            }
        }

        Assert.assertNotNull(isSolutionAttr);

        List<String> values = isSolutionAttr.getValues();
        Assert.assertNotNull(values);
        Assert.assertTrue(values.size() == 1);
        Assert.assertEquals("true", values.get(0));
    }

    @TestOrderAnnotation(order = 48)
    @Test
    public void deleteSolutionUsers() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        String userName = _solution_user_name_non_sp;
        SolutionUser user =
                idmClient.findSolutionUser(tenant.getName(), userName);
        Assert.assertNotNull(user);

        idmClient.deletePrincipal(tenant.getName(), userName);
        user = idmClient.findSolutionUser(tenant.getName(), userName);
        Assert.assertNull(user);

        tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        Assert.assertNotNull(tenantName);
        tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        userName = _solution_user_name_sp;
        user = idmClient.findSolutionUser(tenant.getName(), userName);
        Assert.assertNotNull(user);

        idmClient.deletePrincipal(tenant.getName(), userName);
        user = idmClient.findSolutionUser(tenant.getName(), userName);
        Assert.assertNull(user);
    }

    @TestOrderAnnotation(order = 49)
    @Test
    public void testSetGetTenantProperties() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String testTenantName = props.getProperty(CFG_KEY_IDM_TENANT_2_NAME);
        Assert.assertNotNull(testTenantName);

        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, testTenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, testTenantName);
        Assert.assertNotNull(tenant);

        final long clockTolerance = 20000;
        idmClient.setClockTolerance(testTenantName, clockTolerance);
        Assert.assertEquals(clockTolerance,
                idmClient.getClockTolerance(testTenantName));

        final int delegationCount = 5;
        idmClient.setDelegationCount(testTenantName, delegationCount);
        Assert.assertEquals(delegationCount,
                idmClient.getDelegationCount(testTenantName));

        final int renewCount = 10;
        idmClient.setRenewCount(testTenantName, renewCount);
        Assert.assertEquals(renewCount, idmClient.getRenewCount(testTenantName));

        final long maxhoktoken = 30000;
        idmClient.setMaximumHoKTokenLifetime(testTenantName, maxhoktoken);
        Assert.assertEquals(maxhoktoken,
                idmClient.getMaximumHoKTokenLifetime(testTenantName));

        final long maxbeartoken = 40000;
        idmClient.setMaximumBearerTokenLifetime(testTenantName, maxbeartoken);
        Assert.assertEquals(maxbeartoken,
                idmClient.getMaximumBearerTokenLifetime(testTenantName));

        final long maxBearerRefreshTokenLifeTime = 40000;
        idmClient.setMaximumBearerRefreshTokenLifetime(testTenantName, maxBearerRefreshTokenLifeTime);
        Assert.assertEquals(maxbeartoken,
                idmClient.getMaximumBearerRefreshTokenLifetime(testTenantName));

        final long maxHoKRefreshTokenLifeTime = 30000;
        idmClient.setMaximumHoKRefreshTokenLifetime(testTenantName, maxHoKRefreshTokenLifeTime);
        Assert.assertEquals(maxhoktoken,
                idmClient.getMaximumHoKRefreshTokenLifetime(testTenantName));

        final String signatureAlgorithm =
                "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
        idmClient.setTenantSignatureAlgorithm(testTenantName,
                signatureAlgorithm);
        Assert.assertEquals(signatureAlgorithm,
                idmClient.getTenantSignatureAlgorithm(testTenantName));

        final String brandName =
                "Acme single sign-on";
        idmClient.setBrandName(testTenantName,
                brandName);
        String retrievedBrand = idmClient.getBrandName(testTenantName);
        Assert.assertEquals(brandName, retrievedBrand);

        String testLogonBannerTitle = "Acme logon banner";
        String testLogonBannerContent = "This is a disclaimer.";
        idmClient.setLogonBannerTitle(testTenantName, testLogonBannerTitle);
        idmClient.setLogonBannerContent(testTenantName, testLogonBannerContent);
        idmClient.setLogonBannerCheckboxFlag(testTenantName, true);
        String retrievedLogonBannerContent = idmClient.getLogonBannerContent(testTenantName);
        String retrievedLogonBannerTitle = idmClient.getLogonBannerTitle(testTenantName);
        Assert.assertEquals(retrievedLogonBannerTitle, testLogonBannerTitle);
        Assert.assertEquals(retrievedLogonBannerContent, testLogonBannerContent);
        Assert.assertTrue(idmClient.getLogonBannerCheckboxFlag(testTenantName));

        final String entityId = "EntityId";
        idmClient.setEntityID(testTenantName, entityId);
        Assert.assertEquals(entityId, idmClient.getEntityID(testTenantName));

        final String alias = "vsphere.local";
        idmClient.setLocalIDPAlias(testTenantName, alias);
        Assert.assertEquals(alias, idmClient.getLocalIDPAlias(testTenantName));
        idmClient.setLocalIDPAlias(testTenantName, null);
        Assert.assertNull(idmClient.getLocalIDPAlias(testTenantName));

        Assert.assertTrue(idmClient.isTenantIDPSelectionEnabled(testTenantName));
        boolean enableIdpSelection = false;
        idmClient.setTenantIDPSelectionEnabled(testTenantName, enableIdpSelection);
        Assert.assertFalse(idmClient.isTenantIDPSelectionEnabled(testTenantName));

        PasswordExpiration expiration =
                new PasswordExpiration(true, "a@abc.com", "Password Expired",
                        new int[] { 1, 3, 5 });

        idmClient.updatePasswordExpirationConfiguration(testTenantName,
                expiration);
        PasswordExpiration expiration1 =
                idmClient.getPasswordExpirationConfiguration(testTenantName);

        Assert.assertNotNull(expiration1);
        Assert.assertEquals(expiration.isEmailNotificationEnabled(),
                expiration1.isEmailNotificationEnabled());
        Assert.assertEquals(expiration.getEmailFrom(),
                expiration1.getEmailFrom());
        Assert.assertEquals(expiration.getEmailSubject(),
                expiration1.getEmailSubject());

        int[] days = expiration1.getNotificationDays();
        Assert.assertNotNull(days);
        Assert.assertEquals(expiration.getNotificationDays().length,
                days.length);
        int i = 0;
        for (int day : expiration.getNotificationDays())
        {
            Assert.assertEquals(day, days[i]);
            i++;
        }

        // idm cert
        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);
        X509Certificate cert =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_STS_KEY_ALIAS));

        // add cert of type 'LDAP_TRUSTED_CERT'
        idmClient.addCertificate(testTenantName, cert,
                CertificateType.LDAP_TRUSTED_CERT);

        Collection<Certificate> idmCertificates =
                idmClient.getAllCertificates(testTenantName,
                        CertificateType.LDAP_TRUSTED_CERT);
        Assert.assertNotNull(idmCertificates);
        Assert.assertEquals(1, idmCertificates.size());

        X509Certificate cert1 =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_STS_KEY_ALIAS1));

        // add cert of type 'STS_TRUSTED_CERT'
        idmClient.addCertificate(testTenantName, cert1,
                CertificateType.STS_TRUST_CERT);

        idmCertificates =
                idmClient.getAllCertificates(testTenantName,
                        CertificateType.STS_TRUST_CERT);
        Assert.assertNotNull(idmCertificates);
        Assert.assertEquals(1, idmCertificates.size());

        // delete cert of type 'LDAP_TRUSTED_CERT'
        idmClient.deleteCertificate(testTenantName,
                CertificateUtil.generateFingerprint(cert),
                CertificateType.LDAP_TRUSTED_CERT);
        idmCertificates =
                idmClient.getAllCertificates(testTenantName,
                        CertificateType.LDAP_TRUSTED_CERT);
        Assert.assertEquals(true,
                ((idmCertificates == null) || (idmCertificates.size() == 0)));

        // should still have 'sts' trust cert
        idmCertificates =
                idmClient.getAllCertificates(testTenantName, CertificateType.STS_TRUST_CERT);
        Assert.assertNotNull(idmCertificates);
        Assert.assertEquals(1, idmCertificates.size());

        // delete cert of type 'STS_TRUSTED_CERT'
        idmClient.deleteCertificate(testTenantName,
                CertificateUtil.generateFingerprint(cert1),
                CertificateType.STS_TRUST_CERT);
        idmCertificates =
                idmClient.getAllCertificates(testTenantName, CertificateType.STS_TRUST_CERT);
        Assert.assertEquals(true,
                ((idmCertificates == null) || (idmCertificates.size() == 0)));

        // default provider
        final String adProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(adProviderName);

        final String adProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_ALIAS);
        Assert.assertNotNull(adProviderAlias);

        IdentityStoreData adStore = IdmClientTestUtil.prepareADIdentityStore(idmClient, true);
        IIdentityStoreData store =
                idmClient.getProvider(testTenantName, adProviderName);

        // check before create
        if (store == null)
        {
            idmClient.addProvider(testTenantName, adStore);
        }

        // set using providerName
        idmClient.setDefaultProviders(testTenantName,
                Arrays.asList(new String[] { adProviderName }));

        Collection<String> providers =
                idmClient.getDefaultProviders(testTenantName);
        Assert.assertNotNull(providers);
        Assert.assertEquals(1, providers.size());
        Assert.assertEquals(adProviderName, providers.iterator().next());

        idmClient.deleteProvider(testTenantName, adProviderName);
        providers = idmClient.getDefaultProviders(testTenantName);
        Assert.assertTrue(((providers == null) || (providers.size() == 0)));

        store = idmClient.getProvider(testTenantName, adProviderName);

        // check before create
        if (store == null)
        {
            idmClient.addProvider(testTenantName, adStore);
        }

        // set using provider Alias Name
        idmClient.setDefaultProviders(testTenantName,
                Arrays.asList(new String[] { adProviderAlias }));
        providers = idmClient.getDefaultProviders(testTenantName);
        Assert.assertNotNull(providers);
        Assert.assertEquals(1, providers.size());
        Assert.assertEquals(adProviderAlias, providers.iterator().next());

        idmClient.setDefaultProviders(testTenantName, null);
        providers = idmClient.getDefaultProviders(testTenantName);
        Assert.assertTrue(((providers == null) || (providers.size() == 0)));

        idmClient.deleteProvider(testTenantName, adProviderAlias);
        providers = idmClient.getDefaultProviders(testTenantName);
        Assert.assertTrue(((providers == null) || (providers.size() == 0)));

        // default tenant
        String defaultTenant = idmClient.getDefaultTenant();

        idmClient.setDefaultTenant(testTenantName);
        Assert.assertEquals(testTenantName, idmClient.getDefaultTenant());

        idmClient.deleteTenant(testTenantName);
        String defTenant = idmClient.getDefaultTenant();
        Assert.assertTrue((defTenant == null) || (defTenant.isEmpty()));

        if ((defaultTenant != testTenantName) && (defaultTenant != null)
                && (defaultTenant.isEmpty() == false))
        {
            idmClient.setDefaultTenant(defaultTenant);
            Assert.assertEquals(defaultTenant, idmClient.getDefaultTenant());
        }
    }

    @TestOrderAnnotation(order = 50)
    @Test
    public void testListAllTenants() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        Properties props = getTestProperties();

        String nonsystemTenant1 = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        String nonsystemTenant2 = props.getProperty(CFG_KEY_IDM_TENANT_2_NAME);
        String nonsystemTenant3 = _expTenantName;

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, nonsystemTenant1);
        Assert.assertNotNull(tenant);
        tenant = IdmClientTestUtil.ensureTenantExists(idmClient, nonsystemTenant2);
        Assert.assertNotNull(tenant);
        tenant = IdmClientTestUtil.ensureTenantExists(idmClient, nonsystemTenant3);
        Assert.assertNotNull(tenant);

        // Retrieve all existing tenants
        // We should have 1 system tenant and 3 non-system tenant
        Collection<String> allTenants = idmClient.getAllTenants();
        Assert.assertEquals(4, allTenants.size());
        Assert.assertTrue(allTenants.contains(props
                .getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME)));
        Assert.assertTrue(allTenants.contains(nonsystemTenant1));
        Assert.assertTrue(allTenants.contains(nonsystemTenant2));
        Assert.assertTrue(allTenants.contains(nonsystemTenant3));
    }

    @TestOrderAnnotation(order = 51)
    @Test
    public void testDeleteIdentityStore() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String storeName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(storeName);

        IIdentityStoreData store = idmClient.getProvider(tenantName, storeName);
        Assert.assertNotNull(store);

        idmClient.deleteProvider(tenantName, storeName);

        store = idmClient.getProvider(tenantName, storeName);
        Assert.assertNull(store);

        final String olProviderName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_DOMAIN_NAME);
        Assert.assertNotNull(olProviderName);
        final String olProviderAlias =
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_ALIAS);
        Assert.assertNotNull(olProviderAlias);

        store = idmClient.getProvider(tenantName, olProviderName);
        if (store != null)
        {
            idmClient.deleteProvider(tenantName, olProviderName);
            store = idmClient.getProvider(tenantName, olProviderName);
            Assert.assertNull(store);
        }

        ensureOpenLdapIdentityStoreExistForTenant(tenantName, true);

        store = idmClient.getProvider(tenantName, olProviderName);
        Assert.assertNotNull(store);
        Assert.assertEquals(olProviderName, store.getName());
        Assert.assertNotNull(store.getExtendedIdentityStoreData());
        Assert.assertNotNull(olProviderAlias, store
                .getExtendedIdentityStoreData().getAlias());

        idmClient.deleteProvider(tenantName, olProviderAlias);
        store = idmClient.getProvider(tenantName, olProviderName);
        Assert.assertNull(store);
        store = idmClient.getProvider(tenantName, olProviderAlias);
        Assert.assertNull(store);
    }

    @TestOrderAnnotation(order = 52)
    @Test
    public void testDeleteRelyingParty() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        String rpName = "coke";

        //test deleting rp that exist for the tenant
        RelyingParty rp = idmClient.getRelyingParty(tenantName, rpName);
        Assert.assertNotNull(rp);
        idmClient.deleteRelyingParty(tenantName, rpName);

        rp = idmClient.getRelyingParty(tenantName, rpName);
        Assert.assertNull(rp);

        //test non-exist relying party
        try
        {
            idmClient.deleteRelyingParty(tenantName, rpName);
            Assert.fail("fail to throw NoSuchRelyingPartyException");
        } catch (NoSuchRelyingPartyException ex)
        {
            //succeeded
        }
    }

    @TestOrderAnnotation(order = 53)
    @Test(expected = NoSuchTenantException.class)
    public void testDeleteTenant() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        Properties props = getTestProperties();

        // clean up non-system tenants
        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);
        IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        idmClient.deleteTenant(tenantName);

        tenantName = props.getProperty(CFG_KEY_IDM_TENANT_2_NAME);
        Assert.assertNotNull(tenantName);
        IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
        idmClient.deleteTenant(tenantName);

        IdmClientTestUtil.ensureTenantExists(idmClient, _expTenantName);
        idmClient.deleteTenant(_expTenantName);
        // verify tenant no long exists
        // this will throw the expected exception
        idmClient.getTenant(_expTenantName);
    }

    @TestOrderAnnotation(order = 54)
    @Test
    public void testProbeConnectivity() throws Exception
    {
        Properties props = getTestProperties();

        CasIdmClient client = getIdmClient();

        client.probeProviderConnectivity(
                props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_TENANTNAME),
                props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI),
                AuthenticationType.PASSWORD,
                props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME),
                props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_PASSWORD),
                null);

        //probe openldap ssl connectivity
        List<X509Certificate> openldapsCerts = null;
        X509Certificate openLdapCertificate =  getCertificate(props.getProperty(CFG_KEY_IDM_TENANT_1_OL_SSL_CERT_ALIAS));
        if (openLdapCertificate != null)
            openldapsCerts = java.util.Collections.singletonList(openLdapCertificate);

        client.probeProviderConnectivity(
                props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_TENANTNAME),
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_SSL_HOST_NAME),
                AuthenticationType.PASSWORD,
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_BIND_DN),
                props.getProperty(CFG_KEY_IDM_TENANT_1_OL_PROVIDER_PASSWORD),
                openldapsCerts);
    }

    @TestOrderAnnotation(order = 55)
    @Test
    public void testProbeNonExistingConnectivity() throws Exception
    {
        Properties props = getTestProperties();

        try
        {
            CasIdmClient client = getIdmClient();

            client.probeProviderConnectivity(
                    props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_TENANTNAME),
                    props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_PROVIDERURI),
                    AuthenticationType.PASSWORD,
                    props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_USERNAME),
                    props.getProperty(CFG_KEY_IDM_PROBECONNECTIVITY_BADPASSWORD),
                    null);
        } catch (IDMLoginException e)
        {
            return;
        }
        Assert.fail("should not be able to connect");
    }

    @TestOrderAnnotation(order = 56)
    @Test
    public void testI18N() throws Exception
    {
        Properties props = getTestProperties();
        CasIdmClient client = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String description = props.getProperty(CFG_KEY_I18N_DESCRIPTION_COMMON);
        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        SearchCriteria oneI18NCharMatched =
                new SearchCriteria(props.getProperty(CFG_KEY_I18N_USERNAME_1)
                        .substring(0, 1),
                        props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME));
        Set<PersonUser> i18NUsers =
                client.findPersonUsers(tenantName, oneI18NCharMatched, -1);
        if (i18NUsers.size() > 0)
        {
            for (String userName : Arrays.asList(
                    props.getProperty(CFG_KEY_I18N_USERNAME_1),
                    props.getProperty(CFG_KEY_I18N_USERNAME_2)))
            {
                try
                {
                    client.deletePrincipal(tenantName, userName);
                } catch (com.vmware.identity.idm.InvalidPrincipalException e)
                {
                    //continue, account does not exist so no needs to clean up
                }
            }
        }

        // create two I18N accounts on Lotus
        String userName1 = props.getProperty(CFG_KEY_I18N_USERNAME_1);
        char[] password1 =
                props.getProperty(CFG_KEY_I18N_PASSWORD_1).toCharArray();
        String firstName1 = props.getProperty(CFG_KEY_I18N_FIRSTNAME_1);
        String lastName1 = props.getProperty(CFG_KEY_I18N_LASTNAME_1);

        PersonDetail detail1 =
                new PersonDetail.Builder().firstName(firstName1)
                .lastName(lastName1).description(description).build();
        PrincipalId result1 =
                client.addPersonUser(tenantName, userName1, detail1, password1);
        client.addUserToGroup(tenantName, result1, USERS_GROUP);

        Assert.assertNotNull(result1);
        Set<Group> groups1 = client.findDirectParentGroups(tenantName, result1);
        Assert.assertTrue(groups1.size() == 2);

        String userName2 = props.getProperty(CFG_KEY_I18N_USERNAME_2);
        char[] password2 =
                props.getProperty(CFG_KEY_I18N_PASSWORD_2).toCharArray();
        String firstName2 = props.getProperty(CFG_KEY_I18N_FIRSTNAME_2);
        String lastName2 = props.getProperty(CFG_KEY_I18N_LASTNAME_2);

        PersonDetail detail2 =
                new PersonDetail.Builder().firstName(firstName2)
                .lastName(lastName2).description(description).build();
        PrincipalId result2 =
                client.addPersonUser(tenantName, userName2, detail2, password2);
        client.addUserToGroup(tenantName, result2, USERS_GROUP);

        Assert.assertNotNull(result2);
        Set<Group> groups2 = client.findDirectParentGroups(tenantName, result2);
        Assert.assertTrue(groups2.size() == 2);

        String id =
                String.format("%s@%s", result1.getName(), result1.getDomain());
        String pwd = props.getProperty(CFG_KEY_I18N_PASSWORD_1);
        client.authenticate(tenantName, id, pwd);

        // and read them back
        Set<PersonUser> i18NUsersCreated =
                client.findPersonUsers(tenantName, oneI18NCharMatched, -1);
        Assert.assertEquals(i18NUsersCreated.size(), 2);
    }

    @TestOrderAnnotation(order = 57)
    @Test
    public void testGetAttributes() throws Exception
    {
        Properties props = getTestProperties();
        CasIdmClient client = getIdmClient();
        String tenant1 = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        IdmClientTestUtil.ensureTenantExists(client, tenant1);

        ensureADIdentityStoreExistForTenant(tenant1);
        ensureOpenLdapIdentityStoreExistForTenant(tenant1, true);

        String nonExistentName =
                props.getProperty(CFG_KEY_ALL_TENANTS_NON_EXISTENT_PRINCIPAL_NAME);
        List<Attribute> attrs =
                Arrays.asList(new Attribute(SAMLATTR_GIVEN_NAME),
                        new Attribute(SAMLATTR_SUR_NAME), new Attribute(
                                SAMLATTR_UPN));

        for (IIdentityStoreData storeData : client.getProviders(props
                .getProperty(CFG_KEY_IDM_TENANT_1_NAME)))
        {

            String domainName = storeData.getName();

            //first get attributes for Administrator user to make sure it works
            Collection<AttributeValuePair> adminAttrs =
                    client.getAttributeValues(tenant1, new PrincipalId(
                            "Administrator", domainName), attrs);
            Assert.assertEquals(adminAttrs.size(), 3);

            //now try to get attribute for nonexistent user and should get an exception
            try
            {
                client.getAttributeValues(tenant1, new PrincipalId(
                        nonExistentName, domainName), attrs);
            } catch (InvalidPrincipalException ipe)
            {
                Assert.assertNotNull(ipe);
                continue;
            } catch (Exception e)
            {
                Assert.fail("should get an InvalidPrincipalException and not reach here");
            }
            Assert.fail("should get an InvalidPrincipalException and not reach here");
        }
        // clean up tenant
        IdmClientTestUtil.ensureTenantDoesNotExist(client, tenant1);
    }

    @TestOrderAnnotation(order = 58)
    @Test
    public void testExternalIDPConfigCreateReadOps() throws Exception
    {
        CasIdmClient client = getIdmClient();
        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        String entityId_1 = props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID);
        String alias = "vsphere.local";

        List<String> nameIDFormats =
                Arrays.asList(props.getProperty(
                        CFG_KEY_EXTERNAL_IDP_1_NAME_ID_FORMATS).split(","));

        List<ServiceEndpoint> ssoServices = buildSSOServicesIdp1(props);
        List<ServiceEndpoint> sloServices = buildSLOServicesIdp1(props);
        List<X509Certificate> x509Certs = getSTSKeyCertificates();

        Map<TokenClaimAttribute,List<String>> mappings_1 = new HashMap<>();
        List<String> groupList1 = new ArrayList<>();
        groupList1.add("ExternalIDPUsers");
        mappings_1.put(new TokenClaimAttribute("http://schemas.xmlsoap.org/claims/UPN", "JitUser"), groupList1);

        Map<TokenClaimAttribute,List<String>> mappings_2 = new HashMap<>();
        List<String> groupList2 = new ArrayList<>();
        groupList2.add("SolutionUsers");
        List<String> groupList3 = new ArrayList<>();
        groupList3.add("ExternalIDPUsers");
        groupList3.add("SolutionUsers");
        mappings_2.put(new TokenClaimAttribute("http://schemas.xmlsoap.org/claims/group", "admin"), groupList2);
        mappings_2.put(new TokenClaimAttribute("http://schemas.xmlsoap.org/claims/group", "user"), groupList3);

        // setup two external IDP configurations with unique entityIDs but everything else the same.
        IDPConfig idpConfig_1 = new IDPConfig(entityId_1);
        idpConfig_1.setAlias(alias);
        idpConfig_1.setNameIDFormats(nameIDFormats);
        idpConfig_1.setSsoServices(ssoServices);
        idpConfig_1.setSloServices(sloServices);
        idpConfig_1.setSigningCertificateChain(x509Certs);
        idpConfig_1.setJitAttribute(true);
        idpConfig_1.setTokenClaimGroupMappings(mappings_1);

        String entityId_2 = props.getProperty(CFG_KEY_EXTERNAL_IDP_2_ENTITY_ID);
        IDPConfig idpConfig_2 = new IDPConfig(entityId_2);
        idpConfig_2.setAlias(alias);
        idpConfig_2.setNameIDFormats(nameIDFormats);
        idpConfig_2.setSsoServices(ssoServices);
        idpConfig_2.setSloServices(sloServices);
        idpConfig_2.setSigningCertificateChain(x509Certs);
        idpConfig_2.setJitAttribute(false);
        idpConfig_2.setTokenClaimGroupMappings(mappings_2);

        client.setExternalIdpConfig(tenantName, idpConfig_1);
        client.setExternalIdpConfig(tenantName, idpConfig_2);

        // check the result of query all
        Collection<IDPConfig> idpConfigs =
                client.getAllExternalIdpConfig(tenantName);
        Assert.assertEquals(idpConfigs.size(), 2);

        // two configurations should have unique ids
        Assert.assertEquals(idpConfig_1.getEntityID(), client
                .getExternalIdpConfigForTenant(tenantName, entityId_1)
                .getEntityID());
        Assert.assertEquals(idpConfig_2.getEntityID(), client
                .getExternalIdpConfigForTenant(tenantName, entityId_2)
                .getEntityID());

        Assert.assertEquals(idpConfig_1.getJitAttribute(), client
                .getExternalIdpConfigForTenant(tenantName, entityId_1)
                .getJitAttribute());
        Assert.assertEquals(idpConfig_2.getJitAttribute(), client
                .getExternalIdpConfigForTenant(tenantName, entityId_2)
                .getJitAttribute());

        Assert.assertEquals(1, client.getExternalIdpConfigForTenant(tenantName, entityId_1).getTokenClaimGroupMappings().size());
        Assert.assertEquals(mappings_1, client.getExternalIdpConfigForTenant(tenantName, entityId_1).getTokenClaimGroupMappings());

        Assert.assertEquals(2, client.getExternalIdpConfigForTenant(tenantName, entityId_2).getTokenClaimGroupMappings().size());
        Assert.assertEquals(mappings_2,  client.getExternalIdpConfigForTenant(tenantName, entityId_2).getTokenClaimGroupMappings());

        // and the rest should be the same
        for (IDPConfig config : idpConfigs)
        {
            Assert.assertEquals(config.getAlias(), null); // Alias is null by default
            Assert.assertEquals(config.getAlias(), alias);
            Assert.assertEquals(config.getNameIDFormats(), nameIDFormats);
            Assert.assertEquals(config.getSigningCertificateChain(), x509Certs);
            Assert.assertEquals(config.getSsoServices(), ssoServices);
            Assert.assertEquals(config.getSloServices(), sloServices);
        }

        // check result of query by url
        Collection<IDPConfig> idpConfigsBySSOUrl =
                client.getExternalIdpConfigForTenantByUrl(tenantName, props
                        .getProperty(CFG_KEY_EXTERNAL_IDP_1_SSO_LOCATION_1));
        Assert.assertEquals(idpConfigsBySSOUrl.size(), 2);
        Assert.assertEquals(idpConfig_1.getEntityID(), idpConfigsBySSOUrl
                .iterator().next().getEntityID());

        Collection<IDPConfig> idpConfigsBySLOUrl =
                client.getExternalIdpConfigForTenantByUrl(tenantName, props
                        .getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_LOCATION_1));
        Assert.assertEquals(idpConfigsBySLOUrl.size(), 0);
    }

    @TestOrderAnnotation(order = 59)
    @Test
    public void testExternalIDPConfigJITOps() throws Exception {
        CasIdmClient client = getIdmClient();
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        String entityId_1 = props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID);
        String entityId_2 = props.getProperty(CFG_KEY_EXTERNAL_IDP_2_ENTITY_ID);

        String testJitUserName = "TestJitUser";
        // add JIT user to idpConfig_1 which has JIT enabled
        PrincipalId id = client.addJitUser(tenantName, testJitUserName,
                new PersonDetail.Builder()
                .description(
                        "Person created to test adding/deleting JIT user")
                        .build(), entityId_1, "testJitUser@abc.com");
        PrincipalId retrievedId = client.findActiveUserInSystemDomain(tenantName, "vmwSTSExternalIdpUserId", "testJitUser@abc.com");
        Assert.assertEquals(id, retrievedId);

        // try add JIT user to idpConfig which does not have JIT enabled
        try
        {
            client.addJitUser(tenantName, testJitUserName,
                    new PersonDetail.Builder()
                    .description(
                            "Person created to test adding/deleting JIT user")
                            .build(), entityId_2, "testJitUser@abc.com");
            Assert.fail("Should have thrown an InvalidPrincipalException.");
        } catch (Exception e) {
            Assert.assertTrue(e instanceof InvalidPrincipalException);
            Assert.assertEquals(String.format("User %s cannot be added "
                + "since JIT is not enabled for external IDP with entityID %s",
                testJitUserName, entityId_2), e.getMessage());
        }
    }

    @TestOrderAnnotation(order = 60)
    @Test
    public void testExternalIDPConfigDeleteOps() throws Exception {
        CasIdmClient client = getIdmClient();
        Properties props = getTestProperties();
        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        String entityId_1 = props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID);
        String entityId_2 = props.getProperty(CFG_KEY_EXTERNAL_IDP_2_ENTITY_ID);

        // check removal
        client.removeExternalIdpConfig(tenantName, entityId_1, true);
        client.removeExternalIdpConfig(tenantName, entityId_2);

        boolean deleteNonExistantConfigSucceeded = true;
        try
        {
            client.removeExternalIdpConfig(tenantName, entityId_2);
        } catch (NoSuchExternalIdpConfigException e)
        {
            deleteNonExistantConfigSucceeded = false;
        } finally
        {
            Assert.assertEquals(false, deleteNonExistantConfigSucceeded);
        }
    }

    @TestOrderAnnotation(order = 61)
    @Test
    public void testExternalIDPConfigUpdateOp() throws Exception
    {
        CasIdmClient client = getIdmClient();
        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        List<X509Certificate> x509Certs = buildExternalIdpSTSKeyCertificates();
        String entityId_1 = props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID);

        // create original IDP configurations
        IDPConfig idpConfig = new IDPConfig(entityId_1);
        idpConfig.setNameIDFormats(Arrays.asList(props.getProperty(
                CFG_KEY_EXTERNAL_IDP_1_NAME_ID_FORMATS).split(",")));
        idpConfig.setSsoServices(buildSSOServicesIdp1(props));
        idpConfig.setSloServices(buildSLOServicesIdp1(props));
        idpConfig.setSigningCertificateChain(x509Certs);

        client.setExternalIdpConfig(tenantName, idpConfig);

        //update the original configuration
        idpConfig.setNameIDFormats(Arrays.asList(props.getProperty(
                CFG_KEY_EXTERNAL_IDP_2_NAME_ID_FORMATS).split(",")));
        idpConfig.setSsoServices(buildSSOServicesIdp2(props));
        idpConfig.setSloServices(buildSLOServicesIdp2(props));

        client.setExternalIdpConfig(tenantName, idpConfig);

        //check the configuration is updated
        IDPConfig actual =
                client.getExternalIdpConfigForTenant(tenantName, entityId_1);
        Assert.assertEquals(idpConfig.getEntityID(), actual.getEntityID());
        Assert.assertEquals(idpConfig.getAlias(), actual.getAlias());
        Assert.assertEquals(idpConfig.getNameIDFormats(),
                actual.getNameIDFormats());
        Assert.assertEquals(idpConfig.getSsoServices(), actual.getSsoServices());
        Assert.assertEquals(idpConfig.getSloServices(), actual.getSloServices());
        Assert.assertEquals(idpConfig.getSigningCertificateChain(),
                actual.getSigningCertificateChain());

        client.removeExternalIdpConfig(tenantName, entityId_1);
    }

    @TestOrderAnnotation(order = 62)
    @Test
    public void testExternalIDPNoSuchTenant() throws Exception
    {
        final CasIdmClient client = getIdmClient();
        final Properties props = getTestProperties();
        final String invalidTenantName =
                props.getProperty(CFG_KEY_IDM_TENANT_3_NAME);

        ensureNoSuchTenantExceptionOccurred(new Callable<Void>() {
            @Override
            public Void call() throws Exception
            {
                IDPConfig sampleConfig =
                        new IDPConfig(
                                props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID));
                List<String> nameIDFormats =
                        Arrays.asList(props.getProperty(
                                CFG_KEY_EXTERNAL_IDP_1_NAME_ID_FORMATS).split(
                                        ","));


                List<ServiceEndpoint> ssoServices =
                        buildSSOServicesIdp1(props);
                List<ServiceEndpoint> sloServices =
                        buildSLOServicesIdp1(props);
                List<X509Certificate> x509Certs = getSTSKeyCertificates();

                // setup two external IDP configurations with unique entityIDs but everything else the same.
                sampleConfig.setNameIDFormats(nameIDFormats);
                sampleConfig.setSsoServices(ssoServices);
                sampleConfig.setSloServices(sloServices);
                sampleConfig.setSigningCertificateChain(x509Certs);
                client.setExternalIdpConfig(invalidTenantName, sampleConfig);
                return null;
            }
        });

        ensureNoSuchTenantExceptionOccurred(new Callable<Void>() {
            @Override
            public Void call() throws Exception
            {
                client.removeExternalIdpConfig(invalidTenantName,
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID));
                return null;
            }
        });
        ensureNoSuchTenantExceptionOccurred(new Callable<Void>() {
            @Override
            public Void call() throws Exception
            {
                client.getExternalIdpConfigForTenant(invalidTenantName,
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_ENTITY_ID));
                return null;
            }
        });

        ensureNoSuchTenantExceptionOccurred(new Callable<Void>() {
            @Override
            public Void call() throws Exception
            {
                client.getAllExternalIdpConfig(invalidTenantName);
                return null;
            }
        });

        ensureNoSuchTenantExceptionOccurred(new Callable<Void>() {
            @Override
            public Void call() throws Exception
            {
                client.getExternalIdpConfigForTenantByUrl(
                        invalidTenantName,
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_SSO_LOCATION_1));
                return null;
            }
        });

    }

    @Test
    public void testFindUsersGroupsByNameInSystemProvider() throws Exception
    {
        Properties props = getTestProperties();
        CasIdmClient client = getIdmClient();

        String tenantName = props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME);
        String description = props.getProperty(CFG_KEY_I18N_DESCRIPTION_COMMON);
        IdmClientTestUtil.ensureTenantExists(client, tenantName);

        SearchCriteria adminUsers =
                new SearchCriteria("admin",
                        props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME));
        Set<PersonUser> users =
                client.findPersonUsersByName(tenantName, adminUsers, -1);
        Assert.assertEquals(1, users.size());

        SearchCriteria adminGroups =
                new SearchCriteria("admin",
                        props.getProperty(CFG_KEY_IDM_SYSTEM_TENANT_NAME));
        Set<Group> groups =
                client.findGroupsByName(tenantName, adminGroups, -1);
        Assert.assertEquals(1, groups.size());
    }

    @Test
    public void testAddTenantWithAdminCreds() throws Exception, IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);
        Assert.assertNotNull(tenantName);

        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);

        String adminAccountName = props.getProperty(CFG_KEY_IDM_TENANT_1_ADMIN_ACCOUNT_NAME);
        String adminPwd = props.getProperty(CFG_KEY_IDM_TENANT_1_ADMIN_PWD);
        Assert.assertNotNull(CFG_KEY_IDM_TENANT_1_ADMIN_ACCOUNT_NAME + " config property must not be null", adminAccountName);
        Assert.assertNotNull(CFG_KEY_IDM_TENANT_1_ADMIN_PWD + "config property must not be null", adminPwd);

        Tenant tenantToCreate = new Tenant(tenantName);

        idmClient.addTenant(tenantToCreate, adminAccountName, adminPwd.toCharArray());

        Tenant tenant = idmClient.getTenant(tenantName);
        Assert.assertNotNull(tenant);
        Assert.assertEquals(tenantName, tenant.getName());

        String userPrincipal = adminAccountName + "@" + tenantName;
        idmClient.authenticate(tenantName, userPrincipal, adminPwd);

        validateAdminInAdminGroup(idmClient, tenantName, userPrincipal);

        // update tenant password, ensure can still auth, enum users/groups still
        String newTenantPassword = adminPwd + "1";
        idmClient.changeUserPassword(tenantName, adminAccountName, adminPwd.toCharArray(), newTenantPassword.toCharArray());
        idmClient.authenticate(tenantName, userPrincipal, newTenantPassword);
        try
        {
            idmClient.authenticate(tenantName, userPrincipal, adminPwd);
            Assert.fail("Admin auth with original pwd should fail.");
        }
        catch(IDMLoginException ex)
        {
            //expected
        }

        validateAdminInAdminGroup(idmClient, tenantName, userPrincipal);

        newTenantPassword = adminPwd + "2";
        idmClient.setUserPassword(tenantName, adminAccountName, newTenantPassword.toCharArray());
        idmClient.authenticate(tenantName, userPrincipal, newTenantPassword);
        try
        {
            idmClient.authenticate(tenantName, userPrincipal, adminPwd);
            Assert.fail("Admin auth with original pwd should fail.");
        }
        catch(IDMLoginException ex)
        {
            //expected
        }

        validateAdminInAdminGroup(idmClient, tenantName, userPrincipal);

        try
        {
            idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
        } catch (DuplicateTenantException ex)
        {
            // cleanup
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
            return;
        }

        Assert.fail("Should not reach here, proper exception should be thrown");
    }


    @Test
    public void testGetIdmAuthStats() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();

        Properties props = getTestProperties();

        String tenantName = props.getProperty(CFG_KEY_IDM_TENANT_1_NAME);

        Assert.assertNotNull(tenantName);

        Tenant tenant = IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);

        IdmClientTestUtil.ensureADIdentityStoreExistForTenant(idmClient, tenantName);

        Assert.assertNotNull(tenant);

        final String adUserName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_ID);

        Assert.assertNotNull(adUserName);

        final String password =
                props.getProperty(CFG_KEY_IDM_TENANT_1_ADPROVIDER_USER_LOOKUP_PASSWORD);

        Assert.assertNotNull(password);

        final String adDomainName =
                props.getProperty(CFG_KEY_IDM_TENANT_1_AD_PROVIDER_DOMAIN_NAME);

        Assert.assertNotNull(adDomainName);

        List<IIdmAuthStat> stats1 = idmClient.getIdmAuthStats(tenantName);
        idmClient.authenticate(tenantName, adUserName + "@" + adDomainName, password);
        List<IIdmAuthStat> stats2 = idmClient.getIdmAuthStats(tenantName);
        Assert.assertTrue(stats2.size() > stats1.size() || ( (stats1.size() == stats2.size()) && ( stats1.size() == 10) ) );

        idmClient.getAttributeValues(tenantName, new PrincipalId(adUserName, adDomainName),
                idmClient.getAttributeDefinitions(tenantName));
        List<IIdmAuthStat> stats3 = idmClient.getIdmAuthStats(tenantName);
        Assert.assertTrue(stats3.size() > stats2.size() || ( (stats2.size() == stats3.size()) && ( stats2.size() == 10) ) );

        IdmClientTestUtil.ensureTenantExists(idmClient, tenantName);
    }

    @Test
    public void testRSAAgentConfigs() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        String tenantName = "TestTenant1";
        Tenant tenantToCreate = new Tenant(tenantName);
        try {
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
            idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
        } catch (Exception ex) {
            Assert.fail("should not reach here");
        }

        try
        {
            Tenant tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
            Assert.assertEquals(tenantName, tenant.getName());
        }
        catch (Exception ex)
        {
            Assert.fail("should not reach here");
        }

        try {
            // Check default config
            AuthnPolicy authnPolicy = idmClient.getAuthnPolicy(tenantName);
            RSAAgentConfig config = authnPolicy.get_rsaAgentConfig();
            Assert.assertNull("Default rsaConfigs in a new tenant should be null.", config);

            String siteID = idmClient.getClusterId();
            String siteID2 = "siteID2";

            // Test setting config with one site
            RSAAMInstanceInfo instInfo1 = new RSAAMInstanceInfo(siteID, "TestAgent", "sdConfBytes".getBytes() , "sdoptsTest".getBytes());
            RSAAgentConfig configIn = new RSAAgentConfig( instInfo1);
            idmClient.setRSAConfig(tenantName, configIn);
            RSAAgentConfig configOut = idmClient.getRSAConfig(tenantName);
            AssertRSAAgentConfig(configIn, configOut);

            //test RSAAMInstanceInfo ctr
            try {
                RSAAMInstanceInfo instInfo = new RSAAMInstanceInfo(siteID, "TestAgent", null , null);
                instInfo = new RSAAMInstanceInfo(null, "", "sdConfBytes".getBytes() , null);
                instInfo = new RSAAMInstanceInfo("", "", "sdConfBytes".getBytes() , null);
                Assert.fail("should not reach here");

            } catch (IllegalArgumentException | NullPointerException e) {
            }


            // Test setting full RSA configuration and add a second site
            configIn.set_loginGuide("Test login guidence string");
            configIn.set_connectionTimeOut(10);
            configIn.set_readTimeOut(20);
            configIn.set_logFileSize(2);
            RSAAMInstanceInfo instInfo2 = new RSAAMInstanceInfo(siteID2, "TestAgent2", "sdConfBytes".getBytes() , null);
            configIn.add_instInfo(instInfo2);
            configIn.set_logLevel(RSAAgentConfig.RSALogLevelType.valueOf("DEBUG"));
            configIn.set_maxLogFileCount(20);
            configIn.set_rsaEncAlgList(new HashSet<String>(Arrays.asList("alg1","alg2")));
            HashMap<String,String> idsUserIDAttributeMap = new HashMap<String, String>();
            idsUserIDAttributeMap.put("adTestIDS", "upn");
            idsUserIDAttributeMap.put("localIDS", "email");
            configIn.set_idsUserIDAttributeMaps(idsUserIDAttributeMap);

            idmClient.setRSAConfig(tenantName, configIn);
            configOut = idmClient.getRSAConfig(tenantName);
            AssertRSAAgentConfig(configIn, configOut);

            //Remove second site from RSAAgentConfig

            HashMap<String, RSAAMInstanceInfo> instMap = configIn.get_instMap();
            instMap.remove(siteID2);
            configIn.set_instMap(instMap);

            idmClient.deleteRSAInstanceInfo(tenantName, siteID2);
            configOut = idmClient.getRSAConfig(tenantName);
            AssertRSAAgentConfig(configIn, configOut);

            //Udate siteInfo attributes
            instMap = configIn.get_instMap();
            instInfo1.set_agentName("TestAgentChanged");
            instInfo1.set_sdoptsRec("sdoptsTestModified".getBytes());
            instMap.put(siteID,instInfo1);
            configIn.set_instMap(instMap);

            idmClient.setRSAConfig(tenantName, configIn);
            configOut = idmClient.getRSAConfig(tenantName);
            AssertRSAAgentConfig(configIn, configOut);

            // Test updates RSAAgentConfig attributes
            configIn.set_loginGuide("Modified login guidence string");
            configIn.set_connectionTimeOut(40);
            configIn.set_readTimeOut(50);
            configIn.set_logFileSize(70);
            configIn.set_logLevel(RSAAgentConfig.RSALogLevelType.valueOf("WARN"));
            configIn.set_maxLogFileCount(7);
            configIn.set_rsaEncAlgList(new HashSet<String>(Arrays.asList("ALG1","ALG2")));
            idsUserIDAttributeMap = new HashMap<String, String>();
            idsUserIDAttributeMap.put("adTestIDS", "email");
            idsUserIDAttributeMap.put("localIDS", "upn");
            configIn.set_idsUserIDAttributeMaps(idsUserIDAttributeMap);

            idmClient.setRSAConfig(tenantName, configIn);
            configOut = idmClient.getRSAConfig(tenantName);
            AssertRSAAgentConfig(configIn, configOut);

        } catch (Exception e) {
            Assert.fail("should not reach here");
        }
        finally {

            // Cleanup
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }
    private void AssertRSAAgentConfig(RSAAgentConfig configIn, RSAAgentConfig configOut) {

        Assert.assertEquals(configIn.get_connectionTimeOut(), configOut.get_connectionTimeOut());
        Assert.assertEquals(configIn.get_readTimeOut(), configOut.get_readTimeOut());
        Assert.assertEquals(configIn.get_idsUserIDAttributeMap(), configOut.get_idsUserIDAttributeMap());
        Assert.assertEquals(configIn.get_logFileSize(), configOut.get_logFileSize());
        Assert.assertEquals(configIn.get_logLevel(), configOut.get_logLevel());
        Assert.assertEquals(configIn.get_maxLogFileCount(), configOut.get_maxLogFileCount());
        Assert.assertEquals(configIn.get_rsaEncAlgList(), configOut.get_rsaEncAlgList());

        HashMap<String, RSAAMInstanceInfo> instMapIn = configIn.get_instMap();
        HashMap<String, RSAAMInstanceInfo> instMapOut = configOut.get_instMap();
        Assert.assertEquals(instMapIn.keySet(), instMapOut.keySet());
        Assert.assertEquals(instMapIn, instMapOut);
    }

    @Test
    public void testAuthnPolicy() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        String tenantName = UUID.randomUUID().toString();
        Tenant tenantToCreate = new Tenant(tenantName);
        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
        try
        {
            Tenant tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
            Assert.assertEquals(tenantName, tenant.getName());
        }
        catch (NoSuchTenantException ex)
        {
            Assert.fail("should not reach here");
        }

        try {
            // Check default AuthnPolicy
            AuthnPolicy policyInDefault = new AuthnPolicy(true, true, false,
                    null);
            AuthnPolicy policyOutDefault = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyInDefault, policyOutDefault);

            List<X509Certificate> certs = this
                    .buildExternalIdpSTSKeyCertificates();
            Assert.assertTrue(certs.size() >= 2);
            List<Certificate> trustedCAs = new LinkedList<Certificate>();
            trustedCAs.add(certs.get(0));
            trustedCAs.add(certs.get(1));

            // Set once
            ClientCertPolicy cert = new ClientCertPolicy();
            cert.setRevocationCheckEnabled(true);
            cert.setUseOCSP(true);
            cert.setUseCRLAsFailOver(true);
            cert.setSendOCSPNonce(true);
            cert.setOCSPUrl(new URL("http://www.ocsp1.com"));
            cert.setOCSPResponderSigningCert(this.getSTSKeyCertificates()
                    .get(0));
            cert.setUseCertCRL(true);
            cert.setCRLUrl(new URL("http://www.crl1.com"));
            cert.setCacheSize(10);
            cert.setOIDs(new String[] { "a", "b", "c" });
            cert.setTrustedCAs(trustedCAs.toArray(new Certificate[trustedCAs.size()]));
            cert.setEnableHint(true);
            AuthnPolicy policyIn = new AuthnPolicy(true, true, true, cert);
            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut);


            // Set again
            ClientCertPolicy cert2 = new ClientCertPolicy();
            cert2.setRevocationCheckEnabled(false);
            cert2.setUseOCSP(false);
            cert2.setUseCRLAsFailOver(false);
            cert2.setSendOCSPNonce(false);
            cert2.setOCSPUrl(new URL("http://www.ocsp2.com"));
            cert2.setOCSPResponderSigningCert(this.getSTSKeyCertificates().get(
                    0));
            cert2.setUseCertCRL(false);
            cert2.setCRLUrl(new URL("http://www.crl2.com"));
            cert2.setCacheSize(20);
            cert2.setOIDs(new String[] { "a", "b", "c" });
            cert2.setEnableHint(false);
            trustedCAs.remove(1);
            cert.setTrustedCAs(trustedCAs.toArray(new Certificate[trustedCAs.size()]));
            AuthnPolicy policyIn2 = new AuthnPolicy(true, true, false, cert2);
            idmClient.setAuthnPolicy(tenantName, policyIn2);
            AuthnPolicy policyOut2 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn2, policyOut2);

            // Set null Urls
            cert = new ClientCertPolicy();
            cert.setRevocationCheckEnabled(true);
            cert.setUseOCSP(true);
            cert.setUseCRLAsFailOver(true);
            cert.setSendOCSPNonce(true);
            cert.setOCSPResponderSigningCert(this.getSTSKeyCertificates()
                    .get(0));
            cert.setUseCertCRL(true);
            cert.setCacheSize(10);
            cert.setOIDs(new String[] { "a", "b", "c" });
            policyIn = new AuthnPolicy(true, true, true, cert);
            idmClient.setAuthnPolicy(tenantName, policyIn);
            policyOut = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut);

            // Set again with default certPolicy
            AuthnPolicy policyInNullCertPolicy = new AuthnPolicy(true, true,
                    true, null);
            idmClient.setAuthnPolicy(tenantName, policyInNullCertPolicy);
            AuthnPolicy policyOut3 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyInNullCertPolicy, policyOut3);
        } finally {
            // Cleanup
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }
    @Test
    public void testAlternativeOCSP() throws Exception,
    IDMException
    {
        CasIdmClient idmClient = getIdmClient();
        String tenantName = UUID.randomUUID().toString();
        Tenant tenantToCreate = new Tenant(tenantName);
        IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        idmClient.addTenant(tenantToCreate, DEFAULT_TENANT_ADMIN_NAME, DEFAULT_TENANT_ADMIN_PASSWORD.toCharArray());
        try
        {
            Tenant tenant = idmClient.getTenant(tenantName);
            Assert.assertNotNull(tenant);
            Assert.assertEquals(tenantName, tenant.getName());
        }
        catch (NoSuchTenantException ex)
        {
            Assert.fail("should not reach here");
        }

        try {
            // Check default AuthnPolicy
            AuthnPolicy policyInDefault = new AuthnPolicy(true, true, false,
                    null);
            AuthnPolicy policyOutDefault = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyInDefault, policyOutDefault);

            List<X509Certificate> certs = this
                    .buildExternalIdpSTSKeyCertificates();
            Assert.assertTrue(certs.size() >= 2);
            List<Certificate> trustedCAs = new LinkedList<Certificate>();
            trustedCAs.add(certs.get(0));
            trustedCAs.add(certs.get(1));

            // Set a base certificate policy
            ClientCertPolicy cert = new ClientCertPolicy();
            cert.setRevocationCheckEnabled(true);
            cert.setUseOCSP(true);
            cert.setUseCRLAsFailOver(true);
            cert.setSendOCSPNonce(true);
            cert.setOCSPUrl(new URL("http://www.ocsp1.com"));
            cert.setOCSPResponderSigningCert(this.getSTSKeyCertificates()
                    .get(0));

            cert.setTrustedCAs(trustedCAs.toArray(new Certificate[trustedCAs.size()]));


            //case 1: test one site with one OCSP responder
            AlternativeOCSP altOcsp = new AlternativeOCSP(new URL("http://www.ocsp_Alt_1.com"), this.getSTSKeyCertificates().get(0) );
            List<AlternativeOCSP> ocspList = new ArrayList<AlternativeOCSP>();
            ocspList.add(altOcsp);
            AlternativeOCSPList altOcspList = new AlternativeOCSPList(TENANT_PSC_SITE_1, ocspList);
            HashMap<String, AlternativeOCSPList> altOcspSiteMap = new HashMap<String, AlternativeOCSPList>();
            altOcspSiteMap.put(TENANT_PSC_SITE_1, altOcspList);
            cert.set_siteOCSPMap(altOcspSiteMap);

            AuthnPolicy policyIn = new AuthnPolicy(true, true, true, cert);
            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut);

            //case 2: test adding second ocsp
            AlternativeOCSP altOcsp2 = new AlternativeOCSP(new URL("http://www.ocsp_Alt_2.com"), this.getSTSKeyCertificates().get(0) );
            policyIn.getClientCertPolicy().get_siteOCSPList().get(TENANT_PSC_SITE_1).addAlternativeOCSP(altOcsp2);

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut2 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut2);

            //case 3: test removing second ocsp of the site.
            // AlternativeOCSP altOcsp2 = new AlternativeOCSP(new URL("http://www.ocsp_Alt_2.com"), this.getSTSKeyCertificates().get(0) );
            policyIn.getClientCertPolicy().get_siteOCSPList().get(TENANT_PSC_SITE_1).get_ocspList().remove(altOcsp2);

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut3 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut3);

            //case 4: test adding OCSP for second PSC site
            AlternativeOCSP altOcsp3 = new AlternativeOCSP(new URL("http://www.ocsp_Alt_3.com"), this.getSTSKeyCertificates().get(0) );
            List<AlternativeOCSP> ocspList2 = new ArrayList<AlternativeOCSP>();
            ocspList2.add(altOcsp3);
            AlternativeOCSPList altOcspList2 = new AlternativeOCSPList(TENANT_PSC_SITE_2, ocspList2);
            policyIn.getClientCertPolicy().get_siteOCSPList().put(TENANT_PSC_SITE_2, altOcspList2);

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut4 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut4);

            //case 5: test removing second psc site

            policyIn.getClientCertPolicy().get_siteOCSPList().remove(TENANT_PSC_SITE_2);

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut5 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut5);

            //case 6: test backward compatibility to Admin. To obsolete once all clients move to REST.
            policyIn.getClientCertPolicy().set_siteOCSPMap(null);

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut6 = idmClient.getAuthnPolicy(tenantName);
            Assert.assertTrue(policyOut6.getClientCertPolicy().get_siteOCSPList().size() == 1);

            //case 7: test removing all alternative OCSP
            policyIn.getClientCertPolicy().set_siteOCSPMap(new HashMap<String, AlternativeOCSPList>());

            idmClient.setAuthnPolicy(tenantName, policyIn);
            AuthnPolicy policyOut7 = idmClient.getAuthnPolicy(tenantName);
            AssertAuthnPolicy(policyIn, policyOut7);

        } finally {
            // Cleanup
            IdmClientTestUtil.ensureTenantDoesNotExist(idmClient, tenantName);
        }
    }

    private void AssertAuthnPolicy(AuthnPolicy policyIn, AuthnPolicy policyOut){
        Assert.assertEquals(policyIn.IsPasswordAuthEnabled(),policyOut.IsPasswordAuthEnabled());
        Assert.assertEquals(policyIn.IsWindowsAuthEnabled(),policyOut.IsWindowsAuthEnabled());
        Assert.assertEquals(policyIn.IsTLSClientCertAuthnEnabled(),policyOut.IsTLSClientCertAuthnEnabled());
        if(policyIn.getClientCertPolicy() != null)
            AssertClientCertPolicy(policyIn.getClientCertPolicy(), policyOut.getClientCertPolicy());
        else
            AssertClientCertPolicy(new ClientCertPolicy(), policyOut.getClientCertPolicy());
    }

    private void AssertClientCertPolicy(ClientCertPolicy certIn, ClientCertPolicy certOut){
        Assert.assertEquals(certIn.revocationCheckEnabled(), certOut.revocationCheckEnabled());
        Assert.assertEquals(certIn.useCRLAsFailOver(), certOut.useCRLAsFailOver());
        Assert.assertEquals(certIn.sendOCSPNonce(), certOut.sendOCSPNonce());
        Assert.assertEquals(certIn.getOCSPUrl(), certOut.getOCSPUrl());
        Assert.assertEquals(certIn.getOCSPResponderSigningCert(), certOut.getOCSPResponderSigningCert());
        Assert.assertEquals(certIn.useCertCRL(), certOut.useCertCRL());
        Assert.assertEquals(certIn.getCRLUrl(), certOut.getCRLUrl());
        Assert.assertEquals(certIn.getCacheSize(), certOut.getCacheSize());
        Assert.assertEquals(certIn.getEnableHint(), certOut.getEnableHint());
        if ( certIn.get_siteOCSPList() != null ) {
            Assert.assertTrue(certIn.get_siteOCSPList().equals(certOut.get_siteOCSPList()));
        }

        //OIDs
        if(certIn.getOIDs() == null){
            Assert.assertNull(certOut.getOIDs());
        } else {
            Assert.assertNotNull(certOut.getOIDs());
            Assert.assertEquals(certIn.getOIDs().length, certOut.getOIDs().length);
            for(String s : certIn.getOIDs()){
                Assert.assertTrue(ArrayUtils.contains(certOut.getOIDs(), s));
            }
        }

        //trusted CAs
        if(certIn.getTrustedCAs() == null || certIn.getTrustedCAs().length == 0){
            Assert.assertNull(certOut.getTrustedCAs());
        } else {
            Assert.assertNotNull(certOut.getTrustedCAs());
            Assert.assertEquals(certIn.getTrustedCAs().length, certOut.getTrustedCAs().length);
            for(int i=0; i<certIn.getTrustedCAs().length; i++) {
                Assert.assertEquals(certIn.getTrustedCAs()[i], certOut.getTrustedCAs()[i]);
            }
        }
    }

    private void validateAdminInAdminGroup(CasIdmClient idmClient, String tenantName, String userPrincipal) throws Exception
    {
        Principal principal = idmClient.findUser(tenantName, userPrincipal);
        Set<Group> groups = idmClient.findDirectParentGroups(tenantName, principal.getId());
        boolean adminGroupFound = false;
        PrincipalId adminId = new PrincipalId(ADMIN_GROUP_NAME, tenantName);
        for (Group group: groups)
        {
            if (group.getId().equals(adminId))
            {
                adminGroupFound = true;
                break;
            }
        }
        Assert.assertTrue(adminGroupFound);

    }

    private <T> T ensureNoSuchTenantExceptionOccurred(Callable<T> operation)
            throws Exception
            {
        boolean noSuchTenantExceptionOccurred = false;
        T result = null;
        try
        {
            result = operation.call();
        } catch (NoSuchTenantException nste)
        {
            noSuchTenantExceptionOccurred = true;
        } finally
        {
            Assert.assertEquals(true, noSuchTenantExceptionOccurred);
        }
        return result;
            }

    private List<ServiceEndpoint> buildSSOServicesIdp1(Properties props)
            throws Exception
            {
        ServiceEndpoint ssoService =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_SSO_NAME_1));
        ssoService.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SSO_LOCATION_1));
        ssoService.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SSO_BINDING_1));
        return Arrays.asList(ssoService);
            }

    private List<ServiceEndpoint> buildSLOServicesIdp1(Properties props)
            throws Exception
            {
        ServiceEndpoint sloService1 =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_NAME_1));
        sloService1.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_LOCATION_1));
        sloService1.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_BINDING_1));

        ServiceEndpoint sloService2 =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_NAME_2));
        sloService2.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_LOCATION_2));
        sloService2.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_1_SLO_BINDING_2));

        return Arrays.asList(sloService1, sloService2);
            }

    private List<X509Certificate> getSTSKeyCertificates() throws Exception
    {
        Properties props = getTestProperties();
        KeyStore ks =
                loadKeyStore(CFG_KEY_STS_KEYSTORE,
                        CFG_KEY_STS_KEYSTORE_PASSWORD);
        Certificate[] certChain = ks.getCertificateChain(
                props.getProperty(CFG_KEY_STS_KEY_ALIAS));
        List<X509Certificate> x509Certs = new ArrayList<X509Certificate>();
        for (Certificate cert: certChain) {
            x509Certs.add((X509Certificate)cert);
        }
        return x509Certs;
    }

    private X509Certificate getCertificate(String certAlias) throws Exception
    {
         CertificateFactory cf = CertificateFactory.getInstance("X.509");
         X509Certificate cert = (X509Certificate)cf.generateCertificate(TenantManagementTest.class.getResourceAsStream(certAlias));

         return cert;
    }

    private List<X509Certificate> buildExternalIdpSTSKeyCertificates()
            throws Exception
            {
        Properties props = getTestProperties();
        KeyStore ks =
                loadKeyStore(CFG_KEY_EXTERNAL_IDP_STS_STORE,
                        CFG_KEY_EXTERNAL_IDP_STS_STORE_PASS);

        X509Certificate leaf =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_EXTERNAL_IDP_STS_ALIAS_LEAF_CERT));
        X509Certificate root =
                (X509Certificate) ks.getCertificate(props
                        .getProperty(CFG_KEY_EXTERNAL_IDP_STS_ALIAS_ROOT_CERT));

        List<X509Certificate> x509Certs = Arrays.asList(leaf, root);
        return x509Certs;
            }

    private List<ServiceEndpoint> buildSSOServicesIdp2(Properties props)
            throws Exception
            {
        ServiceEndpoint ssoService =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_2_SSO_NAME_1));
        ssoService.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SSO_LOCATION_1));
        ssoService.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SSO_BINDING_1));
        return Arrays.asList(ssoService);
            }

    private List<ServiceEndpoint> buildSLOServicesIdp2(Properties props)
            throws Exception
            {
        ServiceEndpoint sloService1 =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_NAME_1));
        sloService1.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_LOCATION_1));
        sloService1.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_BINDING_1));

        ServiceEndpoint sloService2 =
                new ServiceEndpoint(
                        props.getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_NAME_2));
        sloService2.setEndpoint(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_LOCATION_2));
        sloService2.setBinding(props
                .getProperty(CFG_KEY_EXTERNAL_IDP_2_SLO_BINDING_2));

        return Arrays.asList(sloService1, sloService2);
            }

    private KeyStore loadKeyStore(String keyStoreNameInfo, String passwordIfno)
    {
        try
        {
            Properties props = getTestProperties();

            KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());

            String password = props.getProperty(passwordIfno);

            Assert.assertNotNull(password);

            String keyStoreResourceName = props.getProperty(keyStoreNameInfo);

            Assert.assertNotNull(keyStoreResourceName);

            ks.load(getClass().getResourceAsStream(keyStoreResourceName),
                    password.toCharArray());

            return ks;
        } catch (Exception e)
        {
            throw new IllegalStateException(e);
        }
    }

    private synchronized CasIdmClient getIdmClient() throws Exception
    {
        if (_idmClient == null)
        {
            Properties props = getTestProperties();

            String hostname = props.getProperty(CFG_KEY_IDM_HOSTNAME);

            Assert.assertNotNull(hostname);

            _idmClient = new CasIdmClient(hostname);
        }

        return _idmClient;
    }

    private synchronized Properties getTestProperties() throws Exception
    {
        if (_testProps == null)
        {
            _testProps = new Properties();

            _testProps.load(getClass()
                    .getResourceAsStream("/config.properties"));
        }

        return _testProps;
    }

    // Verify Non-ExistentUser authenticate fails with expected error(s)
    private void TestNonExistentUserAuthenticate(CasIdmClient idmClient,
            String tenantName, String user, String pwd)
    {
        try
        {
            idmClient.authenticate(tenantName, user, pwd);
            Assert.fail("Should not be able to authenticated non-existent user.");
        } catch (IDMLoginException ex)
        {
            // expected
        } catch (Exception ex)
        {
            Assert.fail(String.format("Unexpected exception: %s",
                    ex.getMessage()));
        }
    }

    private void ValidateMemberOfSolutionUsersGroup(CasIdmClient idmClient,
            String tenantName, String userName) throws Exception
            {
        Set<SolutionUser> solutionMembers =
                idmClient.findSolutionUsersInGroup(tenantName, "SolutionUsers",
                        userName, -1);
        Assert.assertTrue(solutionMembers != null
                && solutionMembers.size() >= 1);
        boolean hasSolutionUser = false;
        for (SolutionUser user : solutionMembers)
        {
            if (user.getId().getName().equalsIgnoreCase(userName))
            {
                hasSolutionUser = true;
                break;
            }
        }
        Assert.assertTrue(hasSolutionUser);
            }

    private void testUserFSP(
            CasIdmClient idmClient,
            String userTenantName,
            PrincipalId principal1,
            PrincipalId principal2)
    {
        try
        {
            boolean bUserAdded = false;
            // Add the user to system domain of tenant ('abc')
            // add FSPs - {FspTestUser1, fsptestuser2}@ssolabs-openldap.eng.vmware.com to Administrators@abc.com
            idmClient.addUserToGroup(userTenantName, principal1,
                    "Administrators");

            try
            {
                bUserAdded =
                        idmClient.addUserToGroup(userTenantName, principal2,
                                "Administrators");
                Assert.assertTrue(bUserAdded);
            } catch (MemberAlreadyExistException ex)
            {
                Assert.fail("Should not reach here");
            }

            try
            {
                bUserAdded = false;
                bUserAdded =
                        idmClient.addUserToGroup(userTenantName, principal2,
                                "Administrators");
            } catch (MemberAlreadyExistException ex)
            {
                Assert.assertTrue(!bUserAdded);
            }

            // retrieve members information of Administrator@abc.com
            Set<PersonUser> members =
                    idmClient.findPersonUsersInGroup(userTenantName,
                            new PrincipalId("Administrators", userTenantName),
                            "", -1);

            boolean bFoundAdUser1 = false;
            boolean bFoundAdUser2 = false;

            // Lookup members in tenant abc.com
            for (PersonUser member : members)
            {
                PersonUser user1 =
                        idmClient
                        .findPersonUser(userTenantName, member.getId());
                PersonUser user2 =
                        idmClient.findPersonUserByObjectId(userTenantName,
                                member.getObjectId());

                if (!bFoundAdUser1 && member.getId().equals(principal1))
                {
                    bFoundAdUser1 = true;
                }

                if (!bFoundAdUser2 && member.getId().equals(principal2))
                {
                    bFoundAdUser2 = true;
                }

                Assert.assertEquals(user1, user2);
            }
            Assert.assertTrue(bFoundAdUser1);
            Assert.assertTrue(bFoundAdUser2);

            // remove 'fsptestuser1@ssolabs-openldap.eng.vmware.com' from administrators@abc.com group
            Assert.assertTrue(idmClient.removeFromLocalGroup(userTenantName,
                    principal2, "Administrators"));

            // retrieve members information of Administrator@abc.com
            members =
                    idmClient.findPersonUsersInGroup(userTenantName,
                            new PrincipalId("Administrators", userTenantName),
                            "", -1);

            bFoundAdUser1 = false;
            bFoundAdUser2 = false;

            // Lookup members in tenant abc.com
            for (PersonUser member : members)
            {
                PersonUser user1 =
                        idmClient
                        .findPersonUser(userTenantName, member.getId());
                PersonUser user2 =
                        idmClient.findPersonUserByObjectId(userTenantName,
                                member.getObjectId());

                if (!bFoundAdUser1 && member.getId().equals(principal1))
                {
                    bFoundAdUser1 = true;
                }

                if (!bFoundAdUser2 && member.getId().equals(principal2))
                {
                    bFoundAdUser2 = true;
                }

                Assert.assertEquals(user1, user2);
            }
            Assert.assertTrue(bFoundAdUser1);
            // fsptestuser1 is removed being member of 'administrators@abc.com'
            Assert.assertTrue(bFoundAdUser2 == false);
        } catch (Exception e)
        {
            throw new AssertionError(e);
        }
    }

    private void testGroupFSP(
            CasIdmClient idmClient,
            String groupTenantName,
            PrincipalId principal1,
            PrincipalId principal2)
    {
        try
        {
            boolean bGroupAdded = false;
            // Add the user to system domain of tenant ('abc')
            // add FSPs - {FspTestgroup1, FspTestgroup2}@ssolabs.eng.vmware.com to Administrators@abc.com
            idmClient.addGroupToGroup(groupTenantName, principal1,
                    "Administrators");

            try
            {
                bGroupAdded =
                        idmClient.addGroupToGroup(groupTenantName, principal2,
                                "Administrators");
                Assert.assertTrue(bGroupAdded);
            } catch (MemberAlreadyExistException ex)
            {
                Assert.fail("Should not reach here");
            }

            try
            {
                bGroupAdded = false;
                bGroupAdded =
                        idmClient.addGroupToGroup(groupTenantName, principal2,
                                "Administrators");
            } catch (MemberAlreadyExistException ex)
            {
                Assert.assertTrue(!bGroupAdded);
            }

            // retrieve members information of Administrator@abc.com
            Set<Group> members =
                    idmClient.findGroupsInGroup(groupTenantName,
                            new PrincipalId("Administrators", groupTenantName),
                            "", -1);

            boolean bFoundAdGroup1 = false;
            boolean bFoundAdGroup2 = false;

            // Lookup members in tenant abc.com
            for (Group member : members)
            {
                Group group1 =
                        idmClient.findGroup(groupTenantName, member.getId());
                Group group2 =
                        idmClient.findGroupByObjectId(groupTenantName,
                                member.getObjectId());

                if (!bFoundAdGroup1 && member.getId().equals(principal1))
                {
                    bFoundAdGroup1 = true;
                }

                if (!bFoundAdGroup2 && member.getId().equals(principal2))
                {
                    bFoundAdGroup2 = true;
                }

                Assert.assertEquals(group1, group2);
            }
            Assert.assertTrue(bFoundAdGroup1);
            Assert.assertTrue(bFoundAdGroup2);

            // remove 'fsptestgroup1@ssolabs.eng.vmware.com' from administrators@abc.com group
            Assert.assertTrue(idmClient.removeFromLocalGroup(groupTenantName,
                    principal2, "Administrators"));

            // retrieve members information of Administrator@abc.com
            members =
                    idmClient.findGroupsInGroup(groupTenantName,
                            new PrincipalId("Administrators", groupTenantName),
                            "", -1);

            bFoundAdGroup1 = false;
            bFoundAdGroup2 = false;

            // Lookup members in tenant abc.com
            for (Group member : members)
            {
                Group group1 =
                        idmClient.findGroup(groupTenantName, member.getId());
                Group group2 =
                        idmClient.findGroupByObjectId(groupTenantName,
                                member.getObjectId());

                if (!bFoundAdGroup1 && member.getId().equals(principal1))
                {
                    bFoundAdGroup1 = true;
                }

                if (!bFoundAdGroup2 && member.getId().equals(principal2))
                {
                    bFoundAdGroup2 = true;
                }

                Assert.assertEquals(group1, group2);
            }
            Assert.assertTrue(bFoundAdGroup1);
            // fsptestgroup2 is removed being member of 'administrators@abc.com'
            Assert.assertTrue(bFoundAdGroup2 == false);
        } catch (Exception e)
        {
            throw new AssertionError(e);
        }
    }
}
