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

package com.vmware.identity.idm.server.vmaf;

import com.vmware.af.DomainInfo;
import com.vmware.af.PasswordCredential;
import com.vmware.af.VmAfClientNativeException;
import com.vmware.af.interop.VmAfClientAdapter;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;

public class VmafClientUtil {
    private static final String AFD_SERVER = "localhost";

    public static void joinActiveDirectory(String user, String password,
	    String domain, String orgUnit) throws VmAfClientNativeException {

	VmAfClientAdapter.joinAD(AFD_SERVER, user, password, domain, orgUnit);
    }

    public static void leaveActiveDirectory(String user, String password)
	    throws VmAfClientNativeException {

	VmAfClientAdapter.leaveAD(AFD_SERVER, user, password);
    }

    public static ActiveDirectoryJoinInfo queryActiveDirectory()
	    throws VmAfClientNativeException {

	DomainInfo domainInfo = VmAfClientAdapter.queryAD(AFD_SERVER);
	return domainInfo.getDomainName() == null ? null
	        : new ActiveDirectoryJoinInfo(domainInfo.getDomainName(),
	                domainInfo.getNetbiosName(),
	                domainInfo.getDistinguishedName());
    }

    public static String getDomainName() throws VmAfClientNativeException {
	return VmAfClientAdapter.getDomainName(AFD_SERVER);
    }

    public static PasswordCredential getMachineAccountCredentials()
	    throws VmAfClientNativeException {

	return VmAfClientAdapter.getMachineAccountCredentials();
    }

}
