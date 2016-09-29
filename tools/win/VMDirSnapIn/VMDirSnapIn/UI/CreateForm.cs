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
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;
using VMwareMMCIDP.UI.Common.Utilities;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
    public partial class CreateForm : Form
    {
        string _objectClass;
        VMDirServerDTO _serverDTO;
        Dictionary<string, VMDirAttributeDTO> _properties;
        private string _parentDn;
        public string Rdn;

        public Dictionary<string, VMDirAttributeDTO> Attributes { get { return _properties; } }

        public CreateForm(string objectClass, VMDirServerDTO serverDTO,string parentDn)
        {
            _objectClass = objectClass;
            _serverDTO = serverDTO;
            _parentDn = parentDn;

            InitializeComponent();
            ColumnHeader attrColumnHeader = new ColumnHeader();
            attrColumnHeader.Text = "Attribute";
            attrColumnHeader.Width = 200;
            listViewProp.Columns.Add(attrColumnHeader);
            ColumnHeader valColumnHeader = new ColumnHeader();
            valColumnHeader.Text = "Value";
            valColumnHeader.Width = 200;
            listViewProp.Columns.Add(valColumnHeader);
            Bind();
        }

        void Bind()
        {
            this.Text = "New " + _objectClass;
            textBoxParentDn.Text = _parentDn;
            MiscUtilsService.CheckedExec(delegate
            { 
            var requiredProps = _serverDTO.Connection.SchemaManager.GetRequiredAttributes(_objectClass);
            _properties = new Dictionary<string, VMDirAttributeDTO>();
            foreach (var prop in requiredProps)
            {
                VMDirAttributeDTO dto = new VMDirAttributeDTO(prop.Name, new List<LdapValue>(), prop);
                _properties.Add(prop.Name, dto);
            }
            var oc = _properties[VMDirConstants.ATTR_OBJECT_CLASS];
            LdapValue val = new LdapValue(_objectClass);
            oc.Values = new List<LdapValue>() { val };
            VMDir.Common.VMDirUtilities.Utilities.RemoveDontShowAttributes(_properties);

            foreach (var item in _properties)
            {
                foreach (var values in item.Value.Values)
                {
                    ListViewItem lvi = new ListViewItem(new string[] { item.Key, values.StringValue });
                    this.listViewProp.Items.Add(lvi);
                }
                if (item.Value.Values.Count == 0)
                {
                    ListViewItem lvi = new ListViewItem(new string[] { item.Key, string.Empty });
                    this.listViewProp.Items.Add(lvi);
                }
            }
            });
        }

        void listViewProp_DoubleClick(object sender, System.EventArgs e)
        {
            ListViewItem lvi = listViewProp.SelectedItems[0];
            if (!string.Equals(lvi.SubItems[0].Text, VMDirConstants.ATTR_OBJECT_CLASS))
            {
                System.Windows.Forms.ListViewItem.ListViewSubItem lvsi = lvi.SubItems[1];
                textBoxEdit.Bounds = new Rectangle(
                    listViewProp.Bounds.Left + lvsi.Bounds.Left,
                    listViewProp.Bounds.Top + lvsi.Bounds.Top,
                    lvsi.Bounds.Width,
                    lvsi.Bounds.Height);
                var type = String.Empty;
                textBoxEdit.Text = lvsi.Text;
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
            textBoxEdit.Visible = false;
            LdapValue val = new LdapValue(lvsi.Text);
            _properties[lvi.SubItems[0].Text].Values = new List<LdapValue>() { val };
        }
        private void btnOK_Click(object sender, EventArgs e)
        {
            if (DoValidate())
            {
                Rdn = textBoxRdn.Text;
                DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        bool DoValidate()
        {
            if (string.IsNullOrWhiteSpace(textBoxParentDn.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_DN_ENT);
                return false;
            }
            if (string.IsNullOrWhiteSpace(textBoxRdn.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_RDN_ENT);
                return false;
            }
            foreach (ListViewItem item in this.listViewProp.Items)
            {
                if (string.IsNullOrWhiteSpace(item.SubItems[1].Text))
                {
                    MMCDlgHelper.ShowWarning(item.SubItems[0].Text + " is required.");
                    return false;
                }
            }
            return true;
        }
    }
}
