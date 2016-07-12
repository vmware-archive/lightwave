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
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDirSnapIn.Utilities;
using VMDirSnapIn.UI;
using VMDirSnapIn.Views;
using VMIdentity.CommonUtils;
namespace VMDirSnapIn.TreeNodes
{
    public class ServerNode : BaseTreeNode
    {
        public string ServerGUID { get; set; }
        public bool IsLoggedIn { get; set; }

        private Dictionary<string, VMDirAttributeDTO> _properties;

        public Dictionary<string, VMDirAttributeDTO> ServerProperties
        {
            get
            {
                if (_properties == null)
                    FillProperties();
                return _properties;
            }
        }

        private void FillProperties()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                TextQueryDTO dto = new TextQueryDTO(VMDirConstants.ATTR_VMW_DSEROOT_DN, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);
                ServerDTO.Connection.Search(dto,
                    (l, e) =>
                    {
                        _properties = ServerDTO.Connection.GetEntryProperties(e);
                    });
            });
        }
        public void RefreshProperties()
        {
            _properties.Clear();
            FillProperties();
            DoSelect();
        }

        public ServerNode(VMDirServerDTO serverDTO, PropertiesControl control)
            : base(serverDTO, control)
        {
            ImageIndex = SelectedImageIndex = (int)VMDirIconIndex.Server;
            this.Text = serverDTO.Server;
            this.Tag = "server";
        }

        public override void DoSelect()
        {
            if (IsLoggedIn)
            {
                PropertiesCtl.Visible = true;
                PropertiesCtl.Init(VMDirConstants.ATTR_VMW_DSEROOT_DN, string.Empty, ServerDTO, ServerProperties);
            }
            else
                PropertiesCtl.Visible = false;
        }
        public override void DoRefresh()
        {
            this.Nodes.Clear();
            this.Nodes.Add(new DirectoryExpandableNode(ServerDTO.BaseDN, string.Empty, ServerDTO, PropertiesCtl));
            Expand();
            RefreshProperties();
        }
        internal void Login()
        {
            this.Nodes.Clear();
            try
            {
                var frm = new frmConnectToServer(ServerDTO);
                if (frm.ShowDialog() == DialogResult.OK)
                {
                    if (ServerDTO.Connection.CreateConnection() == 1)
                    {
                        IsLoggedIn = true;
                        this.Text = ServerDTO.Server;

                        if (string.IsNullOrWhiteSpace(ServerDTO.BaseDN))
                        {
                            TextQueryDTO dto = new TextQueryDTO("", LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
                                new string[] { VMDirConstants.ATTR_ROOT_DOMAIN_NAMING_CONTEXT }, 0, IntPtr.Zero, 0);
                            try
                            {
                                ServerDTO.Connection.Search(dto,
                                    delegate(ILdapMessage searchRequest, List<ILdapEntry> entries)
                                    {
                                        ServerDTO.BaseDN = GetRootDomainNamingContext(entries);
                                    });
                            }
                            catch (Exception)
                            {
                                throw new Exception(VMDirConstants.ERR_DN_RETRIEVAL);
                            }
                        }
                        else
                        {
                            TextQueryDTO dto = new TextQueryDTO(ServerDTO.BaseDN, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
                                new string[] { VMDirConstants.ATTR_DN }, 0, IntPtr.Zero, 0);
                            ServerDTO.Connection.Search(dto, null);
                        }
                        this.Nodes.Add(new DirectoryExpandableNode(ServerDTO.BaseDN, string.Empty, ServerDTO, PropertiesCtl));
                        Expand();
                        DoSelect();
                    }
                    else
                    {
                        throw new Exception(CommonConstants.INVALID_CREDENTIAL);
                    }
                }
            }
            catch (Exception exp)
            {
                ServerDTO.Connection = null;
                VMDirEnvironment.Instance.Logger.LogException(exp);
                MiscUtilsService.ShowError(exp);
            }
        }

        private string GetRootDomainNamingContext(List<ILdapEntry> entries)
        {
            if (entries != null)
            {
                var value = entries[0].getAttributeValues(VMDirConstants.ATTR_ROOT_DOMAIN_NAMING_CONTEXT);
                if (value != null && value.Count > 0)
                    return value[0].StringValue;
            }
            return string.Empty;
        }

        public void Logout()
        {
            try
            {
                ServerDTO.Connection = new LdapConnectionService(ServerDTO.Server, ServerDTO.BindDN, ServerDTO.Password);
                ServerDTO.Connection.CloseConnection();
                ServerDTO.Connection = null;
                IsLoggedIn = false;
                ServerDTO.BaseDN = string.Empty;
                ServerDTO.BindDN = string.Empty;
                Nodes.Clear();
                PropertiesCtl.Visible = false;
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
        }

        public void RemoveServer()
        {
            if (MiscUtilsService.Confirm(CommonConstants.GetDeleteMsg("server", ServerDTO.Server)))
            {
                VMDirEnvironment.Instance.LocalData.RemoveServer(ServerDTO.GUID);
                if (ServerDTO.Connection != null)
                    ServerDTO.Connection.CloseConnection();

                var parent = this.Parent as RootNode;
                if (parent != null)
                {
                    parent.Nodes.Remove(this);
                }
            }
        }

        internal void SuperLog()
        {
            var frm = new SuperLogBrowser(ServerDTO);
            frm.ShowDialog();
            frm.BringToFront();
        }

        internal void SetPageSize()
        {
            var frm = new SetPageSizeForm(ServerDTO.PageSize);
            if (frm.ShowDialog() == DialogResult.OK)
                ServerDTO.PageSize = frm.PageSize;
        }
    }
}
