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

package com.vmware.identity.interop.directory;

import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.ptr.IntByReference;

public final class Directory
{
    private static final IDirectoryClientLibrary library = DirectoryAdapter.getInstance();

    public static
    void createInstance(String domainName, String adminId, String password)
    {
        library.CreateDirectoryInstance(domainName, adminId, password);
    }

    public static
    void deleteInstance(String domainName, String adminId, String password)
    {
        library.DeleteDirectoryInstance(domainName, adminId, password);
    }

    public static
    void
    SetPassword(
        String hostURI,
        String adminDN,
        String adminPassword,
        String userDN,
        String newPassword
    )
    {
        library.SetPassword(hostURI, adminDN, adminPassword, userDN, newPassword);
    }

    public static
    void
    ChangePassword(
        String hostURI,
        String userDN,
        String oldPassword,
        String newPassword
    )
    {
        library.ChangePassword(hostURI, userDN, oldPassword, newPassword);
    }

    public static
    String
    GetLocalLduGuid()
    {
        return library.GetLocalLduGuid();
    }

    public static
    void
    GetReplicationPartners(
        String              hostName,
        String              port,
        String              userName,
        String              password,
        PointerByReference  replPartnerInfo,
        IntByReference      numReplPartner
    )
    {
        library.GetReplicationPartners(
                hostName,
                port,
                userName,
                password,
                replPartnerInfo,
                numReplPartner
                );
    }

    public static
    void
    GetServers(
        String              hostName,
        String              port,
        String              userName,
        String              password,
        PointerByReference  serverInfo,
        IntByReference      numReplServer
    )
    {
        library.GetServers(
            hostName,
            port,
            userName,
            password,
            serverInfo,
            numReplServer
        );
    }

    public static
    void
    AddReplicationAgreement
    (
        int                 twoWayRepl,
        String              srcHostName,
        String              srcPort,
        String              srcUserName,
        String              srcPassword,
        String              tgtHostName,
        String              tgtPort,
        String              tgtUserName,
        String              tgtPassword
    )
    {
        library.AddReplicationAgreement(
            twoWayRepl,
            srcHostName,
            srcPort,
            srcUserName,
            srcPassword,
            tgtHostName,
            tgtPort,
            tgtUserName,
            tgtPassword
        );
    }

    public static
    void
    RemoveReplicationAgreement
    (
        int                 twoWayRepl,
        String              srcHostName,
        String              srcPort,
        String              srcUserName,
        String              srcPassword,
        String              tgtHostName,
        String              tgtPort,
        String              tgtUserName,
        String              tgtPassword
    )
    {
        library.RemoveReplicationAgreement(
            twoWayRepl,
            srcHostName,
            srcPort,
            srcUserName,
            srcPassword,
            tgtHostName,
            tgtPort,
            tgtUserName,
            tgtPassword
        );
    }
}
