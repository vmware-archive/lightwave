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
 * Authentication types used for web based login 
 */
public enum UserAuthenticationType {

    /**
     * A combination of a user name and password will be used for
     * authentication.
     */
    PASSWORD,

    /**
     * Windows based authenticaiton
     */
    WINDOWS,

    /**
     * CAC - Certificate based Authentication
     */
    CERT,
    
    /**
     * RSA SecurID based authentication
     */
    RSA_SECUR_ID
}