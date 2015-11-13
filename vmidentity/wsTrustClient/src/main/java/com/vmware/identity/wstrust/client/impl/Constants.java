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

/**
 * Class for shared constants
 */
public final class Constants {

    private Constants() {
    }

    public static final String WS_1_3_TRUST_JAXB_PACKAGE = "org.oasis_open.docs.ws_sx.ws_trust._200512";
    public static final String WS_1_4_TRUST_JAXB_PACKAGE = "org.oasis_open.docs.ws_sx.ws_trust._200802";
    public static final String WSSE_NAMESPACE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd";
    public static final String WS_TRUST_NAMESPACE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512";
    public static final String WS_TRUST14_NAMESPACE = "http://docs.oasis-open.org/ws-sx/ws-trust/200802";
    public static final String ENCODING_TYPE_BASE64 = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary";
    public static final String BINARY_EXCHANGE_TYPE_SPNEGO = "http://schemas.xmlsoap.org/ws/2005/02/trust/spnego";
    /** Separator between group name and domain name */
    public static final char GROUP_DOMAIN_SEPARATOR = '/';
}
