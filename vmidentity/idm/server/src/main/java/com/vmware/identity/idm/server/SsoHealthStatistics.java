/*
 *
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
 *
 */

package com.vmware.identity.idm.server;

import java.util.Calendar;
import java.util.Hashtable;
import java.util.concurrent.atomic.AtomicInteger;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.SsoHealthStatsData;

/**
 * This class provides SSO health statistics like - uptime of IDM and STS
 * services - No. of tokens generated and per tenant. - No. of tokens renewed
 * and per tenant. We may add some more statistics going forward.
 *
 * @author dmehta
 *
 */
public class SsoHealthStatistics {

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(SsoHealthStatistics.class);
    private static AtomicInteger tokensGenerated = new AtomicInteger();
    private static AtomicInteger tokensRenewed = new AtomicInteger();
    private static AtomicInteger issuedTokens = new AtomicInteger();

    private static Hashtable<String, AtomicInteger> generatedTokensMap = new Hashtable<String, AtomicInteger>();
    private static Hashtable<String, AtomicInteger> renewedTokensMap = new Hashtable<String, AtomicInteger>();

    private static long startUpTimeIDM;
    private static long startUpTimeSTS;

    /**
     * This is the API which is exposed to admin layer to return all the Sso
     * Health Statistics for a given client and other related information, like
     * - uptime for IDM and STS. - total generated and renewed tokens.
     *
     * @param tenant
     * @return
     */
    public SsoHealthStatsData getSsoStatistics(String tenant) throws Exception {
        logger.trace("Returning all SSO Health Statistics Data for tenant {} ", tenant);
        SsoHealthStatsData stats = null;
        try {
            // return complete SsoHealthStatistics object
            stats = new SsoHealthStatsData(tenant, tokensGenerated.intValue(),
                    tokensRenewed.intValue(),
                    getGeneratedTokensForTenant(tenant),
                    getRenewedTokensForTenant(tenant), getUpTimeIDMService(),
                    getUpTimeSTSService());
        } catch (Exception e) {
            logger.error("Caught exception while getting Sso Health Statistics.", e);
            throw e;
        }
        return stats;
    }

    /**
     * remove tenant statistics when tenant gets removed.
     *
     * @param tenant
     */
    public void removeTenantStats(String tenant) {
        try {
                generatedTokensMap.remove(tenant);
                renewedTokensMap.remove(tenant);
        } catch (Exception e) {
            logger.error("Caught exception while clearing Health Statistics.", e);
        }
    }

    /**
     * This will set the upTime of the IDM service at the initialization of
     * Identity Manager.
     */
    public void setUpTimeIDMService() {

        logger.trace("Setting uptime of IDM service.");
        try {
            startUpTimeIDM = System.nanoTime();
        } catch (Exception e) {
            logger.error("Failed to set upTime of IDM Service", e);
        }
    }

    /**
     * This method will increment the tokens generated for a given tenant when
     * new token is generated from IDM and STS
     *
     */
    public void incrementGeneratedTokens(String tenant) {

        logger.trace("Incrementing generated tokens for tenant: " + tenant);
        try {
            AtomicInteger count;
            synchronized (this) {
                if ((count = generatedTokensMap.get(tenant)) == null) {
                    generatedTokensMap.put(tenant, new AtomicInteger(1));
                } else {
                    count.incrementAndGet();
                }
            }
            logger.trace("Tokens generated for tenant: {} is- {} ",tenant, generatedTokensMap.get(tenant));
            // increment total generated tokens as well
            incrementTotalGenerated();

        } catch (Exception e) {
            logger.error("Unable to increment generated tokens", e);
        }
    }

    /**
     * This will increment the total tokens renewed for a given tenant whenever
     * any token is renewed from IDM and STS.
     *
     */
    public void incrementRenewedTokens(String tenant) {

        logger.trace("Incrementing renewed tokens for tenant: {}", tenant);
        try {
            AtomicInteger count;
            synchronized (this) {
                if ((count = renewedTokensMap.get(tenant)) == null) {
                    renewedTokensMap.put(tenant, new AtomicInteger(1));
                } else {
                    count.incrementAndGet();
                }
            }
            logger.trace("Tokens renewed for tenant: {} is- {}", tenant, renewedTokensMap.get(tenant));

            // increment total renewed tokens as well
            incrementTotalRenewed();

        } catch (Exception e) {
            logger.error("Unable to increment renewed tokens", e);
        }
    }

    /**
     * This will return the upTime of the IDM service
     *
     * @return
     */
    private long getUpTimeIDMService() throws Exception {
        long upTimeIDMSec = 0;
        try {

            if (startUpTimeIDM > 0) {
                // subtract current time from startUpTimeIDM and return the
                // total seconds IDM service is up.
                upTimeIDMSec = (System.nanoTime() - startUpTimeIDM) / 1000000000;
            } else {
                logger.info("IDM service not started.");
            }
        } catch (Exception e) {
            logger.error("Failed to get upTime of IDM Service", e);
            throw e;
        }
        return upTimeIDMSec;
    }

    /**
     * This methods returns the time since STS service is up and running.
     *
     * @return
     */
    private long getUpTimeSTSService() throws Exception {
        long upTimeSTSSec = 0;
        try {

            if (startUpTimeSTS > 0) {
                // subtract current time from startUpTimeSTS and return the
                // total seconds STS service is up.
                upTimeSTSSec = (System.nanoTime() - startUpTimeSTS) / 1000000000;
            } else {
                logger.info("STS service not started.");
            }
        } catch (Exception e) {
            logger.error("Failed to get upTime of STS Service", e);
            throw e;
        }
        return upTimeSTSSec;
    }

    /**
     * This will set the upTime of STS service when first token gets generated.
     */
    private void setUpTimeSTSService() throws Exception {

        logger.trace("Setting uptime of STS service.");
        try {
            startUpTimeSTS = System.nanoTime();
        } catch (Exception e) {
            logger.error("Failed to set upTime of STS Service", e);
            throw e;
        }
    }

    private void incrementTotalGenerated() throws Exception {

        if (issuedTokens.getAndIncrement() == 0) {
            setUpTimeSTSService();
        }
        tokensGenerated.incrementAndGet();
        logger.trace("Total tokens generated are: {}", tokensGenerated);
    }

    /**
     * This method will return the number of tokens generated for a given
     * tenant.
     *
     */
    private int getGeneratedTokensForTenant(String tenant) throws Exception {

        // If tenant exists in Map, get the current generated token count.
        int count = 0;
        try {
            AtomicInteger value = generatedTokensMap.get(tenant);
            if (value != null) {
                count = value.intValue();
            }
            logger.trace("Tokens generated for tenant: {} are {}",tenant, count);
        } catch (Exception e) {
            logger.error("Unable to get generated tokens for tenant ", e);
            throw e;
        }
        return count;
    }

    /**
     * This would return total number of generated tokens.
     *
     * @return
     */
    private int getTotalGeneratedTokens() throws Exception {
        int count = 0;
        logger.trace("Getting total generated tokens.");
        try {
            count = tokensGenerated.intValue();

        } catch (Exception e) {
            logger.error("Unable to get generated tokens.", e);
            throw e;
        }
        return count;
    }

    private void incrementTotalRenewed() throws Exception {

        if (issuedTokens.getAndIncrement() == 0) {
            setUpTimeSTSService();
        }
        tokensRenewed.incrementAndGet();

        logger.trace("Total tokens renewed are:{} ", tokensRenewed);
    }

    /**
     * This would return the number of renewed tokens for a given tenant.
     *
     * @return
     */
    private int getRenewedTokensForTenant(String tenant) throws Exception {

        // If tenant exists in Map, get the renewed token count for that tenant.
        Integer renewedTokens = 0;
        try {
            AtomicInteger value = renewedTokensMap.get(tenant);
            if (value != null) {
                renewedTokens = value.intValue();
            }
            logger.trace("Tokens renewed for tenant: {} are {}",tenant, renewedTokens);
        } catch (Exception e) {
            logger.error("Unable to get renewed tokens for tenant ", e);
            throw e;
        }
        return renewedTokens;
    }

    /**
     * This would return total number of renewed tokens.
     *
     * @return
     */
    private int getTotalRenewedTokens() throws Exception {

        int count = 0;
        logger.trace("Getting total renewed tokens.");
        try {
            count = tokensRenewed.intValue();

        } catch (Exception e) {
            logger.error("Unable to get renewed tokens.", e);
            throw e;
        }
        return count;
    }
}
