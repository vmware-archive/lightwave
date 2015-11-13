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

import javax.xml.soap.Name;
import javax.xml.soap.SOAPFault;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Wrapper over SOAPFault. Encapsulates parsing fault messages.
 */
class SoapFault {

    private final SOAPFault fault;
    private final Logger log = LoggerFactory.getLogger(SoapFault.class);

    /**
     * Parses the SOAPFault
     *
     * @param fault
     */
    public SoapFault(SOAPFault fault) {
        log.debug("Creating SoapFault");
        this.fault = fault;
    }

    /**
     * Get the fault code as name
     *
     * @return fault code object as a SAAJ <code>Name</code> object
     */
    public Name getFaultCodeAsName() {
        return this.fault.getFaultCodeAsName();
    }

    /**
     * Get the fault message.
     *
     * @return fault message
     */
    public String getFaultMessage() {
        return constructFaultMessage();
    }

    /**
     * Constructs the fault message from SOAPFault object
     *
     * @param fault
     * @return
     */
    private String constructFaultMessage() {
        return this.fault.getFaultCode() + ": " + this.fault.getFaultString();
    }
}
