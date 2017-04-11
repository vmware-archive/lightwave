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

import com.sun.jna.Platform;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.ptr.IntByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.Validate;

public class DirectoryAdapter extends NativeAdapter
                              implements IDirectoryClientLibrary
{
    private interface DirectoryClientLibrary extends Library
    {
        DirectoryClientLibrary INSTANCE =
                (DirectoryClientLibrary) Native.loadLibrary(
                            Platform.isWindows()?"libvmdirclient.dll":"vmdirclient",
                            DirectoryClientLibrary.class);

        int
        VmDirSetupHostInstance(
            String pszDomainName,
            String pszUsername,
            String pszPassword,
            String pszReplURI,
            String pszReplBindDN,
            String pszReplBindPassword,
            String pszReplBase
            );

        int
        VmDirSetupTenantInstance(
            String domainName,
            String adminId,
            String password
            );

        int
        VmDirSetPassword(
            String hostURI,
            String adminDN,
            String adminPassword,
            String userDN,
            String newPassword
            );

        int
        VmDirChangePassword(
            String hostURI,
            String userDN,
            String oldPassword,
            String newPassword
            );

        int
        VmDirGetLocalLduGuid(
                byte[] lduGuid
            );

        int
        VmDirGetReplicationPartners(
            String              hostName,
            String              port,
            String              userName,
            String              password,
            PointerByReference  replPartnerInfo,
            IntByReference      numReplPartner
        );

        int
        VmDirGetServers(
            String              hostName,
            String              port,
            String              userName,
            String              password,
            PointerByReference  serverInfo,
            IntByReference      numReplServer
        );

        int
        VmDirAddReplicationAgreement
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

        int
        VmDirRemoveReplicationAgreement
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

        int
        VmDirDeleteTenant
        (
            String              adminId,
            String              password,
            String              domainName
        );

    }
    private static final DirectoryAdapter _instance = new DirectoryAdapter();

    public static
    DirectoryAdapter
    getInstance()
    {
        return _instance;
    }

    @Override
    public
    void
    CreateDirectoryInstance(
            String domainName,
            String administratorId,
            String password)
    {
        Validate.validateNotEmpty(domainName, "domain name");
        Validate.validateNotEmpty(administratorId, "administrator id");
        Validate.validateNotEmpty(password, "password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirSetupTenantInstance(
                                            domainName,
                                            administratorId,
                                            password));
    }

    @Override
    public
    void
    DeleteDirectoryInstance(
            String domainName,
            String administratorId,
            String password)
    {
        Validate.validateNotEmpty(administratorId, "administrator id");
        Validate.validateNotEmpty(password, "password");
        Validate.validateNotEmpty(domainName, "domain name");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirDeleteTenant(
                                            administratorId,
                                            password,
                                            domainName));
    }

    @Override
    public
    void
    CreateServiceProviderInstance(
        String domainName,
        String administratorId,
        String password
        )
    {
        Validate.validateNotEmpty(domainName, "domain name");
        Validate.validateNotEmpty(administratorId, "administrator id");
        Validate.validateNotEmpty(password, "password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirSetupHostInstance(
                                                        domainName,
                                                        administratorId,
                                                        password,
                                                        null,
                                                        null,
                                                        null,
                                                        null));
    }

    @Override
    public
    void
    SetPassword(
        String hostURI,
        String adminDN,
        String adminPassword,
        String userDN,
        String newPassword
    )
    {
        Validate.validateNotEmpty(hostURI, "host uri");
        Validate.validateNotEmpty(adminDN, "admin dn");
        Validate.validateNotEmpty(adminPassword, "admin password");
        Validate.validateNotEmpty(userDN, "user dn");
        Validate.validateNotEmpty(newPassword, "new password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirSetPassword(
                                                        hostURI,
                                                        adminDN,
                                                        adminPassword,
                                                        userDN,
                                                        newPassword));
    }

    @Override
    public
    void
    ChangePassword(
        String hostURI,
        String userDN,
        String oldPassword,
        String newPassword
    )
    {
        Validate.validateNotEmpty(hostURI, "host uri");
        Validate.validateNotEmpty(userDN, "user dn");
        Validate.validateNotEmpty(oldPassword, "old password");
        Validate.validateNotEmpty(newPassword, "new password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirChangePassword(
                                                    hostURI,
                                                    userDN,
                                                    oldPassword,
                                                    newPassword));
    }

    @Override
    public
    String
    GetLocalLduGuid()
    {
        byte[] value = new byte[64];
        CheckError(DirectoryClientLibrary.INSTANCE.VmDirGetLocalLduGuid(value));
        return new String(value).split("\0")[0];
    }

    @Override
    public
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
        Validate.validateNotEmpty(hostName, "host name");
        Validate.validateNotEmpty(port,     "port");
        Validate.validateNotEmpty(userName, "user name");
        Validate.validateNotEmpty(password, "password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirGetReplicationPartners(
                hostName,
                port,
                userName,
                password,
                replPartnerInfo,
                numReplPartner
            )
        );
    }

    @Override
    public
    void
    GetServers(
        String              hostName,
        String              port,
        String              userName,
        String              password,
        PointerByReference  serverInfo,
        IntByReference      numServer
    )
    {
        Validate.validateNotEmpty(hostName, "host name");
        Validate.validateNotEmpty(port,     "port");
        Validate.validateNotEmpty(userName, "user name");
        Validate.validateNotEmpty(password, "password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirGetServers(
                hostName,
                port,
                userName,
                password,
                serverInfo,
                numServer
            )
        );
    }

    @Override
    public
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
        Validate.validateNotEmpty(srcHostName,  "src host name");
        Validate.validateNotEmpty(srcPort,      "src port");
        Validate.validateNotEmpty(srcUserName,  "src user name");
        Validate.validateNotEmpty(srcPassword,  "src password");
        Validate.validateNotEmpty(tgtHostName,  "tgt host name");
        Validate.validateNotEmpty(tgtPort,      "tgt port");
        Validate.validateNotEmpty(tgtUserName,  "tgt user name");
        Validate.validateNotEmpty(tgtPassword,  "tgt password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirAddReplicationAgreement(
            twoWayRepl,
            srcHostName,
            srcPort,
            srcUserName,
            srcPassword,
            tgtHostName,
            tgtPort,
            tgtUserName,
            tgtPassword
            )
        );
    }

    @Override
    public
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
        Validate.validateNotEmpty(srcHostName,  "src host name");
        Validate.validateNotEmpty(srcPort,      "src port");
        Validate.validateNotEmpty(srcUserName,  "src user name");
        Validate.validateNotEmpty(srcPassword,  "src password");
        Validate.validateNotEmpty(tgtHostName,  "tgt host name");
        Validate.validateNotEmpty(tgtPort,      "tgt port");
        Validate.validateNotEmpty(tgtUserName,  "tgt user name");
        Validate.validateNotEmpty(tgtPassword,  "tgt password");

        CheckError(DirectoryClientLibrary.INSTANCE.VmDirRemoveReplicationAgreement(
            twoWayRepl,
            srcHostName,
            srcPort,
            srcUserName,
            srcPassword,
            tgtHostName,
            tgtPort,
            tgtUserName,
            tgtPassword
            )
        );
    }

    private DirectoryAdapter()
    {
    }

    private static void CheckError(int errorCode)
    {
        if( errorCode != 0 )
        {
            throw new DirectoryException(errorCode, String.format("VMware directory error [code: %d]", errorCode));
        }
    }
}
