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

import java.net.URL;

import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.ServerCommunicationException;

/**
 * Interface providing methods for SOAP communication
 */
interface SoapBinding {

    /**
     * Sends the message to a SOAP service.
     *
     * @param message
     *            , required
     * @return result of the SOAP call, not null
     *
     * @throws ServerCommunicationException
     *             when there is an error during the communication with the
     *             remote STS server
     * @throws ParserException
     *             when the response cannot be parsed as a SOAP message
     * @throws SoapFaultException
     *             when the server responds with a SOAP fault
     * @throws CertificateValidationException
     *             when cannot establish SSL connection with the STS server
     *             because STS SSL certificate is not trusted
     */
    public SoapMessage sendMessage(SoapMessage message, URL serviceLocationURL) throws ServerCommunicationException,
            ParserException, SoapFaultException, CertificateValidationException;
}
