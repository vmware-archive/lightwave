/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema;
using VMDir.Common.DTO;
using System.Runtime.InteropServices;

namespace VMDir.Common.VMDirUtilities
{
    public class LdapConnectionService
    {

        private String server;

        private String bindDN;

        private String password;

        public String Domain { get; set; }

        private ILdapConnection ldConn;

        private ISuperLoggingConnection _superLoggingConnection;

        private ISchemaConnection schemaConnection;

        private SchemaManager _schemaManager;

        public SchemaManager SchemaManager { get { return _schemaManager; } }

        public ISchemaConnection SchemaConnection { get { return schemaConnection; } }

        public LdapConnectionService(String server, String bindDN, String password)
        {
            this.server = server;
            this.bindDN = bindDN;
            this.password = password;
        }

        //returns 1 if success and 0 if failure
        public int CreateConnection()
        {
            schemaConnection = new SchemaConnection(this.server, this.bindDN, this.password);
            if (schemaConnection == null)
                throw new Exception(VMDirConstants.ERR_SCHEMA_CONNECTION_NULL);
            ldConn = LdapConnection.LdapInit(this.server, LDAPOption.LDAP_PORT);
            if (ldConn == null)
                throw new Exception(VMDirConstants.ERR_LDAP_CONNECTION_NULL);

            bool isServerReachable = CheckServerReachability(this.server, LDAPOption.LDAP_PORT, VMDirConstants.SERVERTIMEOUT_IN_MILLI);
            if (isServerReachable == true)
            {
                ldConn.VmDirSafeLDAPBind(this.server, this.bindDN, this.password);
                _schemaManager = new SchemaManager(this);
                _schemaManager.RefreshSchema();
                return 1;
            }
            return 0;
        }

        public ISuperLoggingConnection GetSuperLoggingConnection()
        {
            if (_superLoggingConnection == null)
            {
                _superLoggingConnection = new SuperLoggingConnection();
                _superLoggingConnection.openA(server, Domain, bindDN, password);
            }
            return _superLoggingConnection;
        }

        bool CheckServerReachability(string serverNameOrAddress, int port, int timeoutMs)
        {
            try
            {
                var addresses = Dns.GetHostAddresses(serverNameOrAddress);

                using (var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
                {
                    IAsyncResult result = socket.BeginConnect(addresses[0], port, null, null);
                    bool success = result.AsyncWaitHandle.WaitOne(timeoutMs, true);
                    if (success)
                    {
                        success = socket.Connected;
                    }
                    return success;
                }
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        public bool CheckCredentials()
        {
            ldConn = new LdapConnection();
            if (ldConn == null)
                throw new Exception(VMDirConstants.ERR_NULL_CONN);

            bool isServerReachable = CheckServerReachability(this.server, LDAPOption.LDAP_PORT, VMDirConstants.SERVERTIMEOUT_IN_MILLI);
            if (isServerReachable == true)
            {
                ldConn.VmDirSafeLDAPBind(this.server, this.bindDN, this.password);
                CloseConnection();
                return true;
            }
            return false;
        }

        public void Rebind()
        {
            try
            {
                CloseConnection();
                CreateConnection();
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        //returns true if success and thorws exception if failure
        public void CloseConnection()
        {
            try
            {
                if (ldConn != null)
                    ldConn.LdapUnbindS();
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        public string GetDN(LdapEntry entry)
        {
            try
            {
                return entry.getDN();
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        public Dictionary<string, VMDirAttributeDTO> GetEntryProperties(ILdapEntry entry)
        {
            Dictionary<string, VMDirAttributeDTO> properties = new Dictionary<string, VMDirAttributeDTO>();
            string[] attNamerArr = entry.getAttributeNames().ToArray();
            foreach (string attrName in attNamerArr)
            {
                LdapValue[] attrValArr = entry.getAttributeValues(attrName).ToArray();
                var typeDTO = _schemaManager.GetAttributeType(attrName);
                VMDirAttributeDTO dto = new VMDirAttributeDTO(attrName, new List<LdapValue>(attrValArr), typeDTO);
                properties[attrName] = dto;
            }
            return properties;
        }

        public void Search(QueryDTO dto, Action<ILdapMessage, List<ILdapEntry>> fn)
        {
            List<ILdapEntry> entries;
            ILdapMessage ldMsg = null;

            MaintainSession(delegate ()
                {
                    ldMsg = ldConn.LdapSearchExtS(dto.SearchBase, (int)dto.SearchScope, dto.GetFilterString(), dto.AttrToReturn, dto.AttrOnly, IntPtr.Zero, 0);
                });
            if (ldMsg == null)
                throw new Exception(VMDirConstants.ERR_NULL_CONN);

            entries = ldMsg.GetEntries();
            if (fn != null)
                fn(ldMsg, entries);
            (ldMsg as LdapMessage).FreeMessage();
        }

        public void PagedSearch(QueryDTO qdto, int pageSize, IntPtr cookie, bool morePages, Action<ILdapMessage, IntPtr, bool, List<ILdapEntry>> fn)
        {
            ILdapMessage ldMsg = null;
            IntPtr[] serverControls = { IntPtr.Zero };
            IntPtr returnedControls = IntPtr.Zero;

            MaintainSession(delegate ()
                {
                    serverControls[0] = ldConn.LdapCreatePageControl(pageSize, cookie, true);

                    ldMsg = ldConn.LdapSearchExtExS(qdto.SearchBase, (int)qdto.SearchScope, qdto.GetFilterString(),
                        qdto.AttrToReturn, qdto.AttrOnly, qdto.TimeOut, qdto.SizeLimit, serverControls);

                    ldConn.LdapParseResult(ldMsg, ref returnedControls, false);
                    if (cookie != IntPtr.Zero)
                    {
                        LdapClientLibrary.ber_bvfree(cookie);
                        cookie = IntPtr.Zero;
                    }
                    ldConn.LdapParsePageControl(returnedControls, ref cookie);

                    morePages = ldConn.HasMorePages(cookie);

                    if (returnedControls != IntPtr.Zero)
                    {
                        LdapClientLibrary.ldap_controls_free(returnedControls);
                        returnedControls = IntPtr.Zero;
                    }
                    LdapClientLibrary.ldap_control_free(serverControls[0]);
                    serverControls[0] = IntPtr.Zero;
                });

            List<ILdapEntry> entries = ldMsg.GetEntries();
            if (fn != null)
                fn(ldMsg, cookie, morePages, entries);

            (ldMsg as LdapMessage).FreeMessage();

        }

        public List<ILdapEntry> SearchAndGetEntries(String searchDN, LdapScope scope, string filter, string[] attribsToReturn, int attrsOnly, ref ILdapMessage ldMsg)
        {
            List<ILdapEntry> entries;
            ILdapMessage searchRequest = null;

            //Cannot free ldMsg in this function  as we lose the references to entries. Free the ldMsg in the calling function.
            MaintainSession(delegate ()
                {
                    searchRequest = ldConn.LdapSearchExtS(searchDN, (int)scope, filter, attribsToReturn, attrsOnly, IntPtr.Zero, 0);
                });
            ldMsg = searchRequest;
            if (ldMsg == null)
                throw new Exception(VMDirConstants.ERR_SERVER_CONNECTION_LOST);
            entries = ldMsg.GetEntries();
            return entries;
        }

        public void AddObject(string dn, LdapMod[] attrs)
        {
            MaintainSession(() => ldConn.AddObject(dn, attrs));
        }

        public void AddObjectClass(ObjectClassDTO dto)
        {
            MaintainSession(delegate ()
                {
                    List<LdapMod> objectclass = new List<LdapMod>();

                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] { dto.Name, null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] { VMDirConstants.DEFAULT_OBJECT_CLASS_NAME, null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SUB_CLASS_OF, new string[] { dto.SuperClass, null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.OBJECT_CLASS_CATEGORY, new string[] { Convert.ToString(dto.GetObjectClassType()), null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.GOVERNS_ID, new string[] { dto.GovernsID, null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SCHEMA_ID_GUID, new string[] { dto.Name, null }));
                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.DEFAULT_OBJECT_CATEGORY, new string[] { "cn=schemacontext", null }));

                    if (dto.Description != null && !String.IsNullOrWhiteSpace(dto.Description))
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.DESCRIPTION, new string[] { dto.Description, null }));

                    if (dto.Must != null && dto.Must.Count > 0)
                    {
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SYSTEM_MUST_CONTAIN, ConvertListToLdapStringArray(dto.Must)));
                    }

                    if (dto.May != null && dto.May.Count > 0)
                    {
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SYSTEM_MAY_CONTAIN, ConvertListToLdapStringArray(dto.May)));
                    }

                    if (dto.Aux != null && dto.Aux.Count > 0)
                    {
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SYSTEM_AUX_CLASSES, ConvertListToLdapStringArray(dto.Aux)));
                    }
                    string dn = "cn=" + dto.Name + ",cn=schemacontext";
                    ldConn.AddObject(dn, objectclass.ToArray());
                });
        }

        private string[] ConvertListToLdapStringArray(List<string> input)
        {
            string[] arr = new string[input.Count + 1];
            Array.Copy(input.ToArray(), arr, input.Count);
            arr[arr.Length - 1] = null;
            return arr;
        }

        public void AddAttributeType(AttributeTypeDTO dto)
        {
            MaintainSession(delegate ()
                {
                    List<LdapMod> attribute = new List<LdapMod>();
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] { dto.Name, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] { VMDirConstants.DEFAULT_ATTR_SCHEMA_NAME, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_ID, new string[] { dto.AttributeID, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.LDAP_DISPLAY_NAME, new string[] { dto.Name, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SYNTAX, new string[] { dto.AttributeSyntax, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SINGLE_VALUED, new string[] { "TRUE", null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.SCHEMA_ID_GUID, new string[] { dto.Name, null }));

                    if (dto.Description != null && !String.IsNullOrWhiteSpace(dto.Description))
                        attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.DESCRIPTION, new string[] { dto.Description, null }));


                    string dn = "cn=" + dto.Name + ",cn=schemacontext";
                    ldConn.AddObject(dn, attribute.ToArray());
                });
        }

        public void ModifyAttributeType(AttributeTypeDTO dto)
        {
            MaintainSession(delegate ()
                {
                    List<LdapMod> attribute = new List<LdapMod>();

                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.DESCRIPTION, new string[] { dto.Description, null }));
                    attribute.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.ATTR_SINGLE_VALUED, new string[] { Convert.ToString(dto.SingleValue), null }));

                    string dn = "cn=" + dto.Name + ",cn=schemacontext";
                    ldConn.ModifyObject(dn, attribute.ToArray());

                });
        }

        public void ModifyObjectClass(ObjectClassDTO dto)
        {
            MaintainSession(delegate ()
                {
                    List<LdapMod> objectclass = new List<LdapMod>();

                    objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.DESCRIPTION, new string[] { dto.Description, null }));

                    if (dto.May != null && dto.May.Count > 0)
                    {
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.SYSTEM_MAY_CONTAIN, ConvertListToLdapStringArray(dto.May)));
                    }
                    if (dto.Aux != null && dto.Aux.Count > 0)
                    {
                        objectclass.Add(new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.SYSTEM_AUX_CLASSES, ConvertListToLdapStringArray(dto.Aux)));
                    }

                    string dn = "cn=" + dto.Name + ",cn=schemacontext";
                    ldConn.ModifyObject(dn, objectclass.ToArray());
                });
        }

        public void DeleteObject(string dn)
        {
            MaintainSession(() => ldConn.DeleteObject(dn));
        }

        public void ModifyObject(string dn, LdapMod[] attrs)
        {
            MaintainSession(() => ldConn.ModifyObject(dn, attrs));
        }

        public void MaintainSession(System.Action fn)
        {
            try
            {
                fn();
            }
            catch (LdapException e)
            {
                if (e.LdapError == VMDirInterop.LDAPConstants.LdapStatus.LDAP_SERVER_DOWN)
                {
                    bool isServerReachable = CheckServerReachability(this.server, LDAPOption.LDAP_PORT, VMDirConstants.SERVERTIMEOUT_IN_MILLI);
                    if (isServerReachable == true)
                    {
                        Rebind();
                        fn();
                    }
                    else
                        throw new Exception(VMDirConstants.ERR_SERVER_CONNECTION_LOST);
                }
                else
                    throw e;
            }
            catch (Exception e)
            {
                throw;
            }
        }
    }
}

