/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.domainmanager;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/5/12
 * Time: 11:53 AM
 * To change this template use File | Settings | File Templates.
 */
public interface IDomainAdapter {

    /* Enumerate domain trusts using implicit credential that service is running as */
    public DomainTrustInfo[] getDomainTrusts(
            String domainName
    );

    /* Check whether the current host is joined to an AD domain
     * If joined, the domain name is returned
     */
    public DomainControllerInfo getDomainJoinInfo();

    /* Get domain controller information for a given domain */
    public DomainControllerInfo getDcInfo(String domainName);

    /* Get domain controller information for a given domain with DS_FORCE_REDISCOVERY*/
    public DomainControllerInfo getDcInfoWithRediscover(String domainName);
}
