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

package com.vmware.identity.interop.ldap;

import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Structure;
import com.sun.jna.win32.W32APITypeMapper;

enum SecWinntAuthFlags
{
    SEC_WINNT_AUTH_ID_ANSI(0x1),
    SEC_WINNT_AUTH_ID_UNICODE(0x2);

    private int _code;

    private SecWinntAuthFlags(int code)
    {
        _code = code;
    }

    public int getCode()
    {
        return _code;
    }
}

/* SecWinntAuthId constructs SEC_WINNT_AUTH_ID
 * which encapsulates credentials information for
 * ldap_bind_s on Windows
 * Intended not to be used by ldap package on Linux
 */
public final class SecWinntAuthId extends Structure
{
    public String _pszUserName;
    public int _userNameLength;
    public String _pszDomain;
    public int _domainNameLength;
    public String _password;
    public int _passwordLength;
    public int _Flags;

    public SecWinntAuthId(String pszUserName, String pszDomain, String password)
    {
        super(SystemUtils.IS_OS_WINDOWS ? W32APITypeMapper.UNICODE : null);

        this._pszUserName = pszUserName;
        this._userNameLength = pszUserName == null ? 0 : pszUserName.length();
        this._pszDomain = pszDomain;
        this._domainNameLength = pszDomain == null ? 0 : pszDomain.length();
        this._password = password;
        this._passwordLength = password == null ? 0 : password.length();
        this._Flags = SecWinntAuthFlags.SEC_WINNT_AUTH_ID_UNICODE.getCode();

        write();
    }

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "_pszUserName", "_userNameLength", "_pszDomain", "_domainNameLength", "_password",
                "_passwordLength", "_Flags"
        });
    }
}
