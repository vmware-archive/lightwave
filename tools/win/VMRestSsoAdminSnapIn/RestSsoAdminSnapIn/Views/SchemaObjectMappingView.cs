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
using System.Linq;
using System.Collections.Generic;
using System.Data;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SchemaObjectMappingView : Form, IView
    {
        private SchemaObjectMappingDto _schemaObjectMappingDto;
        private List<KeyValuePair> _unmappedObjectIds;
        private bool _onload = true;
        public bool HasChanges { get; set; }
        public SchemaObjectMappingView(SchemaObjectMappingDto dto, List<KeyValuePair> unmappedObjectIds)
        {
            InitializeComponent();
            _unmappedObjectIds = unmappedObjectIds;
            if (dto == null)
            {
                dto = new SchemaObjectMappingDto { ObjectId = unmappedObjectIds.FirstOrDefault().Key.ToString() };
                btnAdd.Text = "Add";
            }
            else
            {
                cbObjectId.Enabled = false;
                btnAdd.Text = "Update";
            }
            _schemaObjectMappingDto = dto;
            HasChanges = false;
        }
        private Dictionary<string, string> GetAttributes()
        {
            var dictionary = new Dictionary<string, string>();

            for (var index = 0; index < dgAttributes.Rows.Count - 1; index++)
            {
                var row = dgAttributes.Rows[index];
                dictionary.Add(row.Cells[0].Value.ToString(), row.Cells[1].Value.ToString());
            }
            return dictionary;
        }
        private void BindControls()
        {
            _onload = true;
            txtClassName.Text = _schemaObjectMappingDto.ObjectClass;
            PopulateObjectId();
            PopulateAttributeIds();
            _onload = false;
        }

        private void PopulateAttributeIds()
        {
            dgAttributes.Rows.Clear();
            dgAttributes.Columns.Clear();
            var attributeColumn = new DataGridViewComboBoxColumn
            {
                DataSource = GetAttributesForObjectId(),
                DisplayMember = "Value",
                ValueMember = "Key",
                DisplayStyle = DataGridViewComboBoxDisplayStyle.Nothing,
                Width = 180,
                HeaderText = "Attribute ID",
                MaxDropDownItems = 3
            };
            dgAttributes.Columns.Add(attributeColumn);

            var valueColumn = new DataGridViewTextBoxColumn()
            {
                Width = 220,
                HeaderText = "Value"
            };
            dgAttributes.Columns.Add(valueColumn);

            if (_schemaObjectMappingDto.AttributeMappings != null)
            {
                var rowId = 0;
                foreach (var attribute in _schemaObjectMappingDto.AttributeMappings)
                {
                    dgAttributes.Rows.Add(1);
                    ((DataGridViewComboBoxCell)dgAttributes.Rows[rowId].Cells[0]).Value = attribute.Key;// GetAttribute(attribute.Key);
                    dgAttributes.Rows[rowId].Cells[1].Value = attribute.Value;
                    rowId++;
                }
            }
            dgAttributes.Refresh();
        }

        private List<KeyValuePair> GetAttributesForObjectId()
        {
            ObjectId selectedObjectId;
            var keyvalues = new List<KeyValuePair>();
            var keyValue = (KeyValuePair)cbObjectId.SelectedItem;
            if (Enum.TryParse(keyValue.Key.ToString(), false, out selectedObjectId))
            {
                switch (selectedObjectId)
                {
                    case ObjectId.ObjectIdUser:
                        var user = new UserAttributeId();
                        keyvalues = user.ToKeyValueList();
                        break;
                    case ObjectId.ObjectIdPasswordSettings:
                        var pwd = new PasswordAttributeId();
                        keyvalues = pwd.ToKeyValueList();
                        break;
                    case ObjectId.ObjectIdGroup:
                        var grp = new GroupAttributeId();
                        keyvalues = grp.ToKeyValueList();
                        break;
                    case ObjectId.ObjectIdDomain:
                        var dmn = new DomainAttributeId();
                        keyvalues = dmn.ToKeyValueList();
                        break;
                    default:
                        break;
                }
            }
            return keyvalues;
        }

        private string GetAttribute(string key)
        {
            string description = null;

            ObjectId selectedObjectId;
            var keyValue = (KeyValuePair)cbObjectId.SelectedItem;
            if (Enum.TryParse(keyValue.Key.ToString(), false, out selectedObjectId))
            {
                switch (selectedObjectId)
                {
                    case ObjectId.ObjectIdUser:
                        var user = new UserAttributeId();
                        if (Enum.TryParse(key, true, out user))
                        {
                            description = user.GetDescription();
                        }
                        break;
                    case ObjectId.ObjectIdPasswordSettings:
                        var pwd = new PasswordAttributeId();
                        if (Enum.TryParse(key, true, out pwd))
                        {
                            description = pwd.GetDescription();
                        }
                        break;
                    case ObjectId.ObjectIdGroup:
                        var grp = new GroupAttributeId();
                        if (Enum.TryParse(key, true, out grp))
                        {
                            description = grp.GetDescription();
                        }
                        break;
                    case ObjectId.ObjectIdDomain:
                        var dmn = new DomainAttributeId();
                        if (Enum.TryParse(key, true, out dmn))
                        {
                            description = dmn.GetDescription();
                        }
                        break;
                    default:
                        break;
                }
            }
            return description;
        }

        private void PopulateObjectId()
        {  
            cbObjectId.DataSource = _unmappedObjectIds;
            cbObjectId.DisplayMember = "Value";
            cbObjectId.ValueMember = "Key";            
            cbObjectId.SelectedValue = _schemaObjectMappingDto.ObjectId;
            cbObjectId.Refresh();
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (dgAttributes.SelectedRows != null && dgAttributes.SelectedRows.Count > 0)
            {
                if (dgAttributes.SelectedRows[0].Index != dgAttributes.Rows.Count - 1)
                {
                    dgAttributes.Rows.RemoveAt(dgAttributes.SelectedRows[0].Index);
                    dgAttributes.Refresh();
                    HasChanges = true;
                }
            }
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (IsValid())
            {
                var keyValue = (KeyValuePair)cbObjectId.SelectedItem;
                _schemaObjectMappingDto.ObjectId = keyValue.Key.ToString();
                _schemaObjectMappingDto.ObjectClass = txtClassName.Text;
                _schemaObjectMappingDto.AttributeMappings = new Dictionary<string, string>();

                int index = 0;
                foreach (DataGridViewRow row in dgAttributes.Rows)
                {
                    if (index < dgAttributes.Rows.Count - 1)
                        _schemaObjectMappingDto.AttributeMappings.Add(row.Cells[0].Value.ToString(), row.Cells[1].Value.ToString());
                    index++;
                }
                HasChanges = true;
                Close();
            }
        }
        private bool IsValid()
        {
            if (cbObjectId.SelectedIndex == -1)
            {
                MMCDlgHelper.ShowWarning("Select a valid Object Id");
                return false;
            }
            if (string.IsNullOrEmpty(txtClassName.Text))
            {
                MMCDlgHelper.ShowWarning("Select a valid Class Name");
                return false;
            }
            if (dgAttributes.Rows.Count <= 1)
            {
                MMCDlgHelper.ShowWarning("Select attribute mapping");
                return false;
            }
            if (dgAttributes.Rows.Count > 1)
            {
                foreach (DataGridViewRow row in dgAttributes.Rows)
                {
                    if (row.Cells[0] != null && row.Cells[1] != null && !string.IsNullOrEmpty(row.Cells[0].ToString()) && !string.IsNullOrEmpty(row.Cells[1].ToString()))
                        continue;
                    else
                        MMCDlgHelper.ShowWarning("One or more attribute mapping is invalid");
                    return false;
                }
            }
            return true;
        }
        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void SchemaObjectMappingView_Load(object sender, EventArgs e)
        {
            BindControls();
        }

        public IDataContext DataContext
        {
            get { return _schemaObjectMappingDto; }
        }

        private void cbObjectId_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!_onload)
            PopulateAttributeIds();
        }
    }
}
