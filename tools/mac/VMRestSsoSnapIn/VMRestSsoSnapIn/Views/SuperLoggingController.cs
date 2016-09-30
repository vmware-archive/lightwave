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
using System.Threading;
using System.Text;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace RestSsoAdminSnapIn
{
	public partial class SuperLoggingController : NSWindowController
	{
		public ServerDto ServerDto{ get; set; }
		public string TenantName{ get; set; }
		private List<EventLogDto> _eventLogs;
		public SuperLoggingHelper ServiceHelper;
		private List<FilterCriteriaDto> _filters;
		private bool _status = false;
		private bool _autoRefresh = false;
		private bool _importEventLogs = false;
		private Timer _autoRefreshTimer;
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
		private const string StatusColumnId = "Status";
		private const int MillisecsMultiplier = 1000;

		private enum SuperLoggingStatus
		{
			ON,
			OFF
		}

		public SuperLoggingController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public SuperLoggingController (NSCoder coder) : base (coder)
		{
		}

		public SuperLoggingController () : base ("SuperLogging")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			// Events
			this.CloseButton.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			this.ClearButton.Activated += ClearButton_Activated;
			this.StatusButton.Activated += StatusButton_Activated;
			this.JsonButton.Activated += JsonButton_Activated;
			this.FriendlyButton.Activated += FriendlyButton_Activated;
			this.AutoRefreshButton.Activated += AutoRefreshButton_Activated;
			this.ImportButton.Activated += ImportButton_Activated;
			this.ExportButton.Activated += ExportButton_Activated;
			this.RefreshButton.Activated += RefreshButton_Activated;
			this.FilterButton.Activated += FilterButton_Activated;

			EventsLogTableView.Delegate = new TableDelegate(this);

			_autoRefreshTimer = new Timer (AutoRefreshTimer_Tick, null, -1, -1);
			_filters = new List<FilterCriteriaDto> ();
			ServiceHelper = new SuperLoggingHelper ();
			AutoRefreshButton.StringValue = "0";
			SetAutoRefreshSetting (false);
			GetSuperLoggingStatus ();
			SetSuperLoggingStatus (_status);
			SetFilterText();
			ShowRawView (false);
		}

		private void GetSuperLoggingStatus()
		{
			_importEventLogs = false;
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
			ActionHelper.Execute(delegate
				{
					var eventLogStatus = SnapInContext.Instance.ServiceGateway.SuperLogging.GetStatus(ServerDto, TenantName, auth.Token);
					EventsCountTextField.StringValue = eventLogStatus.Size.ToString();
					_status = eventLogStatus.Enabled;
					Refresh();
				});
			SetSuperLoggingStatus(_status);
		}

		void RefreshButton_Activated (object sender, EventArgs e)
		{
			Refresh ();
		}

		void SetFilterText()
		{
			var isFiltered = _filters.Count > 0;
			FilterStatusTextField.StringValue =
				isFiltered
				? FilterAppliedInfo
				: NoFilterAppliedInfo;
			FilterStatusTextField.TextColor = isFiltered
				? NSColor.Blue
				: NSColor.Gray;
		}

		void FilterButton_Activated (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate {
				var form = new SuperLoggingFilterController (){ Filters = _filters };
				Window.BeginSheet (form.Window, _=> {});
				try {
					nint result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
					_filters = form.Filters;
				} finally {
					Window.EndSheet (form.Window);
					form.Close ();
					form.Dispose ();
				}
				SetFilterText();
				_eventLogs = _eventLogs.OrderByDescending(x=>x.Start).ToList();
				var filteredLogs = ServiceHelper.ApplyFilter(_eventLogs, _filters);
				BindControls(filteredLogs);

				if(filteredLogs.Count == 0)
				{
					ClearSelectedDetails();
				}
			});
		}

		void ExportButton_Activated (object sender, EventArgs e)
		{
			ActionHelper.Execute(delegate
				{
					var filename = GetExportFileName();
					var exportDialog = new NSSavePanel()
					{
						AllowedFileTypes = new []{"json"},
						Title = SelectFileTitle,
						NameFieldStringValue = filename
					};

					var result = exportDialog.RunModal();
					if (result == 1)
					{
						var output = new StringBuilder();
						var ds = EventsLogTableView.DataSource as SuperLogDataSource;
						if(ds != null)
						{
							var eLogs = ds.Entries;
							var eventLogs = new ArrayList();
							foreach (var dto in ds.Entries)
							{
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
							string json = ServiceHelper.GetJsonText(output, eventLogs);
							var filepath = exportDialog.Url.AbsoluteString.Replace("file://",string.Empty);
							File.WriteAllText(filepath, json);
						}
					}

				});
		}

		private string GetExportFileName()
		{
			var timestamp = DateTime.Now.ToString(TimestampFormat);
			var filename = string.Format("eventlog_{0}.json", timestamp);
			return filename;
		}

		void ImportButton_Activated (object sender, EventArgs e)
		{
			if (_autoRefresh)
			{
				UIErrorHelper.ShowAlert(null, ImportSwitchoffWarning);
				return;
			}

			var proceed = true;
			var ds = EventsLogTableView.DataSource as SuperLogDataSource;
			if(ds != null && ds.Entries.Count > 0)
			{
				proceed = UIErrorHelper.ShowConfirm(LoseEventLogsConfirm, "Confirm");
			}

			if (proceed)
			{
				ActionHelper.Execute(delegate
					{
						var timestamp = DateTime.Now.ToString(TimestampFormat);
						var openDialog = new NSOpenPanel
						{
							AllowedFileTypes = new string[] {"json"},
							Title = SelectFileTitle,
							AllowsMultipleSelection = false
						};

						var result = openDialog.RunModal();
						if (result == 1)
						{
							var filepath = openDialog.Url.AbsoluteString.Replace("file://",string.Empty);
							var json = File.ReadAllText(filepath);
							_eventLogs = JsonConvert.JsonDeserialize<List<EventLogDto>>(json);
							var filteredEventLogs = ServiceHelper.ApplyFilter(_eventLogs, _filters);
							BindControls(filteredEventLogs);
						}

					});
				_importEventLogs = true;
			}
		}

		void AutoRefreshButton_Activated (object sender, EventArgs e)
		{
			_autoRefresh = !_autoRefresh;
			SetAutoRefreshSetting(_autoRefresh);
		}

		void FriendlyButton_Activated (object sender, EventArgs e)
		{
			ShowRawView(false);
		}

		void JsonButton_Activated (object sender, EventArgs e)
		{
			ShowRawView(true);
		}

		private void ShowRawView(bool raw)
		{
			JsonScrollView.Hidden = !raw;
			FriendlyScrollView.Hidden = raw;
		}

		void AutoRefreshTimer_Tick (object state)
		{
			InvokeOnMainThread(() => { Refresh (); } );
		}

		void StatusButton_Activated (object sender, EventArgs e)
		{
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
			ActionHelper.Execute (delegate() {
				if (!_status)
				{
					int size = DefaultEventQueueSize;
					size = int.TryParse(EventsCountTextField.StringValue, out size) ? size : DefaultEventQueueSize;
					var success = SnapInContext.Instance.ServiceGateway.SuperLogging.Start(ServerDto, TenantName, auth.Token, size);
				}
				else
				{
					var success = SnapInContext.Instance.ServiceGateway.SuperLogging.Stop(ServerDto, TenantName, auth.Token);
				}
				_status = !_status;
				SetSuperLoggingStatus(_status);
				if (!_status)
				{
					_autoRefresh = false;
					SetAutoRefreshSetting(_autoRefresh);
				}
				Refresh();
				_importEventLogs = false;
			});
		}

		private void SetAutoRefreshSetting(bool shouldAutoRefresh)
		{
			if (shouldAutoRefresh)
			{
				var refreshInterval = 0;
				if (int.TryParse(AutoRefreshIntervalComboBox.StringValue, out refreshInterval))
				{
					_autoRefreshTimer.Change (0, refreshInterval * MillisecsMultiplier);
				}
			}
			else
			{
				_autoRefreshTimer.Change (-1, -1);
			}
			AutoRefreshButton.StringValue = shouldAutoRefresh ? "1" : "0";
		}

		void ClearButton_Activated (object sender, EventArgs e)
		{
			var isConfirm = UIErrorHelper.ShowConfirm(ClearConfirmation, "Confirm");
			if (isConfirm) {
				if (!_importEventLogs) {
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);

					ActionHelper.Execute (delegate() {

						var success = SnapInContext.Instance.ServiceGateway.SuperLogging.Delete (ServerDto, TenantName, auth.Token);
						if (success) {
							Refresh ();
						} else {
							UIErrorHelper.ShowAlert (null, FailedToClearEventLogsError);
						}
					});
				} else {
					_eventLogs.Clear ();
					BindControls (_eventLogs);
				}
				ClearSelectedDetails ();
			}
		}

		public new SuperLogging Window {
			get { return (SuperLogging)base.Window; }
		}

		private void SetSuperLoggingStatus(bool status)
		{
			EventsCountTextField.Enabled = !status;
			var statusStr = status ? SuperLoggingStatus.OFF.ToString() : SuperLoggingStatus.ON.ToString();
			StatusButton.Title = statusStr;
			var statusDesc = status ? SuperLoggingStatus.ON.ToString() : SuperLoggingStatus.OFF.ToString();
			StatusBannerTextField.StringValue = GetStatusMessage (statusDesc, statusStr);
		}

		private string GetStatusMessage(string desc, string status)
		{
			return string.Format ("Super Logging is turned {0}. Click {1} to turn it {2}.", desc, status, status.ToLower());
		}

		void SetEventCounts (List<EventLogDto> filteredLogs)
		{
			EventCountTextField.StringValue = string.Format ("Events: {0}", filteredLogs.Count);
		}

		private void Refresh()
		{
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);

			ActionHelper.Execute (delegate() {

				_eventLogs = SnapInContext.Instance.ServiceGateway.SuperLogging.GetEventLogs(ServerDto, TenantName,auth.Token);
				_eventLogs = _eventLogs.OrderByDescending(x=>x.Start).ToList();
				var filteredLogs = ServiceHelper.ApplyFilter(_eventLogs, _filters);
				BindControls(filteredLogs);

				if(filteredLogs.Count == 0)
				{
					ClearSelectedDetails();
				}
				var hasEvents = filteredLogs!= null && filteredLogs.Count > 0;
				SetButtonStatus (hasEvents);
				LastUpdatedTimestamp.StringValue = DateTime.Now.ToString(DateFormat);
			});
		}

		private void BindControls(List<EventLogDto> eventLogs)
		{
			var rowId = (int)EventsLogTableView.SelectedRow;

			var listView = new SuperLogDataSource { Entries = eventLogs};
			EventsLogTableView.DataSource = listView;
			EventsLogTableView.ReloadData ();

			if (rowId > -1 && eventLogs.Count > rowId) {
				EventsLogTableView.SelectRow (rowId, false);
			} else {

				if (eventLogs.Count > 0) {
					EventsLogTableView.SelectRow (0, false);
				}
			}
			var td = (EventsLogTableView.Delegate as TableDelegate);
			if(td != null)
				td.OnSelectionChange ();
			var hasEvents = eventLogs.Count > 0;
			SetButtonStatus (hasEvents);
			SetEventCounts (eventLogs);
		}

		private void SetButtonStatus(bool hasEvents)
		{
			ClearButton.Enabled = hasEvents;
			ExportButton.Enabled = hasEvents;
		}

		private void ClearSelectedDetails()
		{
			StatusTextField.StringValue = (NSString)string.Empty;
			StartTextField.StringValue = (NSString)string.Empty;
			EventTypeTextField.StringValue = (NSString)string.Empty;
			CorrelationIdTextField.StringValue = (NSString)string.Empty;
			AccountTextField.StringValue = (NSString)string.Empty;
			ProviderTextField.StringValue = (NSString)string.Empty;
			DurationTextField.StringValue = (NSString)string.Empty;
			JsonTextField.Value = (NSString)string.Empty;
			FriendlyTextView.Value = (NSString)string.Empty;
		}

		public class TableDelegate : NSTableViewDelegate
		{
			private SuperLoggingController _controller;
			private Dictionary<EventLevel,NSColor> colorMap;
			public TableDelegate (SuperLoggingController controller)
			{
				_controller = controller;
				colorMap = GetColorMap();
			}

			private Dictionary<EventLevel,NSColor> GetColorMap()
			{
				var map = new Dictionary<EventLevel,NSColor> ();
				map.Add (EventLevel.ALL, NSColor.Cyan);
				map.Add (EventLevel.FATAL, NSColor.Red);
				map.Add (EventLevel.ERROR, NSColor.Orange);
				map.Add (EventLevel.WARN, NSColor.DarkGray);
				map.Add (EventLevel.INFO, NSColor.Purple);
				map.Add (EventLevel.DEBUG, NSColor.Blue);
				map.Add (EventLevel.TRACE, NSColor.Brown);
				return map;
			}

			private NSColor GetOutcomeColor (EventLevel level)
			{
				NSColor color;
				return colorMap.TryGetValue(level, out color) ? color : NSColor.Cyan;
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					NSTextFieldCell textCell = cell as NSTextFieldCell;
					if (textCell != null) {
						var collection = ((SuperLogDataSource)(_controller.EventsLogTableView.DataSource)).Entries;
						if (collection != null) {
							var item = collection [(int)row];
							if (tableColumn.Identifier == StatusColumnId) {
								textCell.TextColor = GetOutcomeColor (item.Level);
							}
						}
					}
				});
			}

			public void OnSelectionChange ()
			{
				ActionHelper.Execute (delegate {
					var row = (int)_controller.EventsLogTableView.SelectedRow;
					var collection = ((SuperLogDataSource)(_controller.EventsLogTableView.DataSource)).Entries;
					if (row >= 0 && collection != null) {
						var item = collection [row];
						_controller.EventTypeTextField.StringValue = item.Type;
						_controller.CorrelationIdTextField.StringValue = item.CorrelationId;
						_controller.ProviderTextField.StringValue = item.ProviderName;
						_controller.AccountTextField.StringValue = item.AccountName;
						_controller.StartTextField.StringValue = DateTimeHelper.UnixToWindowsMilliSecs (item.Start).ToString (DateFormat);
						_controller.DurationTextField.StringValue = item.ElapsedMillis.ToString ();
						_controller.StatusTextField.StringValue = item.Level.ToString();
						_controller.LevelIndicatorTextField.BackgroundColor = GetOutcomeColor (item.Level);
						_controller.JsonTextField.Value = JsonConvert.JsonSerialize (item.Metadata);
						_controller.FriendlyTextView.Value = _controller.ServiceHelper.GetText(item);
					}
				});
			}

			public override void SelectionDidChange (NSNotification notification)
			{
				OnSelectionChange ();
			}
		}
	}
}
