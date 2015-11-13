package com.vmware.identity.websso.client;

import java.security.cert.X509Certificate;
import java.util.List;

/**
 * @author schai
 *
 */
public class IDPConfigurationFactory {

    private static final int DEFAULT_CLOCK_TOLERANCE_SEC = 600; //in seconds

	/**
     * create an instance of IDPConfiguration that not using site-affinity.
     *
     * @param alias
     * @param entityID
     * @param signingCertificate
     * @param nameIDFormats
     * @param singleSignOnServices
     * @param singleLogoutServices
     * @param clockTolerance
     * @return IDPConfiguration
     */
    public static IDPConfiguration createIDPConfigurationWithoutSiteAffinity(
            String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices)
    {
        return new IDPConfiguration(alias, entityID, signingCertificate,
                nameIDFormats, singleSignOnServices, singleLogoutServices, DEFAULT_CLOCK_TOLERANCE_SEC,
                false);
    }

    /**
     * create an instance of 'site-affinitized' IDPConfiguration
     *
     * @param alias
     * @param entityID
     * @param signingCertificate
     * @param nameIDFormats
     * @param singleSignOnServices
     * @param singleLogoutServices
     * @param clockTolerance
     * @return IDPConfiguration
     */
    public static IDPConfiguration createAffinitizedIDPConfiguration(String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices)
    {
        return new IDPConfiguration(alias, entityID,
                signingCertificate,
                nameIDFormats, singleSignOnServices, singleLogoutServices, DEFAULT_CLOCK_TOLERANCE_SEC,
                true);
    }

}
