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

package com.vmware.identity.idm;

/**
 * Created by IntelliJ IDEA.
 * User: mpotapova
 * Date: 1/19/12
 * Time: 4:59 PM
 * To change this template use File | Settings | File Templates.
 */
public enum AuthenticationType {

    /**
     * A combination of a user name and password will be used for
     * authentication.
     */
    PASSWORD,

    /**
     * The SSO server will use kerberos credentials to authenticate against
     * the external server if it is supported.
     * <p>
     * This method is supported as long as the external server supports
     * kerberozied bind (i.e. Active Directory, VMware directory (lotus)
     */
    USE_KERBEROS,

    /**
     * The SSO server will reuse the process session credentials to
     * authenticate against the external server.
     * <p>
     * This method is only supported if the external server is of type
     * "Active Directory" and the SSO server runs as a user already
     * authenticated against the same Windows Domain the external server
     * belongs to (i.e. a Windows Domain user, the Local System account of the
     * Network Service account).
     *
     * Commented out for now since it is not used yet.
     */
    //REUSE_SESSION

    /**
     * Used internally only for LDAP binding to Vmdir.
     */
    SRP
}
