/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.configure;

/*
* A throwable being thrown when STS web application is not deployed or when tomcat is in process of exploding the WAR.
*/
public class STSWebappNotDeployedException extends Exception {

    private static final long serialVersionUID = 495883060542471873L;

    public STSWebappNotDeployedException(String message) {
        super(message);
    }

    public STSWebappNotDeployedException(Throwable ex) {
        super(ex);
    }

    public STSWebappNotDeployedException(String message, Throwable ex) {
        super(message, ex);
    }
}
