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
package com.vmware.identity.idm;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Static representation of the configuration parameters for the IDM
 * 	It gets initialized from specified InputStream and then can be accessed
 *    using specific get...() methods
 *  Get()... methods will fail if not initialized
 *    (more specifically assertInitialized() will fail)
 */
public final class ServerConfig {
	private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(ServerConfig.class);

	// property keys
	private static final String TENANT = "tenant";
	private static final String DEFAULT = "-default";
	private static final String ENTITY_ID = "entity-id";
	private static final String CLOCK_TOLERANCE = "clock-tolerance";
	private static final String DELEGATION_COUNT = "delegation-count";
	private static final String BRAND_NAME = "brand-name";
	private static final String RENEW_COUNT = "renew-count";
	private static final String MAXIMUM_BEARER_TOKEN_LIFETIME = "maximum-bearer-token-lifetime";
	private static final String MAXIMUM_HOK_TOKEN_LIFETIME = "maximum-hok-token-lifetime";
	private static final String CREDENTIALS_FILENAME = "credentials.key-store-filename";
	private static final String CREDENTIALS_ALIAS = "credentials.key-store-alias";
	private static final String CREDENTIALS_PASSWORD = "credentials.key-store-password";

	private static final String RELYING_PARTY = "relying-party";
	private static final String URL = "url";
	private static final String AUTHN_REQUESTS_SIGNED = "authn-requests-signed";

	private static final String ASSERTION_CONSUMER_SERVICE = "assertion-consumer-service";
    private static final String SINGLE_LOGOUT_SERVICE = "single-logout-service";
	private static final String BINDING = "binding";
	private static final String ENDPOINT = "endpoint";

	private static final String AD_PROVIDER = "ad-provider";
	private static final String KDC = "kdc";
	private static final String USER_NAME = "user-name";
	private static final String USER_PASSWORD = "user-password";
	private static final String SEARCH_BASE_DN = "search-base-dn";

	private static final String ADMIN_USERNAME = "-admin-username";
	private static final String ADMIN_PASSWORD = "-admin-password";

	private static Properties props;

	private static void assertInitialized() {
		Validate.notNull(props);
	}

	/**
	 * Initialize from specified input stream
	 * @param is
	 * @throws IOException
	 */
	public static void initialize(InputStream is) throws IOException {
		logger.debug("ServerConfig initialize called");

		if (is != null) {
			props = new Properties();
			props.load(is);
		} else {
			// clear
			props = null;
		}
	}

	/**
	 * Return tenant name for specified index or return null
	 * @param index
	 * @return
	 */
	public static String getTenant(int index) {
		logger.debug("ServerConfig queried for tenant {}", index);

		assertInitialized();

		String key = TENANT + "-" + index;
		return props.getProperty(key, null);
	}

	/**
	 * Return default tenant
	 * @return
	 */
	public static String getDefaultTenant() {
		logger.debug("ServerConfig queried for default tenant");

		assertInitialized();

		String key = TENANT + DEFAULT;
		return props.getProperty(key, null);
	}

	 public static String getTenantAdminUsername() {
	        logger.debug("ServerConfig queried for tenant test admin username");
	        assertInitialized();
	        String key = TENANT + ADMIN_USERNAME;
	        return props.getProperty(key, null);
	    }

	    public static String getTenantAdminPassword() {
	        logger.debug("ServerConfig queried for tenant test admin password");
	        assertInitialized();
	        String key = TENANT + ADMIN_PASSWORD;
	        return props.getProperty(key, null);
	    }

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static String getTenantEntityId(String tenant) {
		logger.debug("ServerConfig queried entity ID for tenant {}", tenant);

		assertInitialized();

		String key = tenant + "." + ENTITY_ID;
		return props.getProperty(key, null);
	}

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static Long getTenantClockTolerance(String tenant) {
		logger.debug("ServerConfig queried clock tolerance for tenant {}", tenant);

		assertInitialized();

		String key = tenant + "." + CLOCK_TOLERANCE;
		return Long.parseLong(props.getProperty(key, null));
	}

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static Integer getTenantDelegationCount(String tenant) {
		logger.debug("ServerConfig queried delegation count for tenant {}", tenant);

		assertInitialized();

		String key = tenant + "." + DELEGATION_COUNT;
		return Integer.parseInt(props.getProperty(key, null));
	}

	 /**
     * Return tenant setting
     * @param tenant
     * @return
     */
    public static String getTenantBrandName(String tenant) {
        logger.debug("ServerConfig queried brand name for tenant {}", tenant);

        assertInitialized();

        String key = tenant + "." + BRAND_NAME;
        String brandname= props.getProperty(key, null);
        logger.debug("brandname obtained ", brandname);
        return brandname;
    }

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static Integer getTenantRenewCount(String tenant) {
        logger.debug("ServerConfig queried renew count for tenant {} ", tenant);

		assertInitialized();

		String key = tenant + "." + RENEW_COUNT;
		return Integer.parseInt(props.getProperty(key, null));
	}

	 /**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static Long getTenantMaximumBearerTokenLifetime(String tenant) {
		logger.debug("ServerConfig queried maximum bearer token lifetime for tenant {}", tenant);

		assertInitialized();

		String key = tenant + "." + MAXIMUM_BEARER_TOKEN_LIFETIME;
		return Long.parseLong(props.getProperty(key, null));
	}

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static Long getTenantMaximumHokTokenLifetime(String tenant) {
		logger.debug("ServerConfig queried maximum hok token lifetime for tenant {}", tenant);

		assertInitialized();

		String key = tenant + "." + MAXIMUM_HOK_TOKEN_LIFETIME;
		return Long.parseLong(props.getProperty(key, null));
	}

	/**
	 * Return tenant setting
	 * @param tenant
	 * @return
	 */
	public static CredentialDescriptor getTenantCredentialDescriptor(String tenant) {
		logger.debug("ServerConfig queried credential descriptor for tenant {}", tenant);

		assertInitialized();

		CredentialDescriptor retval = new CredentialDescriptor();

		String key = tenant + "." + CREDENTIALS_FILENAME;
		retval.setFilename(props.getProperty(key, null));
		key = tenant + "." + CREDENTIALS_ALIAS;
		retval.setAlias(props.getProperty(key, null));
		key = tenant + "." + CREDENTIALS_PASSWORD;
		retval.setPassword(props.getProperty(key, null));

		return retval;
	}

	/**
	 * Return relying party name for specified tenant and index or return null
	 * @param tenant
	 * @param index
	 * @return
	 */
	public static String getRelyingParty(String tenant, int index) {
		logger.debug("ServerConfig queried for relying party for tenant {} and index {}",tenant, index);

		assertInitialized();

		String key = tenant + "." + RELYING_PARTY + "-" + index;
		return props.getProperty(key, null);
	}

	/**
	 * Return AD provider name for specified tenant and index or return null
	 * @param tenant
	 * @param index
	 * @return
	 */
	public static String getADProvider(String tenant, int index) {
		logger.debug("ServerConfig queried for AD provider for tenant {} and index {}",tenant, index);

		assertInitialized();

		String key = tenant + "." + AD_PROVIDER + "-" + index;
		return props.getProperty(key, null);
	}

	/**
	 * Return relying party setting
	 * @param relyingParty
	 * @return
	 */
	public static String getRelyingPartyUrl(String relyingParty) {
		logger.debug("ServerConfig queried URL for relying party {}", relyingParty);

		assertInitialized();

		String key = relyingParty + "." + URL;
		return props.getProperty(key, null);
	}

	/**
	 * Return relying party setting
	 * @param relyingParty
	 * @return
	 */
	public static Boolean getRelyingPartyAuthnRequestsSigned(String relyingParty) {
		logger.debug("ServerConfig queried AuthnRequestsSigned flag for relying party {}", relyingParty);

		assertInitialized();

		String key = relyingParty + "." + AUTHN_REQUESTS_SIGNED;
		return Boolean.parseBoolean(props.getProperty(key, null));
	}

	/**
	 * Return relying party setting
	 * @param relyingParty
	 * @return
	 */
	public static CredentialDescriptor getRelyingPartyCredentialDescriptor(String relyingParty) {
		logger.debug("ServerConfig queried credential descriptor for relying party {}", relyingParty);

		assertInitialized();

		CredentialDescriptor retval = new CredentialDescriptor();

		String key = relyingParty + "." + CREDENTIALS_FILENAME;
		retval.setFilename(props.getProperty(key, null));
		key = relyingParty + "." + CREDENTIALS_ALIAS;
		retval.setAlias(props.getProperty(key, null));
		key = relyingParty + "." + CREDENTIALS_PASSWORD;
		retval.setPassword(props.getProperty(key, null));

		return retval;
	}

	/**
	 * Return assertion consumer service name for specified relying party and index or return null
	 * @param relyingParty
	 * @param index
	 * @return
	 */
	public static String getAssertionConsumerService(String relyingParty, int index) {
		logger.debug("ServerConfig queried for AssertionConsumerService for relying party {} and index {}",relyingParty, index);

		assertInitialized();

		String key = relyingParty + "." + ASSERTION_CONSUMER_SERVICE + "-" + index;
		return props.getProperty(key, null);
	}

	/**
	 * Return default assertion consumer service setting
	 * @param relyingParty
	 * @return
	 */
	public static String getDefaultAssertionConsumerService(String relyingParty) {
		logger.debug("ServerConfig queried for default assertion consumer service for relying party {}", relyingParty);

		assertInitialized();

		String key = relyingParty + "." + ASSERTION_CONSUMER_SERVICE + DEFAULT;
		return props.getProperty(key, null);
	}

    /**
     * Return single logout service name for specified relying party and index or return null
     * @param relyingParty
     * @param index
     * @return
     */
    public static String getSingleLogoutService(String relyingParty, int index) {
        logger.debug("ServerConfig queried for SingleLogoutService for relying party {} and index {}",relyingParty , index);

        assertInitialized();

        String key = relyingParty + "." + SINGLE_LOGOUT_SERVICE + "-" + index;
        return props.getProperty(key, null);
    }


    /**
	 * Return service setting
	 * @param service
	 * @return
	 */
	public static String getServiceBinding(String service) {
		logger.debug("ServerConfig queried binding for service {} ", service);

		assertInitialized();

		String key = service + "." + BINDING;
		return props.getProperty(key, null);
	}

	/**
	 * Return service setting
	 * @param service
	 * @return
	 */
	public static String getServiceEndpoint(String service) {
		logger.debug("ServerConfig queried Endpoint for service {}", service);

		assertInitialized();

		String key = service + "." + ENDPOINT;
		return props.getProperty(key, null);
	}

	/**
	 * Return KDC name for specified AD Provider and index or return null
	 * @param adProvider
	 * @param index
	 * @return
	 */
	public static String getKdc(String adProvider, int index) {
		logger.debug("ServerConfig queried for KDC for AD Provider {} and index {}",adProvider, index);

		assertInitialized();

		String key = adProvider + "." + KDC + "-" + index;
		return props.getProperty(key, null);
	}

	/**
	 * Return AD Provider setting
	 * @param adProvider
	 * @return
	 */
	public static String getADProviderUserName(String adProvider) {
		logger.debug("ServerConfig queried user name for AD provider {}", adProvider);

		assertInitialized();

		String key = adProvider + "." + USER_NAME;
		return props.getProperty(key, null);
	}

	/**
	 * Return AD Provider setting
	 * @param adProvider
	 * @return
	 */
	public static String getADProviderUserPassword(String adProvider) {
		logger.debug("ServerConfig queried user password for AD provider {}", adProvider);

		assertInitialized();

		String key = adProvider + "." + USER_PASSWORD;
		return props.getProperty(key, null);
	}

	/**
	 * Return AD Provider setting
	 * @param adProvider
	 * @return
	 */
	public static String getADProviderSearchBaseDN(String adProvider) {
		logger.debug("ServerConfig queried search base DN for AD provider {}", adProvider);

		assertInitialized();

		String key = adProvider + "." + SEARCH_BASE_DN;
		return props.getProperty(key, null);
	}
}
