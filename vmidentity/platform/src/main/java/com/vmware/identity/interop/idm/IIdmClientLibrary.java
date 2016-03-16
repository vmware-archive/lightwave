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

package com.vmware.identity.interop.idm;

import com.sun.jna.Pointer;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;


public interface IIdmClientLibrary {

    public enum SSPI_AUTH_STATUS
    {
        COMPLETE(0),
        INITIAL(1),
        CONTINUE_NEEDED(2),
        ERROR(3);

        private final int _code;

        private SSPI_AUTH_STATUS(int code)
        {
           _code = code;
        }

        public int getCode()
        {
           return _code;
        }
    }

    String getComputerName();

    UserInfo
    AuthenticateByPassword(
        String pszUserName,
        String pszDomainName,
        String pszPassword
        );

    AuthenticationContext CreateAuthenticationContext();

    AuthResult
    AuthenticateBySSPI(
        AuthenticationContext context,
        byte[]                gssBLOB
        );

    void FreeAuthenticationContext(Pointer pContext);

    UserInfo getUserInfo(AuthenticationContext context);

    boolean
    LdapSaslBind(Pointer pLdapConnection, String userName, String domainName, String password);
}
