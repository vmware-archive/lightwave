using System;
using System.Collections.Generic;
using VMDirInterop;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;

namespace LdapTest
{
    public class LdapUser
    {
        public readonly string UserName;

        private LdapUser(ILdapConnection ldapConnection, string UserName)
        {
            this.UserName = UserName;
            this.DN = String.Format("cn={0},cn=Users,dc=vsphere,dc=local", UserName);

            LdapMod[] lMods = new LdapMod[7];

            lMods[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, "cn", new string[] { UserName });
            lMods[1] = new LdapMod(0, "objectClass", new string[] { "person", "top", "organizationalPerson", "user" });
            lMods[2] = new LdapMod(0, "givenName", new string[] { "Laddy McLdap" });
            lMods[3] = new LdapMod(0, "userPrincipalName", new string[] { String.Format("{0}@vsphere.local", UserName) });
            lMods[4] = new LdapMod(0, "sAMAccountName", new string[] { UserName });
            lMods[5] = new LdapMod(0, "vmwPasswordNeverExpires", new string[] { "TRUE" });
            lMods[6] = new LdapMod(0, "userpassword", new string[] { "Admin!23" });

            ldapConnection.AddObject(DN, lMods);
        }

        public static LdapUser CreateUser(ILdapConnection ldapConnection, string UserName)
        {
            return new LdapUser(ldapConnection, UserName);
        }

        public static LdapUser CreateRandomUser(ILdapConnection ldapConnection)
        {
            string UserName = Guid.NewGuid().ToString("N");
            return CreateUser(ldapConnection, UserName);
        }

        public string DN { get; private set; }
    }
}
