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
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Presenters;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Views
{
    public partial class SuperLogging : Form
    {
        private readonly ServerDto _serverDto;
        private string _tenant;
        private bool _status;
        private bool _autoRefresh;
        private List<EventLogDto> _eventLogs;
        private List<FilterCriteriaDto> _filters;
        private SuperLoggingHelper _serviceHelper;
        private bool importEventLogs;
        private const int DefaultEventQueueSize = 500;
        private const string DateFormat = "dd-MMM-yy hh:mm:ss";
        private const string FileFilter = "Json files (*.json)|*.json|All files (*.*) |*.*";
        private const string TimestampFormat = "ddMMyyyyhhmmss";
        private const string ImportSwitchoffWarning = "Please turn off Auto Refresh and click on Import again.";
        private const string LoseEventLogsConfirm = "You will lose all the event logs entries. Do you wish to proceed?";
        private const string SelectFileTitle = "Select json eventlog file to import";
        private const string FailedToClearEventLogsError = "Failed to clear the logs";
        private const string FilterAppliedInfo = "Filter applied";
        private const string NoFilterAppliedInfo = "No filter applied";
        private const string ClearConfirmation = "This will clear all the event logs. Are you sure?";
        private const string ExportedConfirmation = "Events successfully exported to file {0}.";

        private enum SuperLoggingStatus
        {
            ON,
            OFF
        }

        public SuperLogging(ServerDto serverDto, string tenant)
        {
            InitializeComponent();
            _serverDto = serverDto;
            _tenant = tenant;
            _filters = new List<FilterCriteriaDto>();
            _serviceHelper = new SuperLoggingHelper();
            importEventLogs = false;
        }

        public SuperLogging()
        {
            InitializeComponent();
            _serverDto = null;
            _tenant = null;
            _filters = new List<FilterCriteriaDto>();
            _serviceHelper = new SuperLoggingHelper();
            importEventLogs = true;
        }

        private void btnOn_Click(object sender, EventArgs e)
        {
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenant);
            ActionHelper.Execute(delegate
            {
                if (!_status)
                {
                    int size = (int)txtEventsToCapture.Value;
                    var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                    var success = service.SuperLogging.Start(_serverDto, _tenant, auth.Token, size);
                }
                else
                {
                    var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                    var success = service.SuperLogging.Stop(_serverDto, _tenant, auth.Token);
                }
                _status = !_status;
                SetSuperLoggingStatus(_status);
                if (!_status)
                {
                    chkAutoRefresh.Checked = false;
                }
                RefreshView();
            }, auth);
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView1.Items.Count > 0 && listView1.SelectedItems.Count > 0)
            {
                var dto = (EventLogDto)listView1.SelectedItems[0].Tag;
                BindDetails(dto);                
            }
        }

        private void ClearSelectedDetails()
        {
            recordCount.Text = string.Empty;
            txtJsonRaw.Text = string.Empty;
            webBrowser.DocumentText = string.Empty;
            txtEventType.Text = string.Empty;
            txtCorrelationId.Text = string.Empty;
            txtStart.Text = string.Empty;
            txtDuration.Text = string.Empty;
            txtAccount.Text = string.Empty;
            txtProvider.Text = string.Empty;
            lblStatus.Text = string.Empty;
            lblLevel.Visible = false;
        }

        private void BindDetails(EventLogDto dto)
        {
            var output = new StringBuilder();
            if (dto.Metadata != null)
            {
                output.Append(JsonConvert.JsonSerialize(dto.Metadata));
            }
            txtJsonRaw.Text = output.ToString();

            var html = _serviceHelper.GetHtml(dto);
            webBrowser.DocumentText = html;

            var start = DateTimeHelper.UnixToWindowsMilliSecs(dto.Start);
            txtEventType.Text = dto.Type;
            txtCorrelationId.Text = dto.CorrelationId;
            txtStart.Text = start.ToString(DateFormat);
            txtDuration.Text = dto.ElapsedMillis.ToString();
            txtAccount.Text = dto.AccountName;
            txtProvider.Text = dto.ProviderName;
            lblStatus.Text = dto.Level.ToString();
            lblLevel.BackColor = GetColorByLevel(dto.Level);
            lblLevel.Visible = true;
        }
        private void SuperLogging_Load(object sender, EventArgs e)
        {
            _status = false;
            refreshInterval.SelectedIndex = 0;
            DoubleBuffered(listView1, true);
            recordCount.Text = string.Empty;
            ChangeFilterText();
            tabDetailsView.SelectedIndex = 1;
            rdoJsonView.Checked = false;
            rdoFriendlyView.Checked = true;
            ShowRawView(false);
            SetButtonStatus(false);
            pnlSuperLoggingStatus.Visible = !importEventLogs;
            pnlImportStatus.Visible = importEventLogs;
            pnlRefresh.Visible = !importEventLogs;
            lblFileName.Visible = false;
            lblFile.Visible = false;
            if (!importEventLogs)
            {   
                SetSuperLoggingStatus(_status);
                GetSuperLoggingStatus();
            }
        }
        private void SetSuperLoggingStatus(bool status)
        {
            txtEventsToCapture.Enabled = !status;
            btnStatus.Text = status ? SuperLoggingStatus.OFF.ToString() : SuperLoggingStatus.ON.ToString();
            var statusDescription = status ? SuperLoggingStatus.ON.ToString() : SuperLoggingStatus.OFF.ToString();
            lblStatusMessage.Text = GetStatusMessage(statusDescription);
        }

        private string GetStatusMessage(string statusDescription)
        {
            return string.Format("Super Logging is turned {0}. Click {1} to turn it {2}.", statusDescription, btnStatus.Text, btnStatus.Text.ToLower());
        }

        private void autoRefresh_Tick(object sender, EventArgs e)
        {
            RefreshView();
        }
        private void GetSuperLoggingStatus()
        {
            importEventLogs = false;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenant);
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                var eventLogStatus = service.SuperLogging.GetStatus(_serverDto, _tenant, auth.Token);
                txtEventsToCapture.Value = (decimal)eventLogStatus.Size;
                _status = eventLogStatus.Enabled;
                if (_status)
                {
                    RefreshView();
                }
            }, auth);
            SetSuperLoggingStatus(_status);
        }

        private void RefreshView()  
        {
            importEventLogs = false;
            lblFileName.Visible = false;
            lblFile.Visible = false;
            lblFileName.Text = string.Empty;
            var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenant);
            ActionHelper.Execute(delegate
            {
                var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                _eventLogs = service.SuperLogging.GetEventLogs(_serverDto, _tenant, auth.Token);
                _eventLogs = _eventLogs.OrderByDescending(x => x.Start).ToList();
                var filteredEventLogs = _serviceHelper.ApplyFilter(_eventLogs, _filters);
                BindControls(filteredEventLogs);
                if (filteredEventLogs.Count == 0)
                    ClearSelectedDetails();
            }, auth);
        }

        private void BindControls(List<EventLogDto> eventLogs)
        {
            var selIndices = listView1.SelectedIndices;
            var selectedIndex = selIndices.Count > 0 ? selIndices[0] : 0;
            
            listView1.Items.Clear();
            var hasEvents = false;
            if (eventLogs != null)
            {
                foreach (var eventLog in eventLogs)
                {
                    var start = DateTimeHelper.UnixToWindowsMilliSecs(eventLog.Start);
                    var item = new ListViewItem(new[] 
                    {
                        eventLog.Level.ToString(),
                        start.ToString(DateFormat), 
                        eventLog.Type, 
                        eventLog.CorrelationId, 
                        eventLog.AccountName, 
                        eventLog.ProviderName,
                        eventLog.ElapsedMillis.ToString() 
                    })
                    {
                        Tag = eventLog
                    };
                    item.BackColor = GetColorByLevel(eventLog.Level);
                    listView1.Items.Add(item);
                }

                if (listView1.Items.Count > 0)
                {
                    if (selectedIndex < listView1.Items.Count)
                    {
                        listView1.Items[selectedIndex].Selected = true;
                        listView1.EnsureVisible(selectedIndex);
                    }
                    else
                        listView1.Items[0].Selected = true;
                }
                hasEvents = eventLogs.Count > 0;
            }
            lastRefreshTime.Text = DateTime.Now.ToString(DateFormat);
            recordCount.Text = string.Format("Number of events: {0}", eventLogs.Count);
            SetButtonStatus(hasEvents);
        }

        private void btnJsonView_CheckedChanged(object sender, EventArgs e)
        {
            ShowRawView(true); 
        }

        private void rdoFriendlyView_CheckedChanged(object sender, EventArgs e)
        {
            ShowRawView(false);
        }

        private void ShowRawView(bool raw)
        {
            pnlFriendlyView.Visible = !raw;
            pnlJsonView.Visible = raw;
        }

        private void chkAutoRefresh_CheckedChanged(object sender, EventArgs e)
        {
            _autoRefresh = chkAutoRefresh.Checked;
            SetAutoRefreshSetting(_autoRefresh);
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            RefreshView();
        }

        private void SetAutoRefreshSetting(bool shouldAutoRefresh)
        {
            if (shouldAutoRefresh)
            {
                var interval = int.Parse(refreshInterval.SelectedItem.ToString());
                autoRefresh.Interval =  interval * 1000;
                autoRefresh.Start();
            }
            else
            {
                autoRefresh.Stop();
            }            
        }

        private void SetButtonStatus(bool hasEvents)
        {
            btnClear.Enabled = hasEvents;
            btnExport.Enabled = hasEvents;
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            var confirm = ErrorMessageDisplayHelper.ConfirmMessage(this, ClearConfirmation);
            if (confirm)
            {
                if (!importEventLogs)
                {
                    var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(_serverDto, _tenant);
                    ActionHelper.Execute(delegate
                    {
                        var service = ScopeNodeExtensions.GetServiceGateway(_serverDto.ServerName);
                        var success = service.SuperLogging.Delete(_serverDto, _tenant, auth.Token);
                        if (success)
                        {
                            RefreshView();
                        }
                        else
                        {
                            ErrorMessageDisplayHelper.ShowError(FailedToClearEventLogsError);
                        }
                    }, auth);
                }
                else
                {
                    _eventLogs.Clear();
                    BindControls(_eventLogs);
                }
                ClearSelectedDetails();
            }
        }

        private void btnExport_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                var filename = GetExportFileName();
                var exportDialog = new SaveFileDialog
                {
                    FileName = filename,
                    Filter = FileFilter,
                };

                if (exportDialog.ShowDialog() == DialogResult.OK)
                {
                    var output = new StringBuilder();
                    var eventLogs = new ArrayList();
                    foreach (ListViewItem item in listView1.Items)
                    {
                        var dto = (EventLogDto)item.Tag;
                        var obj = new {
                            type = dto.Type,
                            correlationId = dto.CorrelationId,
                            level = dto.Level.ToString(),
                            start = dto.Start,
                            elapsedMillis = dto.ElapsedMillis,
                            metadata = dto.Metadata
                        };
                        eventLogs.Add(obj);
                    }
                    string json = _serviceHelper.GetJsonText(output, eventLogs);
                    File.WriteAllText(exportDialog.FileName, json);
                    var fName = new FileInfo(exportDialog.FileName).Name;
                    var message = string.Format(ExportedConfirmation, fName);
                    ErrorMessageDisplayHelper.ShowInfo(message);
                }

            }, null);
        }

        private string GetExportFileName()
        {
            var timestamp = DateTime.Now.ToString(TimestampFormat);
            var filename = string.Format("eventlog_{0}.json", timestamp);
            return filename;
        }

        private void btnImport_Click(object sender, EventArgs e)
        {
            if (_autoRefresh)
            {
                ErrorMessageDisplayHelper.ShowError(ImportSwitchoffWarning);
                return;
            }

            var proceed = true;
            if (listView1.Items.Count > 0)
            {
                proceed = ErrorMessageDisplayHelper.ConfirmMessage(this, LoseEventLogsConfirm);
            }

            if (proceed)
            {
                ActionHelper.Execute(delegate
                {
                    var timestamp = DateTime.Now.ToString(TimestampFormat);
                    var openDialog = new OpenFileDialog
                    {
                        Filter = FileFilter,
                        Title = SelectFileTitle,
                        CheckFileExists = true,
                        CheckPathExists = true,
                        Multiselect = false
                    };

                    if (openDialog.ShowDialog() == DialogResult.OK)
                    {
                        lblFileName.Visible = true;
                        lblFile.Visible = true;
                        lblFileName.Text = new FileInfo(openDialog.FileName).Name;
                        var json = File.ReadAllText(openDialog.FileName);
                        _eventLogs = JsonConvert.JsonDeserialize<List<EventLogDto>>(json);
                        var filteredEventLogs = _serviceHelper.ApplyFilter(_eventLogs, _filters);
                        BindControls(filteredEventLogs);
                    }

                }, null);
                importEventLogs = true;
            }
        }

        private void btnAddFilter_Click(object sender, EventArgs e)
        {
            ActionHelper.Execute(delegate
            {
                if (_filters == null)
                    _filters = new List<FilterCriteriaDto>();
                var frm = new SuperLoggingFilter()
                {
                    Filters = _filters
                };
                frm.ShowDialog(this);
                _filters = frm.Filters;

                ChangeFilterText();
                var eventLogs = _serviceHelper.ApplyFilter(_eventLogs, _filters);
                BindControls(eventLogs);

                if (eventLogs.Count == 0)
                    ClearSelectedDetails();
            }, null);
        }

        private void ChangeFilterText()
        {
            var filter = _filters.Count > 0;
            lblFilter.Text = filter ? FilterAppliedInfo : NoFilterAppliedInfo;
            lblFilter.ForeColor = filter ? Color.Blue : Color.DarkGray;
        }
       
        private Color GetColorByLevel(EventLevel level)
        {
            var color = Color.Lavender;

            if(level == EventLevel.FATAL)
            {
                color = Color.Red;
            }
            else if (level == EventLevel.ERROR)
            {
                color = Color.Pink;
            }
            else if (level == EventLevel.WARN)
            {
                color = Color.LightYellow;
            }
            else if (level == EventLevel.INFO)
            {
                color = Color.LightGreen;
            }
            else if (level == EventLevel.DEBUG)
            {
                color = Color.LightBlue;
            }
            else if (level == EventLevel.TRACE)
            {
                color = Color.LightSteelBlue;
            }
            return color;
        }

        private void SuperLogging_Resize(object sender, EventArgs e)
        {            
            var sparewidth = listView1.Width - 249;            
            listView1.Columns[0].Width = 50;
            listView1.Columns[1].Width = 120;
            listView1.Columns[2].Width = (int) (0.17 * sparewidth);
            listView1.Columns[3].Width = (int) (0.30 * sparewidth);
            listView1.Columns[4].Width = (int) (0.25 * sparewidth);
            listView1.Columns[5].Width = (int) (0.24 * sparewidth);
            listView1.Columns[6].Width = 80;
        }

        public void DoubleBuffered(Control control, bool enable)
        {
            var doubleBufferPropertyInfo = control.GetType().GetProperty("DoubleBuffered", BindingFlags.Instance | BindingFlags.NonPublic);
            doubleBufferPropertyInfo.SetValue(control, enable, null);
        }

        private void refreshInterval_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
