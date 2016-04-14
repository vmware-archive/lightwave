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
using System.Linq;
using System.Text;
using Microsoft.ManagementConsole;
using System.IO;
using System.Windows.Forms;
using VMDirSnapIn.UI;
using VMDir.Common.DTO;
using VMDirSnapIn.Services;
using VMDirSnapIn.ScopeNodes.ListViews;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMIdentity.CommonUtils;
using VMDir.Common.VMDirUtilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.ScopeNodes
{
    public class DirectoryNode : ScopeNode
    {
        const int ACTION_ADD = 1;

        public string _name;
        public VMDirServerDTO ServerDTO { get; protected set; }
        public string Name { get { return _name; } }
        public DirectoryNodeDetailsView ListView { get; set; }

        public Dictionary<string, VMDirBagItem> _properties;
        public List<KeyValuePair<string, string>> _kvData;

        public DirectoryNode(string dn, VMDirServerDTO serverDTO)
        {
            ServerDTO = serverDTO;
            _name = dn;
            DisplayName = VMDirServerDTO.DN2CN(_name);
            ImageIndex = SelectedImageIndex = (int)VMDirImageIndex.DirectoryObject;

            InitNode();

            AddViewDescription();
            
        }

        void AddViewDescription()
        {
            MmcListViewDescription lvd = new MmcListViewDescription();
            lvd.DisplayName = DisplayName;
            lvd.ViewType = typeof(DirectoryNodeDetailsView);
            lvd.Options = MmcListViewOptions.ExcludeScopeNodes;

            this.ViewDescriptions.Add(lvd);
            this.ViewDescriptions.DefaultIndex = 0;
        }

        void InitNode()
        {
            this.EnabledStandardVerbs = StandardVerbs.Properties | StandardVerbs.Refresh
                | StandardVerbs.Delete;
            this.ActionsPaneItems.Add(new Microsoft.ManagementConsole.Action("Add",
                                       "Add", -1, ACTION_ADD));
        }

        protected override void OnAction(Microsoft.ManagementConsole.Action action, AsyncStatus status)
        {
            base.OnAction(action, status);

            switch ((int)action.Tag)
            {
                case ACTION_ADD:
                    Add();
                    break;
            }
        }

        void Add()
        {
            try
            {
				/*
                string dn = string.Format("cn=def3,{0}", _name);
                DirectoryAttribute[] attr = new DirectoryAttribute[]
                {
                    new DirectoryAttribute("cn", "def3"),
                    new DirectoryAttribute("name", "def2"),
                    new DirectoryAttribute("objectclass", "vmIdentity-Group"),
                    new DirectoryAttribute("vmidentity-account", "def2")
                };
                ServerDTO.Connection.Add(dn, attr);
                this.Children.Add(new DirectoryNode(dn, ServerDTO));
                */
                var frmSelect = new SelectObjectClass(ServerDTO.Connection.SchemaManager);
                if (SnapIn.Console.ShowDialog(frmSelect) == DialogResult.OK)
                {
                    var frm = new CreateForm(frmSelect.SelectedObject, ServerDTO);
                    if (SnapIn.Console.ShowDialog(frm) == DialogResult.OK)
                    {
                        var attributes = frm.Attributes.Select(x => LdapTypesService.MakeAttribute(x)).ToArray();

                        var cnVal = frm.Attributes.First(x => x.Key == "cn").Value.Value;
                        string dn = string.Format("cn={0},{1}", cnVal, _name);
                        ServerDTO.Connection.AddObject(dn, attributes);
                        this.Children.Add(new DirectoryNode(dn, ServerDTO));
                    }
                }
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
        }

        protected override void OnDelete(SyncStatus status)
        {
            if (!MiscUtilsService.Confirm(string.Format(CommonConstants.CONFIRM_DELETE, "object", DisplayName)))
                return;

            base.OnDelete(status);

            try
            {
                ServerDTO.Connection.DeleteObject(_name);
                var parent = this.Parent;
                if (parent != null)
                {
                    parent.Children.Remove(this);
                }
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
            }
        }

        protected override void OnAddPropertyPages(PropertyPageCollection propertyPageCollection)
        {
            var attrPage = new AttributeEditorPropertyPage(_name, ServerDTO);
            attrPage.Title = "Attribute Editor";
            propertyPageCollection.Add(attrPage.Page);
        }

        private void PopulateChildren(string itemDN)
        {
            this.Children.Clear();
            ILdapMessage ldMsg = null;
            try
            {
                string[] entries = ServerDTO.Connection.SearchAndGetDN(itemDN, LdapScope.SCOPE_ONE_LEVEL, "(objectClass=*)", new string[]{"entryDN"}, 0, ref ldMsg);
                if (entries != null)
                {
                    for (int i = 0; i < entries.Length; i++)
                    {
                        this.Children.Add(new DirectoryNode(entries[i], ServerDTO));
                    }
                }
            }
            catch (Exception exp)
            {
                MMCDlgHelper.ShowException(exp);
            }
        }

        protected override void OnExpand(AsyncStatus status)
        {
            base.OnExpand(status);
            PopulateChildren(_name);
        }

        protected override void OnRefresh(AsyncStatus status)
        {
            base.OnRefresh(status);
            PopulateChildren(_name);
            ListView.Refresh();
            
        }
    }
}
