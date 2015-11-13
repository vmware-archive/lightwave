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

package com.vmware.identity.interop.accountmanager;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

public class LsaUserInfoLevel1Native extends Structure
{
    public int uid;
    public int gid;
    public String pszName;
    public String pszPasswd;
    public String pszGecos;
    public String pszShell;
    public String pszHomedir;
    public String pszSid;

    public String pszDN;
    public String pszUPN;
    public int bIsGeneratedUPN;
    public int bIsLocalUser;
    public Pointer pLMHash;
    public int dwLMHashLen;
    public Pointer pNTHash;
    public int dwNTHashLen;

    @Override
    protected List<String> getFieldOrder()
    {
        return Arrays.asList(new String[] {
                "uid", "gid", "pszName", "pszPasswd", "pszGecos",
                "pszShell", "pszHomedir", "pszSid",
                "pszDN", "pszUPN", "bIsGeneratedUPN", "bIsLocalUser",
                "pLMHash", "dwLMHashLen", "pNTHash", "dwNTHashLen"
        });
    }
}
