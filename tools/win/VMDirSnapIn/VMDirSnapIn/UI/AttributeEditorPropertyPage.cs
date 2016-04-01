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
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.DirectoryServices.Protocols;
using System.Collections;
using Microsoft.ManagementConsole;
using VMDirSnapIn.UI.PropertyPageMgmt;
using VMDirSnapIn.Utilities;
using VMDir.Common.DTO;
using VMDirSnapIn.Services;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
    public partial class AttributeEditorPropertyPage : UserControl
    {
        string _dn;
        VMDirPropertyPage _parent;
        public VMDirPropertyPage Page { get { return _parent; } }
        public VMDirServerDTO ServerDTO { get; protected set; }
        public string Title
        {
            get { return _parent.Title; }
            set { _parent.Title = value; }
        }

        private Dictionary<string, VMDirBagItem> _properties;
        private List<KeyValuePair<string, string>> _kvData;
        private Dictionary<string, string> _pendingMods;

        public AttributeEditorPropertyPage(string dn, VMDirServerDTO serverDTO)
        {
            ServerDTO = serverDTO;
            _dn = dn;
            InitializeComponent();
            MMCInit();
            _properties = new Dictionary<string, VMDirBagItem>();
            _kvData = new List<KeyValuePair<string, string>>();
            _pendingMods = new Dictionary<string, string>();
            Bind();
        }

        void MMCInit()
        {
            _parent = new VMDirPropertyPage();
            _parent.Control = this;
            _parent.Apply += new CancelEventHandler(_parent_Apply);
        }

        void _parent_Apply(object sender, CancelEventArgs e)
        {
            try
            {
                if (_properties.Count > 0 && _pendingMods.Count > 0)
                {
                    LdapMod[] user = new LdapMod[_pendingMods.Count];
                    int i = 0;
                    foreach (var entry in _pendingMods)
                    {
                        string[] entries = entry.Value.Split(',');
                        string[] values = new string[entries.Length + 1];
                        entries.CopyTo(values, 0);
                        values[entries.Length] = null;
                        user[i] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, entry.Key, values);
                        i++;
                    }
                    ServerDTO.Connection.ModifyObject(_dn, user);
                    _pendingMods.Clear();
                }
            }
            catch (Exception exp)
            {
                MiscUtilsService.ShowError(exp);
                e.Cancel = true;
            }
        }

        void Bind()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                VMDir.Common.VMDirUtilities.Utilities.GetItemProperties(_dn, ServerDTO, _properties);
                VMDir.Common.VMDirUtilities.Utilities.RemoveDontShowAttributes(_properties);
                MiscUtilsService.ConvertToKVData(_properties,_kvData);
                RefreshView();
            });
        }
        void RefreshView()
        {
            listView1.Items.Clear();
            foreach (var item in _kvData)
            {
                ListViewItem lvi = new ListViewItem(item.Key);
                lvi.SubItems.Add(item.Value);
                listView1.Items.Add(lvi);
            }
        }

        IEnumerable<KeyValuePair<string, string>> GetCurrentOptionalProperties()
        {
            return _properties.Where(x => !x.Value.IsRequired)
                .Select(x => new KeyValuePair<string, string>(x.Key, x.Value.Description));
        }

        private void btnManageAttributes_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var optionalProps = GetCurrentOptionalProperties();
                if (!_properties.ContainsKey("objectClass"))
                    throw new Exception(VMDirConstants.UNABLE_TO_FETCH_DATA);
                var oc = _properties["objectClass"].Value;
                string cn = "";
                if (oc is string)
                    cn = oc.ToString();
                else if (oc is LdapValue[])
                {
                    LdapValue[] val = oc as LdapValue[];
                    cn = val[(val.Count() - 1)].StringValue;
                }
                var frm = new AddOrRemoveAttributes(cn, optionalProps, ServerDTO);
                if (frm.ShowDialog() == DialogResult.OK)
                {
                    var retainList = frm.OptionalAttributes.Intersect(optionalProps);
                    var removeList = optionalProps.Except(retainList).ToList();
                    foreach (var item in removeList)
                    {
                        _properties.Remove(item.Key);
                    }
                    var addList = frm.OptionalAttributes.Except(retainList);
                    foreach (var item in addList)
                    {
                        var dto = ServerDTO.Connection.SchemaManager.GetAttributeType(item.Key);
                        _properties.Add(item.Key, new VMDirBagItem { Description = dto.Description, IsReadOnly = dto.ReadOnly, Value = null });
                        _kvData.Add(new KeyValuePair<string, string>(item.Key, null));
                    }
                    RefreshView();
                }
            });
        }

        private void listView1_DoubleClick(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                if (listView1.SelectedItems != null && listView1.SelectedItems.Count > 0)
                {
                    var attr = listView1.SelectedItems[0].Text;
                    var val = listView1.SelectedItems[0].SubItems[1].Text;
                    if (!String.Equals(attr, "objectClass", StringComparison.OrdinalIgnoreCase))
                    {
                        var frm = new EditAttribute(attr, val);
                        if (frm.ShowDialog() == DialogResult.OK)
                        {
                            _properties[attr].Value = frm.value;
                            listView1.SelectedItems[0].SubItems[1].Text = frm.value;
                            if (_pendingMods.ContainsKey(attr))
                                _pendingMods[attr] = frm.value;
                            else
                                this._pendingMods.Add(attr, frm.value);
                            _parent.Dirty = true;
                        }
                    }
                }
            });
        }
    }
}
