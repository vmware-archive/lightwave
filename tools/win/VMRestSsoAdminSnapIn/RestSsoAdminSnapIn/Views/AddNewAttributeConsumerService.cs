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
using System.Data;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class AddNewAttributeConsumerService : Form, IView
    {
        private AttributeConsumerServiceDto _dto;
        public AddNewAttributeConsumerService()
        {
            InitializeComponent();
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtName.Text.Trim()))
            {
                MMCDlgHelper.ShowWarning("Enter a valid name");
                return;
            }
            _dto = new AttributeConsumerServiceDto { Name = txtName.Text, Index = (int)nudIndex.Value, Attributes = GetAttributes(), IsDefault = chkDefault.Checked };
            DialogResult = DialogResult.OK;
            Close();
        }

        private List<AttributeDto> GetAttributes()
        {
            var list = new List<AttributeDto>();
            foreach (DataGridViewRow item in lstAttributes.Rows)
            {
                if (item.Cells[0].Value != null && item.Cells[1].Value != null && item.Cells[2].Value != null)
                {
                    var attribute = new AttributeDto { Name = item.Cells[0].Value.ToString(), FriendlyName = item.Cells[1].Value.ToString(), NameFormat = item.Cells[2].Value.ToString() };
                    list.Add(attribute);
                }
            }
            return list;
        }

        public Dto.IDataContext DataContext
        {
            get { return _dto; }
        }
    }
}
