/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.test.util.config;

public abstract class STSConfiguration {
    protected String serverName;
    protected String tenantName;
    protected String metadataURL;

    protected STSConfiguration(String serverName) {
        this.serverName = serverName;
    }

    protected void setMetadataURL(String metadataUrl) {
        this.metadataURL = metadataUrl;
    }

    protected void setTenantName(String tenantName) { this.tenantName = tenantName; }

    public String getMetadataURL() {
        return metadataURL;
    }
}
