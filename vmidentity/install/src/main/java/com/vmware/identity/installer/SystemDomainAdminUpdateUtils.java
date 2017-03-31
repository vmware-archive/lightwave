package com.vmware.identity.installer;

import java.net.URI;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import sun.misc.BASE64Decoder;

import com.vmware.identity.idm.server.CryptoAESE;
import com.vmware.identity.interop.ldap.ILdapConnection;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapConnectionExWithGetConnectionString;
import com.vmware.identity.interop.ldap.ILdapEntry;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.InvalidCredentialsLdapException;
import com.vmware.identity.interop.ldap.LdapBindMethod;
import com.vmware.identity.interop.ldap.LdapConnectionFactory;
import com.vmware.identity.interop.ldap.LdapConstants;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapOption;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapSetting;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;

public class SystemDomainAdminUpdateUtils {
    public static class SystemDomainAdmin {
        public String IdpDn;
        public String UserDn;
        public String UserUpn;
        public String Password;

        @Override
        public String toString(){
            String nl = System.getProperty("line.separator");
            StringBuilder result = new StringBuilder();
            result.append(" IdpDn: " + IdpDn + nl);
            result.append(" UserDn: " + UserDn + nl);
            result.append(" UserUpn: " + UserUpn + nl);
            return result.toString();
        }
    }

    public static boolean connectWithSrp(URI uri, String userUpn, String password) throws Exception {
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnection connection = LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        try {
            connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            ((ILdapConnectionExWithGetConnectionString)connection).bindSaslSrpConnection(userUpn, password);
            return true;
        } catch (InvalidCredentialsLdapException ex) {
            System.out.println(String.format("Admin can't be authenticated with Srp, Upn: %s", userUpn));
            return false;
        } finally {
            connection.close();
        }
    }

    public static Collection<SystemDomainAdmin> findAdminsToUpdateToSRP(URI uri, String adminUserDn, String adminPassword, String rootDn)
            throws Exception {
        Collection<SystemDomainAdmin> admins = new HashSet<SystemDomainAdmin>();
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnection connection = LdapConnectionFactory.getInstance().getLdapConnection(uri, connOptions, true);
        try {
            connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.setOption(LdapOption.LDAP_OPT_REFERRALS, false);
            connection.bindConnection(adminUserDn, adminPassword, LdapBindMethod.LDAP_BIND_SIMPLE);

            // looking for tenants, and fetching their vmwSTSTenantKey used to decrypt system domain admin's password
            ILdapMessage tenantSearchResult = connection.search(
                    String.format("cn=Tenants,cn=IdentityManager,cn=Services,%s", rootDn),
                    LdapScope.SCOPE_ONE_LEVEL,
                    "(objectClass=vmwSTSTenant)",
                    new String[] {"vmwSTSTenantKey", null },
                    false);
            ILdapEntry[] tenantEntries = tenantSearchResult.getEntries();
            for (ILdapEntry tenantEntry : tenantEntries) {
                String tenantKey = null;
                LdapValue[] tenantKeyValue = tenantEntry.getAttributeValues("vmwSTSTenantKey");
                if ((tenantKeyValue != null) && (tenantKeyValue.length == 1)) {
                    tenantKey = tenantKeyValue[0].getString();
                }

                // looking for system domains whose authType is PASSWORD, and fetching vmwSTSUserName and vmwSTSPassword
                ILdapEntry[] systemDomainEntries = null;
                try {
                    String domainsFilter = String.format("cn=IdentityProviders,%s", tenantEntry.getDN());
                    ILdapMessage systemDomainsSearchResult = connection.search(
                            domainsFilter,
                            LdapScope.SCOPE_ONE_LEVEL,
                            "(&(objectClass=vmwSTSIdentityStore)(vmwSTSDomainType=SYSTEM_DOMAIN)(vmwSTSAuthenticationType=PASSWORD))",
                            new String[] { "vmwSTSUserName", "vmwSTSPassword", null },
                            false);
                    systemDomainEntries = systemDomainsSearchResult.getEntries();
                } catch (NoSuchObjectLdapException ex) {
                    continue;
                } catch (Exception ex) {
                    System.out.println(String.format("skip tenant, Dn: %s, Exception: %s", tenantEntry.getDN(), ex.toString()));
                    continue;
                }
                if(systemDomainEntries == null) {
                    continue;
                }
                for (ILdapEntry systemDomainEntry : systemDomainEntries) {
                    SystemDomainAdmin admin = new SystemDomainAdmin();
                    admin.IdpDn = systemDomainEntry.getDN();
                    LdapValue[] value = systemDomainEntry.getAttributeValues("vmwSTSUserName");
                    if ((value != null) && (value.length == 1)) {
                        admin.UserDn = value[0].getString();
                        try {
                            // looking for system domain Admin whose vmwSRPSecret is set, and fetching userPrincipalName
                            ILdapMessage userUpnSearchResult = connection.search(
                                    admin.UserDn,
                                    LdapScope.SCOPE_BASE,
                                    "(vmwSRPSecret=*)",
                                    new String[] {"userPrincipalName", null },
                                    false);
                            ILdapEntry[] userEntries = userUpnSearchResult.getEntries();
                            if(userEntries != null && userEntries.length == 1){
                                value = userEntries[0].getAttributeValues("userPrincipalName");
                                if ((value != null) && (value.length == 1)) {
                                    admin.UserUpn = value[0].getString();
                                    value = systemDomainEntry.getAttributeValues("vmwSTSPassword");
                                    if ((value != null) && (value.length == 1)) {
                                        String encryptedPassword = value[0].getString();
                                        CryptoAESE cryptoAES = new CryptoAESE(tenantKey);
                                        admin.Password = cryptoAES.decrypt(new BASE64Decoder().decodeBuffer(encryptedPassword));
                                        admins.add(admin);
                                    }
                                }
                            }
                        } catch (Exception ex) {
                            System.out.println(String.format("skip admin, Dn: %s, Exception: %s", admin.UserDn, ex.toString()));
                            continue;
                        }
                    }
                }
            }
            return admins;
        } finally {
            connection.close();
        }
    }

    public static void updateSystemDomainAdmin(URI uri, String adminUserDn, String adminPassword, SystemDomainAdmin admin) throws Exception {
        List<LdapSetting> connOptions = Collections.emptyList();
        ILdapConnection connection = LdapConnectionFactory.getInstance()
                .getLdapConnection(uri, connOptions, true);
        try {
            connection.setOption(LdapOption.LDAP_OPT_PROTOCOL_VERSION, LdapConstants.LDAP_VERSION3);
            connection.setOption(LdapOption.LDAP_OPT_REFERRALS, false);
            connection.bindConnection(adminUserDn, adminPassword, LdapBindMethod.LDAP_BIND_SIMPLE);
            connection.modifyObject(admin.IdpDn, new LdapMod(
                    LdapModOperation.REPLACE, "vmwSTSUserName",
                    new LdapValue[] { LdapValue.fromString(admin.UserUpn) }));
            connection.modifyObject(admin.IdpDn, new LdapMod(
                    LdapModOperation.REPLACE, "vmwSTSAuthenticationType",
                    new LdapValue[] { LdapValue.fromString("SRP") }));
        } finally {
            connection.close();
        }
    }
}
