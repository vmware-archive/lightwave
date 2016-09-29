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
using VMDirSnapIn.TreeNodes;
using VMwareMMCIDP.UI.Common.Utilities;
using VMDirSnapIn.Utilities;

namespace VMDirSnapIn.UI
{
    public partial class ExportResult : Form
    {
        private List<DirectoryNonExpandableNode> _result;
        private List<string> _attrList;
        private int _currPage;
        private int _pageSize;
        enum ExportScope{
            CURR_PAGE=0,
            FETCHED_PAGE
        }
        public ExportResult(List<DirectoryNonExpandableNode> result, List<string> attrList, int currPage, int pageSize)
        {
            this._result = result;
            this._attrList = attrList;
            this._currPage=currPage;
            this._pageSize=pageSize;
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            this.comboBoxFileFormat.Items.Add("csv");
            this.comboBoxScope.Items.Add("Current Result Page");
            this.comboBoxScope.Items.Add("All Fetched Pages");
            this.comboBoxFileFormat.SelectedIndex = 0;
            this.comboBoxScope.SelectedIndex = 0;
            this.checkBoxAttToExport.CheckState = CheckState.Checked;
            this.comboBoxAttrToReturn.Items.AddRange(_attrList.ToArray());
        }
        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void buttonExport_Click(object sender, EventArgs e)
        {
            if (!ValidateForm())
            {
                return;
            }
            MiscUtilsService.CheckedExec(delegate()
            {
                StringBuilder sb = new StringBuilder();
                var start = 0;
                var end = _result.Count();
                if (comboBoxScope.SelectedIndex == (int)ExportScope.CURR_PAGE)
                {
                    start = (_currPage - 1) * _pageSize;
                    end = _currPage * _pageSize > _result.Count ? _result.Count : _currPage * _pageSize;
                }
                HashSet<string> attrToExport = new HashSet<string>();
                if (checkBoxAttToExport.CheckState == CheckState.Checked)
                {
                    foreach (var item in _attrList)
                    {
                        attrToExport.Add(item);
                    }
                }
                else
                {
                    foreach (ListViewItem item in listViewAttrToExport.Items)
                    {
                        attrToExport.Add(item.SubItems[0].Text);
                    }
                }

                foreach (var item in attrToExport)
                {
                    sb.Append(item + ",");
                }
                sb.Append(Environment.NewLine);
                for (var i = start; i < end; i++)
                {
                    foreach (var item in attrToExport)
                    {
                        sb.Append("\"");
                        if (_result[i].NodeProperties.ContainsKey(item))
                        {
                            foreach (var val in _result[i].NodeProperties[item].Values)
                                sb.Append(val.StringValue + " ");
                        }
                        sb.Append("\"");
                        sb.Append(",");
                    }
                    sb.Append(Environment.NewLine);
                }
                if (MMCMiscUtil.SaveDataToFile(sb.ToString(), "Export Result", MMCUIConstants.CSV_FILTER))
                {
                    MMCDlgHelper.ShowInformation(VMDirConstants.STAT_RES_EXPO_SUCC);
                }
            });
        }

        private bool ValidateForm()
        {
            if (comboBoxFileFormat.SelectedItem == null)
            {
                if (comboBoxFileFormat.Items.Contains(comboBoxFileFormat.Text))
                {
                    comboBoxFileFormat.SelectedIndex = comboBoxFileFormat.Items.IndexOf(comboBoxFileFormat.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_FILE_FORMAT);
                    return false;
                }
            }
            if (comboBoxScope.SelectedItem == null)
            {
                if (comboBoxScope.Items.Contains(comboBoxScope.Text))
                {
                    comboBoxScope.SelectedIndex = comboBoxScope.Items.IndexOf(comboBoxScope.Text);
                }
                else
                {
                    MMCDlgHelper.ShowWarning(VMDirConstants.WRN_SCOPE);
                    return false;
                }
            }
            if (checkBoxAttToExport.CheckState == CheckState.Unchecked && listViewAttrToExport.Items.Count <= 0)
            {
                MMCDlgHelper.ShowWarning(VMDirConstants.WRN_ATTR);
                return false;
            }
            return true;
        }

        private void buttonAttrAdd_Click(object sender, EventArgs e)
        {
            var item = comboBoxAttrToReturn.SelectedItem;
            var lvi = new ListViewItem(new string[] { item.ToString() });
            listViewAttrToExport.Items.Add(lvi);
            this.comboBoxAttrToReturn.SelectedIndex = 0;
            this.comboBoxAttrToReturn.Items.Remove(item);
        }

        private void buttonAttrRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewAttrToExport.SelectedItems)
            {
                this.listViewAttrToExport.Items.Remove(item);
                this.comboBoxAttrToReturn.Items.Add(item);
            }
        }

        private void buttonAttrRemoveAll_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in this.listViewAttrToExport.Items)
            {
                this.comboBoxAttrToReturn.Items.Add(item);
            }
            this.listViewAttrToExport.Items.Clear();
        }
        void checkBoxAttToExport_CheckedChanged(object sender, System.EventArgs e)
        {
            if (this.checkBoxAttToExport.CheckState == CheckState.Checked)
            {
                comboBoxAttrToReturn.Enabled = false;
                listViewAttrToExport.Enabled = false;
                buttonAttrAdd.Enabled = false;
                buttonAttrRemove.Enabled = false;
                buttonAttrRemoveAll.Enabled = false;
            }
            else
            {
                comboBoxAttrToReturn.Enabled = true;
                listViewAttrToExport.Enabled = true;
                buttonAttrAdd.Enabled = true;
                buttonAttrRemove.Enabled = true;
                buttonAttrRemoveAll.Enabled = true;
            }
        }
    }
}
