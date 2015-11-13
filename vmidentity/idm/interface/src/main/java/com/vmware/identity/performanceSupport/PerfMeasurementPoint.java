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
package com.vmware.identity.performanceSupport;


/**
 * <p>
 * List of transactions for which performance data are measured within identity
 * server system.
 * <p>
 * Generally speaking each of them is associated with an instance of
 * {@code PerfMeasurementInterface}, and a string indicating the transaction
 * on that interface.
 *
 */
public enum PerfMeasurementPoint
{
    /*
     * This one can be used when monitored point is, for example, of filter style
     */
    Default             (PerfMeasurementInterface.PerfMeasurementInterfaceDefault,
                        "DefaultTransaction"),

    /*
     * Section for SOAP message layer
     */
    SoapHandlerOBChallenge    (PerfMeasurementInterface.ISoapMsgHandlerOB,
                              "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Issue"),
    SoapHandlerOBIssue        (PerfMeasurementInterface.ISoapMsgHandlerOB,
                              "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue"),
    SoapHandlerOBRenew        (PerfMeasurementInterface.ISoapMsgHandlerOB,
                              "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Renew"),
    SoapHandlerOBValidate     (PerfMeasurementInterface.ISoapMsgHandlerOB,
                              "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Validate"),

    /*
     * Section for SOAP message layer and STSService SEI layer
     */
    SoapActionChallenge (PerfMeasurementInterface.MultiTenantSTS,
                        "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Issue"),
    SoapActionIssue     (PerfMeasurementInterface.MultiTenantSTS,
                        "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue"),
    SoapActionRenew     (PerfMeasurementInterface.MultiTenantSTS,
                        "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Renew"),
    SoapActionValidate  (PerfMeasurementInterface.MultiTenantSTS,
                        "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Validate"),

    /*
     * Section for Authenticate interface
     */
    CompositeAuthenticate(PerfMeasurementInterface.Authenticator, "CompositeAuthenticate"),

    /*
     * Section for TokenAuthority interface
     */
    IssueToken(PerfMeasurementInterface.TokenAuthority, "issueToken by samlAuthority"),
    // ... insert more items here

    /*
     * Section for TokenValidator interface
     */
    Validate(PerfMeasurementInterface.TokenValidator, "validate samlToken "),

    /*
     * Section for IdmServer RMI interface
     */
    IDMAuthenticate             (PerfMeasurementInterface.RMIIdmServer,
                                "IdentityManager.authenticate by username/password"),
    IDMAuthenticateGSS          (PerfMeasurementInterface.RMIIdmServer,
                                "IdentityManager.authenticate by GSS"),
    IDMGetAttributeValues       (PerfMeasurementInterface.RMIIdmServer,
                                "IdentityManager.getAttributeValues"),
    IDMFindDirectParentGroups   (PerfMeasurementInterface.RMIIdmServer,
                                "IdentityManager.findDirectParentGroups"),

    IDMPeriodicRefreshTenantCertificates (PerfMeasurementInterface.RMIIdmServer,
                                "IdentityManager.TenantCacheCheckerThread.refreshTenantCertificate"),
    // ... insert more items here

    /*
     *  Section for Platform interface access from IDM, where the performanceSupport package
     *  locates and accessible from STS.
     */
    // Platform cannot be instrumented internally because it would otherwise require IDM as
    // a dependency of Platform
    LdapBindConnection  (PerfMeasurementInterface.PlatformAPI, "ServerUtil.bindConnection"),
    KerberosLogin       (PerfMeasurementInterface.JavaxSecurityAuthzAPI, "LoginContext.login");
    // ... insert more items here

    static {
        //Ensure the uniqueness of the instance with the fields specified
        for (PerfMeasurementPoint ptCurrent : PerfMeasurementPoint.values())
        {
            int numMatched = 0;
            for (PerfMeasurementPoint pt : PerfMeasurementPoint.values())
            {
                if (ptCurrent.match(pt.itf, pt.txId))
                    numMatched++;
            }
            assert (numMatched == 1) : "duplicated fields for " + ptCurrent;
        }
    }

    private final PerfMeasurementInterface itf;
    private final String txId;

    private PerfMeasurementPoint(PerfMeasurementInterface aItf, String aTxId)
    {
        assert aItf != null;
        assert !aTxId.isEmpty();

        itf = aItf;
        txId = aTxId;
    }

    /**
     * Lookup the matching instance with the specified {@code aItf} and {code aTxId}
     *
     * @param aItf     cannot be null
     * @param aTxId    cannot be empty
     * @return         the unique matching instance
     */
    public static PerfMeasurementPoint lookupById(PerfMeasurementInterface aItf, String aTxId)
    {
        for (PerfMeasurementPoint e : PerfMeasurementPoint.values())
        {
            if (e.match(aItf, aTxId))
                return e;
        }
        return Default;
    }

    /**
     * Getter
     * @return
     */
    public PerfMeasurementInterface getItf()
    {
        return itf;
    }

    /**
     * Getter
     * @return
     */
    public String getTxId()
    {
        return txId;
    }

    private boolean match(PerfMeasurementInterface aItf, String aTxId)
    {
        return this.itf == aItf && this.txId.equalsIgnoreCase(aTxId);
    }
}
