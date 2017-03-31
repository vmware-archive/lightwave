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

public interface IDirectoryClientLibrary
{
    void
    CreateServiceProviderInstance(
        String domainName,
        String administratorId,
        String password
    );

    void
    CreateDirectoryInstance(
        String domainName,
        String administratorId,
        String password
    );

    void
    DeleteDirectoryInstance(
        String domainName,
        String administratorId,
        String password
    );

    void
    SetPassword(
        String hostURI,
        String adminDN,
        String adminPassword,
        String userDN,
        String newPassword
    );

    void
    ChangePassword(
        String hostURI,
        String userDN,
        String oldPassword,
        String newPassword
    );

    String
    GetLocalLduGuid();

    void
    GetReplicationPartners(
        String              hostName,
        String              port,
        String              userName,
        String              password,
        PointerByReference  replPartnerInfo,
        IntByReference      numReplPartner
    );

    void
    GetServers(
        String              hostName,
        String              port,
        String              userName,
        String              password,
        PointerByReference  serverInfo,
        IntByReference      numReplServer
    );

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
    );

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
    );

}

