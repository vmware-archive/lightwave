/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.net.MalformedURLException;
import java.net.URL;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.Validate;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * configuration data struct for idp settings.
 *
 */
public class IDPConfiguration {

    private String alias;

    private String entityID;

    private X509Certificate signingCertificate;

    private List<String> nameIDFormats;

    private List<SingleSignOnService> singleSignOnServices;

    private List<SingleLogoutService> singleLogoutServices;

    // non-SAML attributes
    private static final Logger logger = LoggerFactory
            .getLogger(IDPConfiguration.class);

    private boolean useSiteAffinityIfAvailable = true;

    private int clockTolerance; // optional, in seconds

    /**
     * Construct IDPConfiguration object. This object correspond to SAML
     * metadata protocal IDP EntityDescriptorType.
     *
     * @param alias
     *            Required.
     * @param entityID
     *            Required.
     * @signingCertificate Optional (Required to authenticate sso/slo
     *                     signature).
     * @param singleSignOnServices
     *            Required.
     * @param singleLogoutServices
     *            Required for SLO.
     * @param nameIDFormats
     *            Optional. name id formats expected by idp. Not avail means no
     *            restriction.
     * @param clockTolerance
     *            Clock tolerance used for time constraint validation of server
     *            response.
     * @param useSiteAffinityIfAvailable
     *            Is SSO site affinity to be used in determine IDP host node.
     */
    public IDPConfiguration(String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices, int clockTolerance,
            boolean useSiteAffinityIfAvailable) {
        Validate.notEmpty(entityID, "entityID");
        Validate.notEmpty(alias, "alias");
        Validate.notNull(singleSignOnServices);
        this.entityID = entityID;
        this.alias = alias;
        this.nameIDFormats = nameIDFormats;
        this.signingCertificate = signingCertificate;
        this.singleSignOnServices = singleSignOnServices;
        this.singleLogoutServices = singleLogoutServices;
        this.clockTolerance = clockTolerance;
        this.useSiteAffinityIfAvailable = useSiteAffinityIfAvailable;
    }

    /**
     * Construct IDPConfiguration object with default clock tolerance value of
     * 600 seconds. This object correspond to SAML metadata protocal IDP
     * EntityDescriptorType.
     *
     * @param alias
     *            Required.
     * @param entityID
     *            Required.
     * @signingCertificate Optional (Required to authenticate sso/slo
     *                     signature).
     * @param singleSignOnServices
     *            Required.
     * @param singleLogoutServices
     *            Required for SLO.
     * @param nameIDFormats
     *            Optional. name id formats expected by idp. Not avail means no
     *            restriction.
     */
    public IDPConfiguration(String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices) {
		this(alias, entityID, signingCertificate, nameIDFormats,
                singleSignOnServices, singleLogoutServices, 600, true);
    }


    public void setAlias(String alias) {
        Validate.notEmpty(alias, "alias");
        this.alias = alias;
    }

    public String getAlias() {
        return alias;
    }

    public void setEntityID(String entityID) {
        Validate.notEmpty(entityID, "entityID");
        this.entityID = entityID;
    }

    public String getEntityID() {
        return entityID;
    }

    public void setSigningCertificate(X509Certificate signingCertificate) {
        this.signingCertificate = signingCertificate;
    }

    public X509Certificate getSigningCertificate() {
        return signingCertificate;
    }

    public void setNameIDFormats(List<String> nameIDFormats) {
        this.nameIDFormats = nameIDFormats;
    }

    public List<String> getNameIDFormats() {
        return nameIDFormats;
    }

    public void setSingleSignOnServices(List<SingleSignOnService> singleSignOnServices) {
        Validate.notNull(singleSignOnServices, "SingleSignOnService can not be set to null");
        this.singleSignOnServices = singleSignOnServices;
    }

    /**
     * Site-Affinity capable querying of SSO services objects.
     *
     * @return list of registered SingleSignOnService endpoints
     */
    public List<SingleSignOnService> getSingleSignOnServices() {

        if (isUseSiteAffinityIfAvailable()) {
            List<SingleSignOnService> updatedSsos = new ArrayList<SingleSignOnService>();
            String siteFQDN = null;

            try {
                siteFQDN = SiteAffinity.getAffinitiedDC();
            } catch (WebssoClientException e) {
                // AFD can not determine the affinitized node. We should return
                // the registered SSO service nodes
                logger.error("AFD siteaffinity failed!");
            }

            if (siteFQDN == null) {
                //site affinity is not available, fall back to traditional handling.
                return this.singleSignOnServices;
            }

            //update the list
            for (SingleSignOnService service : this.singleSignOnServices) {
                String updatedURL;
                try {
                    updatedURL = updateHostNameInURL(service.getLocation(),
                            siteFQDN);
                } catch (MalformedURLException e) {
                    // do not update if it is not in URL format
                    logger.warn(
                            "The given entity ID is not in URL form. Not able to update single sign-on service location.",
                            e);
                    updatedSsos.add(service);
                    continue;
                }
                updatedSsos.add(new SingleSignOnService(updatedURL, service
                        .getBinding()));
            }
            return updatedSsos;
        } else {
            return this.singleSignOnServices;
        }

    }

    public void setSingleLogoutServices(List<SingleLogoutService> singleLogoutServices) {
        this.singleLogoutServices = singleLogoutServices;
    }

    /**
     * Site-Affinity capable querying of SLO services objects.
     *
     * @return list of registered SingleLogoutService endpoints
     */
    public List<SingleLogoutService> getSingleLogoutServices() {
        if (isUseSiteAffinityIfAvailable()) {
            List<SingleLogoutService> updatedSlos = new ArrayList<SingleLogoutService>();
            String siteFQDN = null;

            try {
                siteFQDN = SiteAffinity.getAffinitiedDC();
            } catch (WebssoClientException e) {
                // AFD can not determine the affinitized node. We should return
                // the registered SSO service nodes
                logger.error("AFD siteaffinity failed!");
            }

            if (siteFQDN == null) {
                //site affinity is not available, fall back to traditional handling.
                return this.singleLogoutServices;
            }

            for (SingleLogoutService service : this.singleLogoutServices) {
                String updatedURL;
                try {
                    updatedURL = updateHostNameInURL(service.getLocation(),
                            siteFQDN);
                } catch (MalformedURLException e) {
                    // do not update if it is not in URL format
                    logger.warn(
                            "The given entity ID is not in URL form. Not able to update single logout service location.",
                            e);
                    updatedSlos.add(service);
                    continue;
                }
                updatedSlos.add(new SingleLogoutService(updatedURL, service
                        .getBinding()));
            }
            return updatedSlos;

        } else {
            return this.singleLogoutServices;
        }
    }

    /**
     * Return clock tolerance used in time sensitive validation for IDP
     * response. The value is in seconds
     *
     */
    public int getClockTolerance() {
        return clockTolerance;
    }

    /**
     * Tolerance used in time sensitive validation for IDP response. The default
     * value is 600 seconds.
     *
     * @param clockTolerance
     *            in seconds
     */
    public void setClockTolerance(int clockTolerance) {
        Validate.isTrue(clockTolerance >= 0, "Negative clock tolerance is not allowed!");
        this.clockTolerance = clockTolerance;
    }

    /**
     * Check whether a given entityID represent one of the PSC node in the PSC
     * farm or not.
     *
     * In traditional mode, it is simple comparison to the registered IDP entity id.
     *
     * With SSO site-affinity enabled IDP, an issuer of incoming message could
     * be a failed-over PSC node. In that case the entityID of the issuer could
     * be different than the registered IDP metadata.
     *
     * It compares the node against a list of PSC of the current domain.
     *
     * @param entityID
     *            entityID of any node in PSC farm for the domain
     * @return if the entityID matches
     */
    public boolean isSameEntity(String failoverEntityID) {

        // Step 1 compare entityID directly

        Validate.notEmpty(failoverEntityID, "Empty failover entityID");
        boolean result = failoverEntityID.equals(this.getEntityID());

        try {
            // Step 2. Check if the failover entityID URL is from the registred
            // PSC
            if (!result && isUseSiteAffinityIfAvailable()) {
                URL failover_URL = new URL(failoverEntityID);
                String hostNameCandidate = failover_URL.getHost();

                List<String> domainControllers = null;

                try {
                    domainControllers = SiteAffinity.enumDCEntries();
                } catch (WebssoClientException e) {
                    logger.error("AFD siteaffinity is failing! WebSSO is not using siteaffinity for this request.");
                    // fall back to tradition mode
                }

                if (domainControllers != null) {
                    if (domainControllers.contains(hostNameCandidate)) {

                        // Step 3. match the rest part of entityID to avoid mess
                        // with
                        // another tenant.
                        result = failoverEntityID.equals(updateHostNameInURL(
                                this.getEntityID(), hostNameCandidate));
                    }
                    //else done searching
                }

            }
        } catch (MalformedURLException e) {
            logger.error("The given entity ID is not in URL form. Compared to registered IDP node only. Failover entityID is "
                    + failoverEntityID);
            // return result
        }
        return result;

    }

    /**
     * Update the hostname portion of a url
     *
     * @param regEntityID
     * @param siteFQDN
     * @return updated url
     * @throws MalformedURLException
     */
    private String updateHostNameInURL(String url, String siteFQDN)
            throws MalformedURLException {

        Validate.notEmpty(url, "Null URL string.");
        URL oldUrl = new URL(url);

        URL newUrl = new URL(oldUrl.getProtocol(), siteFQDN, oldUrl.getPort(),
                oldUrl.getFile());
        return newUrl.toExternalForm();
    }

    /**
     *
     * @return true will use site-affinity feature if AFD client enables it.
     */
    public boolean isUseSiteAffinityIfAvailable() {
        return useSiteAffinityIfAvailable;
    }


}
