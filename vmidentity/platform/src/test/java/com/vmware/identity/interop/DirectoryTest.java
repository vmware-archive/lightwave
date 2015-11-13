/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.interop;

import org.junit.Test;

import com.vmware.identity.interop.directory.*;
import com.vmware.identity.interop.ldap.*;

public class DirectoryTest
{
    String hostName = "gary-sles";
    int hostPort = 389;
    String hostDN = "ldap://gary-sles:389";
    String adminDN   = "cn=administrator,cn=users,dc=vsphere,dc=local";
    String adminName = "administrator";
    String adminPassword = "vmware";
    String userDN = "cn=user1,cn=users,dc=vsphere,dc=local";
    String userOldPassword = "Vmware@ng12345";
    String userNewPassword = "Vmware@ng67890";

    void testBind(String user, String password)
    {

        ILdapConnectionEx connection = LdapConnectionFactoryEx.getInstance().getLdapConnection(hostName, hostPort);
        connection.setOption( LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
        connection.bindConnection(
                            user,
                            password,
                            LdapBindMethod.LDAP_BIND_SIMPLE);
    }

    @Test
    public void testSetPassword() throws Exception
    {
        System.out.println("Executing testSetPassword() test");
        Directory.SetPassword(hostDN, adminDN, adminPassword, userDN, userOldPassword);
        testBind(userDN, userOldPassword);
    }

    @Test
    public void testChangePassword() throws Exception
    {
        System.out.println("Executing testChangePassword() test");
        Directory.ChangePassword(hostDN, userDN, userOldPassword, userNewPassword);
        testBind(userDN, userNewPassword);
    }

    @Test
    public void testGetDefaultLdu() throws Exception
    {
        System.out.println("Executing GetLocalLduGuid() test");
        System.out.println(Directory.GetLocalLduGuid());
    }

    public static void main(String args[]) throws Exception{
        org.junit.runner.JUnitCore.main("com.vmware.identity.interop.DirectoryTest");
    }
}
