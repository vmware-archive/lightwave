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
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;
using VMDir.Common.Schema;
using VMDirSnapIn.Services;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
    public partial class CreateForm : Form
    {
        string _objectClass;
        VMDirServerDTO _serverDTO;
        KeyValueBag _properties;

        public KeyValueBag Attributes { get { return _properties; } }

        public CreateForm(string objectClass, VMDirServerDTO serverDTO)
        {
            _objectClass = objectClass;
            _serverDTO = serverDTO;

            InitializeComponent();
            Bind();
            VMDir.Common.VMDirUtilities.Utilities.RemoveDontShowAttributes(_properties);
        }

        void Bind()
        {
            this.Text = "New " + _objectClass;
            var requiredProps = _serverDTO.Connection.SchemaManager.GetRequiredAttributesWithContentRules(_objectClass);

            _properties = new KeyValueBag();
            foreach (var prop in requiredProps)
            {
                var val = MiscUtilsService.GetInstanceFromType(prop.Type);
                var item = new VMDirBagItem { Value = val, Description = prop.Description, IsRequired = true, IsReadOnly=prop.ReadOnly };
                _properties.Add(prop.Name, item);
            }
            var oc = _properties["objectClass"];
            oc.IsReadOnly = true;
            oc.Value = _objectClass;

            VMDirBagItem itemCN = null;
            if (!_properties.TryGetValue("cn", out itemCN))
            {
                _properties.Add("cn", new VMDirBagItem { Value = "", Description="", IsRequired = true });
            }

            props.SelectedObject = _properties;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            if (DoValidate())
            {
                DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        bool DoValidate()
        {
            var requiredPropsNotFilled = _properties.Where(x =>
            {
                var val = x.Value;
                if (val.IsRequired)
                {
                    if (val.Value == null) return true;
                    if (val.Value is string)
                        return string.IsNullOrEmpty(val.Value as string);
                    else if (val.Value is List<string>)
                        return (val.Value as List<string>).Count == 0;
                }
                return false;
            });
            if (requiredPropsNotFilled.Count() > 0)
            {
                string error = string.Format("{0} is a required property", requiredPropsNotFilled.First().Key);
                MiscUtilsService.ShowError(error);
                return false;
            }
            return true;
        }

        private void tsDelete_Click(object sender, EventArgs e)
        {
            var item = props.SelectedGridItem;
            if ((item.PropertyDescriptor as KeyValuePropertyDescriptor).IsRequired)
                MiscUtilsService.ShowError(item.Label + " is a required attribute and cannot be deleted");
            else
            {
                _properties.Remove(item.Label);
                props.Refresh();
            }

        }

        private void tsAdd_Click(object sender, EventArgs e)
        {
            var attr = _serverDTO.Connection.SchemaManager.GetAttributeType("description");
            var val = MiscUtilsService.GetInstanceFromType(attr.Type);
            _properties.Add(attr.Name, new VMDirBagItem { Value = val, Description = attr.Description });
            props.Refresh();
        }

        IEnumerable<KeyValuePair<string, string>> GetCurrentOptionalProperties()
        {
            return _properties.Where(x => !x.Value.IsRequired)
                .Select(x => new KeyValuePair<string, string>(x.Key, x.Value.Description));
        }

        private void btnManageAttributes_Click(object sender, EventArgs e)
        {
            var optionalProps = GetCurrentOptionalProperties();
            var frm = new AddOrRemoveAttributes(_objectClass, optionalProps, _serverDTO);
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
                    var dto = _serverDTO.Connection.SchemaManager.GetAttributeType(item.Key);
                    var val = MiscUtilsService.GetInstanceFromType(dto.Type);
                    _properties.Add(item.Key, new VMDirBagItem { Description = dto.Description, IsReadOnly = dto.ReadOnly, Value = val });
                }
                props.Refresh();
            }
        }
    }
}
