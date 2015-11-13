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

import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPFactory;

import com.vmware.identity.wstrust.client.ServerCommunicationException;

/**
 * Mock implementation of the SoapBinding interface. For testing purposes.
 */
public class SoapBindingMock implements SoapBinding {

    private final String _exceptionMessage;
    private int _delay = 0;

    /**
     * Creates new mock object that will throw no errors
     */
    public SoapBindingMock() {
        _exceptionMessage = null;
    }

    /**
     * Creates new mock object that will throw a SoapFault for every request
     *
     * @param exceptionMessage
     *            the FaultCode of the SoapFault
     */
    public SoapBindingMock(String exceptionMessage) {
        _exceptionMessage = exceptionMessage;
    }

    @Override
    public SoapMessage sendMessage(SoapMessage message, URL serviceURL) throws ServerCommunicationException,
            SoapFaultException {
        delay();
        if (_exceptionMessage != null) {
            try {
                throw new SoapFaultException("SOAP fault occured", new SoapFault(SOAPFactory.newInstance().createFault(
                        _exceptionMessage, new QName(_exceptionMessage))));
            } catch (SOAPException e) {
                throw new RuntimeException("Error creating SOAPFault object");
            }
        }

        return message;
    }

    /**
     * @return number of seconds the response will be delayed
     */
    public int getDelay() {
        return _delay;
    }

    /**
     * Sets how much the response will be delayed
     *
     * @param delay
     *            number seconds >= 0
     */
    public void setDelay(int delay) {
        _delay = delay;
    }

    private void delay() {
        if (_delay < 1) {
            return;
        }

        try {
            Thread.sleep(_delay * 1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}
