/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using System.Collections.Generic;
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;
using System.Linq;
using System;
using VMDirInterop.LDAP;
using System.Drawing;
using System.ComponentModel;
using VMIdentity.CommonUtils;
using VmdirUtil = VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class PropertiesControl : UserControl
    {
        private Dictionary<string, VMDirAttributeDTO> _properties;
        private List<AttributeDTO> _currAttrDTOList;
        private List<AttributeDTO> _optAttrDTOList;
        private List<AttributeDTO> _oprAttrDTOList;
        private HashSet<string> _modData;
        private List<AttributeTypeDTO> _mayAttrTyDTOList;
        private string _objectClass = string.Empty;
        private string _dn = string.Empty;
        private VMDirServerDTO _serverDTO;
        private string oldVal = string.Empty;
        enum GroupTag
        {
            OPERATIONAL_ATT=0,
            CURRENT_ATTR,
            OPTIONAL_ATT
        }

        public PropertiesControl()
        {
            InitializeComponent();
            MMCMiscUtil.SetDoubleBuffered(this.listViewProp);
            this.Dock = DockStyle.Fill;

            ColumnHeader attrColumnHeader = new ColumnHeader();
            attrColumnHeader.Text = "Attribute";
            attrColumnHeader.Width = 400;
            listViewProp.Columns.Add(attrColumnHeader);
            ColumnHeader valColumnHeader = new ColumnHeader();
            valColumnHeader.Text = "Value";
            valColumnHeader.Width = 400;
            listViewProp.Columns.Add(valColumnHeader);
            ColumnHeader typColumnHeader = new ColumnHeader();
            typColumnHeader.Text = "Syntax";
            typColumnHeader.Width = 400;
            listViewProp.Columns.Add(typColumnHeader);

            _properties = new Dictionary<string, VMDirAttributeDTO>();
            _currAttrDTOList = new List<AttributeDTO>();
            _optAttrDTOList = new List<AttributeDTO>();
            _oprAttrDTOList = new List<AttributeDTO>();
            _mayAttrTyDTOList = new List<AttributeTypeDTO>();
            _modData = new HashSet<string>();
            SetEditState(false);
        }
        internal void Init(string dn, string oc, VMDirServerDTO serverDTO, Dictionary<string, VMDirAttributeDTO> properties)
        {
            if (serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            _properties = properties;
            _objectClass = oc;
            _dn = dn;
            _serverDTO = serverDTO;
            ClearData();
            ClearContext();
            GetData();
            FillListView();
            SetEditState(false);
        }

        public void ClearData()
        {
            _currAttrDTOList.Clear();
            _oprAttrDTOList.Clear();
            _optAttrDTOList.Clear();
            _modData.Clear();
        }
        public void ClearContext()
        {
            this.listViewProp.Items.Clear();
            this.listViewProp.Groups.Clear();
            SetEditState(false);
        }
        public void GetData()
        {
            _currAttrDTOList = VmdirUtil.Utilities.ConvertToAttributeDTOList(_properties);

            if (getGroup(GroupTag.OPTIONAL_ATT) != null)
                GetOptionalAttribute();
            if (getGroup(GroupTag.OPERATIONAL_ATT) != null)
                GetOperationalAttribute();
        }

        private void GetOptionalAttribute()
        {
            MiscUtilsService.CheckedExec(delegate
                 {
                     if (string.IsNullOrWhiteSpace(_objectClass))
                         _objectClass = VmdirUtil.Utilities.GetAttrLastVal(_properties, VMDirConstants.ATTR_OBJECT_CLASS);
                         _mayAttrTyDTOList.Clear();
                         _mayAttrTyDTOList = _serverDTO.Connection.SchemaManager.GetOptionalAttributes(_objectClass);
                         _optAttrDTOList.Clear();
                         foreach (var item in _mayAttrTyDTOList)
                             if (item != null)
                                 _optAttrDTOList.Add(new AttributeDTO(item.Name, string.Empty, item));
                     foreach (var item in _currAttrDTOList)
                     {
                         if (item.AttrSyntaxDTO.SingleValue)
                             _optAttrDTOList.RemoveAll(x => x.Name.Equals(item.Name));
                     }
                     _optAttrDTOList.Sort((x, y) => string.Compare(x.Name, y.Name, StringComparison.InvariantCultureIgnoreCase));
                 });
        }

        private void GetOperationalAttribute()
        {
            TextQueryDTO dto = new TextQueryDTO(_dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC,
                new string[] { "+" }, 0, IntPtr.Zero, 0);
            var operationalProperties = new Dictionary<string, VMDirAttributeDTO>();
            _serverDTO.Connection.Search(dto, (l, e) =>
            {
                if (e.Count > 0)
                    operationalProperties = _serverDTO.Connection.GetEntryProperties(e[0]);
            });
            _oprAttrDTOList = VmdirUtil.Utilities.ConvertToAttributeDTOList(operationalProperties);
        }
        public void FillListView()
        {
            var clvg = new ListViewGroup("Current Attributes") { Tag = GroupTag.CURRENT_ATTR };          
            listViewProp.Groups.Add(clvg);
            var lviList = new List<ListViewItem>();
            foreach (var item in _currAttrDTOList)
            {
                var val = item.Value;
                if (string.Equals(item.AttrSyntaxDTO.Type, "Generalized Time"))
                    val = VmdirUtil.Utilities.ConvertGeneralizedTimeIntoReadableFormat(item.Value);
                lviList.Add(new ListViewItem(new string[] { item.Name, val, item.AttrSyntaxDTO.Type }) { Tag = clvg.Tag, Group = clvg, BackColor = Color.WhiteSmoke });
            }
            listViewProp.Items.AddRange(lviList.ToArray());

            if(_serverDTO.OptionalAttrFlag)
                ShowOptionalAttribute();
            else
                HideAttribute(GroupTag.OPTIONAL_ATT);

            if (_serverDTO.OperationalAttrFlag)
                ShowOperationalAttribute();
            else
                HideAttribute(GroupTag.OPERATIONAL_ATT);
        }

        private void ShowOptionalAttribute()
        {
            if (_serverDTO == null || _serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            MiscUtilsService.CheckedExec(delegate
            {
                if (_optAttrDTOList.Count <= 0)
                    GetOptionalAttribute();
                var optlvg = new ListViewGroup("Optional Attributes") { Tag = GroupTag.OPTIONAL_ATT };
                listViewProp.Groups.Add(optlvg);

                var lviList = new List<ListViewItem>();
                foreach (var item in _optAttrDTOList)
                {
                    lviList.Add(new ListViewItem(new string[] { item.Name, item.Value, item.AttrSyntaxDTO.Type }) { Tag = optlvg.Tag, Group = optlvg, BackColor = Color.WhiteSmoke });
                }
                listViewProp.Items.AddRange(lviList.ToArray());
            });
        }

        private void HideAttribute(GroupTag grpTag)
        {
            foreach (ListViewGroup grp in listViewProp.Groups)
            {
                if ((GroupTag)grp.Tag == grpTag)
                {
                    while (grp.Items.Count > 0)
                    {
                        listViewProp.Items.Remove(grp.Items[0]);
                    }
                    listViewProp.Groups.Remove(grp);
                    break;
                }
            }
        }

        private void ShowOperationalAttribute()
        {
            if (_serverDTO == null || _serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            MiscUtilsService.CheckedExec(delegate
            {
                if (_oprAttrDTOList.Count <= 0)
                    GetOperationalAttribute();
                var oprlvg = new ListViewGroup("Operational Attributes") { Tag = GroupTag.OPERATIONAL_ATT };
                this.listViewProp.Groups.Insert(0, oprlvg);

                var lviList = new List<ListViewItem>();
                foreach (var item in _oprAttrDTOList)
                {
                    var val = item.Value;
                    if (string.Equals(item.AttrSyntaxDTO.Type, "Generalized Time"))
                        val = VmdirUtil.Utilities.ConvertGeneralizedTimeIntoReadableFormat(item.Value);
                    lviList.Add(new ListViewItem(new string[] { item.Name, val, item.AttrSyntaxDTO.Type }) { Tag = oprlvg.Tag, Group = oprlvg, BackColor = Color.WhiteSmoke });
                }
                listViewProp.Items.AddRange(lviList.ToArray());
            });
        }
        public void RefreshPropertiesView()
        {
            ClearData();
            _properties.Clear();
            ClearContext();
            MiscUtilsService.CheckedExec(delegate
            {
                TextQueryDTO dto = new TextQueryDTO(_dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);
                _serverDTO.Connection.Search(dto,
                    (l, e) =>
                    {
                        if (e.Count > 0)
                            _properties = _serverDTO.Connection.GetEntryProperties(e[0]);
                    });
            });
            GetData();
            FillListView();
        }

        private ListViewGroup getGroup(GroupTag groupTag)
        {
            foreach (ListViewGroup lvg in listViewProp.Groups)
                if ((GroupTag)lvg.Tag == groupTag)
                    return lvg;
            return null;
        }

        private void contextMenuStripProp_Opening(object sender, CancelEventArgs e)
        {
            if (this.listViewProp.SelectedIndices.Count != 1)
            {
                e.Cancel = true;
            }
            else
            {
                var key = this.listViewProp.SelectedItems[0].Text;
                if (_serverDTO.Connection.SchemaManager.isSingleValue(key))
                    e.Cancel = true;
            }
        }

        private void addAnotherToolStripMenuItem_Click(object sender, EventArgs e)
        {
            listViewProp.ListViewItemSorter = new ListViewColumnSorter();

            ListViewItem lvi = this.listViewProp.SelectedItems[0];
            var key = lvi.SubItems[0].Text;
            var type = lvi.SubItems[2].Text;
            var index = this.listViewProp.SelectedIndices[0] + 1;
            var tag = this.listViewProp.SelectedItems[0].Group.Tag;
            ListViewItem newLvi = new ListViewItem(new string[] { key, string.Empty, type }) { Tag = tag, Group = getGroup((GroupTag)tag), BackColor = Color.WhiteSmoke };
            listViewProp.Items.Insert(index, newLvi);
            listViewProp.ListViewItemSorter = null;
        }

        void listViewProp_DoubleClick(object sender, System.EventArgs e)
        {
            ListViewItem lvi = listViewProp.SelectedItems[0];

            var pt = listViewProp.PointToClient(Control.MousePosition);
            var pt2 = listViewProp.PointToScreen(listViewProp.Bounds.Location);
            var top = lvi.SubItems[2].Bounds.Top;
            var bottom = lvi.SubItems[2].Bounds.Bottom;
            var left = lvi.SubItems[2].Bounds.Left;
            var right = lvi.SubItems[2].Bounds.Right;
            if (pt.X >= left && pt.X <= right && pt.Y >= top && pt.Y <= bottom)
            {
                var type = _serverDTO.Connection.SchemaManager.GetAttributeType(lvi.SubItems[0].Text);
                AttributeHelpDTO attrHelp = null;
                if (type.AttributeSyntax != null)
                    VmdirUtil.VMDirCommonEnvironment.Instance.AttrHelpDict.TryGetValue(type.AttributeSyntax, out attrHelp);

                var frm = new AttrInfoForm(attrHelp);
                frm.Text = type.AttributeSyntax;
                var maxHt = Screen.PrimaryScreen.Bounds.Height;
                var maxWd = Screen.PrimaryScreen.Bounds.Width;
                var x = pt2.X + left;
                var y = pt2.Y + top;
                if (y + frm.Height > maxHt)
                    y = y - frm.Height;
                if (x + frm.Width > maxWd)
                    x = x - frm.Width;
                frm.Location = new Point(x, y);
                frm.ShowDialog();
            }

            else
            {
                System.Windows.Forms.ListViewItem.ListViewSubItem lvsi = lvi.SubItems[1];
                textBoxEdit.Bounds = new Rectangle(
                    listViewProp.Bounds.Left + lvsi.Bounds.Left,
                    listViewProp.Bounds.Top + lvsi.Bounds.Top,
                    lvsi.Bounds.Width,
                    lvsi.Bounds.Height);
                textBoxEdit.Text = lvsi.Text;
                oldVal = lvsi.Text;
                textBoxEdit.Visible = true;
                textBoxEdit.Focus();
            }
        }

        void textBoxEdit_LostFocus(object sender, System.EventArgs e)
        {
            exitEditing();
        }
        void textBoxEdit_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            if (e.KeyChar == (char)Keys.Enter)
                exitEditing();
        }

        private void exitEditing()
        {
            ListViewItem lvi = listViewProp.SelectedItems[0];
            System.Windows.Forms.ListViewItem.ListViewSubItem lvsi = lvi.SubItems[1];
            lvsi.Text = textBoxEdit.Text;
            if (!string.Equals(oldVal, lvsi.Text)) 
            {
                _modData.Add(lvi.SubItems[0].Text);
                lvi.BackColor = Color.LightYellow;
                SetEditState(true);
            }
            oldVal = string.Empty;
            textBoxEdit.Visible = false;

        }

        private void buttonSubmit_Click(object sender, EventArgs e)
        {
            if (_serverDTO==null || _serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            MiscUtilsService.CheckedExec(delegate
            {
                if (_modData.Count > 0)
                {
                    var finalMods = new Dictionary<string, List<string>>();
                    foreach (ListViewItem item in listViewProp.Items)
                    {
                        var key = item.SubItems[0].Text;
                        var val = item.SubItems[1].Text;
                        if (_modData.Contains(key))
                        {
                            if (finalMods.ContainsKey(key))
                            {
                                finalMods[key].Add(val);
                            }
                            else
                            {
                                finalMods.Add(key, new List<string>() { val });
                            }
                        }
                    }
                    var frm = new SubmitModConfirm(finalMods);
                    if (frm.ShowDialog() != DialogResult.OK)
                        return;

                    //LdapMod[] attrMods = new LdapMod[finalMods.Count];
                    List<AttributeModStatus> modificationStatus = new List<AttributeModStatus>();
                    int i = 0;
                    foreach (var m in finalMods)
                    {
                        LdapMod[] ldapVal = new LdapMod[1];
                        var values = m.Value.Where(x => !string.IsNullOrWhiteSpace(x)).ToArray();
                        Array.Resize(ref values, values.Count() + 1);
                        ldapVal[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, m.Key, values);
                        try
                        {
                            _serverDTO.Connection.ModifyObject(_dn, ldapVal);
                            modificationStatus.Add(new AttributeModStatus(m.Key,true,"Success"));
                        }
                        catch(Exception exp){
                            modificationStatus.Add(new AttributeModStatus(m.Key, false, exp.Message));
                        }
                        i++;
                    }
                    var frm2 = new SubmitModStatus(modificationStatus);
                    frm2.ShowDialog();
                    RefreshPropertiesView();
                }
            });
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            if (_serverDTO == null || _serverDTO.Connection == null)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RELOGIN);
                return;
            }
            ClearContext();
            _modData.Clear();
            FillListView();
            SetEditState(false);
        }

        public void SetEditState(bool state)
        {
            this.buttonSubmit.Visible = state;
            this.buttonReset.Visible = state;
        }
        public void ClearView()
        {
            _properties = null;
            _mayAttrTyDTOList.Clear();
            ClearData();
            ClearContext();
        }
    }
}
