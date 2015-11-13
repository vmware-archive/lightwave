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
 * Define specific layers at which performance data is measured within identity
 * server system
 *
 */
public enum PerfMeasurementInterface
{
    PerfMeasurementInterfaceDefault("DefaultItf"), // can be used when there is only one monitored function in the interface.
    // STS
    ISoapMsgHandlerOB("SOAP message interface handler outbound"),
    MultiTenantSTS("Layer immediately underneath STS Service endpoint interface"),
    Authenticator("Authenticator interface"),
    TokenAuthority("Token authority interface"),
    TokenValidator("Token Validator interface"),

    //IDM
    RMIIdmServer("IdentityManager RMI interface"),

    // two supporting APIs for IDM's authentication functionality
    PlatformAPI("PlatformAPI"),
    JavaxSecurityAuthzAPI("javax.security.auth API");

    private final String description;

    private PerfMeasurementInterface(String desc)
    {
        assert !desc.isEmpty();
        this.description = desc;
    }

    /**
     * Getter
     * @return description of the interface
     */
    public String getDescription()
    {
        return description;
    }
}
