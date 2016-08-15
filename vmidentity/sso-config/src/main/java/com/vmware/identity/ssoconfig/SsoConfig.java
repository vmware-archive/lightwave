/*
 * Copyright (c) 2011-12 VMware, Inc. All rights reserved.
 */

package com.vmware.identity.ssoconfig;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import java.net.Socket;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.InvalidParameterException;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Scanner;
import java.util.Set;
import java.util.Date;
import java.util.Iterator;
import java.security.cert.CertificateException;

import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.OptionGroup;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.OptionBuilder;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.PosixParser;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.SystemUtils;
import org.apache.commons.lang.Validate;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.DomainTrustsInfo;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.SsoHealthStatsData;
import com.vmware.identity.idm.SsoHealthStatisticsException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.ILdapQueryStat;
import com.vmware.identity.token.impl.SecureXmlParserFactory;
/**
 * Command line tool for tenant configuration file importing and exporting.
 *
 * @author  schai
 * Usage: import|export [-t aTenantName] config_filename
 */
public final class SsoConfig {

    private static final String HOSTNAME = "localhost";
    private static final String LDAPS_SCHEMA = "ldaps";
    private static final String UNDEFINED_CONFIG = "UndefinedConfig";
    private static final String OPTION_IMPORT = "import";
    private static final String OPTION_EXPORT = "export";
    private static final String OPTION_REGISTER_SP = "register_sp";
    private static final String OPTION_REMOVE_SP = "remove_sp";
    private static final String OPTION_REGISTER_IDP = "register_idp";
    private static final String OPTION_REMOVE_IDP = "remove_idp";
    private static final String OPTION_GET_SSO_SAML2_METADATA = "get_sso_saml2_metadata";
    private static final String OPTION_GET_IDENTITY_SOURCES = "get_identity_sources";
    private static final String OPTION_SET_DEFAULT_IDENTITY_SOURCES = "set_default_identity_sources";
    private static final String OPTION_IDS_TEST_CONNECTION = "check_ldaps_cert_validation";
    private static final String OPTION_IDS_TEST_SUPPORTS_TLS = "check_ldaps_server_supports_tls";
    private static final String OPTION_GET_DOMAIN_JOIN_STATUS = "get_domain_join_status";
    private static final String OPTION_SET_IDENTITY_STORE_FLAGS = "set_identity_store_flags";
    private static final String OPTION_GET_IDENTITY_STORE_FLAGS = "get_identity_store_flags";
    private static final String OPTION_GET_IDM_AUTH_STATS = "get_idm_auth_stats";
    private static final String OPTION_ADD_GROUP = "add_group";
    private static final String OPTION_JIT_SWITCH = "enable_jit";
    private static final String OPTION_CHANGE_USER_PWD = "change_user_password";
    private static final String OPTION_UPDATE_SYSTEM_DOMAIN_STORE_PWD = "update_system_domain_store_password";
    private static final String OPTION_GET_SSO_STATISTICS = "get_sso_statistics";
    private static final String OPTION_REMOVE_JIT_USERS = "remove_jit_users";
    private static final String OPTION_ADD_CLAIM_GROUP_MAP = "add_claim_group_map";
    private static final String OPTION_DELETE_CLAIM_GROUP_MAP = "delete_claim_group_map";
    private static final String OPTION_SET_UPN_SUFFIX = "set_external_idp_upn_suffix";
    private static final String OPTION_GET_TC_CERT_AUTHN = "get_tc_cert_authn";
    private static final String OPTION_SET_TC_CERT_AUTHN = "set_tc_cert_authn";
    private static final String OPTION_GET_AUTHN_POLICY = "get_authn_policy";
    private static final String OPTION_SET_AUTHN_POLICY = "set_authn_policy";
    private static final String OPTION_ADD_ALT_OCSP = "add_alt_ocsp";
    private static final String OPTION_GET_ALT_OCSP = "get_alt_ocsp";
    private static final String OPTION_DELETE_ALT_OCSP = "delete_alt_ocsp";


    private static final String OPTION_SET_LOGON_BANNER = "set_logon_banner";
    private static final String OPTION_PRINT_LOGON_BANNER = "print_logon_banner";
    private static final String OPTION_DISABLE_LOGON_BANNER = "disable_logon_banner";
    private static final String OPTION_SET_IDP_SELECTION_FLAG = "set_idp_selection_flag";
    private static final String OPTION_GET_LOCAL_IDP_ALIAS = "get_local_idp_alias";
    private static final String OPTION_SET_LOCAL_IDP_ALIAS = "set_local_idp_alias";
    private static final String OPTION_GET_EXT_IDP_ALIAS = "get_external_idp_alias";
    private static final String OPTION_SET_EXT_IDP_ALIAS = "set_external_idp_alias";

    //general constants
    private static final String OPTION_TENANT = "t";
    private static final String OPTION_TENANT_ARG = "tenantName";
    private static final String OPTION_ALL_SITES = "allSites";

    private static final String OPTION_USER = "u";
    private static final String OPTION_USER_ARG = "userName";

    private static final String OPTION_ENTITY_ID = "e";
    private static final String OPTION_ENTITY_ID_ARG = "externalIdpEntityId";

    private static final String OPTION_SERVICE_PROVIDER = "sp";
    private static final String OPTION_SERVICE_PROVIDER_ARG = "service provider name";

    private static final String OPTION_PROVIDER = "i";
    private static final String OPTION_PROVIDER_ARG = "identityStoreName";

    private static final String OPTION_FLAGS = "f";
    private static final String OPTION_FLAGS_ARG = "flags";
    private static final String OPTION_FLAGS_SEPARATOR = "/";

    private static final String OPTION_FLAGS_MATCHING_RULE_IN_CHAIN = "matching_rule_in_chain";
    private static final String OPTION_FLAGS_DIRECT_GROUPS_ONLY = "direct_groups";
    private static final String OPTION_FLAGS_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS = "no_group_base_dn_for_nested_groups";
    private static final String OPTION_FLAGS_ENABLE_SITE_AFFINITY = "enable_site_affinity";
    private static final String OPTION_FLAGS_FSP_GROUPS = "include_external_forest_groups";
    private static final String OPTION_FLAGS_TOKEN_GROUPS = "token_Groups";

    private static final String OPTION_ACTIVITY_KIND = "a";
    private static final String OPTION_ACTIVITY_KIND_ARG = "activityKind";

    private static final String OPTION_GROUP_NAME = "g";
    private static final String OPTION_GROUP_NAME_ARG = "groupName";

    private static final String OPTION_GROUP_DESCRIPTION = "gd";
    private static final String OPTION_GROUP_DESCRIPTION_ARG = "groupDescription";

    private static final String OPTION_VERBOSE = "v";
    private static final String OPTION_VERBOSE_ARG = "verbose";

    private static final String OPTION_CLAIM_NAME = "claim_name";
    private static final String OPTION_CLAIM_NAME_ARG = "claimName";

    private static final String OPTION_CLAIM_VALUE = "claim_value";
    private static final String OPTION_CLAIM_VALUE_ARG = "claimValue";

    private static final String OPTION_UPN_SUFFIX = "upn_suffix";
    private static final String OPTION_UPN_SUFFIX_ARG = "upnSuffix";

    private static final String OPTION_CA_FILES = "cacerts";
    private static final String OPTION_CA_FILES_ARG = "caCert1,caCert2,..";

    private static final String OPTION_LOGON_BANNER_TITLE = "title";
    private static final String OPTION_LOGON_BANNER_TITLE_ARG = "logon_banner_title";

    private static final String OPTION_ALIAS = "alias";
    private static final String OPTION_ALIAS_ARG = "idp alias";

    private static final String OPTION_ENABLE_LOGON_BANNER_CHECKBOX = "enable_checkbox";
    private static final String OPTION_ENABLE_LOGON_BANNER_CHECKBOX_ARG = "Y/N";

    private static final String OPTION_AUTHN_CERT_ENABLED = "certAuthn";
    private static final String OPTION_AUTHN_CERT_ENABLED_ARG = "true or false";
    private static final String OPTION_AUTHN_PWD_ENABLED = "pwdAuthn";
    private static final String OPTION_AUTHN_PWD_ENABLED_ARG = "true or false";
    private static final String OPTION_AUTHN_WIN_ENABLED = "winAuthn";
    private static final String OPTION_AUTHN_WIN_ENABLED_ARG = "true or false";
    private static final String OPTION_AUTHN_RSASECURID_ENABLED = "securIDAuthn";
    private static final String OPTION_AUTHN_RSASECURID_ENABLED_ARG = "true or false";
    private static final String OPTION_AUTHN_CERT_REVOCATION_CHECK = "revocationCheck";
    private static final String OPTION_AUTHN_CERT_REVOCATION_CHECK_ARG = "true or false";
    private static final String OPTION_AUTHN_CERT_USE_OCSP = "useOcsp";
    private static final String OPTION_AUTHN_CERT_USE_OCSP_ARG = "true or false";
    private static final String OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER = "failoverToCrl";
    private static final String OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER_ARG = "true or false";

    private static final String OPTION_AUTHN_CERT_USE_CERT_CRL = "useCertCrl";
    private static final String OPTION_AUTHN_CERT_USE_CERT_CRL_ARG = "true or false";
    private static final String OPTION_AUTHN_CERT_CRL_URL = "crlUrl";
    private static final String OPTION_AUTHN_CERT_CRL_URL_ARG = "url";
    private static final String OPTION_AUTHN_CERT_OIDS = "certPolicies";
    private static final String OPTION_AUTHN_CERT_OIDS_ARG = "Policy OIDs seperated by comma";

    private static final String OPTION_SITE_ID = "siteID";

    //Alternative OCSP responder options
    private static final String OPTION_AUTHN_CERT_OCSP_URL = "ocspUrl";
    private static final String OPTION_AUTHN_CERT_OCSP_URL_ARG = "url";
    private static final String OPTION_AUTHN_CERT_OCSP_CERT = "ocspSigningCert";
    private static final String OPTION_AUTHN_CERT_OCSP_CERT_ARG = "certficate";

    //RSA config commands
    private static final String OPTION_SET_RSA_CONFIG = "set_rsa_config";
    private static final String OPTION_SET_RSA_SITE = "set_rsa_site";
    private static final String OPTION_SET_RSA_USERID_ATTR_MAP = "set_rsa_userid_attr_map";
    private static final String OPTION_GET_RSA_CONFIG = "get_rsa_config";

    //RSA config options
    private static final String OPTION_RSA_LOG_LEVEL = "logLevel";
    private static final String OPTION_RSA_LOG_LEVEL_ARG = "Level";
    private static final String OPTION_RSA_LOG_FILE_SIZE = "logFileSize";
    private static final String OPTION_RSA_LOG_FILE_SIZE_ARG = "Size";
    private static final String OPTION_RSA_MAX_LOG_FILE_COUNT = "maxLogFileCount";
    private static final String OPTION_RSA_MAX_LOG_FILE_COUNT_ARG = "Count";
    private static final String OPTION_RSA_CONN_TIME_OUT = "connTimeOut";
    private static final String OPTION_RSA_CONN_TIME_OUT_ARG = "Seconds";
    private static final String OPTION_RSA_READ_TIME_OUT = "readTimeOut";
    private static final String OPTION_RSA_READ_TIME_OUT_ARG = "Seconds";
    private static final String OPTION_RSA_ENC_ALG_LIST = "encAlgList";
    private static final String OPTION_RSA_ENC_ALG_LIST_ARG = "Alg1,Alg2,...";
    private static final String OPTION_RSA_LDAP_ATTR = "ldapAttr";
    private static final String OPTION_RSA_LDAP_ATTR_ARG = "AttrName";
    private static final String OPTION_RSA_IDS_NAME = "idsName";
    private static final String OPTION_RSA_IDS_NAME_ARG = "Name";
    private static final String OPTION_RSA_LOGIN_GUIDE = "logonGuide";
    private static final String OPTION_RSA_LOGIN_GUIDE_ARG = "guideText";
    //RSA site configuration opts
    private static final String OPTION_SITE_ID_ARG = "PSC site ID";
    private static final String OPTION_RSA_AGENT_NAME = "agentName";
    private static final String OPTION_RSA_AGENT_NAME_ARG = "Name";
    private static final String OPTION_RSA_SDCONF_REC = "sdConfFile";
    private static final String OPTION_FILE_PATH_ARG = "Path";
    private static final String OPTION_RSA_SDOPTS_REC = "sdOptsFile";

    // Browser based authentication types display params
    private static final String DISPLAY_PARAM_PASSWORD_AUTH = "IsPasswordAuthEnabled";
    private static final String DISPLAY_PARAM_WINDOWS_AUTH = "IsWindowsAuthEnabled";
    private static final String DISPLAY_PARAM_CERT_AUTH = "IsTLSClientCertAuthnEnabled";
    private static final String DISPLAY_PARAM_RSA_SECURID_AUTH = "IsSecurIDAuthnEnabled";
    private static final int FLAG_AUTHN_TYPE_ALLOW_PASSWORD = 0x1;
    private static final int FLAG_AUTHN_TYPE_ALLOW_WINDOWS = 0x2;
    private static final int FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE = 0x4;
    private static final int FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID = 0x8;

    // use matching_rule_in_chain for nested groups searches
    public static final int FLAG_AD_MATCHING_RULE_IN_CHAIN = 0x1;
    // utilize group search base dn for nested groups resolution in token
    public static final int FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS = 0x2;
    // resolve only direct parent groups
    public static final int FLAG_DIRECT_GROUPS_ONLY = 0x4;
    // enable site affinity to do ldap connection to AD
    public static final int FLAG_ENABLE_SITE_AFFINITY = 0x8;
    //resolve groups from a trusted forest by fsp
    private static final int FLAG_FSP_GROUPS = 0x10;
    //resolve groups by using tokenGroups
    private static final int FLAG_TOKEN_GROUPS = 0x20;
    private static final int LDAPS_PORT = 636;

    private static final int[] knownFlags = {
        FLAG_AD_MATCHING_RULE_IN_CHAIN,
        FLAG_DIRECT_GROUPS_ONLY,
        FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS,
        FLAG_ENABLE_SITE_AFFINITY,
        FLAG_FSP_GROUPS,
        FLAG_TOKEN_GROUPS};

    private static final HashMap<Integer, String> flagsToStrings;
    private static final HashMap<String, Integer> stringsToFlags;

    static
    {
        flagsToStrings = new HashMap<Integer, String>(knownFlags.length);
        flagsToStrings.put(FLAG_AD_MATCHING_RULE_IN_CHAIN, OPTION_FLAGS_MATCHING_RULE_IN_CHAIN);
        flagsToStrings.put(FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS, OPTION_FLAGS_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS);
        flagsToStrings.put(FLAG_DIRECT_GROUPS_ONLY, OPTION_FLAGS_DIRECT_GROUPS_ONLY);
        flagsToStrings.put(FLAG_ENABLE_SITE_AFFINITY, OPTION_FLAGS_ENABLE_SITE_AFFINITY);
        flagsToStrings.put(FLAG_FSP_GROUPS, OPTION_FLAGS_FSP_GROUPS);
        flagsToStrings.put(FLAG_TOKEN_GROUPS, OPTION_FLAGS_TOKEN_GROUPS);

        stringsToFlags = new HashMap<String, Integer>(knownFlags.length);
        stringsToFlags.put(OPTION_FLAGS_MATCHING_RULE_IN_CHAIN, FLAG_AD_MATCHING_RULE_IN_CHAIN);
        stringsToFlags.put(OPTION_FLAGS_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS, FLAG_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS);
        stringsToFlags.put(OPTION_FLAGS_DIRECT_GROUPS_ONLY, FLAG_DIRECT_GROUPS_ONLY);
        stringsToFlags.put(OPTION_FLAGS_ENABLE_SITE_AFFINITY, FLAG_ENABLE_SITE_AFFINITY);
        stringsToFlags.put(OPTION_FLAGS_FSP_GROUPS, FLAG_FSP_GROUPS);
        stringsToFlags.put(OPTION_FLAGS_TOKEN_GROUPS, FLAG_TOKEN_GROUPS);
    }

    private static final String CONFIGURATION_FILE_NAME_ARG = "configFile";
    private static final String OPTION_BOOLEAN_ARG = "Y/N";
    private static final String LOGON_BANNER_FILE_NAME_ARG = "logonBannerFile";





    public static void main(String[] args) {
        Options options = createCmdLineOptions();
        try
        {
            if (( args == null ) || (args.length < 1) )
            {
               printUsage(options);
            }
            else
            {
                preProcessArgs(args);
                CommandLineParser parser = new PosixParser();
                CommandLine cmdLine = parser.parse(options, args);

                if ( cmdLine.hasOption(OPTION_IMPORT) )
                {
                    importTenantConfiguration(getTenantName(cmdLine), getFileName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_EXPORT))
                {
                    exportTenantConfiguration(getTenantName(cmdLine), getFileName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_REGISTER_SP))
                {
                     register_sp(getTenantName(cmdLine), getFileName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_REMOVE_SP))
                {
                    remove_sp(getTenantName(cmdLine), getServiceProviderName(cmdLine));
               }
                else if (cmdLine.hasOption(OPTION_REGISTER_IDP))
                {
                    register_idp(getTenantName(cmdLine), getFileName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_REMOVE_IDP))
                {
                    remove_idp(getTenantName(cmdLine), getEntityId(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_JIT_SWITCH))
                {
                    enable_jit(getTenantName(cmdLine), getEntityId(cmdLine), getBooleanValue(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_GET_SSO_SAML2_METADATA))
                {
                    get_sso_saml2_metadata(getTenantName(cmdLine), getFileName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_GET_IDENTITY_SOURCES))
                {
                    getIdentitySources(getTenantName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_SET_DEFAULT_IDENTITY_SOURCES))
                {
                    setDefaultIdentitySources(getTenantName(cmdLine), getProviders(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_IDS_TEST_CONNECTION))
                {
                    testIdentitySourcesConnection();
                }
                else if (cmdLine.hasOption(OPTION_IDS_TEST_SUPPORTS_TLS))
                {
                    testIdentitySourceServerSupportsTls();
                }
                else if (cmdLine.hasOption(OPTION_GET_DOMAIN_JOIN_STATUS))
                {
                    displayDomainJoinStatusData();
                }
                else if (cmdLine.hasOption(OPTION_SET_IDENTITY_STORE_FLAGS))
                {
                    set_provider_flags( getTenantName(cmdLine), getProvider(cmdLine), getFlags(cmdLine) );
                }
                else if (cmdLine.hasOption(OPTION_GET_IDENTITY_STORE_FLAGS))
                {
                    get_provider_flags( getTenantName(cmdLine), getProvider(cmdLine) );
                }
                else if (cmdLine.hasOption(OPTION_GET_IDM_AUTH_STATS))
                {
                    get_idm_auth_stats( getTenantName(cmdLine), getActivityKind(cmdLine), getVerbose(cmdLine) );
                }
                else if (cmdLine.hasOption(OPTION_ADD_GROUP))
                {
                    add_group_to_tenants( getGroupName(cmdLine), getGroupDescription(cmdLine), getTenantNames(cmdLine) );
                }
                else if (cmdLine.hasOption(OPTION_CHANGE_USER_PWD))
                {
                    String password = "";
                    char[] currentPassword = System.console().readPassword("Enter current password: %s", password);
                    char[] newPassword = System.console().readPassword("Enter new password: %s", password);
                    change_user_password( getTenantName(cmdLine),
                            getUserName(cmdLine),
                            currentPassword,
                            newPassword );
                }
                else if (cmdLine.hasOption(OPTION_UPDATE_SYSTEM_DOMAIN_STORE_PWD))
                {
                    String password = "";
                    char[] newPassword = System.console().readPassword("Enter new password: %s", password);
                    update_system_domain_store_password( getTenantName(cmdLine), newPassword);
                }
                else if (cmdLine.hasOption(OPTION_GET_SSO_STATISTICS))
                {
                    get_sso_statistics(getTenantName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_REMOVE_JIT_USERS))
                {
                    remove_jit_users(getTenantName(cmdLine), getEntityId(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_ADD_CLAIM_GROUP_MAP))
                {
                    add_claim_group_mapping(getTenantName(cmdLine), getEntityId(cmdLine),
                            getClaimName(cmdLine), getClaimValue(cmdLine), getGroupName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_DELETE_CLAIM_GROUP_MAP))
                {
                    delete_claim_group_mapping(getTenantName(cmdLine), getEntityId(cmdLine),
                            getClaimName(cmdLine), getClaimValue(cmdLine), getGroupName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_SET_UPN_SUFFIX)) {
                    set_upn_suffix_for_external_idp(getTenantName(cmdLine), getEntityId(cmdLine), getUpnSuffix(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_GET_TC_CERT_AUTHN))
                {
                    get_TC_cert_authn();
                }
                else if (cmdLine.hasOption(OPTION_SET_TC_CERT_AUTHN))
                {
                    set_TC_cert_authn(getCaFiles(cmdLine) );
                }
                else if (cmdLine.hasOption(OPTION_GET_AUTHN_POLICY))
                {
                    get_authn_policy(getTenantName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_SET_AUTHN_POLICY))
                {
                    set_authn_policy(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_ADD_ALT_OCSP))
                {
                    add_alt_ocsp(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_GET_ALT_OCSP))
                {
                    get_alt_ocsp(getTenantName(cmdLine));
                }
                else if (cmdLine.hasOption(OPTION_DELETE_ALT_OCSP))
                {
                    delete_alt_ocsp(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_RSA_CONFIG))
                {
                    set_rsa_config(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_GET_RSA_CONFIG))
                {
                    get_rsa_config(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_RSA_SITE))
                {
                    set_rsa_site(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_RSA_USERID_ATTR_MAP))
                {
                    set_rsa_ids_userIDAttr_mapping(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_LOGON_BANNER))
                {
                    set_logon_banner(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_PRINT_LOGON_BANNER))
                {
                    print_logon_banner(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_DISABLE_LOGON_BANNER))
                {
                    diable_logon_banner(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_IDP_SELECTION_FLAG))
                {
                    set_idp_selection_flag(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_GET_LOCAL_IDP_ALIAS))
                {
                    get_local_idp_alias(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_LOCAL_IDP_ALIAS))
                {
                    set_local_idp_alias(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_GET_EXT_IDP_ALIAS))
                {
                    get_ext_idp_alias(cmdLine);
                }
                else if (cmdLine.hasOption(OPTION_SET_EXT_IDP_ALIAS))
                {
                    set_ext_idp_alias(cmdLine);
                }
                else
                {
                    printUsage(options);
                }
            }
        }
        catch(InvalidParameterException ex)
        {
            System.out.println(String.format("ERROR: %s", ex.getMessage()));
            printUsage(options);
        }
        catch(ParseException ex)
        {
            System.out.println(String.format("ERROR: %s", ex.getMessage()));
            printUsage(options);
        }
        catch (Exception e) {
            System.out.println(String.format("ERROR: %s", e.getMessage()));
            e.printStackTrace();
        }
    } //end of main

	/**
     * parse get_rsa_config command and display the configurations on terminal window
     * @param cmdLine
     * @throws Exception
     */
    private static void get_rsa_config(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String tenant = getTenantName(cmdLine);

        RSAAgentConfig rsaConfig = client.getRSAConfig(tenant);
        displayRSAConfig(rsaConfig);
    }

    /**
     * Display give rsa agent configuration
     * @param rsaConfig
     */
    private static void displayRSAConfig(RSAAgentConfig rsaConfig) {
        if (rsaConfig == null)
            return;

        displayParamNameAndValue(OPTION_RSA_LOGIN_GUIDE,
                rsaConfig.get_loginGuide());
        displayParamNameAndValue(OPTION_RSA_LOG_LEVEL,
                rsaConfig.get_logLevel().toString());
        displayParamNameAndValue(OPTION_RSA_LOG_FILE_SIZE,
                rsaConfig.get_logFileSize());
        displayParamNameAndValue(OPTION_RSA_MAX_LOG_FILE_COUNT,
                rsaConfig.get_maxLogFileCount());
        displayParamNameAndValue(OPTION_RSA_CONN_TIME_OUT,
                rsaConfig.get_connectionTimeOut());
        displayParamNameAndValue(OPTION_RSA_READ_TIME_OUT,
                rsaConfig.get_readTimeOut());
        displayParamNameAndValue(OPTION_RSA_ENC_ALG_LIST,
                rsaConfig.get_rsaEncAlgList().toString());
        displayParamNameAndValue("idsUserIDAttributeMaps",
                rsaConfig.get_idsUserIDAttributeMap() == null? null : rsaConfig.get_idsUserIDAttributeMap().toString());

        displayRSASites(rsaConfig.get_instMap());

    }

    private static void displayRSASites(HashMap<String, RSAAMInstanceInfo> get_instMap) {
        System.out.println(String.format("Sites [\n"));
        if (get_instMap != null && !get_instMap.isEmpty()) {
            for (String key : get_instMap.keySet()) {
                RSAAMInstanceInfo site = get_instMap.get(key);

                displayParamNameAndValue("["+OPTION_SITE_ID,
                        site.get_siteID());
                displayParamNameAndValue(OPTION_RSA_AGENT_NAME,
                        site.get_agentName());
                displayParamNameAndValue(OPTION_RSA_SDCONF_REC,
                        site.get_sdconfRec()!=null? "Binary value" : "Not set");
                displayParamNameAndValue(OPTION_RSA_SDOPTS_REC,
                        site.get_sdoptsRec()!=null? "Binary value" : "Not set");
            }
        }
        System.out.println(String.format("]\n"));
    }

    /**
     * Add or update the <IDS,userIDAttribute> key-value pair.
     * @param tenantName
     * @param entityId
     * @param claimName
     * @param claimValue
     * @param groupName
     * @throws Exception
     */
    /**
     * Parse set_rsa_userIDAttr_mappping command and set/update the  <IDS,userIDAttribute> key-value pair to directory
     * @param cmdLine
     * @throws Exception
     */
    private static void set_rsa_ids_userIDAttr_mapping(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String tenant = getTenantName(cmdLine);

        String siteID = cmdLine.getOptionValue(OPTION_SITE_ID,
                null);
        if (siteID == null) {
            //use the current site id if not specified.
            siteID = client.getClusterId();
        }

        String idsName = cmdLine.getOptionValue(OPTION_RSA_IDS_NAME,
                null);
        if (idsName == null) {
            //use the current site id if not specified.
            throw new InvalidArgumentException(String.format("Expecting argument %s", OPTION_RSA_IDS_NAME));
        }

        String ldapAttr = cmdLine.getOptionValue(OPTION_RSA_LDAP_ATTR,
                null);
        if (ldapAttr == null) {
            //use the current site id if not specified.
            throw new InvalidArgumentException(String.format("Expecting argument %s", OPTION_RSA_LDAP_ATTR));
        }

        RSAAgentConfig rsaConfig =  client.getRSAConfig(tenant);

        rsaConfig.add_idsUserIDAttributeMap(idsName, ldapAttr);
        client.setRSAConfig(tenant, rsaConfig);
    }

    private static void add_claim_group_mapping(String tenantName, String entityId,
            String claimName, String claimValue, String groupName) throws Exception {
        ValidateUtil.validateNotEmpty(entityId, "Entity ID");
        ValidateUtil.validateNotEmpty(claimName, "Claim Type");
        ValidateUtil.validateNotEmpty(claimValue, "Claim Value");
        ValidateUtil.validateNotEmpty(groupName, "Group Name");

        CasIdmClient client = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = client.getDefaultTenant().toString();
        }
        IDPConfig idpConfig = client.getExternalIdpConfigForTenant(tenantName, entityId);
        if (idpConfig == null) {
            throw new IDMException(String.format("Failed to add claim group mapping. "
                    + "External IDP %s is not found.", entityId));
        }
        TokenClaimAttribute tokenClaim = new TokenClaimAttribute(claimName, claimValue);
        Map<TokenClaimAttribute, List<String>> mappings = idpConfig.getTokenClaimGroupMappings();
        if (mappings == null) {
            mappings = new HashMap<TokenClaimAttribute, List<String>>();
        }
        String groupSid = client.findGroup(tenantName, groupName + "@" + client.getSystemTenant()).getObjectId();
        if (mappings.containsKey(tokenClaim)) {
            List<String> groupList = idpConfig.getTokenClaimGroupMappings().get(tokenClaim);
            if (groupList != null && !groupList.contains(groupSid)) {
                groupList.add(groupSid);
            }
        } else {
            List<String> groupList = new ArrayList<>();
            groupList.add(groupSid);
            idpConfig.getTokenClaimGroupMappings().put(tokenClaim, groupList);
        }
        client.setExternalIdpConfig(tenantName, idpConfig);
    }

    private static void delete_claim_group_mapping(String tenantName, String entityId,
            String claimName, String claimValue, String groupName) throws Exception {
        ValidateUtil.validateNotEmpty(entityId, "Entity ID");
        ValidateUtil.validateNotEmpty(claimName, "Claim Type");
        ValidateUtil.validateNotEmpty(claimValue, "Claim Value");
        ValidateUtil.validateNotEmpty(groupName, "Group Name");

        CasIdmClient client = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = client.getDefaultTenant().toString();
        }
        IDPConfig idpConfig = client.getExternalIdpConfigForTenant(tenantName, entityId);
        if (idpConfig == null) {
            throw new IDMException(String.format("Failed to add claim group mapping. "
                    + "External IDP %s is not found.", entityId));
        }
        String groupSid = client.findGroup(tenantName, groupName + "@" + client.getSystemTenant()).getObjectId();
        TokenClaimAttribute tokenClaim = new TokenClaimAttribute(claimName, claimValue);
        if (idpConfig.getTokenClaimGroupMappings().containsKey(tokenClaim)) {
            idpConfig.getTokenClaimGroupMappings().get(tokenClaim).remove(groupSid);
        }
        client.setExternalIdpConfig(tenantName, idpConfig);
    }

    private static void set_upn_suffix_for_external_idp(String tenantName, String entityId, String upnSuffix) throws Exception {
        ValidateUtil.validateNotEmpty(entityId, "Entity ID");
        ValidateUtil.validateNotEmpty(upnSuffix, "UPN suffix");
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = client.getDefaultTenant().toString();
        }
        IDPConfig idpConfig = client.getExternalIdpConfigForTenant(tenantName, entityId);
        if (idpConfig == null) {
            throw new IDMException(String.format("Failed to set upn suffix. "
                    + "External IDP %s is not found.", entityId));
        }
        idpConfig.setUpnSuffix(upnSuffix);
        client.setExternalIdpConfig(tenantName, idpConfig);
    }

    private static String[] getCaFiles(CommandLine cmdLine) {
        String inputFiles = cmdLine.getOptionValue(OPTION_CA_FILES, null);
        if (inputFiles != null) {
            return inputFiles.split(",");
        }
        return null;
    }

    private static String getTcStsInstanceConfDir() {
        String tomcatConfLocation;
        if(isLightwave()) {
            tomcatConfLocation = "/opt/vmware/vmware-sts/conf/";
        } else {
            tomcatConfLocation = "/usr/lib/vmware-sso/vmware-sts/conf/";
        }
        return SystemUtils.IS_OS_WINDOWS ? FileSystems.getDefault().getPath(System
                .getenv("VMWARE_RUNTIME_DATA_DIR"), "VMwareSTSService/conf/").toString()
                : tomcatConfLocation;
    }

    // Decision factor : The RPM name is vmware-sts in lightwave and vmware-identity-sts in Vsphere.
    // TODO : Add the product release in registry entry.
    private static boolean isLightwave() {

        boolean lightwave = true;
        String rpmInfo = null;
        Process p = null;
        try{
            p = Runtime.getRuntime().exec("rpm -qa vmware-sts");
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                    p.getInputStream()))) {
                rpmInfo = reader.readLine();
            }
        }
        catch (IOException e) {
            System.err.println("Failed to detect release");
            e.printStackTrace();
        }

        if(rpmInfo == null || rpmInfo.isEmpty()) {
            lightwave = false;
        }
        return lightwave;
    }

    private static String getTCServerConfigFileFullName() {
        return FileSystems.getDefault().getPath(getTcStsInstanceConfDir(), "server.xml").toString();
    }

    private static String getTCTrustedCAFileFullName() {
        return FileSystems.getDefault().getPath(getTcStsInstanceConfDir(), "trustedca.jks").toString();
    }

    private static void get_TC_cert_authn() throws Exception {
        String tcServerConfigFile = getTCServerConfigFileFullName();
        TCServerManager manager = new TCServerManager(tcServerConfigFile);

        displayParamNameAndValue(TCServerManager.trustStoreFileAttrName, manager.getAttrValue(TCServerManager.trustStoreFileAttrName));
        displayParamNameAndValue(TCServerManager.trustStorePasswordAttrName, manager.getAttrValue(TCServerManager.trustStorePasswordAttrName));
        displayParamNameAndValue(TCServerManager.trustStoreTypeAttrName, manager.getAttrValue(TCServerManager.trustStoreTypeAttrName));
    }

    private static void set_TC_cert_authn(String[] caFileNames) throws Exception {
        // Backup TomCat configuration file for roll back if needed.
        String tcServerConfigFile = getTCServerConfigFileFullName();
        String tcConfigOrig = tcServerConfigFile + ".orig";
        if(Files.exists(FileSystems.getDefault().getPath(tcServerConfigFile))){
            Files.copy(FileSystems.getDefault().getPath(tcServerConfigFile), new FileOutputStream(tcConfigOrig));
        }
        else
        {
            throw new IllegalStateException(String.format("TomCat server.xml not found at %s", tcServerConfigFile));
        }

        String defaultTrustedCaFile = getTCTrustedCAFileFullName();
        String defaultTrustedCaPassword = "changeme";
        String defaultTrustedCaType = "JKS";
        // Get trustedCaFile from TomCat configuration if it has been configured. Otherwise use the default value.
        TCServerManager tcManager = new TCServerManager(tcServerConfigFile);
        String trustedCaFile = tcManager
                .getAttrValue(TCServerManager.trustStoreFileAttrName);
        if (trustedCaFile == null) {
            tcManager.setAttrValue(TCServerManager.trustStoreFileAttrName, defaultTrustedCaFile);
            trustedCaFile = defaultTrustedCaFile;
        }
        // Get trustedCaPassword from TomCat configuration if it has been configured. Otherwise use the default value.
        String trustedCaPassword = tcManager
                .getAttrValue(TCServerManager.trustStorePasswordAttrName);
        if (trustedCaPassword == null) {
            tcManager.setAttrValue(TCServerManager.trustStorePasswordAttrName, defaultTrustedCaPassword);
            trustedCaPassword = defaultTrustedCaPassword;
        }
        // Get trustedCaType from TomCat configuration if it has been configured. Otherwise use the default value.
        String trustedCaType = tcManager
                .getAttrValue(TCServerManager.trustStoreTypeAttrName);
        if (trustedCaType == null) {
            tcManager.setAttrValue(TCServerManager.trustStoreTypeAttrName, defaultTrustedCaType);
            trustedCaType = defaultTrustedCaType;
        }

        // Backup trustedCAStore file for roll back if needed.
        String trustedCaFileOrig = trustedCaFile + ".orig";
        if(Files.exists(FileSystems.getDefault().getPath(trustedCaFile))){
            Files.copy(FileSystems.getDefault().getPath(trustedCaFile), new FileOutputStream(trustedCaFileOrig));
        }

        // Set attributes in tcManager.
        KeyStoreManager ksManager = null;
        // set clientCertAuth value to false in order to prompt the client certificate
        // only for the resourse/url configured in tomcat security realm
        tcManager.setAttrValue(TCServerManager.clientAuthAttrName, TCServerManager.clientAuthAttributeCertFalse);
        tcManager.setAttrValue(TCServerManager.trustStoreFileAttrName, trustedCaFile);
        tcManager.setAttrValue(TCServerManager.trustStorePasswordAttrName, trustedCaPassword);
        tcManager.setAttrValue(TCServerManager.trustStoreTypeAttrName, trustedCaType);
        // import trusted CA certificates to trustedCaFile.
        if(caFileNames != null){
            ksManager = new KeyStoreManager(trustedCaFile, trustedCaPassword, trustedCaType);
            ksManager.importCerts(caFileNames, null);
        }

        try
        {
            // Save the configuration change to server.xml files
            tcManager.saveToXmlFile();
            if(ksManager != null){
                ksManager.saveToXmlFile();
            }
        }
        catch(Exception e){
            try {
                // Roll back to original server.xml file.
                if (Files
                        .exists(FileSystems.getDefault().getPath(tcConfigOrig))) {
                    Files.copy(FileSystems.getDefault().getPath(tcConfigOrig),
                            new FileOutputStream(tcServerConfigFile));
                }
                // Roll back to original trustedca file.
                if (Files.exists(FileSystems.getDefault().getPath(
                        tcServerConfigFile))) {
                    Files.copy(
                            FileSystems.getDefault()
                                    .getPath(trustedCaFileOrig),
                            new FileOutputStream(trustedCaFile));
                }
            } catch (IOException ioe) {
                System.out.println(String.format("Rolling back failed with exception: %s", ioe.toString()));
                System.out.println(String.format("To restore original configuration, please mannually copy %s to %s", tcConfigOrig, tcServerConfigFile));
                System.out.println(String.format("  and copy %s to %s", trustedCaFileOrig, trustedCaFile));
            }

            throw e;
        }
    }

    private static void get_authn_policy(String tenant) throws Exception {

        // Display Authentication policy of tenant
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        AuthnPolicy tenantAuthnPolicy = client.getAuthnPolicy(tenant);
        System.out.println(String.format("Authentication policy on tenant : '%s' :" ,tenant));
        displayAuthenticationPolicy(tenantAuthnPolicy);

        // Display Authentication policy (if any) of identity sources belonging to tenant
        Collection<IIdentityStoreData> providers = client.getProviders(tenant);
        for(IIdentityStoreData provider : providers) {
             provider = client.getProviderWithInternalInfo(tenant, provider.getName());
             int[] providerAuthnTypes = provider.getExtendedIdentityStoreData().getAuthnTypes();
             if(providerAuthnTypes != null) {
                 System.out.println(String.format("Authentication policy on identity source : '%s' ",provider.getName()));
                 boolean passwdEnabled = false;
                 boolean windowsEnabled = false;
                 boolean certEnabled = false;
                 boolean rsaSecurIdEnabled = false;
                 if(ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_PASSWORD)) {
                     passwdEnabled = true;
                 }
                 if(ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_WINDOWS)) {
                     windowsEnabled = true;
                 }
                 if(ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE)) {
                     certEnabled = true;
                 }
                 if(ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID)) {
                     rsaSecurIdEnabled = true;
                 }
                 displayAuthnEnableStatus(passwdEnabled, windowsEnabled, certEnabled, rsaSecurIdEnabled);
            } else {
                System.out.println(String.format("Authentication policy for identity source : '%s' is not set", provider.getName()));
            }
        }
    }

    private static void displayAuthenticationPolicy(AuthnPolicy policy) throws Exception {

        displayAuthnEnableStatus(policy.IsPasswordAuthEnabled(),
                                 policy.IsWindowsAuthEnabled(),
                                 policy.IsTLSClientCertAuthnEnabled(),
                                 policy.IsRsaSecureIDAuthnEnabled());

        if (policy.getClientCertPolicy() != null) {
            displayParamNameAndValue("revocationCheckEnabled", policy.getClientCertPolicy().revocationCheckEnabled());
            displayParamNameAndValue("useOCSP", policy.getClientCertPolicy().useOCSP());
            displayParamNameAndValue("useCRLAsFailOver", policy.getClientCertPolicy().useCRLAsFailOver());
            URL url = policy.getClientCertPolicy().getOCSPUrl();
            if (null != url) {
                displayParamNameAndValue("OCSPUrl (obsoleted! Please use the \"-add_alt_ocsp command\" to add per-site alternative ocsp responder/responders)", url.toString());
            }
            displayAltOcsp(policy.getClientCertPolicy());
            displayParamNameAndValue("useCertCRL", policy.getClientCertPolicy().useCertCRL());
            url = policy.getClientCertPolicy().getCRLUrl();
            displayParamNameAndValue("CRLUrl", url == null ? null : url.toString());
            String[] oids = policy.getClientCertPolicy().getOIDs();
            if (oids != null) {
                for (String oid : oids) {
                    displayParamNameAndValue("oid", oid);
                }
            }
            Certificate[] trustedCAs = policy.getClientCertPolicy().getTrustedCAs();
            if(trustedCAs != null){
                for(Certificate c: trustedCAs){
                    displayParamNameAndValue("trustedCA", ((X509Certificate)c).getSubjectDN().toString());
                }
            }
        }
        System.out.println("\n\n");
    }

    private static void displayAuthnEnableStatus(boolean password, boolean windows, boolean cert, boolean rsaSecurID) {
        displayParamNameAndValue(DISPLAY_PARAM_PASSWORD_AUTH, password);
        displayParamNameAndValue(DISPLAY_PARAM_WINDOWS_AUTH, windows);
        displayParamNameAndValue(DISPLAY_PARAM_CERT_AUTH, cert);
        displayParamNameAndValue(DISPLAY_PARAM_RSA_SECURID_AUTH, rsaSecurID);
    }

    private static void set_authn_policy(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String providerName = cmdLine.getOptionValue(OPTION_PROVIDER, null);
        String tenant = getTenantName(cmdLine);

        boolean passwordEnabled = false;
        boolean windowsEnabled = false;
        boolean certEnabled = false;
        boolean rsaSecurIdEnbaled = false;

        String passwordAuthEnabled = cmdLine.getOptionValue(OPTION_AUTHN_PWD_ENABLED, null);
        String windowsAuthEnabled = cmdLine.getOptionValue(OPTION_AUTHN_WIN_ENABLED, null);
        String certAuthEnabled = cmdLine.getOptionValue(OPTION_AUTHN_CERT_ENABLED, null);
        String rsaSecurIdAuthEnabled = cmdLine.getOptionValue(OPTION_AUTHN_RSASECURID_ENABLED, null);

        if(providerName != null) {
            // Set authentication policy on identity provider
            System.out.println("Setting authentication policy for identity provider :" + providerName);
            IIdentityStoreData provider = client.getProviderWithInternalInfo(tenant, providerName);
            int[] providerAuthnTypes = provider.getExtendedIdentityStoreData().getAuthnTypes();

            passwordEnabled = passwordAuthEnabled != null ?
                              Boolean.parseBoolean(passwordAuthEnabled) : ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_PASSWORD);
            windowsEnabled = windowsAuthEnabled != null ?
                             Boolean.parseBoolean(windowsAuthEnabled) : ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_WINDOWS) ;
            certEnabled = certAuthEnabled != null ?
                          Boolean.parseBoolean(certAuthEnabled) : ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE);
            rsaSecurIdEnbaled = rsaSecurIdAuthEnabled != null ?
                                Boolean.parseBoolean(rsaSecurIdAuthEnabled) : ArrayUtils.contains(providerAuthnTypes, FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID);
            AuthnPolicy providerAuthnPolicy = new AuthnPolicy(passwordEnabled, windowsEnabled, certEnabled, rsaSecurIdEnbaled, null, null);
            client.setAuthnPolicyForProvider(tenant, providerName, providerAuthnPolicy);
        } else {
            // Set authentication policy on tenant
            System.out.println("Setting authentication policyon tenant :" + tenant);
            AuthnPolicy authnPolicy = client.getAuthnPolicy(tenant);

            passwordEnabled = passwordAuthEnabled != null ? Boolean.parseBoolean(passwordAuthEnabled) : authnPolicy.IsPasswordAuthEnabled();
            windowsEnabled = windowsAuthEnabled != null ? Boolean.parseBoolean(windowsAuthEnabled) : authnPolicy.IsWindowsAuthEnabled();
            certEnabled = certAuthEnabled != null ? Boolean.parseBoolean(certAuthEnabled) : authnPolicy.IsTLSClientCertAuthnEnabled();
            rsaSecurIdEnbaled = rsaSecurIdAuthEnabled != null ? Boolean.parseBoolean(rsaSecurIdAuthEnabled) : authnPolicy.IsRsaSecureIDAuthnEnabled();

            ClientCertPolicy certPolicy = new ClientCertPolicy();
            if (authnPolicy.getClientCertPolicy() != null) {
                certPolicy = authnPolicy.getClientCertPolicy();
            }

            String inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_REVOCATION_CHECK,
                    null);
            if (inputValue != null)
                certPolicy.setRevocationCheckEnabled(Boolean
                        .parseBoolean(inputValue));

            inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_USE_OCSP, null);
            if (inputValue != null)
                certPolicy.setUseOCSP(Boolean.parseBoolean(inputValue));

            inputValue = cmdLine.getOptionValue(
                    OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER, null);
            if (inputValue != null)
                certPolicy.setUseCRLAsFailOver(Boolean.parseBoolean(inputValue));

            inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_OCSP_URL, null);
            if (inputValue != null) {
                System.out.println("\"-ocspUrl\" option is obsoleted on \"-set_authn_policy\". " +
                        "Please use \"-add_alt_ocsp\" command for configuring alternative OCSP responder" + tenant);
            }

            inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_USE_CERT_CRL,
                    null);
            if (inputValue != null) {
                certPolicy.setUseCertCRL(Boolean.parseBoolean(inputValue));
            }

            inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_CRL_URL, null);
            if (inputValue != null) {
                certPolicy.setCRLUrl(inputValue.isEmpty() || inputValue.equalsIgnoreCase("null")? null : new URL(inputValue));
            }

            inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_OIDS, null);
            if (inputValue != null) {
                certPolicy.setOIDs(inputValue.isEmpty() || inputValue.equalsIgnoreCase("null")? null : inputValue.split(","));
            }

            inputValue = cmdLine.getOptionValue(OPTION_CA_FILES, null);
            if(inputValue != null){
                CertificateFactory certFactory = CertificateFactory
                        .getInstance("X.509");
                Collection<Certificate> certs = new ArrayList<Certificate>();
                String[] trustedCAFiles = inputValue.split(",");
                for (String fn : trustedCAFiles) {
                    if (fn != null) {
                        fn = fn.trim();
                        if (fn.startsWith("~" + File.separator)) {
                            fn = System.getProperty("user.home") + fn.substring(1);
                        }
                        Path path = Paths.get(fn);
                        InputStream inStream = new FileInputStream(path.toString());
                        certs.add(certFactory.generateCertificate(inStream));
                        inStream.close();
                    }
                }
                certPolicy.setTrustedCAs(certs.toArray(new Certificate[certs.size()]));
            }
            client.setAuthnPolicy(tenant, new AuthnPolicy(passwordEnabled, windowsEnabled, certEnabled, rsaSecurIdEnbaled, certPolicy,authnPolicy.get_rsaAgentConfig()));
        }
        System.out.println("Authentication policy is set successfully");
        get_authn_policy(tenant);
    }

    /**
     * Adding an alternative OCSP responder configuration
     * @param cmdLine
     * @throws Exception
     */
    private static void add_alt_ocsp(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String tenant = getTenantName(cmdLine);

        if(tenant.isEmpty()) {
            tenant = client.getDefaultTenant().toString();
        }

        System.out.println("Adding alternative OCSP responder for tenant :" + tenant);
        AuthnPolicy authnPolicy = client.getAuthnPolicy(tenant);

        ClientCertPolicy certPolicy = new ClientCertPolicy();
        if (authnPolicy.getClientCertPolicy() != null) {
            certPolicy = authnPolicy.getClientCertPolicy();
        }

        //Parse optional site ID
        String siteID = cmdLine.getOptionValue(OPTION_SITE_ID,
                null);
        if (siteID == null) {
            siteID = client.getClusterId();
        }

        //Parse required responder URL option.
        String urlStr = cmdLine.getOptionValue(OPTION_AUTHN_CERT_OCSP_URL, null);
        Validate.notEmpty(urlStr, "ocspUrl is required for this command!");

        URL ocspUrl;
        try {
            ocspUrl = new URL(urlStr);
        } catch (MalformedURLException e) {
            System.out.format("MalformedURLException for url %s:", urlStr);
            return;
        }

        X509Certificate ocspSignCert = null;

        //Parse optional signing cert
        String inputValue = cmdLine.getOptionValue(OPTION_AUTHN_CERT_OCSP_CERT, null);
        if(inputValue != null){
            CertificateFactory certFactory = CertificateFactory
                    .getInstance("X.509");
            String[] certFiles = inputValue.split(",");

            //takes the first cert only if multiple certificates file are provided in cli.
            String certStr = certFiles[0];

            if (certStr != null) {
                certStr = certStr.trim();
                if (certStr.startsWith("~" + File.separator)) {
                    certStr = System.getProperty("user.home") + certStr.substring(1);
                }
                Path path = Paths.get(certStr);
                InputStream inStream = new FileInputStream(path.toString());
                ocspSignCert = (X509Certificate) certFactory.generateCertificate(inStream);
                inStream.close();
            }
        }

        //Retrieve current site AlternativeOCSPList
        HashMap<String, AlternativeOCSPList> ocspMap = certPolicy.get_siteOCSPList();
        if (ocspMap == null) {
            ocspMap = new HashMap<String, AlternativeOCSPList>();
        }

        AlternativeOCSPList siteAltOCSPList = ocspMap.get(siteID);
        if (siteAltOCSPList == null) {
            siteAltOCSPList = new AlternativeOCSPList(siteID, null);
        }

        //Add the responder info to the list for the site
        siteAltOCSPList.addAlternativeOCSP(new AlternativeOCSP(ocspUrl, ocspSignCert));
        ocspMap.put(siteID, siteAltOCSPList);

        certPolicy.set_siteOCSPMap(ocspMap);

        client.setAuthnPolicy(tenant, new AuthnPolicy(authnPolicy.IsPasswordAuthEnabled(),
                authnPolicy.IsWindowsAuthEnabled(), authnPolicy.IsTLSClientCertAuthnEnabled(),
                authnPolicy.IsRsaSecureIDAuthnEnabled(), certPolicy,authnPolicy.get_rsaAgentConfig()));

        System.out.println("OCSP reponder is added successfully!");
        displayAltOcspSite(siteAltOCSPList);
    }

   
    /**
     * Display current configuration of alternative OCSP responders for smart card authentication.
     * 
     * @param tenantName
     * @throws Exception
     */
    private static void get_alt_ocsp(String tenantName) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        AuthnPolicy tenantAuthnPolicy = client.getAuthnPolicy(tenantName);

        if (null != tenantAuthnPolicy) {
            displayAltOcsp(tenantAuthnPolicy.getClientCertPolicy());
        } else {
            System.out.println("No AuthnPolicy configured");
            return;
        }
	}

    /**
     * Display current configuration of alternative OCSP responders for smart card authentication.
     *
     * @param cmdLine
     * @throws Exception
     */
    private static void displayAltOcsp(ClientCertPolicy tenantClientCertPolicy) throws Exception {

        if (tenantClientCertPolicy == null) {
            System.out.println("No ClientCertPolicy configured");
            return;
        }

        System.out.println("Alternative OCSP responders:");

        HashMap<String, AlternativeOCSPList> ocspSiteMap = tenantClientCertPolicy.get_siteOCSPList();
        if (ocspSiteMap != null) {

            for (String key : ocspSiteMap.keySet()) {
                displayAltOcspSite(ocspSiteMap.get(key));
            }
        }
    }

	/**
     * Display site-specific configuration of alternative OCSP responders for smart card authentication.
     * @param alternativeOCSPList
     */
    private static void displayAltOcspSite(AlternativeOCSPList alternativeOCSPList) {


        System.out.println("[");
        displayParamNameAndValue("site", alternativeOCSPList.get_siteID());
        for (AlternativeOCSP altOCSP : alternativeOCSPList.get_ocspList()) {
            System.out.println("    [");
            displayParamNameAndValue("    OCSP url", altOCSP.get_responderURL().toString());
            X509Certificate cert =  altOCSP.get_responderSigningCert();
            displayParamNameAndValue("    OCSP signing CA cert", (cert != null)? "binary value]": "not set");
            System.out.println("    ]");
        }
        System.out.println("]");

    }

    /**
     * Delete alternative OCSP responder configuration for one site or all sites.
     * @param cmdLine
     * @throws Exception
     */
    private static void delete_alt_ocsp(CommandLine cmdLine) throws Exception {
        String tenant = getTenantName(cmdLine);
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        if(tenant.isEmpty()) {
            tenant = client.getDefaultTenant().toString();
        }

        System.out.println("Deleting alternative OCSP responder for tenant :" + tenant);
        AuthnPolicy authnPolicy = client.getAuthnPolicy(tenant);

        ClientCertPolicy certPolicy = new ClientCertPolicy();
        if (authnPolicy.getClientCertPolicy() != null) {
            certPolicy = authnPolicy.getClientCertPolicy();
        }

        //delete all if asked
        String inputVal = getValueByOption(cmdLine, OPTION_ALL_SITES);
        if (inputVal != null) {
            System.out.println("Deleting alternative OCSP responder configurations for all sites!");
            //Note: to support backward compatibility to Admin API which current does not recognize ocsp map,
            //the set would ignore null but honor if the map is empty.
            certPolicy.set_siteOCSPMap(new HashMap<String, AlternativeOCSPList>());
        } else {

            String siteID = cmdLine.getOptionValue(OPTION_SITE_ID,
                    null);
            if (siteID == null) {
                siteID = client.getClusterId();
            }
            System.out.println("Deleting alternative OCSP responder configurations for site: "+siteID);

            HashMap<String, AlternativeOCSPList> ocspSiteMap = certPolicy.get_siteOCSPList();
            if (null != ocspSiteMap) {
                ocspSiteMap.remove(siteID);
            }
        }
        client.setAuthnPolicy(tenant, new AuthnPolicy(authnPolicy.IsPasswordAuthEnabled(),
                authnPolicy.IsWindowsAuthEnabled(), authnPolicy.IsTLSClientCertAuthnEnabled(),
                authnPolicy.IsRsaSecureIDAuthnEnabled(), certPolicy,authnPolicy.get_rsaAgentConfig()));

        displayAltOcsp(certPolicy);

    }
    private static void set_rsa_config(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String tenant = getTenantName(cmdLine);

        //update on existing config if it was defined.
        RSAAgentConfig rsaConfig =  client.getRSAConfig(tenant);

        //create rsaConfig if this is a new configuration
        if (rsaConfig == null) {
            rsaConfig = new RSAAgentConfig();
        }

        String inputValue = cmdLine.getOptionValue(OPTION_RSA_LOGIN_GUIDE, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_loginGuide(inputValue);
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_LOG_LEVEL, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_logLevel(inputValue.isEmpty()? null : RSAAgentConfig.RSALogLevelType.valueOf(inputValue));
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_LOG_FILE_SIZE, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_logFileSize(Integer.valueOf(inputValue));
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_MAX_LOG_FILE_COUNT, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_maxLogFileCount(Integer.valueOf(inputValue));
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_CONN_TIME_OUT, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_connectionTimeOut( Integer.valueOf(inputValue));
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_READ_TIME_OUT, null);
        if (inputValue != null && !inputValue.isEmpty()) {
            rsaConfig.set_readTimeOut( Integer.valueOf(inputValue));
        }

        inputValue = cmdLine.getOptionValue(OPTION_RSA_ENC_ALG_LIST, null);
        if (inputValue != null) {
            Set<String> algSet = new HashSet<String>(Arrays.asList(inputValue.split(",")));
            rsaConfig.set_rsaEncAlgList(inputValue.isEmpty() || inputValue.equalsIgnoreCase("null")?
                    null : algSet);
        }

        client.setRSAConfig(tenant, rsaConfig);
    }

    private static void set_rsa_site(CommandLine cmdLine) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        String tenant = getTenantName(cmdLine);

        String siteID = cmdLine.getOptionValue(OPTION_SITE_ID,
                null);
        if (siteID == null) {
            //use the current site id if not specified.
            siteID = client.getClusterId();
        }

        //update on existing config if it was defined.
        RSAAgentConfig rsaConfig =  client.getRSAConfig(tenant);

        //create rsaConfig if this is a new configuration
        if (rsaConfig == null) {
            rsaConfig = new RSAAgentConfig();
        }

        RSAAMInstanceInfo instInfo = rsaConfig.get_instMap().get(siteID);

        //read agent name
        String agentName = cmdLine.getOptionValue(OPTION_RSA_AGENT_NAME, null);
        if (agentName == null) {
            if (instInfo == null) {
                throw new IllegalArgumentException(
                        String.format("%s is required for new rsa site configuration.", OPTION_RSA_AGENT_NAME));
            } else {
                agentName = instInfo.get_agentName();
            }
        }

        //read sdconf_rec file
        String sdConfFilePath = cmdLine.getOptionValue(OPTION_RSA_SDCONF_REC, null);
        byte[] sdConfData;

        //validate and update rsaConfig
        if (sdConfFilePath == null) {
            if (instInfo == null) {
                throw new IllegalArgumentException(
                        String.format("%s is required for new rsa site configuration.", OPTION_RSA_SDCONF_REC));
            } else {
                sdConfData = instInfo.get_sdconfRec();
            }
        } else {
            Path sdConfPath = Paths.get(sdConfFilePath);
            sdConfData = Files.readAllBytes(sdConfPath);
        }

        //read sdopts file
        String inputValue = cmdLine.getOptionValue(OPTION_RSA_SDOPTS_REC, null);
        byte[] sdOptsData = null;
        //validate and update rsaConfig
        if (inputValue != null && !inputValue.isEmpty() ) {
            if ( inputValue.equals("null")) {
                //remove the file path
                sdOptsData = null;
            } else {
                Path sdOptsPath = Paths.get(inputValue);
                sdOptsData = Files.readAllBytes(sdOptsPath);
            }
        } else {
            if (instInfo != null) {
                //optional setting not provided. read from the current setting
                sdOptsData = instInfo.get_sdoptsRec();
            }
        }
        RSAAMInstanceInfo newInstInfo = new RSAAMInstanceInfo(siteID, agentName, sdConfData, sdOptsData);
        client.addRSAInstanceInfo(tenant, newInstInfo);
    }

    private static void get_idm_auth_stats(String tenant, ActivityKind activity, Boolean verbose) throws Exception {
        CasIdmClient client = new CasIdmClient(HOSTNAME);
        List<IIdmAuthStat> stats = client.getIdmAuthStats(tenant);
        printAuthStats(stats, tenant, activity, verbose);
    }

    private static Boolean getVerbose(CommandLine cmdLine) {
        String v = cmdLine.getOptionValue(OPTION_VERBOSE, "").trim();
        if (v.equalsIgnoreCase("YES") ||
            v.equalsIgnoreCase("Y") ||
            v.equalsIgnoreCase("TRUE")) {
            return true;
        }

        return false;
    }

    private static void set_logon_banner(CommandLine cmdLine) throws Exception {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        String tenantName = getTenantName(cmdLine);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        if (cmdLine.hasOption(OPTION_LOGON_BANNER_TITLE)) {
            String title = getValueByOption(cmdLine, OPTION_LOGON_BANNER_TITLE);
            idmClient.setLogonBannerTitle(tenantName, title);
        }

        if (cmdLine.getArgs() != null && cmdLine.getArgs().length > 0) {
            String logonBannerFileName = getFileName(cmdLine);
            Scanner sc = null;
            try {
                sc = new Scanner(new File(logonBannerFileName));
                String logonBannerContent = sc.useDelimiter("\\Z").next();
                idmClient.setLogonBannerContent(tenantName, logonBannerContent);
            } finally {
                if (sc != null) {
                    sc.close();
                }
            }
        }

        if (cmdLine.hasOption(OPTION_ENABLE_LOGON_BANNER_CHECKBOX)) {
            String checkboxOptionStr = getValueByOption(cmdLine, OPTION_ENABLE_LOGON_BANNER_CHECKBOX);
            // default false if option is not provided
            boolean enableCheckbox = false;
            if (!checkboxOptionStr.isEmpty()) {
                if (checkboxOptionStr.equalsIgnoreCase("y")) {
                    enableCheckbox = true;
                } else if (!checkboxOptionStr.equalsIgnoreCase("n")) {
                    throw new IllegalArgumentException("Invalid value. 'Y' to enable, 'N' to disable.");
                }
            }
            idmClient.setLogonBannerCheckboxFlag(tenantName, enableCheckbox);
        }
    }

    private static void print_logon_banner(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        String logonBannerTitle = idmClient.getLogonBannerTitle(tenantName);
        String logonBannerContent = idmClient.getLogonBannerContent(tenantName);

        System.out.println("Logon Banner Title:");
        System.out.println(logonBannerTitle + System.lineSeparator());
        System.out.println("Logon Banner Content:");
        System.out.println(logonBannerContent  + System.lineSeparator());
        System.out.println("Checkbox enabled : " + idmClient.getLogonBannerCheckboxFlag(tenantName));
    }

    private static void diable_logon_banner(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.disableLogonBanner(tenantName);
    }

    private static void set_idp_selection_flag(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.setTenantIDPSelectionEnabled(tenantName, getBooleanValue(cmdLine));
    }

    private static void get_local_idp_alias(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        String alias = idmClient.getLocalIDPAlias(tenantName);

        System.out.println("Alias: " + alias == null ? "NULL" : alias);
    }

    private static void set_local_idp_alias(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        String alias = getValueByOption(cmdLine, OPTION_ALIAS);

        // alias cannot be empty string on server side, set to null if empty
        idmClient.setLocalIDPAlias(tenantName, alias.isEmpty() ? null : alias);
    }

    private static void get_ext_idp_alias(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        String entityId = getEntityId(cmdLine);
        String alias = idmClient.getExternalIDPAlias(tenantName, entityId);

        System.out.println("Alias: " + alias == null ? "NULL" : alias);
    }

    private static void set_ext_idp_alias(CommandLine cmdLine) throws Exception {
        String tenantName = getTenantName(cmdLine);
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        String entityId = getEntityId(cmdLine);
        String alias = getValueByOption(cmdLine, OPTION_ALIAS);

        // alias cannot be empty string on server side, set to null if empty
        idmClient.setExternalIDPAlias(tenantName, entityId, alias.isEmpty() ? null : alias);
    }

    /**
     * Add a group to a collection of tenants.
     *
     * If the list of tenants is empty, applies this change to all tenants
     *
     * @param tenants
     * @throws Exception
     */
    private static void add_group_to_tenants(String groupName, String description, Collection<String> tenants) throws Exception {
       CasIdmClient client = new CasIdmClient(HOSTNAME);

       if (tenants.isEmpty())
       {
           tenants = client.getAllTenants();
       }

       GroupDetail detail = new GroupDetail(description);

       for (String tenantName : tenants) {
          try
          {
              client.addGroup(tenantName, groupName, detail);
          }
          catch (InvalidPrincipalException e)
          {
              // The group already existed on that tenant - we can ignore this
          }
       }

    }

    private static void printAuthStats(List<IIdmAuthStat> stats, String tenant,
            ActivityKind activity, Boolean verbose) {
        int line = 0;
        String prefix1 = "    ";

        for (IIdmAuthStat stat : stats) {

            if (stat.getOpKind() != activity) {
                continue;
            }

            System.out.format("No. %d:", line++);
            System.out.println();
            System.out.print(prefix1);
            System.out.print("[Tenant:");
            System.out.print(tenant);
            System.out.println("] ");

            System.out.print(prefix1);
            System.out.print("[OpKind:");
            System.out.print(stat.getOpKind());
            System.out.println("] ");

            System.out.print(prefix1);
            System.out.print("[UserName:");
            System.out.print(stat.getUserName());
            System.out.println("] ");

            System.out.print(prefix1);
            System.out.print("[StartTime:");
            System.out.print(new Date(stat.getStartTime()));
            System.out.println("] ");

            System.out.print(prefix1);
            System.out.print("[TimeTaken:");
            System.out.print(stat.getTimeTaken());
            System.out.println(" Ms] ");

            Map<String, String> ext = stat.getExtensions();
            for (String key : ext.keySet()) {
                System.out.println(prefix1 + "[" + key + "]: " + ext.get(key));
            }

            if (verbose) {
                System.out.print(prefix1);
                System.out.print("[ProviderType:");
                System.out.print(stat.getProviderType());
                System.out.println("] ");

                System.out.print(prefix1);
                System.out.print("[ProviderName:");
                System.out.print(stat.getProviderName());
                System.out.println("] ");

                System.out.print(prefix1);
                printOutProviderFlags(stat.getProviderFlag());
            }

            printLdapQueries(prefix1, stat.getLdapQueryStats(), verbose);

            System.out.println();
        }

        System.out.println();
    }

    private static void printLdapQueries(String prefix1, List<ILdapQueryStat> ldapQueries, boolean verbose){
        if(ldapQueries.size() == 0) {
            return;
        }

        int line = 0;
        String prefix2 = "    " + prefix1;
        System.out.println(prefix1 + "LdapQueries [Begin]:");
        if(verbose){
            for (ILdapQueryStat r : ldapQueries) {
                System.out.format(prefix1 + "No. %d:", line++);
                System.out.println();
                System.out.print(prefix2);
                System.out.print("[BaseDN:");
                System.out.print(r.getBaseDN());
                System.out.println("] ");

                System.out.print(prefix2);
                System.out.print("[ConnectionString:");
                System.out.print(r.getConnectionString());
                System.out.println("] ");

                System.out.print(prefix2);
                System.out.print("[QueryString:");
                System.out.print(r.getQueryString());
                System.out.println("] ");

                System.out.print(prefix2);
                System.out.print("[TimeTakenInMs:");
                System.out.print(r.getTimeTakenInMs());
                System.out.println(" Ms] ");

                System.out.print(prefix2);
                System.out.print("[The Number of Query:");
                System.out.print(r.getCount());
                System.out.println("] ");
                System.out.println();
            }
        }
        else
        {
            int numOfQueries = 0;
            long timeTakenInMs = 0;
            for(ILdapQueryStat stat : ldapQueries)
            {
                numOfQueries += stat.getCount();
                timeTakenInMs += stat.getTimeTakenInMs();
            }

            System.out.print(prefix2);
            System.out.print("[The number of LDAP queries:");
            System.out.print(numOfQueries);
            System.out.println("] ");

            System.out.print(prefix2);
            System.out.print("[Total time taken:");
            System.out.print(timeTakenInMs);
            System.out.println(" ms] ");

        }
        System.out.println(prefix1 + "LdapQueries [End]");
    }

    /**
     * <p>
     * Retrieve identity source information assigned to given tenant.
     * <li> If optional param tenant is not provided, Use the associated system default tenant</li>
     * <li> If optional param tenant is provided, use the given tenant name</li>
     *
     * @param tenantName
     *          Name of tenant (This is an optional parameter)
     * @param xmlFile
     *          Xml file of tenant. (Mandatory parameter)
     * @throws Exception
     */
    private static void getIdentitySources(String tenantName) throws Exception{
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }
        ArrayList<IIdentityStoreData> identitySources = new ArrayList<IIdentityStoreData>(idmClient.getProviders(tenantName));
        System.out.println(String.format("Total number of identitysources retrieved for tenant:%s : %d",tenantName,identitySources.size()));
        System.out.println(String.format("(If the value is undefined against a param, then you might notice \"%s\" against it.)",UNDEFINED_CONFIG));
        // TODO : Currently we are displaying on standard output. We can log this to a file if required.
        for(IIdentityStoreData identitySource : identitySources){
            System.out.println(""); // new line
            System.out.println("********** IDENTITY SOURCE INFORMATION **********");
            displayParamNameAndValue("IdentitySourceName", identitySource.getName());
            displayParamNameAndValue("DomainType", identitySource.getDomainType().name() != null ? identitySource.getDomainType().name() : StringUtils.EMPTY);

            // Display extendedIdentityStoreData info
            if(identitySource.getExtendedIdentityStoreData() != null) {
                displayIdentityStoreData(identitySource.getExtendedIdentityStoreData());
            }
        }
    }

    private static void setDefaultIdentitySources(String tenantName, Collection<String> providers) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.setDefaultProviders(tenantName, providers);
    }

    private static Collection<String> getProviders(CommandLine cmdLine)
    {
         String[] providers = cmdLine.getOptionValues(OPTION_PROVIDER);

         if (providers != null)
         {
             return Arrays.asList(providers);
         }

         return new ArrayList<String>();
    }

    /**
     * Display the given DomainStatus data
     *
     * @param tenantname
     */
    private static void displayDomainJoinStatusData() throws Exception{
        try {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        ActiveDirectoryJoinInfo joinInfo = idmClient.getActiveDirectoryJoinStatus();
        System.out.println("********** DOMAIN JOIN INFORMATION **********\n");
        displayParamNameAndValue("Domain Join Status",joinInfo.getJoinStatus().toString());
        if(joinInfo.getName()!= null)
            displayParamNameAndValue("Domain Name",joinInfo.getName());
        if(joinInfo.getAlias() != null)
            displayParamNameAndValue("Domain Alias",joinInfo.getAlias());
        if(joinInfo.getDn()!= null)
            displayParamNameAndValue("DN",joinInfo.getDn());
        Collection<DomainTrustsInfo> trustsInfo = idmClient.getDomainTrustInfo();
        if(trustsInfo != null)
        {
            Iterator<DomainTrustsInfo> it = trustsInfo.iterator();
            System.out.println("\n********** TRUSTED DOMAINS INFORMATION**********\n");
            System.out.println("\nNO OF TRUSTED DOMAINS : "+trustsInfo.size());
            while(it.hasNext())
            {
                DomainTrustsInfo e = it.next();
                if(e != null && e.dcInfo != null)
                {
                    displayParamNameAndValue("\nDOMAIN NAME", e.dcInfo.domainName);
                    displayParamNameAndValue("    Domain NetBiosName", e.dcInfo.domainNetBiosName);
                    displayParamNameAndValue("    Domain IPAddress", e.dcInfo.domainIpAddress);
                    displayParamNameAndValue("    Domain FQDN", e.dcInfo.domainFQDN);
                    displayParamNameAndValue("    Domain DnsForestName", e.dcInfo.domainDnsForestName);
                    displayParamNameAndValue("    Domain isInBound", e.IsInBound ? "True" : "False");
                    displayParamNameAndValue("    Domain IsOutBound", e.IsOutBound ? "True" : "False");
                    displayParamNameAndValue("    Domain IsInforest", e.IsInforest ? "True" : "False");
                    displayParamNameAndValue("    Domain IsPrimary", e.IsPrimary ? "True" : "False");
                    displayParamNameAndValue("    Domain IsNativeMode", e.IsNativeMode ? "True" : "False");
                    displayParamNameAndValue("    Domain isExternal", e.isExternal ? "True" : "False");
                }
            }
        }
        else
        {
            System.out.println("\n**********NO TRUSTED DOMAINS INFORMATION**********");
        }
        }
        catch(Exception ex) {
           System.out.println("Display domain JoinStatus and TrustInfo failed : " + ex.getMessage());
           ex.printStackTrace();
        }
    }

    /**
     * Display the given identitystore data
     *
     * @param identityStore
     */
    private static void displayIdentityStoreData(IIdentityStoreDataEx identityStore){
        displayParamNameAndValue("alias", identityStore.getAlias());
        displayParamNameAndValue("authenticationType", identityStore.getAuthenticationType() != null ? identityStore.getAuthenticationType().name() : StringUtils.EMPTY);
        displayParamNameAndValue("userBaseDN",identityStore.getUserBaseDn());
        displayParamNameAndValue("groupBaseDN", identityStore.getGroupBaseDn());
        displayParamNameAndValue("username",identityStore.getUserName());
        displayParamNameAndValue("providerType", identityStore.getProviderType() != null ? identityStore.getProviderType().name() : StringUtils.EMPTY);
        displayParamNameAndValue("servicePrincipalName",identityStore.getServicePrincipalName());

        Map<String,String> attributeMap = identityStore.getAttributeMap();
        if(attributeMap != null){
            System.out.println("****** Attributes ******");
            for(Entry<String,String> entry : attributeMap.entrySet()){
                displayParamNameAndValue(entry.getKey(), entry.getValue());
            }
        }

        System.out.println("****** Connection Strings ******");
        for(String connectionString : identityStore.getConnectionStrings()){
            displayParamNameAndValue("connectionString", connectionString);
        }

        Set<String> upnSuffixes = identityStore.getUpnSuffixes();
        if(upnSuffixes != null){
            System.out.println("****** UPN Suffixes ******");
            for(String upnSuffix : upnSuffixes){
               displayParamNameAndValue("upnSuffix", upnSuffix);
            }
        }

        displayParamNameAndValue("SearchTimeoutInSeconds",String.valueOf(identityStore.getSearchTimeoutSeconds()));


        IdentityStoreSchemaMapping identityStoreSchemaMapping = identityStore.getIdentityStoreSchemaMapping();
        if(identityStoreSchemaMapping != null){
            System.out.println("******* IdentityStore Schema Mappings ******");
            ArrayList<IdentityStoreObjectMapping> identityStoreObjectMappings = new ArrayList<IdentityStoreObjectMapping>(identityStoreSchemaMapping.getObjectMappings());
            for(IdentityStoreObjectMapping identityObjectMapping : identityStoreObjectMappings){
                System.out.println("---- Identity Object -----");
                displayParamNameAndValue("objectClass", identityObjectMapping.getObjectClass());
                displayParamNameAndValue("objectId", identityObjectMapping.getObjectId());
                ArrayList<IdentityStoreAttributeMapping> identityAttributeMappings = new ArrayList<IdentityStoreAttributeMapping>(identityObjectMapping.getAttributeMappings());
                for(IdentityStoreAttributeMapping identityAttributeMapping : identityAttributeMappings){
                    displayParamNameAndValue("attributeId", identityAttributeMapping.getAttributeId());
                    displayParamNameAndValue("attributeName", identityAttributeMapping.getAttributeName());
                }
            }
        }

        printOutProviderFlags(identityStore.getFlags());
    }

    /**
     * <p>
     * Output the given parameter and value to Standard output.
     * </p>
     * @param paramName
     *     Name of parameter to display
     * @param paramValue
     *     Value of the given param. If the value is null or empty, then {@value SsoConfig.UNDEFINED_CONFIG} is used.
     */
    private static void displayParamNameAndValue(String paramName, String paramValue){
        if(StringUtils.isEmpty(paramValue) || paramValue == null){
            paramValue=UNDEFINED_CONFIG;
        }
        System.out.println(String.format("%s:   %s", paramName,paramValue));
    }

    /**
     * <p>
     * Output the given parameter and value to Standard output.
     * </p>
     * @param paramName
     *     Name of parameter to display
     * @param paramValue
     *     Value of the given param.
     */
    private static void displayParamNameAndValue(String paramName, boolean paramValue){
        System.out.println(String.format("%s:   %b", paramName, paramValue));
    }

    /**
     * <p>
     * Output the given parameter and value to Standard output.
     * </p>
     * @param paramName
     *     Name of parameter to display
     * @param paramValue
     *     Value of the given param.
     */
    private static void displayParamNameAndValue(String paramName, int paramValue){
        System.out.println(String.format("%s:   %d", paramName, paramValue));
    }

      /**
     * import tenant configuration defined in a configuration file.
     *  @param tenantName    name string, optional.
     *  @param xmlFile        xml file name, required.
      * @throws Exception
     */
    private static void importTenantConfiguration(String tenantName, String xmlFile ) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        Document doc = readDomDoc(xmlFile);

        //Import tenantConfiguration does not create new tenant any more.
        try {
            idmClient.getTenant(tenantName);
        }
        catch (NoSuchTenantException e) {
            throw e;
        }

        idmClient.importTenantConfiguration(tenantName, doc);
    }

     /**
     * export tenant configuration defined to a configuration file.
     *  @param tenantName    name string, optional.
     *  @param xmlFile        xml file name, required.
      * @throws Exception
     */
    private static void exportTenantConfiguration(String tenantName, String xmlFile ) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (xmlFile.isEmpty()) {
            throw new InvalidParameterException();
        }
        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        Document tenantDoc = idmClient.exportTenantConfiguration(tenantName, true);
        writeDomDoc( tenantDoc, xmlFile );
    }

    private static void register_sp(String tenantName, String xmlFile ) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        Document doc = readDomDoc(xmlFile);

        // TODO: this is weird that import tenant config does all sorts of input now ...
        // we should make distinction....
        idmClient.importTenantConfiguration(tenantName, doc);
    }

    private static void register_idp(String tenantName, String xmlFile ) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        Document doc = readDomDoc(xmlFile);

        idmClient.importExternalIDPConfiguration(tenantName, doc);
    }

    private static void remove_idp(String tenantName, String entityId) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.removeExternalIdpConfig(tenantName, entityId);
    }

    private static void remove_sp(String tenantName, String sp) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.deleteRelyingParty(tenantName, sp);
    }

    private static void enable_jit(String tenantName, String entityId, boolean enableJit) throws Exception {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        IDPConfig idpConfig = idmClient.getExternalIdpConfigForTenant(tenantName, entityId);
        if (idpConfig == null) {
            throw new IDMException(String.format("Failed to enable/disable Jit. "
                    + "External IDP %s is not found.", entityId));
        }
        idpConfig.setJitAttribute(enableJit);
        idmClient.setExternalIdpConfig(tenantName, idpConfig);
    }

    private static void change_user_password(String tenantName, String userName, char[] currentPassword, char[] newPassword) throws Exception
    {
        if (currentPassword == null || newPassword == null) {
            throw new InvalidParameterException("Current/new passwords should be provided.");
        }

        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.changeUserPassword(tenantName, userName, currentPassword, newPassword);
    }

    private static void update_system_domain_store_password(String tenantName, char[] newPassword) throws Exception
    {
        if (newPassword == null) {
            throw new InvalidParameterException("New password should be provided.");
        }

        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.updateSystemDomainStorePassword(tenantName, newPassword);
    }

    private static void get_sso_saml2_metadata(String tenantName, String xmlFile ) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);

        if (xmlFile.isEmpty()) {
            throw new InvalidParameterException();
        }
        if (tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        Document tenantDoc = idmClient.getSsoSaml2Metadata(tenantName);
        writeDomDoc( tenantDoc, xmlFile );
    }

    private static void set_provider_flags(String tenantName, String providerName, int flags) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        IIdentityStoreData ids = idmClient.getProvider(tenantName, providerName);
        if ( ids.getDomainType() != DomainType.EXTERNAL_DOMAIN )
        {
            throw new InvalidParameterException(
                String.format(
                    "Provider [%s] represents [%s] domain type. Only DomainType.EXTERNAL_DOMAIN is currently supported for set provider flags.",
                    providerName, ids.getDomainType().name())
            );
        }

        IIdentityStoreData updatedIds = IdentityStoreData.CreateExternalIdentityStoreData(
                ids.getName(),
                ids.getExtendedIdentityStoreData().getAlias(),
                ids.getExtendedIdentityStoreData().getProviderType(),
                ids.getExtendedIdentityStoreData().getAuthenticationType(),
                ids.getExtendedIdentityStoreData().getFriendlyName(),
                ids.getExtendedIdentityStoreData().getSearchTimeoutSeconds(),
                ids.getExtendedIdentityStoreData().getUserName(),
                ids.getExtendedIdentityStoreData().useMachineAccount(),
                ids.getExtendedIdentityStoreData().getServicePrincipalName(),
                ids.getExtendedIdentityStoreData().getPassword(),
                ids.getExtendedIdentityStoreData().getUserBaseDn(),
                ids.getExtendedIdentityStoreData().getGroupBaseDn(),
                ids.getExtendedIdentityStoreData().getConnectionStrings(),
                ids.getExtendedIdentityStoreData().getAttributeMap(),
                ids.getExtendedIdentityStoreData().getIdentityStoreSchemaMapping(),
                ids.getExtendedIdentityStoreData().getUpnSuffixes(),
                flags,
                ids.getExtendedIdentityStoreData().getCertificates(),
                ids.getExtendedIdentityStoreData().getAuthnTypes());

        idmClient.setProvider(tenantName, updatedIds);
    }

    private static void get_provider_flags(String tenantName, String providerName) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        IIdentityStoreData ids = idmClient.getProvider(tenantName, providerName);
        if ( ids == null )
        {
            throw new InvalidParameterException(
                String.format(
                        "Provider [%s] does not exist.", providerName)
            );
        }

        if ( ids.getDomainType() != DomainType.EXTERNAL_DOMAIN )
        {
            throw new InvalidParameterException(
                String.format(
                    "Provider [%s] represents [%s] domain type. Only DomainType.EXTERNAL_DOMAIN is currently supported for set provider flags.",
                    providerName, ids.getDomainType().name())
            );
        }

        int flags = ids.getExtendedIdentityStoreData().getFlags();

        printOutProviderFlags(flags);
    }

    private static void get_sso_statistics(String tenantName) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        SsoHealthStatsData statsData = idmClient.getSsoStatistics(tenantName);
        if ( statsData == null )
        {
            throw new SsoHealthStatisticsException(tenantName);
        }
        System.out.println(""); // new line
        System.out.println("********** SSO STATISTICS INFORMATION **********");
        displayParamNameAndValue("TenantName", statsData.getTenant());
        displayParamNameAndValue("ToTalTokensGenerated", Integer.toString(statsData.getTotalTokensGenerated()));
        displayParamNameAndValue("ToTalTokensRenewed", Integer.toString(statsData.getTotalTokensRenewed()));
        displayParamNameAndValue("Generated Tokens for Tenant '" + statsData.getTenant() + "'", Integer.toString(statsData.getGeneratedTokensForTenant()));
        displayParamNameAndValue("Renewed Tokens for Tenant '" + statsData.getTenant() + "'", Integer.toString(statsData.getRenewedTokensForTenant()));
        displayParamNameAndValue("Uptime IDM (in secs)", Long.toString(statsData.getUptimeIDM()));
        displayParamNameAndValue("Uptime STS (in secs)", Long.toString(statsData.getUptimeSTS()));
    }

    private static void remove_jit_users(String tenantName, String extIDPEntityId) throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        if(tenantName.isEmpty()) {
            tenantName = idmClient.getDefaultTenant().toString();
        }

        idmClient.removeExternalIdpConfig(tenantName, extIDPEntityId, true);
    }

    private static void printOutProviderFlags(int flags)
    {
        System.out.print(
                String.format("Flags=%d; ", flags) );

        if(flags != 0)
        {
            System.out.print("[the following known flags: ");

            boolean first = true;
            for(int flag : knownFlags)
            {
                if ( (flags & flag) != 0 )
                {
                    if (!first)
                    {
                        System.out.print( ", " );
                    }
                    System.out.print( flagsToStrings.get(flag) );
                    first = false;
                }
            }
            System.out.println("]");
        }
        else
        {
            System.out.println("[Default: recursively computing nested groups, no site affinity is enabled for AD over Ldap identity providers.]");
        }
    }

    private static void writeDomDoc(Document tenantDoc, String xmlFile)
        throws TransformerFactoryConfigurationError, TransformerException
    {
        // Prepare the DOM document for writing
        Source source = new DOMSource(tenantDoc);

        File file = new File(xmlFile);
        Result result = new StreamResult(file);

        // Write the DOM document to the file
        Transformer xformer = TransformerFactory.newInstance().newTransformer();
        xformer.transform(source, result);
    }

    private static Document readDomDoc(String xmlFile)
        throws ParserConfigurationException, SAXException, IOException
    {
        if (xmlFile.isEmpty()) {
            throw new InvalidParameterException("xml document location");
        }
        SecureXmlParserFactory builderFactory = new SecureXmlParserFactory();
        DocumentBuilder docBuilder = builderFactory.newDocumentBuilder();

        Document doc;
        doc = docBuilder.parse(new File(xmlFile));
        return doc;
    }

    /*
     * To support original syntax of cmd line options, without '-'
     * tweak cmd line for those old entries to add them.
     * Do not extend this list. All new options will be - prefixed.
     * eventually we can clean this up and not support original...
     */
    static void preProcessArgs(String[] args)
    {
        //parse command type.
        if ( (args[0].equalsIgnoreCase(OPTION_IMPORT))
             ||
             (args[0].equalsIgnoreCase(OPTION_EXPORT))
             ||
             (args[0].equalsIgnoreCase(OPTION_REGISTER_SP))
             ||
             (args[0].equalsIgnoreCase(OPTION_REGISTER_IDP))
             ||
             (args[0].equalsIgnoreCase(OPTION_GET_SSO_SAML2_METADATA))
             ||
             (args[0].equalsIgnoreCase(OPTION_GET_IDENTITY_SOURCES))
             ||
             (args[0].equalsIgnoreCase(OPTION_GET_DOMAIN_JOIN_STATUS))
           )
        {
            args[0] = '-' + args[0];
        }
    }

    /*
     * Builds the cmdLine options used for cmd line arg parsing.
     */
    @SuppressWarnings("static-access")
    private static Options createCmdLineOptions()
    {
        //Usage: import|export|register_sp|register_idp|get_sso_saml2_metadata|get_identity_sources|set_identity_store_flags} [-t tenantname] config_filename";

        Options options = new Options();

        OptionGroup operationGroup = new OptionGroup();

        //import
        Option importOption = new Option(OPTION_IMPORT, "import metadata document");
        operationGroup.addOption(importOption);

        //export
        Option exportOption = new Option(OPTION_EXPORT, "export metadata document");
        operationGroup.addOption(exportOption);

        //register_sp
        Option registerSPOption = new Option(OPTION_REGISTER_SP, "register service provider");
        operationGroup.addOption(registerSPOption);

        //register_idp
        Option registerIDPOption = new Option(OPTION_REGISTER_IDP, "register identity provider");
        operationGroup.addOption(registerIDPOption);

        //remove idp
        Option removeIDPOption = new Option(OPTION_REMOVE_IDP, "remove external SAML 2.0 Identity provider registration");
        operationGroup.addOption(removeIDPOption);

        //remove sp
        Option removeSPOption = new Option(OPTION_REMOVE_SP, "remove external SAML 2.0 Service provider registration");
        operationGroup.addOption(removeSPOption);

        // get_sso_saml2_metadata
        Option sso2SamlMetadataOption = new Option(OPTION_GET_SSO_SAML2_METADATA, "export sso saml 2 metadata");
        operationGroup.addOption(sso2SamlMetadataOption);

        //enable_jit
        Option enableJitOption = new Option(OPTION_JIT_SWITCH, "enable/disable jit for an external idp per tenant");
        operationGroup.addOption(enableJitOption);

        // add jit claim group mapping
        Option addClaimGroupMap = new Option(OPTION_ADD_CLAIM_GROUP_MAP, "add jit claim group mapping to the external idp");
        operationGroup.addOption(addClaimGroupMap);

        // delete jit claim group mapping
        Option deleteClaimGroupMap = new Option(OPTION_DELETE_CLAIM_GROUP_MAP, "delete jit claim group mapping to the external idp");
        operationGroup.addOption(deleteClaimGroupMap);

        //change user password
        Option changeUserPwd = new Option(OPTION_CHANGE_USER_PWD, "change user password per tenant");
        operationGroup.addOption(changeUserPwd);

        //update_system_domain_store_password
        Option updateSysDomainStorePwd = new Option(OPTION_UPDATE_SYSTEM_DOMAIN_STORE_PWD, "update system domain store password per tenant");
        operationGroup.addOption(updateSysDomainStorePwd);

        // get_identity_sources
        Option identitySourcesOption = new Option(OPTION_GET_IDENTITY_SOURCES, "retrieve identity sources configuration");
        operationGroup.addOption(identitySourcesOption);

        // set default identity sources
        Option setDefaultIdentitySources = new Option(OPTION_SET_DEFAULT_IDENTITY_SOURCES, "set default identity sources per tenant");
        operationGroup.addOption(setDefaultIdentitySources);

        // identity_sources_test_connection
        Option identitySourcesTestConnectionOption = new Option(OPTION_IDS_TEST_CONNECTION, "identity sources connection test");
        operationGroup.addOption(identitySourcesTestConnectionOption);

        // identity_sources_test_connection
        Option identitySourcesTestServerSupportsTls = new Option(OPTION_IDS_TEST_SUPPORTS_TLS, "identity sources server supports TLS test");
        operationGroup.addOption(identitySourcesTestServerSupportsTls);

        // get_domain_join_status
        Option domainJoinStatusOption = new Option(OPTION_GET_DOMAIN_JOIN_STATUS, "retrieve domain status information");
        operationGroup.addOption(domainJoinStatusOption);

        // get_identity_store_flags
        Option getIdentityStoreFlagsOption = new Option(OPTION_GET_IDENTITY_STORE_FLAGS, "get behavior options for a specified identity store");
        operationGroup.addOption(getIdentityStoreFlagsOption);

        // set_identity_store_flags
        Option setIdentityStoreFlagsOption = new Option(OPTION_SET_IDENTITY_STORE_FLAGS, "set behavior options for a specified identity store");
        operationGroup.addOption(setIdentityStoreFlagsOption);

        // getProviderActivities
        Option getIdmAuthStatsOption = new Option(OPTION_GET_IDM_AUTH_STATS, "get IDM authentication statistics");
        operationGroup.addOption(getIdmAuthStatsOption);
        // get_sso_statistics
        Option getSsoStatistics = new Option(OPTION_GET_SSO_STATISTICS, "get Sso Statistics for a Tenant");
        operationGroup.addOption(getSsoStatistics);

        // add_config_group_flags
        Option addConfigGroupOption = new Option(OPTION_ADD_GROUP, "add a group to a list of tenants");
        operationGroup.addOption(addConfigGroupOption);

        // remove jit users
        Option removeJitUsersOption = new Option(OPTION_REMOVE_JIT_USERS, "remove all Jit users associated to the external idp");
        operationGroup.addOption(removeJitUsersOption);

        // set upn suffix
        Option setUpnSuffix = new Option(OPTION_SET_UPN_SUFFIX, "set upn suffix for the external idp");
        operationGroup.addOption(setUpnSuffix);

        // getTCCertAuthn
        Option getTCCertAuthnOption = new Option(OPTION_GET_TC_CERT_AUTHN, "get Tomcat client certificate authentication configuration");
        operationGroup.addOption(getTCCertAuthnOption);

        // setTCCertAuthn
        Option setTCCertAuthnOption = new Option(OPTION_SET_TC_CERT_AUTHN, "set Tomcat client certificate authentication configuration");
        operationGroup.addOption(setTCCertAuthnOption);

        // getAuthnPolicy
        Option getAuthnPolicyOption = new Option(OPTION_GET_AUTHN_POLICY, "get Tenant client authentication policy");
        operationGroup.addOption(getAuthnPolicyOption);

        // setAuthnPolicy
        Option setAuthnPolicyOption = new Option(OPTION_SET_AUTHN_POLICY, "set Tenant client authentication policy");
        operationGroup.addOption(setAuthnPolicyOption);

        // addAltOCSP
        Option addAltOCSP = new Option(OPTION_ADD_ALT_OCSP, "add an alternative OCSP responder configuration for smart card authentication");
        operationGroup.addOption(addAltOCSP);

        // getAltOCSP
        Option getAltOCSP = new Option(OPTION_GET_ALT_OCSP, "display alternative OCSP responder configurations for smart card authentication");
        operationGroup.addOption(getAltOCSP);

        // deleteAltOCSP
        Option deleteAltOCSP = new Option(OPTION_DELETE_ALT_OCSP, "delete one or all alternative OCSP responder configurations for smart card authentication");
        operationGroup.addOption(deleteAltOCSP);

        // setLogonBanner
        Option setLogonBanner = new Option(OPTION_SET_LOGON_BANNER, "set tenant logon banner");
        operationGroup.addOption(setLogonBanner);

        // printLogonBanner
        Option printLogonBanner = new Option(OPTION_PRINT_LOGON_BANNER, "print tenant logon banner title and content");
        operationGroup.addOption(printLogonBanner);

        // disableLogonBanner
        Option disableLogonBanner = new Option(OPTION_DISABLE_LOGON_BANNER, "disable logon banner");
        operationGroup.addOption(disableLogonBanner);

        // set tenant idp selection flag
        Option setIDPSelectionFlag = new Option(OPTION_SET_IDP_SELECTION_FLAG, "set idp selection flag");
        operationGroup.addOption(setIDPSelectionFlag);

        // get local idp alias
        Option getLocalIDPDisplayName = new Option(OPTION_GET_LOCAL_IDP_ALIAS, "get local idp alias");
        operationGroup.addOption(getLocalIDPDisplayName);

        // set local idp alias
        Option setLocalIDPDisplayName = new Option(OPTION_SET_LOCAL_IDP_ALIAS, "set local idp alias");
        operationGroup.addOption(setLocalIDPDisplayName);

        // get external idp alias
        Option getExtIDPDisplayName = new Option(OPTION_GET_EXT_IDP_ALIAS, "get external idp alias");
        operationGroup.addOption(getExtIDPDisplayName);

        // set external idp alias
        Option setExtIDPDisplayName = new Option(OPTION_SET_EXT_IDP_ALIAS, "set external idp alias");
        operationGroup.addOption(setExtIDPDisplayName);

        // tenant
        Option tenant = OptionBuilder.withArgName( OPTION_TENANT_ARG )
                .hasArg()
                .withDescription( "tenant to be used [default tenant will be used if omitted]" )
                .create( OPTION_TENANT );

        Option groupName = OptionBuilder.withArgName( OPTION_GROUP_NAME_ARG )
                .hasArg()
                .withDescription( "name of the group to be used" )
                .create( OPTION_GROUP_NAME );

        Option groupDesc = OptionBuilder.withArgName( OPTION_GROUP_DESCRIPTION_ARG )
                .hasArg()
                .withDescription( "description of the group to be used" )
                .create( OPTION_GROUP_DESCRIPTION );

        // claim name
        Option claimName = OptionBuilder.withArgName( OPTION_CLAIM_NAME_ARG )
                .hasArg()
                .withDescription( "claim name for the mapping" )
                .create( OPTION_CLAIM_NAME );

        // claim value
        Option claimValue = OptionBuilder.withArgName( OPTION_CLAIM_VALUE_ARG )
                .hasArg()
                .withDescription( "claim value for the mapping" )
                .create( OPTION_CLAIM_VALUE );

        // user
        Option user = OptionBuilder.withArgName( OPTION_USER_ARG )
                .hasArg()
                .withDescription( "user name to be used" )
                .create( OPTION_USER );

        // logon banner title
        Option title = OptionBuilder.withArgName( OPTION_LOGON_BANNER_TITLE_ARG )
                .hasArg()
                .withDescription( "logon banner title" )
                .create( OPTION_LOGON_BANNER_TITLE );

        // enable logon banner checkbox
        Option enableCheckbox = OptionBuilder.withArgName(OPTION_ENABLE_LOGON_BANNER_CHECKBOX_ARG)
                .hasArg()
                .withDescription("enable logon banner checkbox")
                .create(OPTION_ENABLE_LOGON_BANNER_CHECKBOX);

        // entity id
        Option entityId = OptionBuilder.withArgName( OPTION_ENTITY_ID_ARG )
                .hasArg()
                .withDescription( "external idp entity id" )
                .create( OPTION_ENTITY_ID );

        // service provider name
        Option sp = OptionBuilder.withArgName( OPTION_SERVICE_PROVIDER_ARG )
                .hasArg()
                .withDescription( "service provider name" )
                .create( OPTION_SERVICE_PROVIDER );

        // display name
        Option alias = OptionBuilder.withArgName( OPTION_ALIAS_ARG )
                .hasArg()
                .withDescription( "idp display name" )
                .create( OPTION_ALIAS );

        // upn suffix
        Option upnSuffix = OptionBuilder.withArgName( OPTION_UPN_SUFFIX_ARG )
                .hasArg()
                .withDescription( "external idp upn suffix" )
                .create( OPTION_UPN_SUFFIX );

        // identity_store
        Option provider = OptionBuilder.withArgName( OPTION_PROVIDER_ARG )
                .hasArg()
                .withDescription( "name of the identity store to set flags for" )
                .create( OPTION_PROVIDER );

        // flags
        Option flags = OptionBuilder.withArgName( OPTION_FLAGS_ARG )
            .hasArg()
            .withDescription(
                String.format(
                    "Flags used for identity store behavior tuning. Identity Store Specific.%s" +
                    "The value can be either an integer or %s separated named options which are semantically or'ed.%s" +
                    "Some of the known named options are:%s" +
                    "%s - %s%s" +
                    "%s - %s%s" +
                    "%s - %s%s" +
                    "%s - %s%s" +
                    "%s - %s%s",
                    System.lineSeparator(),
                    OPTION_FLAGS_SEPARATOR, System.lineSeparator(),
                    System.lineSeparator(),
                    OPTION_FLAGS_MATCHING_RULE_IN_CHAIN,
                        "Use AD matching rule in chain operator to resolve nested groups. When not set, nested groups are searched recursively. (AD-over-Ldap provider)",
                        System.lineSeparator(),
                    OPTION_FLAGS_DIRECT_GROUPS_ONLY,
                        "Compute direct parent groups only. When not set, nested groups are calculated. (AD-over-Ldap provider)",
                        System.lineSeparator(),
                    OPTION_FLAGS_DO_NOT_USE_BASE_DN_FOR_NESTED_GROUPS,
                        "Do not use group search base dn in nested group membership resolution (domain dn is used instead). When not set, a groups search base dn from identity store configuration is used. (AD-over-Ldap provider)",
                        System.lineSeparator(),
                    OPTION_FLAGS_ENABLE_SITE_AFFINITY,
                        "Enable site affinity when establishing connection to AD. When not set, by default, user specified connection string(s) are used. (AD-over-Ldap provider)",
                        System.lineSeparator(),
                    OPTION_FLAGS_FSP_GROUPS,
                        "Compute groups from joined domain if it is in a different forest by Foreign security principal. (AD provider)",
                        System.lineSeparator(),
                    OPTION_FLAGS_TOKEN_GROUPS,
                        "Compute groups by using tokenGroups attribute. This attribute specifies the list of SIDs due to a transitive group membership expansion operation on a given user or computer."
                        + "Token Groups cannot be retrieved if no Global Catalog is present to retrieve the transitive reverse memberships. (AD provider)",
                        System.lineSeparator()
                    )
            )
            .create( OPTION_FLAGS );

        // ActivitKind
        Option activityKind = OptionBuilder.withArgName( OPTION_ACTIVITY_KIND_ARG )
                .hasArg()
                .withDescription( "Valid values: Authenticate and GetAttributes, case insensitive" )
                .create( OPTION_ACTIVITY_KIND );


        // Verbose
        Option verbose = OptionBuilder.withArgName( OPTION_VERBOSE_ARG )
                .hasArg()
                .withDescription( "Verbose?: Y or N" )
                .create( OPTION_VERBOSE );
        options.addOptionGroup(operationGroup);
        options.addOption(tenant);
        options.addOption(groupName);
        options.addOption(groupDesc);
        options.addOption(claimName);
        options.addOption(claimValue);
        options.addOption(user);
        options.addOption(title);
        options.addOption(enableCheckbox);
        options.addOption(entityId);
        options.addOption(sp);
        options.addOption(alias);
        options.addOption(provider);
        options.addOption(flags);
        options.addOption(activityKind);
        options.addOption(verbose);
        options.addOption(upnSuffix);

        options.addOption(
                OptionBuilder.withArgName( OPTION_CA_FILES_ARG )
                .hasArg()
                .withDescription( "root or issuing CA files seperated by comma" )
                .create( OPTION_CA_FILES ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_ENABLED_ARG )
                .hasArg()
                .withDescription( "cert authentiation switch" )
                .create( OPTION_AUTHN_CERT_ENABLED ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_PWD_ENABLED_ARG )
                .hasArg()
                .withDescription( "password authentiation switch" )
                .create( OPTION_AUTHN_PWD_ENABLED ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_WIN_ENABLED_ARG )
                .hasArg()
                .withDescription( "Windows authentiation switch" )
                .create( OPTION_AUTHN_WIN_ENABLED ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_RSASECURID_ENABLED_ARG )
                .hasArg()
                .withDescription( "RSA SecureID authentiation switch" )
                .create( OPTION_AUTHN_RSASECURID_ENABLED ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_REVOCATION_CHECK_ARG )
                .hasArg()
                .withDescription( "cert revocation check switch" )
                .create( OPTION_AUTHN_CERT_REVOCATION_CHECK ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_USE_OCSP_ARG )
                .hasArg()
                .withDescription( "use OCSP switch" )
                .create( OPTION_AUTHN_CERT_USE_OCSP ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER_ARG )
                .hasArg()
                .withDescription( "use CRL as Fail Over switch" )
                .create( OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_OCSP_URL_ARG )
                .hasArg()
                .withDescription( "Alternative OCSP responder URL." )
                .create( OPTION_AUTHN_CERT_OCSP_URL ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_OCSP_CERT_ARG )
                .hasArg()
                .withDescription( "Signing certificate for the alternative OCSP responder." )
                .create( OPTION_AUTHN_CERT_OCSP_CERT ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_USE_CERT_CRL_ARG )
                .hasArg()
                .withDescription( "use certificate CRL switch" )
                .create( OPTION_AUTHN_CERT_USE_CERT_CRL ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_CRL_URL_ARG )
                .hasArg()
                .withDescription( "Certificate CRL override. null value allowed." )
                .create( OPTION_AUTHN_CERT_CRL_URL ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_AUTHN_CERT_OIDS_ARG )
                .hasArg()
                .withDescription( "Allowed OIDs, seperated by comma. null value allowed." )
                .create( OPTION_AUTHN_CERT_OIDS ));

        //RSA config commands
        OptionGroup rsaConfigGroup = new OptionGroup();

        rsaConfigGroup.addOption(new Option(OPTION_SET_RSA_CONFIG, "set RSA agent configuration"));
        rsaConfigGroup.addOption(new Option(OPTION_GET_RSA_CONFIG, "get RSA agent configuration"));
        rsaConfigGroup.addOption(new Option(OPTION_SET_RSA_SITE, "set RSA site-specific configuration"));
        rsaConfigGroup.addOption(new Option(OPTION_SET_RSA_USERID_ATTR_MAP, "add or update a userID LDAP attribute map."));
        options.addOptionGroup(rsaConfigGroup);

        //RSA config command options
        options.addOption(
                OptionBuilder.withArgName( OPTION_SITE_ID_ARG )
                .hasArg()
                .withDescription( "PSC site ID. Default to local site if not specified" )
                .create( OPTION_SITE_ID ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_AGENT_NAME_ARG )
                .hasArg()
                .withDescription( "RSA agent name defined in RSA AM." )
                .create( OPTION_RSA_AGENT_NAME ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_FILE_PATH_ARG )
                .hasArg()
                .withDescription( "path to sdconf_rec file for the agent." )
                .create( OPTION_RSA_SDCONF_REC ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_FILE_PATH_ARG )
                .hasArg()
                .withDescription( "optional. Path to sdOpts_rec file for the agent." )
                .create( OPTION_RSA_SDOPTS_REC ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_LOGIN_GUIDE_ARG )
                .hasArg()
                .withDescription( "SecurID login guidence text string." )
                .create( OPTION_RSA_LOGIN_GUIDE ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_LOG_LEVEL_ARG )
                .hasArg()
                .withDescription( "RSA agent log level: [OFF, INFO, DEBUG, WARN, ERROR, FATAL]. Default level is INFO." )
                .create( OPTION_RSA_LOG_LEVEL ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_LOG_FILE_SIZE_ARG )
                .hasArg()
                .withDescription( "RSA agent log file size limit in number of MB." )
                .create( OPTION_RSA_LOG_FILE_SIZE ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_MAX_LOG_FILE_COUNT_ARG )
                .hasArg()
                .withDescription( "RSA agent maximum number of log files." )
                .create( OPTION_RSA_MAX_LOG_FILE_COUNT ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_CONN_TIME_OUT_ARG )
                .hasArg()
                .withDescription( "RSA agent connection time out value in seconds." )
                .create( OPTION_RSA_CONN_TIME_OUT ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_READ_TIME_OUT_ARG )
                .hasArg()
                .withDescription( "RSA agent read time out value in seconds." )
                .create( OPTION_RSA_READ_TIME_OUT ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_ENC_ALG_LIST_ARG )
                .hasArg()
                .withDescription( "RSA encryption key wrapping algorithm name list. Default is the complete list: AE/16,AES/24,AES/32" )
                .create( OPTION_RSA_ENC_ALG_LIST ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_LDAP_ATTR_ARG )
                .hasArg()
                .withDescription( "LDAP attribute used as RSA userID" )
                .create( OPTION_RSA_LDAP_ATTR ));

        options.addOption(
                OptionBuilder.withArgName( OPTION_RSA_IDS_NAME_ARG )
                .hasArg()
                .withDescription( "identity source this rule applied to." )
                .create( OPTION_RSA_IDS_NAME ));

        return  options;
    }

    private static ActivityKind getActivityKind(CommandLine cmdLine)
    {
        return ActivityKind.valueOf(cmdLine.getOptionValue(OPTION_ACTIVITY_KIND, "").toUpperCase());
    }

    private static String getTenantName(CommandLine cmdLine)
    {
        return getValueByOption(cmdLine, OPTION_TENANT);
    }

    private static String getUserName(CommandLine cmdLine)
    {
        return getValueByOption(cmdLine, OPTION_USER);
    }

    private static String getValueByOption(CommandLine cmdLine, String option) {
        String name = "";

        name = cmdLine.getOptionValue(option, "");

        if (name != null)
        {
            name = name.trim();
        }
        else
        {
            name = "";
        }

        return name;
    }

    private static Collection<String> getTenantNames(CommandLine cmdLine)
    {
        Collection<String> tenants = new LinkedList<String>();
        String[] tenantNames = cmdLine.getOptionValues(OPTION_TENANT);

        if (tenantNames != null)
        {
           for (String tenant : tenantNames)
           {
               tenants.add(tenant.trim());
           }
        }

        return tenants;
    }

    private static String getGroupName(CommandLine cmdLine) {
       String groupName = "";

       groupName = cmdLine.getOptionValue(OPTION_GROUP_NAME, "");

       if (groupName != null)
       {
           groupName = groupName.trim();
       }
       else
       {
           groupName = "";
       }

       return groupName;
    }

    private static String getGroupDescription(CommandLine cmdLine) {
       String groupDescription = "";

       groupDescription = cmdLine.getOptionValue(OPTION_GROUP_DESCRIPTION, "");

       if (groupDescription != null)
       {
           groupDescription = groupDescription.trim();
       }
       else
       {
           groupDescription = "";
       }

       return groupDescription;
    }

    private static String getClaimName(CommandLine cmdLine)
    {
        return getValueByOption(cmdLine, OPTION_CLAIM_NAME);
    }

    private static String getClaimValue(CommandLine cmdLine)
    {
        return getValueByOption(cmdLine, OPTION_CLAIM_VALUE);
    }

    private static String getUpnSuffix(CommandLine cmdLine)
    {
        return getValueByOption(cmdLine, OPTION_UPN_SUFFIX);
    }

    private static String getEntityId(CommandLine cmdLine)
    {
        String entityId = "";

        entityId = cmdLine.getOptionValue(OPTION_ENTITY_ID, "");

        if (entityId != null && !entityId.isEmpty())
        {
            entityId = entityId.trim();
        }
        else
        {
            throw new InvalidParameterException("EntityId value should be specified.");
        }

        return entityId;
    }

    private static String getServiceProviderName(CommandLine cmdLine)
    {
        String sp = "";

        sp = cmdLine.getOptionValue(OPTION_SERVICE_PROVIDER, "");

        if (sp != null && !sp.isEmpty())
        {
            sp = sp.trim();
        }
        else
        {
            throw new InvalidParameterException("service provider name should be specified.");
        }

        return sp;
    }

    private static boolean getBooleanValue(CommandLine cmdLine)
    {
        String v = "";
        String[] args = cmdLine.getArgs();
        if ( ( args != null ) && (args.length > 0) )
        {
            v = args[0];
            if (v != null)
            {
                v = v.trim();
            }
            else
            {
                v = "";
            }
        }

        if (v.equalsIgnoreCase("Y"))
        {
            return true;
        }
        else if (v.equalsIgnoreCase("N"))
        {
            return false;
        }
        else
        {
            throw new InvalidParameterException("invalid boolean value");
        }
    }

    private static String getFileName(CommandLine cmdLine)
    {
        String fileName = "";
        String[] args = cmdLine.getArgs();
        if ( ( args != null ) && (args.length > 0) )
        {
            fileName = args[0];
            if (fileName != null)
            {
                fileName = fileName.trim();
            }
            else
            {
                fileName = "";
            }
        }

        if (fileName.isEmpty())
        {
            throw new InvalidParameterException("xml document location");
        }
        return fileName;
    }

    private static String getProvider(CommandLine cmdLine)
    {
        String provider = null;

        provider = cmdLine.getOptionValue(OPTION_PROVIDER, "");

        if (provider != null)
        {
            provider = provider.trim();
        }
        else
        {
            provider = "";
        }

        if ( provider.length() == 0 )
        {
            throw new InvalidParameterException("Identity store name is required.");
        }
        return provider;
    }

    private static int getFlags(CommandLine cmdLine)
    {
        int flags = 0;

        String flagsStr = cmdLine.getOptionValue(OPTION_FLAGS, "");
        if (( flagsStr == null ) || (flagsStr.isEmpty()))
        {
            throw new InvalidParameterException("Flags parameter value should be specified.");
        }

        try
        {
            flags = Integer.parseInt(flagsStr);
        }
        catch(NumberFormatException ex)
        {
            // could be literal
            String[] options = flagsStr.split(OPTION_FLAGS_SEPARATOR);
            for(String opt : options)
            {
                if ( stringsToFlags.containsKey(opt) )
                {
                    flags |= stringsToFlags.get(opt);
                }
                else
                {
                    throw new InvalidParameterException("Unknown flag pareameter specified: [" + opt + "]");
                }
            }
        }

        return flags;
    }

    /**
     * Validate Identity Source Ldaps connection certificate
     *
     * @param tenantName
     * @throws Exception
     */
    private static void testIdentitySourcesConnection() throws Exception {
        final CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        Collection<String> tenants = idmClient.getAllTenants();

        int total = 0;
        int failed = 0;
        for (final String tenantName : tenants) {
            ArrayList<IIdentityStoreData> identitySources = new ArrayList<IIdentityStoreData>(idmClient.getProviders(tenantName));

            System.out.println();
            System.out.println(String.format("********** IDENTITY SOURCE CONNECTION LDAPS INFORMATION FOR %s **********", tenantName));

            Collection<Certificate> tenantCertificates = idmClient.getAllCertificates(tenantName, CertificateType.LDAP_TRUSTED_CERT);
            final Collection<X509Certificate> tenantX509Certificates = new ArrayList<>();
            for (Certificate certificate : tenantCertificates) {
                tenantX509Certificates.add((X509Certificate)certificate);
            }

            IdentityVerifierResult result = checkIdentitySources(identitySources, new IdentityVerifier() {

                @Override
                public void verify(IIdentityStoreDataEx identitySourceDetails, URI connectionURI) throws Exception {

                    //if it is a legacy IdS should get the certificates from tenant
                    Collection<X509Certificate> certs = identitySourceDetails.getCertificates() != null ?
                            identitySourceDetails.getCertificates() : tenantX509Certificates;
                    idmClient.probeProviderConnectivityWithCertValidation(tenantName, connectionURI.toString(),
                                    identitySourceDetails.getAuthenticationType(),
                                    identitySourceDetails.getUserName(),
                                    identitySourceDetails.getPassword(), certs);
                }
            });

            failed += result.getFailed();
            total += result.getTotal();
        }

        System.out.println();
        System.out.println(String.format("****** TOTAL: %s, FAILED: %s ******", total, failed));

        System.out.println(getWarningMessage());
        if (failed == 0) {
            System.out.println(getEnableDisableMessage("LdapsCertValidation"));
        }
    }

    private static IdentityVerifierResult checkIdentitySources(ArrayList<IIdentityStoreData> identitySources, IdentityVerifier verifier) throws Exception
    {
        int total = 0;
        int failed = 0;
        for (IIdentityStoreData identitySource : identitySources)
        {
            if (identitySource.getExtendedIdentityStoreData() != null)
            {
                IIdentityStoreDataEx identitySourceDetails = identitySource.getExtendedIdentityStoreData();
                for (String connectionString : identitySourceDetails.getConnectionStrings())
                {
                    URI connectionURI = new URI(connectionString);
                    if (connectionURI.getScheme() == null)
                        continue;

                    boolean isLdaps = connectionURI.getScheme().compareToIgnoreCase(LDAPS_SCHEMA) == 0;
                    if (isLdaps)
                    {
                        total++;

                        try {
                            verifier.verify(identitySourceDetails, connectionURI);
                            displayParamNameAndValue("Test connectionString", connectionString + " PASSED");
                        } catch (Exception e) {
                            displayParamNameAndValue("Test connectionString", connectionString + " FAILED - " + e.toString());
                            failed++;
                        }
                    }
                }
            }
        }

       return new IdentityVerifierResult(total, failed);
    }

    private static void testIdentitySourceServerSupportsTls() throws Exception
    {
        CasIdmClient idmClient = new CasIdmClient(HOSTNAME);
        Collection<String> tenants = idmClient.getAllTenants();

        int total = 0;
        int failed = 0;
        for (String tenantName : tenants) {
            ArrayList<IIdentityStoreData> identitySources = new ArrayList<IIdentityStoreData>(idmClient.getProviders(tenantName));

            System.out.println();
            System.out.println(String.format("********** IDENTITY SOURCE CONNECTION LDAPS WITH TLSv1 MINIMUM PROTOCOL FOR %s **********", tenantName));

            IdentityVerifierResult result = checkIdentitySources(identitySources, new IdentityVerifier() {
                @Override
                public void verify(IIdentityStoreDataEx identitySourceDetails, URI connectionURI) throws Exception {
                    checkMinimumProtocolIsSupported(connectionURI);
                }
            });

            failed += result.getFailed();
            total += result.getTotal();
        }

        System.out.println();
        System.out.println(String.format("****** TOTAL: %s, FAILED: %s ******", total, failed));
    }

    private static void checkMinimumProtocolIsSupported(URI uri) throws Exception
    {
        SSLContext context = null;
        SSLSocket sslSocket = null;
        String[] enabledProtocols = new String[] { "TLSv1",  "TLSv1.1",  "TLSv1.2" };
        try {
           context = SSLContext.getInstance("TLS");
           TrustManager tm = new X509TrustManager() {

               @Override
               public void checkClientTrusted(X509Certificate[] chain, String thumbp)
                       throws CertificateException {
                   //no-op for client
               }

               @Override
               public void checkServerTrusted(X509Certificate[] chain, String thumbp)
                       throws CertificateException {
                   //accept all
               }

               @Override
               public X509Certificate[] getAcceptedIssuers() {
                   return null;
               }
           };

           context.init(null, new TrustManager[]{tm}, null);

           SSLSocketFactory socketFactory = context.getSocketFactory();

           // SSLSocketFactory only builds the super type, need to cast to SSL socket.
           Socket s = socketFactory.createSocket(uri.getHost(), uri.getPort() == -1 ? LDAPS_PORT : uri.getPort());

           sslSocket = (SSLSocket) s;
           sslSocket.setEnabledProtocols( enabledProtocols );

           sslSocket.startHandshake();

        } finally {
            if (sslSocket != null)
            {
                try {
                    sslSocket.close();
                 } catch (IOException e) {
                    // do nothing
                 }
            }
        }
    }

    private static String getWarningMessage()
    {
         StringBuilder warning = new StringBuilder();
         warning.append(System.lineSeparator());
         warning.append("****** WARNING ******");
         warning.append(System.lineSeparator());
         warning.append("Please ensure that you store the certificates for each domain controller in the domain if you are configured to connect to Active Directory as an LDAP server.");
         warning.append( System.lineSeparator());
         warning.append("sso-config only verifies the certificate of the domain controller that responded at the time the tool was run. ");
         warning.append(System.lineSeparator());
         warning.append("If you do not store the certificate for each domain controller, future login attempts may fail.");
         warning.append(System.lineSeparator());

         return warning.toString();
    }

    private static String getEnableDisableMessage(String keyName)
    {
        String registryCommand;
        if (SystemUtils.IS_OS_LINUX)
        {
            registryCommand = "/opt/likewise/bin/lwregshell %s '[HKEY_THIS_MACHINE\\Software\\VMware\\Identity\\Configuration]' %s %s %s";
            StringBuilder command = new StringBuilder();
            command.append(System.lineSeparator());
            command.append("To change the setting for certificate validation please run the commands:");
            command.append(System.lineSeparator());
            command.append("add value: ");
            command.append(String.format(registryCommand, "add_value", keyName,  "REG_SZ", Boolean.toString(true)));
            command.append(System.lineSeparator());
            command.append("set value: ");
            command.append(String.format(registryCommand, "set_value", keyName, "", Boolean.toString(true)));
            return command.toString();
        }
        else
        {
            registryCommand = String.format("reg add HKLM\\Software\\VMware\\Identity\\Configuration /v %s /t REG_SZ /d %s", keyName, Boolean.toString(true));
            return String.format("%sTo change the setting for certificate validation please run the command:%s%s",
                    System.lineSeparator(), System.lineSeparator(), registryCommand);
        }
    }

    private static void printUsage(Options options)
    {
        StringBuilder usageStringBuilder = new StringBuilder();
        usageStringBuilder.append("One of" + System.lineSeparator());

        //-import [-t tenantname] config_filename
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                 OPTION_IMPORT,
                 OPTION_TENANT, OPTION_TENANT_ARG, CONFIGURATION_FILE_NAME_ARG));

        //-export[-t tenantname] config_filename
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                OPTION_EXPORT,
                OPTION_TENANT, OPTION_TENANT_ARG, CONFIGURATION_FILE_NAME_ARG));

        //-register_sp [-t tenantname] config_filename
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                OPTION_REGISTER_SP,
                OPTION_TENANT, OPTION_TENANT_ARG, CONFIGURATION_FILE_NAME_ARG));

        //-remove_sp [-t tenantname] -sp service provider name
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_REMOVE_SP, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_SERVICE_PROVIDER,OPTION_SERVICE_PROVIDER_ARG));

        //-register_idp [-t tenantname] config_filename
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                OPTION_REGISTER_IDP,
                OPTION_TENANT, OPTION_TENANT_ARG, CONFIGURATION_FILE_NAME_ARG));

        //-get_sso_saml2_metadata [-t tenantname] config_filename
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                OPTION_GET_SSO_SAML2_METADATA,
                OPTION_TENANT, OPTION_TENANT_ARG, CONFIGURATION_FILE_NAME_ARG));

        //-remove_idp [-t tenantname] -e entityId
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_REMOVE_IDP, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG));

        //-enable_jit [-t tenantname] -e entityid Y/N
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> <%s>" + System.lineSeparator(),
                OPTION_JIT_SWITCH, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG,
                OPTION_BOOLEAN_ARG));

        //-set_idp_selection_flag [-t tenantname] Y/N
        usageStringBuilder.append(String.format("-%s [-%s <%s>] <%s>" + System.lineSeparator(),
                OPTION_SET_IDP_SELECTION_FLAG, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_BOOLEAN_ARG));

        //-set_external_idp_upn_suffix [-t tenantname] -e entityid -upn_suffix upnSuffix
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> -%s <%s>" + System.lineSeparator(),
                OPTION_SET_UPN_SUFFIX, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG,
                OPTION_UPN_SUFFIX, OPTION_UPN_SUFFIX_ARG));

        //-change_user_password [-t tenantname] -u userName
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_CHANGE_USER_PWD, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_USER, OPTION_USER_ARG));

        //-update_system_domain_store_password [-t tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_UPDATE_SYSTEM_DOMAIN_STORE_PWD, OPTION_TENANT, OPTION_TENANT_ARG));

        //-get_identity_sources [-t tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_GET_IDENTITY_SOURCES, OPTION_TENANT, OPTION_TENANT_ARG));

        //-set_default_identity_sources [-t tenantname] -i <identity_store_name> ... <identity_store_name>
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> ... <%s>" + System.lineSeparator(),
                OPTION_SET_DEFAULT_IDENTITY_SOURCES, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_PROVIDER, OPTION_PROVIDER_ARG, OPTION_PROVIDER_ARG));

        //-identity_sources_test_ldaps_connection
        usageStringBuilder.append(String.format("-%s %s", OPTION_IDS_TEST_CONNECTION, System.lineSeparator()));

        //-identity_sources_test_ldaps_connection
        usageStringBuilder.append(String.format("-%s %s", OPTION_IDS_TEST_SUPPORTS_TLS, System.lineSeparator()));

        //-get_domain_join_status
        usageStringBuilder.append(String.format("-%s" + System.lineSeparator(),
                OPTION_GET_DOMAIN_JOIN_STATUS));

        //-get_identity_store_flags [-t tenantname] -i identity_store_name
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_GET_IDENTITY_STORE_FLAGS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_PROVIDER, OPTION_PROVIDER_ARG));

        //-set_identity_store_flags [-t tenantname] -i identity_store_name -f flags
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> -%s <%s>" + System.lineSeparator(),
                OPTION_SET_IDENTITY_STORE_FLAGS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_PROVIDER, OPTION_PROVIDER_ARG, OPTION_FLAGS, OPTION_FLAGS_ARG));

        //-get_external_idp_alias [-t tenantname] -e entityId
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_GET_EXT_IDP_ALIAS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ENTITY_ID, OPTION_ENTITY_ID_ARG));

        //set_external_idp_alias [-t tenantname] -e entityId -alias idp alias
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> -%s <%s>" + System.lineSeparator(),
                OPTION_SET_EXT_IDP_ALIAS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ENTITY_ID, OPTION_ENTITY_ID_ARG,
                OPTION_ALIAS, OPTION_ALIAS_ARG));

        //-get_local_idp_alias [-t tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_GET_LOCAL_IDP_ALIAS, OPTION_TENANT, OPTION_TENANT_ARG));

        //set_local_idp_alias [-t tenantname] -alias idp alias
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_SET_LOCAL_IDP_ALIAS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ALIAS, OPTION_ALIAS_ARG));

        //-set_logon_banner [-t tenantname] [-title logon_banner_title] [logonBannerHTMLFile] [-enable_checkbox Y/N]
        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>] [<%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_SET_LOGON_BANNER, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_LOGON_BANNER_TITLE,OPTION_LOGON_BANNER_TITLE_ARG,
                LOGON_BANNER_FILE_NAME_ARG, OPTION_ENABLE_LOGON_BANNER_CHECKBOX, OPTION_ENABLE_LOGON_BANNER_CHECKBOX_ARG));

        //-print_logon_banner [-tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_PRINT_LOGON_BANNER, OPTION_TENANT, OPTION_TENANT_ARG));

        //-disable_logon_banner [-tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_DISABLE_LOGON_BANNER, OPTION_TENANT, OPTION_TENANT_ARG));

        //-get_idm_auth_stats -t <TenantName> -a <ActivityKind> [-v Verbose]
        usageStringBuilder.append(String.format("-%s -%s <%s> -%s <%s> [-%s %s]" + System.lineSeparator(),
                OPTION_GET_IDM_AUTH_STATS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ACTIVITY_KIND, OPTION_ACTIVITY_KIND_ARG,
                OPTION_VERBOSE, OPTION_VERBOSE_ARG));

        //-add_config_group -g <groupName> [-gd <groupDescription>] [-t <TenantName> ... <TenantName>],
        usageStringBuilder.append(String.format("-%s -%s <%s> [-%s <%s>] [-%s <%s> ... <%s>]" + System.lineSeparator(),
                OPTION_ADD_GROUP, OPTION_GROUP_NAME, OPTION_GROUP_NAME_ARG, OPTION_GROUP_DESCRIPTION,
                OPTION_GROUP_DESCRIPTION_ARG, OPTION_TENANT, OPTION_TENANT_ARG, OPTION_TENANT_ARG));

	    //-get_sso_statistics [-t tenantname]
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
		OPTION_GET_SSO_STATISTICS, OPTION_TENANT, OPTION_TENANT_ARG));

        //-remove_jit_users [-t tenantname] -e entityId
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s>" + System.lineSeparator(),
                OPTION_REMOVE_JIT_USERS, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG));

        //-add_claim_group_map [-t tenantname] -e entityId -cn claimName -cv claimValue -g groupName
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> -%s <%s> -%s <%s> -%s <%s>" + System.lineSeparator(),
                OPTION_ADD_CLAIM_GROUP_MAP, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG,
                OPTION_CLAIM_NAME, OPTION_CLAIM_NAME_ARG,
                OPTION_CLAIM_VALUE, OPTION_CLAIM_VALUE_ARG,
                OPTION_GROUP_NAME, OPTION_GROUP_NAME_ARG));

        //-delete_claim_group_map [-t tenantname] -e entityId -claim_name claimName -claim_value claimValue -g groupName
        usageStringBuilder.append(String.format("-%s [-%s <%s>] -%s <%s> -%s <%s> -%s <%s> -%s <%s>" + System.lineSeparator(),
                OPTION_DELETE_CLAIM_GROUP_MAP, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ENTITY_ID,OPTION_ENTITY_ID_ARG,
                OPTION_CLAIM_NAME, OPTION_CLAIM_NAME_ARG,
                OPTION_CLAIM_VALUE, OPTION_CLAIM_VALUE_ARG,
                OPTION_GROUP_NAME, OPTION_GROUP_NAME_ARG));

                //Qiang's new stuff
        usageStringBuilder.append(String.format("-%s" + System.lineSeparator(), OPTION_GET_TC_CERT_AUTHN));
        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_SET_TC_CERT_AUTHN, OPTION_CA_FILES, OPTION_CA_FILES_ARG));

        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                        OPTION_GET_AUTHN_POLICY, OPTION_TENANT, OPTION_TENANT_ARG));

        usageStringBuilder
                        .append(String.format(
                                        "-%s [-%s <%s>]  [-%s <%s>] [-%s <%s>]  [-%s <%s>] [-%s <%s>] [-%s <%s>]"
                                                        + " [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>]"
                                                        + System.lineSeparator(),
                OPTION_SET_AUTHN_POLICY,
                    OPTION_TENANT, OPTION_TENANT_ARG,
                    OPTION_AUTHN_CERT_ENABLED, OPTION_AUTHN_CERT_ENABLED_ARG,
                    OPTION_AUTHN_PWD_ENABLED, OPTION_AUTHN_PWD_ENABLED_ARG,
                    OPTION_AUTHN_WIN_ENABLED, OPTION_AUTHN_WIN_ENABLED_ARG,
                    OPTION_AUTHN_RSASECURID_ENABLED, OPTION_AUTHN_RSASECURID_ENABLED_ARG,
                    OPTION_AUTHN_CERT_REVOCATION_CHECK, OPTION_AUTHN_CERT_REVOCATION_CHECK_ARG,
                    OPTION_AUTHN_CERT_USE_OCSP, OPTION_AUTHN_CERT_USE_OCSP_ARG,
                    OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER, OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER_ARG,
                    OPTION_AUTHN_CERT_USE_CERT_CRL, OPTION_AUTHN_CERT_USE_CERT_CRL_ARG,
                    OPTION_AUTHN_CERT_CRL_URL, OPTION_AUTHN_CERT_CRL_URL_ARG,
                    OPTION_AUTHN_CERT_OIDS, OPTION_AUTHN_CERT_OIDS_ARG,
                    OPTION_CA_FILES, OPTION_CA_FILES_ARG)
                                        );

        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_ADD_ALT_OCSP, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_SITE_ID, OPTION_SITE_ID_ARG,
                OPTION_AUTHN_CERT_OCSP_URL, OPTION_AUTHN_CERT_OCSP_URL_ARG,
                OPTION_AUTHN_CERT_OCSP_CERT, OPTION_AUTHN_CERT_OCSP_CERT_ARG ));

        usageStringBuilder.append(String.format("-%s [-%s <%s>]" + System.lineSeparator(),
                OPTION_GET_ALT_OCSP, OPTION_TENANT, OPTION_TENANT_ARG
                 ));

        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s] [-%s <%s>]" + System.lineSeparator(),
                OPTION_DELETE_ALT_OCSP, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_ALL_SITES, OPTION_SITE_ID, OPTION_SITE_ID_ARG ));

        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_GET_RSA_CONFIG, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_SITE_ID, OPTION_SITE_ID_ARG));

        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>] [-%s <%s>]" +
                " [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_SET_RSA_CONFIG, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_RSA_LOG_LEVEL, OPTION_RSA_LOG_LEVEL_ARG,
                OPTION_RSA_LOG_FILE_SIZE, OPTION_RSA_LOG_FILE_SIZE_ARG,
                OPTION_RSA_MAX_LOG_FILE_COUNT, OPTION_RSA_MAX_LOG_FILE_COUNT_ARG,
                OPTION_RSA_CONN_TIME_OUT, OPTION_RSA_CONN_TIME_OUT_ARG,
                OPTION_RSA_READ_TIME_OUT, OPTION_RSA_READ_TIME_OUT_ARG,
                OPTION_RSA_ENC_ALG_LIST, OPTION_RSA_ENC_ALG_LIST_ARG,
                OPTION_RSA_LOGIN_GUIDE, OPTION_RSA_LOGIN_GUIDE_ARG
                ));

        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_SET_RSA_SITE, OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_SITE_ID, OPTION_SITE_ID_ARG,
                OPTION_RSA_AGENT_NAME, OPTION_RSA_AGENT_NAME_ARG,
                OPTION_RSA_SDCONF_REC, OPTION_FILE_PATH_ARG,
                OPTION_RSA_SDOPTS_REC, OPTION_FILE_PATH_ARG
                ));


        usageStringBuilder.append(String.format("-%s [-%s <%s>] [-%s <%s>] [-%s <%s>] [-%s <%s>]" + System.lineSeparator(),
                OPTION_SET_RSA_USERID_ATTR_MAP,
                OPTION_TENANT, OPTION_TENANT_ARG,
                OPTION_RSA_IDS_NAME, OPTION_RSA_IDS_NAME_ARG,
                OPTION_RSA_LDAP_ATTR, OPTION_RSA_LDAP_ATTR_ARG,
                OPTION_SITE_ID, OPTION_SITE_ID_ARG
                ));

        HelpFormatter help = new HelpFormatter();
        help.setOptionComparator(new OptionComparator());
        help.printHelp(
            120,
            usageStringBuilder.toString(),
            System.lineSeparator()+"Commands and options: "+ System.lineSeparator(),
            options,
            getPaddingString(help.getLeftPadding()) +
            "<" + CONFIGURATION_FILE_NAME_ARG + ">" +
            getPaddingString(help.getDescPadding()) +
            "path to the xml configuration file." +
            System.lineSeparator() +
            getPaddingString(help.getLeftPadding()) +
                       "<" + LOGON_BANNER_FILE_NAME_ARG + ">" +
            getPaddingString(help.getDescPadding()) +
            "path to the logon banner file."
        );
    }

    private static class OptionComparator implements Comparator<Option>
    {
        private static final HashMap<String, Integer> order;

        static
        {
            // Comparator follows the order the values are put into the map
            int i = 0;
            order = new HashMap<String, Integer>();
            order.put(OPTION_IMPORT, ++i);
            order.put(OPTION_EXPORT, ++i);
            order.put(OPTION_REGISTER_SP, ++i);
            order.put(OPTION_REMOVE_SP, ++i);
            order.put(OPTION_REGISTER_IDP, ++i);
            order.put(OPTION_REMOVE_IDP, ++i);
            order.put(OPTION_GET_SSO_SAML2_METADATA, ++i);
            order.put(OPTION_JIT_SWITCH, ++i);
            order.put(OPTION_CHANGE_USER_PWD, ++i);
            order.put(OPTION_UPDATE_SYSTEM_DOMAIN_STORE_PWD, ++i);
            order.put(OPTION_GET_IDENTITY_SOURCES, ++i);
            order.put(OPTION_SET_DEFAULT_IDENTITY_SOURCES, ++i);
            order.put(OPTION_IDS_TEST_CONNECTION, ++i);
            order.put(OPTION_IDS_TEST_SUPPORTS_TLS, ++i);
            order.put(OPTION_SET_IDENTITY_STORE_FLAGS, ++i);
            order.put(OPTION_GET_IDENTITY_STORE_FLAGS, ++i);
            order.put(OPTION_GET_IDM_AUTH_STATS, ++i);
            order.put(OPTION_ADD_GROUP, ++i);
            order.put(OPTION_GROUP_NAME, ++i);
            order.put(OPTION_GROUP_DESCRIPTION, ++i);
            order.put(OPTION_CLAIM_NAME, ++i);
            order.put(OPTION_CLAIM_VALUE, ++i);
            order.put(OPTION_TENANT, ++i);
            order.put(OPTION_USER, ++i);
            order.put(OPTION_LOGON_BANNER_TITLE, ++i);
            order.put(OPTION_ENABLE_LOGON_BANNER_CHECKBOX, ++i);
            order.put(OPTION_ENTITY_ID, ++i);
            order.put(OPTION_SERVICE_PROVIDER, ++i);
            order.put(OPTION_ALIAS, ++i);
            order.put(OPTION_UPN_SUFFIX, ++i);
            order.put(OPTION_PROVIDER, ++i);
            order.put(OPTION_FLAGS, ++i);
            order.put(OPTION_ACTIVITY_KIND, ++i);
            order.put(OPTION_VERBOSE, ++i);
            order.put(OPTION_GET_DOMAIN_JOIN_STATUS, ++i);
            order.put(OPTION_GET_SSO_STATISTICS, ++i);
            order.put(OPTION_REMOVE_JIT_USERS, ++i);
            order.put(OPTION_ADD_CLAIM_GROUP_MAP, ++i);
            order.put(OPTION_DELETE_CLAIM_GROUP_MAP, ++i);
            order.put(OPTION_SET_UPN_SUFFIX, ++i);
            order.put(OPTION_GET_TC_CERT_AUTHN, ++i);
            order.put(OPTION_SET_TC_CERT_AUTHN, ++i);
            order.put(OPTION_GET_AUTHN_POLICY, ++i);
            order.put(OPTION_SET_AUTHN_POLICY, ++i);
            order.put(OPTION_ADD_ALT_OCSP, ++i);
            order.put(OPTION_GET_ALT_OCSP, ++i);
            order.put(OPTION_DELETE_ALT_OCSP, ++i);
            order.put(OPTION_AUTHN_CERT_ENABLED, ++i);
            order.put(OPTION_AUTHN_CERT_REVOCATION_CHECK, ++i);
            order.put(OPTION_AUTHN_CERT_USE_OCSP, ++i);
            order.put(OPTION_AUTHN_CERT_USE_CRL_AS_FAIL_OVER, ++i);
            order.put(OPTION_AUTHN_CERT_OCSP_URL, ++i);
            order.put(OPTION_AUTHN_CERT_OCSP_CERT, ++i);
            order.put(OPTION_AUTHN_CERT_USE_CERT_CRL, ++i);
            order.put(OPTION_AUTHN_CERT_CRL_URL, ++i);
            order.put(OPTION_AUTHN_CERT_OIDS, ++i);
            order.put(OPTION_CA_FILES, ++i);
            order.put(OPTION_AUTHN_PWD_ENABLED, ++i);
            order.put(OPTION_AUTHN_WIN_ENABLED, ++i);
            order.put(OPTION_AUTHN_RSASECURID_ENABLED, i++);
            order.put(OPTION_SET_LOGON_BANNER, ++i);
            order.put(OPTION_PRINT_LOGON_BANNER, ++i);
            order.put(OPTION_DISABLE_LOGON_BANNER, ++i);
            order.put(OPTION_SET_IDP_SELECTION_FLAG, ++i);
            order.put(OPTION_GET_LOCAL_IDP_ALIAS, ++i);
            order.put(OPTION_SET_LOCAL_IDP_ALIAS, ++i);
            order.put(OPTION_GET_EXT_IDP_ALIAS, ++i);
            order.put(OPTION_SET_EXT_IDP_ALIAS, ++i);
            order.put(OPTION_SET_RSA_CONFIG, ++i);
            order.put(OPTION_SET_RSA_SITE, ++i);
            order.put(OPTION_SET_RSA_USERID_ATTR_MAP, ++i);
            order.put(OPTION_GET_RSA_CONFIG, ++i);
            order.put(OPTION_SITE_ID, ++i);
            order.put(OPTION_RSA_AGENT_NAME, ++i);
            order.put(OPTION_RSA_SDCONF_REC, ++i);
            order.put(OPTION_FILE_PATH_ARG, ++i);
            order.put(OPTION_RSA_SDOPTS_REC, ++i);
            order.put(OPTION_RSA_LOGIN_GUIDE, ++i);
            order.put(OPTION_RSA_LOG_LEVEL, ++i);
            order.put(OPTION_RSA_LOG_FILE_SIZE, ++i);
            order.put(OPTION_RSA_MAX_LOG_FILE_COUNT, ++i);
            order.put(OPTION_RSA_CONN_TIME_OUT, ++i);
            order.put(OPTION_RSA_READ_TIME_OUT, ++i);
            order.put(OPTION_RSA_ENC_ALG_LIST, ++i);
            order.put(OPTION_RSA_LDAP_ATTR, ++i);
            order.put(OPTION_RSA_IDS_NAME, ++i);

        }
        @Override
        public int compare(Option o1, Option o2)
        {
            return order.get(o1.getOpt()) - order.get(o2.getOpt());
        }
    }

    private static String getPaddingString(int padding)
    {
        StringBuilder sb = new StringBuilder();
        for(int i = 0; i < padding; i++)
        {
            sb.append(" ");
        }
        return sb.toString();
    }
} //end of package
