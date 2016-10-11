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
using System.Windows.Forms;
using VmDirInterop.SuperLogging;
using VmDirInterop.SuperLogging.Interfaces;
using VMDir.Common.DTO;
using VMDirSnapIn.Utilities;

namespace VMDirSnapIn.UI
{
    public enum FilterColumn
    {
        None=-1,
        ClientIP,
        ClientPort,
        LoginDN,
        Operation,
        ErrorCode,
        Duration
    }

    public enum NumericFilterOperation
    {
        None=-1,
        GTE,
        LTE,
        EQ
    }

    public enum StringFilterOperation
    {
        None=-1,
        Equals,
        BeginsWith,
        EndsWith,
        Contains
    }

    public partial class SuperLogBrowser : Form
    {
        VMDirServerDTO _serverDTO;
        bool _enabled = false;
        ISuperLoggingCookie _cookie = null;
        Dictionary<int, ISuperLogEntry> _viewCache = new Dictionary<int, ISuperLogEntry>();
        const int FETCH_WINDOW_SIZE = 25;
        const int INITIAL_LIST_SIZE = 25;
        SuperLogFilterHelper _filterHelper = new SuperLogFilterHelper();
        public Dictionary<int, ISuperLogEntry> ViewCache
        {
            get
            {
                if (_filterHelper.IsEnabled())
                    return _filterHelper.ViewCache;
                else
                    return _viewCache;
            }
        }

        ISuperLoggingConnection SuperLog
        {
            get
            {
                return _serverDTO.Connection.GetSuperLoggingConnection();
            }
        }

        public SuperLogBrowser(VMDirServerDTO dto)
        {
            _serverDTO = dto;
            InitializeComponent();

            InitUI();
        }

        private void InitUI()
        {
            UpdateStatus();
            RefreshList();
        }

        private void RefreshList()
        {
            _viewCache.Clear();
            _filterHelper.ViewCache.Clear();
            lvLogInfo.VirtualListSize = 0;
            if (_enabled)
            {
                _cookie = new SuperLoggingCookie();
                FillCache(0);
            }
        }

        private void UpdateStatus()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                _enabled = SuperLog.isEnabled();
                btnSuperLogOnOff.Text = _enabled ?
                    "Turn superlogging off" : "Turn superlogging on";
                if (_enabled)
                {
                    uint nCapacity = SuperLog.getCapacity();
                    lblSuperLogStatus.Text = string.Format(
                        "Superlogging is on with a buffer size of {0} entries",
                        SuperLog.getCapacity());
                    txtBufferSize.Value = nCapacity;
                }
                else
                {
                    lblSuperLogStatus.Text = "Superlogging is turned off. Click the button to turn it on";
                }
                btnChangeBufferSize.Enabled = txtBufferSize.Enabled = _enabled;
            });
        }

        private void btnSuperLogOnOff_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
           {
               if (SuperLog.isEnabled())
                   SuperLog.disable();
               else
                   SuperLog.enable();
               UpdateStatus();
           });
        }

        private void btnClearEntries_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                if (MessageBox.Show(
                    "This will clear all the superlog entries at the server. Continue?",
                    "Clear Entries?", MessageBoxButtons.YesNo)
                    == DialogResult.Yes)
                {
                    SuperLog.clear();
                    RefreshList();
                }
            });
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            RefreshList();
        }

        void FillCache(int itemIndex)
        {
            if (_viewCache.ContainsKey(itemIndex))
                return;

            MiscUtilsService.CheckedExec(delegate
            {
                var list = SuperLog.getPagedEntries(_cookie, FETCH_WINDOW_SIZE);
                if (list != null)
                {
                    int i = 0;
                    int count = list.getCount();
                    foreach (var dto in list.getEntries())
                    {
                        _viewCache[itemIndex + i++] = dto;
                    }

                    if (_filterHelper.IsEnabled())
                    {
                        _filterHelper.Filter(_viewCache);
                        lvLogInfo.VirtualListSize = ViewCache.Count;
                    }
                    else
                    {
                        if (count < FETCH_WINDOW_SIZE)
                            lvLogInfo.VirtualListSize = itemIndex + count;
                        else
                            lvLogInfo.VirtualListSize = itemIndex + count + FETCH_WINDOW_SIZE;
                    }
                }
            });
        }

        private void lvLogInfo_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
           {
               if (_cookie != null)
               {
                   FillCache(e.ItemIndex);

                   if (ViewCache.ContainsKey(e.ItemIndex))
                   {
                       var dto = ViewCache[e.ItemIndex];
                       e.Item = new ListViewItem(dto.getClientIP());
                       e.Item.SubItems.Add(dto.getClientPort().ToString());
                       e.Item.SubItems.Add(dto.getLoginDN());
                       e.Item.SubItems.Add(dto.getOperation());
                       e.Item.SubItems.Add(dto.getErrorCode().ToString());

                       var span = dto.getEndTime() - dto.getStartTime();
                       e.Item.SubItems.Add(string.Format("{0} ms", span));
                   }
               }
           });
        }

        private void btnChangeBufferSize_Click(object sender, EventArgs e)
        {
            MiscUtilsService.CheckedExec(delegate
            {
                var capacity = txtBufferSize.Value;
                if (MessageBox.Show(
                    string.Format("Set superlog buffer size to {0}?", capacity),
                    "Change buffer size?", MessageBoxButtons.YesNo)
                    == DialogResult.Yes)
                {
                    SuperLog.setCapacity(Convert.ToUInt32(capacity));
                    UpdateStatus();
                    RefreshList();
                }
            });
        }

        private void chkAutoRefresh_CheckedChanged(object sender, EventArgs e)
        {
            ChangeAutoRefreshSettings();
        }

        private void ChangeAutoRefreshSettings()
        {
            bool autoRefresh = chkAutoRefresh.Checked;
            if (autoRefresh)
            {
                timerAutoRefresh.Interval = (int)txtAutoRefresh.Value * 1000;
                timerAutoRefresh.Enabled = true;
            }
            else
                timerAutoRefresh.Enabled = false;
        }

        private void timerAutoRefresh_Tick(object sender, EventArgs e)
        {
            timerAutoRefresh.Enabled = false;
            RefreshList();
            timerAutoRefresh.Enabled = true;
        }

        private void txtAutoRefresh_ValueChanged(object sender, EventArgs e)
        {
            ChangeAutoRefreshSettings();
        }

        private void btnFilter_Click(object sender, EventArgs e)
        {
            ApplyFilter();
        }

        void ApplyFilter()
        {
            MiscUtilsService.CheckedExec(delegate
            {
                _filterHelper.FilterColumn = (FilterColumn)cbFilterColumn.SelectedIndex;
                _filterHelper.SetFilterOperation(cbFilterCriteria.SelectedIndex);
                _filterHelper.FilterText = txtFilter.Text;

                if (_filterHelper.Filter(_viewCache))
                {
                    lvLogInfo.VirtualListSize = _filterHelper.ViewCache.Count;
                }
                lvLogInfo.Invalidate();
            });
        }

        private void cbFilterColumn_SelectedIndexChanged(object sender, EventArgs e)
        {
            cbFilterCriteria.SelectedIndex = -1;
            cbFilterCriteria.Items.Clear();

            if (cbFilterColumn.Text == "Duration")
            {
                cbFilterCriteria.Items.Add(">=");
                cbFilterCriteria.Items.Add("<=");
                cbFilterCriteria.Items.Add("==");
            }
            else
            {
                cbFilterCriteria.Items.Add("equals");
                cbFilterCriteria.Items.Add("begins with");
                cbFilterCriteria.Items.Add("ends with");
                cbFilterCriteria.Items.Add("contains");
            }
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            cbFilterColumn.SelectedIndex = -1;
            cbFilterCriteria.SelectedIndex = -1;
            txtFilter.Text = "";
            ApplyFilter();
            RefreshList();
        }

        private void lvLogInfo_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var indices = lvLogInfo.SelectedIndices;
            if (indices == null || indices.Count == 0)
                return;
            var entry = _viewCache[indices[0]];
            if(entry != null)
                MessageBox.Show(entry.ToString());
        }
    }
}
