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

package com.vmware.identity.idm.server.provider.activedirectory;

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 1/28/13
 * Time: 4:45 PM
 * To change this template use File | Settings | File Templates.
 */

import com.vmware.identity.interop.domainmanager.DomainTrustInfo;


public class ServerKrbUtils
{
    //http://www.adiscon.com/common/en/securityreference/kerberos-failures.php
    //see also http://technet.microsoft.com/en-us/library/bb463166.aspx
    /**
     * This is due to a workstation restriction on the account, or a logon time
     * restriction, or logon attempt outside logon hours, or account disabled,
     * expired, or locked out
     */
    public static final int KDC_ERR_CLIENT_REVOKED = 0x12; // Clients credentials have been revoked

    /**
     * Password has expired - change password to reset
     */
    public static final int KDC_ERR_KEY_EXPIRED = 0x17; // Password has expired - change password to reset

    public static boolean IsTwoWayTrusted(DomainTrustInfo trust)
    {
        return trust.IsInforest || (trust.IsInBound && trust.IsOutBound);
    }

    public static boolean IsOneWayTrusted(DomainTrustInfo trust)
    {
        return trust.IsOutBound;
    }

    public static boolean IsTwoWayForestTrust(DomainTrustInfo trust)
    {
        return trust.IsRoot && trust.IsInBound && trust.IsOutBound;

    }
}