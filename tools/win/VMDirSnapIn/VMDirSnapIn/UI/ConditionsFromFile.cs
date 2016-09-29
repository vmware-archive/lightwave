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
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class ConditionsFromFile : Form
    {
        private List<FilterDTO> _filters;
        private List<string> _attrList;
        public ConditionsFromFile(List<FilterDTO> filters, List<string> attrList)
        {
            _filters = filters;
            _attrList = attrList;
            InitializeComponent();
            this.comboBoxAttr.Items.AddRange(_attrList.ToArray());
            this.comboBoxCond.Items.AddRange(VMDirConstants.ConditionList);
            this.comboBoxAttr.SelectedIndex = 0;
            this.comboBoxCond.SelectedIndex = 0;
        }

        private void buttonBrowse_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate()
            {
                var values = MMCMiscUtil.ReadAllFromFile("Select File", MMCUIConstants.TXT_FILTER);
                this.textBox1.Text = values;
            });
        }

        private void buttonApply_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                this.DialogResult = DialogResult.None;
                return;
            }
            _filters.Clear();
            var charArr=new char[] { '\r', '\n' };
            foreach (var item in this.textBox1.Text.Split('\n'))
            {
                var val = item.Trim(charArr);
                if(!string.IsNullOrWhiteSpace(val))
                    _filters.Add(new FilterDTO(comboBoxAttr.SelectedItem.ToString(), (Condition) comboBoxCond.SelectedIndex, val));
            }
            this.Close();
        }

        private bool ValidateForm()
        {
            if (comboBoxAttr.SelectedItem == null)
            {
                if (comboBoxAttr.Items.Contains(comboBoxAttr.Text))
                {
                    comboBoxAttr.SelectedIndex = comboBoxAttr.Items.IndexOf(comboBoxAttr.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_ATTR);
                    return false;
                }
            }
            if (comboBoxCond.SelectedItem == null)
            {
                if (comboBoxCond.Items.Contains(comboBoxCond.Text))
                {
                    comboBoxCond.SelectedIndex = comboBoxCond.Items.IndexOf(comboBoxCond.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_COND);
                    return false;
                }
            }
            if (string.IsNullOrWhiteSpace(this.textBox1.Text))
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_VAL);
                return false;
            }
            return true;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
