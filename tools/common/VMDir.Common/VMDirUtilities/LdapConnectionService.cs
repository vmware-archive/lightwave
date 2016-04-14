/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using VMDir.Common.Schema;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDirInterop.LDAPExceptions;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using VMDirInterop;
using VmDirInterop.SuperLogging.Interfaces;
using VmDirInterop.SuperLogging;

namespace VMDir.Common.VMDirUtilities
{
    public class LdapConnectionService
    {

        private String server;

        private String bindDN;

        private String password;

        public  String Domain { get; set; }

        private ILdapConnection ldConn;

        private ISuperLoggingConnection _superLoggingConnection;

        private SchemaManager _schemaManager;

        public SchemaManager SchemaManager { get { return _schemaManager; } }

        public LdapConnectionService (String server, String bindDN, String password)
        {
            this.server = server;
            this.bindDN = bindDN;
            this.password = password;
        }

        //returns 1 if success and 0 if failure
        public  int CreateConnection ()
        {
            try {
                ldConn = new LdapConnection ();
                if (ldConn == null)
                    throw new Exception ("Ldap Connection is null");

                bool isServerReachable = CheckServerReachability (this.server, LDAPOption.LDAP_PORT, VMDirConstants.SERVERTIMEOUT_IN_MILLI);
                if (isServerReachable == true) {
                    ldConn.VmDirSafeLDAPBind (this.server, this.bindDN, this.password);
                    _schemaManager = new SchemaManager (this);
                    _schemaManager.RefreshSchema ();
                    return 1;
                }
                return 0;

            } catch (Exception e) {
                return 0; 
            }
        }

        public ISuperLoggingConnection GetSuperLoggingConnection ()
        {
            if (_superLoggingConnection == null) {
                _superLoggingConnection = new SuperLoggingConnection ();
                _superLoggingConnection.openA (server, Domain, bindDN, password);
            }
            return _superLoggingConnection;
        }

        bool CheckServerReachability (string serverNameOrAddress, int port, int timeoutMs)
        {
            try {
                var addresses = Dns.GetHostAddresses (serverNameOrAddress);

                using (var socket = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)) {
                    IAsyncResult result = socket.BeginConnect (addresses [0], port, null, null);
                    bool success = result.AsyncWaitHandle.WaitOne (timeoutMs, true);
                    if (success) {
                        success = socket.Connected;
                    }
                    return success;
                }
            } catch (Exception e) {
                throw e;
            }
        }

        public void Rebind ()
        {
            try {
                CloseConnection ();
                CreateConnection ();
            } catch (Exception e) {
                throw e;
            }
        }

        //returns true if success and thorws exception if failure
        public void CloseConnection ()
        {
            try {
                if (ldConn != null)
                    ldConn.LdapUnbindS ();
            } catch (Exception e) {
                throw e;
            }
        }

        public string GetDN (LdapEntry entry)
        {
            try {
                return entry.getDN ();
            } catch (Exception e) {
                throw  e;
            }
        }

        public void Search(String searchDN, LdapScope scope, string filter, string[] attribsToReturn, int attrsOnly, Action<ILdapMessage, List<ILdapEntry>> fn)
        {
            List<ILdapEntry> entries;
            ILdapMessage searchRequest = null;

            MaintainSession(delegate()
            {
                searchRequest = ldConn.LdapSearchExtS(searchDN, (int)scope, filter, attribsToReturn, attrsOnly, IntPtr.Zero, 0);
            });
            if (searchRequest == null)
                throw new Exception("Failed to do LDAP Search possibly due to lost connection. Close connection and try again");

            entries = searchRequest.GetEntries();
            if (fn != null)
                fn(searchRequest, entries);
        }

        public List<ILdapEntry> SearchAndGetEntries (String searchDN, LdapScope scope, string filter, string[] attribsToReturn, int attrsOnly, ref ILdapMessage ldMsg)
        {
            List<ILdapEntry> entries;
            ILdapMessage searchRequest = null;

            //Cannot free ldMsg in this function  as we lose the references to entries. Free the ldMsg in the calling function.
            MaintainSession (delegate() {
                searchRequest = ldConn.LdapSearchExtS (searchDN, (int)scope, filter, attribsToReturn, attrsOnly, IntPtr.Zero, 0);
            });
            ldMsg = searchRequest;
            if (ldMsg == null)
                throw new Exception ("Failed to do LDAP Search possibly due to lost connection. Close connection and try again");
            entries = ldMsg.GetEntries ();
            return entries;
        }

        public string[] SearchAndGetDN (String searchDN, LdapScope scope, string filter, string[] attributes, int attrsOnly, ref ILdapMessage ldMsg)
        {
            string[] dn;
            ILdapMessage searchRequest = null;
            MaintainSession (delegate() {
                searchRequest = ldConn.LdapSearchExtS (searchDN, (int)scope, filter, attributes, attrsOnly, IntPtr.Zero, 0);
            });
            ldMsg = searchRequest;
            if (ldMsg == null)
                throw new Exception ("Failed to do LDAP Search possibly due to lost connection. Close connection and try again");
            List<ILdapEntry> entries = ldMsg.GetEntries ();
            if (entries.Count <= 0)
                return null;
            dn = new string[entries.Count];
            int i = 0;
            foreach (LdapEntry entry in entries) {
                dn [i++] = entry.getDN ();
            }
            (ldMsg as LdapMessage).FreeMessage ();
            return dn;
        }

        public void AddObject (string dn, LdapMod[] attrs)
        {
            MaintainSession (() => ldConn.AddObject (dn, attrs));
        }

        public void DeleteObject (string dn)
        {
            MaintainSession (() => ldConn.DeleteObject (dn));
        }

        public void ModifyObject (string dn, LdapMod[] attrs)
        {
            MaintainSession (() => ldConn.ModifyObject (dn, attrs));
        }

        public void MaintainSession (System.Action fn)
        {
            try {
                fn ();
            } catch (LdapException e) {
                if (e.LdapError == VMDirInterop.LDAPConstants.LdapStatus.LDAP_SERVER_DOWN) {
                    bool isServerReachable = CheckServerReachability (this.server, LDAPOption.LDAP_PORT, VMDirConstants.SERVERTIMEOUT_IN_MILLI);
                    if (isServerReachable == true) {
                        Rebind ();
                        fn ();
                    } else
                        throw new Exception ("Connection to Server lost. Please disconnect and connect again.");
                } else
                    throw e;
            } catch (Exception e) {
                throw e;
            }
        }

    }
}

