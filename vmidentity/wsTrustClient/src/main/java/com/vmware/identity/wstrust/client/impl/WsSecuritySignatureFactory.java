/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;

/**
 * Contains factory methods for various kinds of {@link WsSecuritySignature}
 * implementations.
 */
class WsSecuritySignatureFactory {

    /**
     * Creates WsSecuritySignature which KeyInfo elements points to an Assertion
     * which contains the signing certificate.
     *
     * @param config
     *            the {@link HolderOfKeyConfig} used in this signature
     * @param assertionId
     *            the Id of the Assertion which contain the signing certificate.
     * @return
     */
    public static WsSecuritySignature createWsSecuritySignatureAssertion(HolderOfKeyConfig config, String assertionId) {

        return new WsSecuritySignatureAssertion(config, assertionId);
    }

    /**
     * Creates WsSecuritySignature that inserts the Certificate in the
     * Holder-Of-Key config into the signature as a KeyInfo value.
     *
     * @param config
     *            the {@link HolderOfKeyConfig} used in this signature
     * @return
     */
    public static WsSecuritySignature createWsSecuritySignatureCertificate(HolderOfKeyConfig config) {

        return new WsSecuritySignatureCertificate(config);
    }

    /**
     * Creates WsSecuritySignature that does not sign messages - i.e. its
     * sign(SoapMessage) method does not change the SoapMessage.
     *
     * @return
     */
    public static WsSecuritySignature createWsEmptySecuritySignature() {
        return new WsEmptySecuritySignature();
    }
}
