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
using System.Threading;
using Foundation;
using AppKit;
using System.Text;
using VMDir.Common.DTO;
using VmDirInterop.SuperLogging;
using VmDirInterop.SuperLogging.Interfaces;
using VmIdentity.UI.Common.Utilities;
using VmIdentity.UI.Common;
using VMDirSnapIn.DataSource;
using VMDir.Common;
using System.Linq;
using VMDirInterop.LDAP;

namespace VMDirSnapIn.UI
{
	public partial class SuperLoggingBrowserWindowController : NSWindowController
	{
		private VMDirServerDTO _serverDTO;
		private bool _enabled = false;
		private ISuperLoggingCookie _cookie = null;
		private Dictionary<int, ISuperLogEntry> _viewCache = new Dictionary<int, ISuperLogEntry>();
		private const int FETCH_WINDOW_SIZE = 25;
		private const int INITIAL_LIST_SIZE = 25;
		private const int INIT_BUFFER_SIZE = 10000;
		private const string INIT_REFRESH_INTERVAL = "2";
		private Timer timer;
		private TimerState timerState;
		private int pageSize;

		private ISuperLoggingConnection SuperLog
		{
			get
			{
				return _serverDTO.Connection.GetSuperLoggingConnection();
			}
		}

		public SuperLoggingBrowserWindowController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public SuperLoggingBrowserWindowController (NSCoder coder) : base (coder)
		{
		}

		public SuperLoggingBrowserWindowController (VMDirServerDTO serverDTO) : base ("SuperLoggingBrowserWindow")
		{
			_serverDTO = serverDTO;
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			InitUI();
			BtnRefresh.Activated += btnRefresh_Click;
			BtnOff.Activated += btnSuperLogOnOff_Click;
			BtnFilter.Activated += btnFilter_Click;
			BtnClear.Activated += btnClear_Click;
			BtnBufferSizeChange.Activated += btnChangeBufferSize_Click;
			ChkAutoRefresh.Activated += chkAutoRefresh_CheckedChanged;
			CboColumns.Activated += CboColumns_Changed;
			CbOperator.Activated += (object sender, EventArgs e) => 
			{
				EnableDisableFilter();
			};
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			TxtFilterValue.Changed += (object sender, EventArgs e) => {
				EnableDisableFilter();
			};
			SuperLogsTableView.DoubleClick += SuperLog_DoubleClick;
			SuperLogsTableView.Delegate = new TableDelegate (this);
			ChangeAutoRefreshSettings ();
			EnableDisableFilter();
		}

		void SuperLog_DoubleClick(object sender, EventArgs e)
		{
			var row = (int)SuperLogsTableView.SelectedRow;
			if(row > -1 && _viewCache.ContainsKey(row))
			{
				var item = _viewCache [row];

				if (item is SuperLogSearchEntry) {
					var text = (item as SuperLogSearchEntry).ToString ();
					UIHelper.ShowGenericWindowAsSheet (text, "Search Details", this.Window);
				}
			}
		}

		void CboColumns_Changed (object sender, EventArgs e)
		{
			var row = (int)CboColumns.SelectedIndex;
			if (row >= 0) {
				if (row == 0) {
					CbOperator.SelectItem (0);
					TxtFilterValue.StringValue = string.Empty;
					CbOperator.Enabled = false;
					TxtFilterValue.Enabled = false;
				} else {
					CbOperator.Enabled = true;
					TxtFilterValue.Enabled = true;
				}
			}
			EnableDisableFilter();
		}

		private void InitUI()
		{
			timerState = new TimerState ();
			timer = new Timer (timerAutoRefresh_Tick, timerState, -1, -1);
			timerState.timer = timer;
			SetDefaults ();
			UpdateStatus();
		}

		private void SetDefaults()
		{
			TxtBufferSize.IntValue = INIT_BUFFER_SIZE;
			TxtRefreshInterval.StringValue = INIT_REFRESH_INTERVAL;
			CboColumns.SelectItem (0);
			CbOperator.Enabled = false;
			TxtFilterValue.Enabled = false;
		}
		private void RefreshList(int startindex = 0)
		{
			UIErrorHelper.CheckedExec (delegate {
				if(startindex == 0)
				{
					_viewCache.Clear ();
					this.SuperLogsTableView.DataSource = new SuperLoggingTableViewDataSource (null);
					this.SuperLogsTableView.ReloadData ();
				}
				if (_enabled) {
					if (_cookie == null)
						_cookie = new SuperLoggingCookie ();
					FillCache (startindex, FETCH_WINDOW_SIZE);
				}
			});
		}

		private void UpdateStatus()
		{
			try {
				_enabled = SuperLog.isEnabled ();
				if (_enabled) {
					uint nCapacity = SuperLog.getCapacity ();
					Status.StringValue = string.Format (
						"Superlogging is on with a buffer size of {0} entries",
						nCapacity);
					TxtBufferSize.IntValue = (int)nCapacity;
					pageSize = TxtBufferSize.IntValue;
				} else {
					Status.StringValue = "Superlogging is turned off. Click the button to turn it on";
				}
				BtnBufferSizeChange.Enabled = TxtBufferSize.Enabled = _enabled;
			} catch (Exception exc) {
				if (ChkAutoRefresh.StringValue == "1") {
					ChkAutoRefresh.StringValue = "0";
					timer.Change (-1, -1);
				}
				_enabled = false;
				Status.StringValue = "Superlogging is turned off. Click the button to turn it on";
				BtnBufferSizeChange.Enabled = TxtBufferSize.Enabled = true;
				UIErrorHelper.ShowAlert(exc.Message, "Operation could not complete successfully.");
			}
			BtnOff.Title = "Turn " + (_enabled ? "OFF" : "ON");
		}
		public new SuperLoggingBrowserWindow Window {
			get { return (SuperLoggingBrowserWindow)base.Window; }
		}

		private void btnFilter_Click(object sender, EventArgs e)
		{
			UIErrorHelper.CheckedExec (delegate {
				var row = (int)CboColumns.SelectedIndex;
				var op = (int)CbOperator.SelectedIndex;
				if (row > 0 && !string.IsNullOrEmpty (TxtFilterValue.StringValue) && op >= 0) {
				
					var entries = new List<SuperLogDto> ();
					for (int startIndex = _viewCache.Keys.Count; startIndex >= 0; startIndex--) {
						ISuperLogEntry item;
						if (_viewCache.TryGetValue (startIndex, out item)) {
							var endTime = item.getEndTime ();
							var startTime = item.getStartTime ();
							var errorCode = (int)item.getErrorCode ();
							var errordesc = GetErrorDescription (errorCode);
							var dto = new SuperLogDto {
								Port = item.getServerPort ().ToString (),
								LoginDN = item.getLoginDN (),
								Operation = item.getOperation (),
								ErrorCode = errordesc,
								Duration = (endTime - startTime).ToString (),
								DurationLong = (long)(endTime - startTime),
								ClientIP = item.getClientIP (),
								String = item.getString ()
							};
							entries.Add (dto);
						}
					}

					var filtered = new List<SuperLogDto> ();
					SuperLoggingColumn column; 
					Operation oper; 

					if (Enum.TryParse (row.ToString (), out column) && Enum.TryParse (op.ToString (), out oper)) {
						switch (column) {
						case SuperLoggingColumn.Port:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.Port.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals: 
								filtered = entries.Where (x => x.Port == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.Port.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.Port.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						case SuperLoggingColumn.LoginDN:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.LoginDN.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals: 
								filtered = entries.Where (x => x.LoginDN == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.LoginDN.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.LoginDN.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						case SuperLoggingColumn.Operation:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.Operation.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals: 
								filtered = entries.Where (x => x.Operation == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.Operation.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.Operation.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						case SuperLoggingColumn.ErrorCode:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.ErrorCode.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals:
								filtered = entries.Where (x => x.ErrorCode == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.ErrorCode.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.ErrorCode.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						case SuperLoggingColumn.Duration:
							long value;
							if (long.TryParse (TxtFilterValue.StringValue, out value)) {
								switch (oper) {
								case Operation.StartsWith:
									filtered = entries.Where (x => x.Duration.StartsWith (TxtFilterValue.StringValue)).ToList ();
									break;
								case Operation.GreaterThan:
									filtered = entries.Where (x => x.DurationLong > value).ToList ();
									break;
								case Operation.Equals:
									filtered = entries.Where (x => x.DurationLong == value).ToList ();
									break;
								case Operation.EndsWith: 
									filtered = entries.Where (x => x.Duration.EndsWith (TxtFilterValue.StringValue)).ToList ();
									break;
								case Operation.LessThan:
									filtered = entries.Where (x => x.DurationLong < value).ToList ();
									break;
								case Operation.Contains:
									filtered = entries.Where (x => x.Duration.Contains (TxtFilterValue.StringValue)).ToList ();
									break;
								default:
									break;
								}
							} else
								throw new Exception ("The filer value is not a valid duration");
							break;
						case SuperLoggingColumn.ClientIP:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.ClientIP.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals:
								filtered = entries.Where (x => x.ClientIP == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.ClientIP.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.ClientIP.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						case SuperLoggingColumn.String:
							switch (oper) {
							case Operation.StartsWith: 
							case Operation.GreaterThan:
								filtered = entries.Where (x => x.String.StartsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Equals:
								filtered = entries.Where (x => x.String == TxtFilterValue.StringValue).ToList ();
								break;
							case Operation.EndsWith: 
							case Operation.LessThan:
								filtered = entries.Where (x => x.String.EndsWith (TxtFilterValue.StringValue)).ToList ();
								break;
							case Operation.Contains:
								filtered = entries.Where (x => x.String.Contains (TxtFilterValue.StringValue)).ToList ();
								break;
							default:
								break;
							}
							break;
						}
						this.SuperLogsTableView.DataSource = new SuperLoggingTableViewDataSource (filtered);
						this.SuperLogsTableView.ReloadData ();
					}
				}
			});
		}

		private void EnableDisableFilter()
		{
			var row = (int)CboColumns.SelectedIndex;
			var op = (int)CbOperator.SelectedIndex;
			BtnFilter.Enabled = (row > 0 && !string.IsNullOrEmpty (TxtFilterValue.StringValue) && op >= 0);
		}

		private void btnSuperLogOnOff_Click(object sender, EventArgs e)
		{
			try
			{
				if (SuperLog.isEnabled())
					SuperLog.disable();
				else
					SuperLog.enable();
				UpdateStatus();
				RefreshList();
			}
			catch (Exception exp)
			{
				UIErrorHelper.ShowAlert(exp.ToString(),"Error");
			}
		}
		private void btnClear_Click(object sender, EventArgs e)
		{
			CboColumns.SelectItem(0);
			CbOperator.SelectItem(0);
			TxtFilterValue.StringValue = string.Empty;
			RefreshList();

		}
		partial void OnClearEntries(NSObject sender)
		{
			UIErrorHelper.CheckedExec(delegate
			{
				if (UIErrorHelper.ConfirmDeleteOperation("This will clear all the superlog entries at the server. Continue?"))
				{
					SuperLog.clear();
					RefreshList();
					TxtFilterValue.StringValue = string.Empty;
					CboColumns.SelectItem(0);
					CbOperator.SelectItem(0);
				}
			});
		}

		private void btnRefresh_Click(object sender, EventArgs e)
		{
			RefreshList();
		}

		static string GetErrorDescription (int errorCode)
		{
			var errordesc = "Success";
			if (errorCode != 0) {
				try {
					errordesc = ErrorCheckerHelper.ErrorCodeToString(errorCode);
				}
				catch (Exception exc) {
					errordesc = errorCode.ToString ();
				}
			}
			return errordesc;
		}

		void FillCache(int itemIndex, int windowSize)
		{
			UIErrorHelper.CheckedExec(delegate
				{
					var list = SuperLog.getPagedEntries(_cookie, (uint)windowSize);
					if (list != null)
					{ 
						int i = 0;
						foreach (var dto in list.getEntries())
						{
							_viewCache[itemIndex + i++] = dto;
						}
					}
					var entries = new List<SuperLogDto>();
					for (int startIndex = _viewCache.Keys.Count-1, i=0 ; startIndex >= 0; startIndex--, i++) {
						if(i> pageSize)
						{
							_viewCache.Remove(startIndex);
						}
						else
						{
							ISuperLogEntry item;
							if (_viewCache.TryGetValue (startIndex, out item)) {
								var endTime = item.getEndTime ();
								var startTime = item.getStartTime ();
								var errorCode = (int)item.getErrorCode ();
								var errordesc = GetErrorDescription (errorCode);
								var dto = new SuperLogDto {
									Port = item.getServerPort ().ToString (),
									LoginDN = item.getLoginDN (),
									Operation = item.getOperation (),
									ErrorCode = errordesc,
									Duration = (endTime - startTime).ToString (),
									DurationLong = (long)(endTime - startTime),
									ClientIP = item.getClientIP (),
									String = item.getString ()
								};
								entries.Add (dto);
							}
						}
					}

					this.SuperLogsTableView.DataSource = new SuperLoggingTableViewDataSource (entries);
					this.SuperLogsTableView.ReloadData ();
				});
		}

		private void btnChangeBufferSize_Click(object sender, EventArgs e)
		{
			UIErrorHelper.CheckedExec(delegate
				{
					var capacity = TxtBufferSize.IntValue;
					var message = string.Format("Set superlog buffer size to {0}?", capacity);
					ConfirmationDialogController cwc = new ConfirmationDialogController (message);
					nint result = NSApplication.SharedApplication.RunModalForWindow (cwc.Window);
					if (result == (nint)VMIdentityConstants.DIALOGOK) {
						SuperLog.setCapacity(Convert.ToUInt32(capacity));
						UpdateStatus();
						pageSize = capacity;
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
			bool autoRefresh = ChkAutoRefresh.StringValue == "1";
			if (autoRefresh) {
				var interval = TxtRefreshInterval.IntValue * 1000;
				timer.Change (interval, interval);
			} else {
				timer.Change (-1, -1);
//				timer.Dispose ();
//				timerState.timer.Dispose ();
//				timerState.timer = null;
//				timer = null;
			}
		}
		private void TimerRefresh()
		{
			UpdateStatus ();
			RefreshList (_viewCache.Count);
			btnFilter_Click (this, EventArgs.Empty);
		}
		private void timerAutoRefresh_Tick(Object state)
		{
			InvokeOnMainThread (TimerRefresh);
		}

		private void txtAutoRefresh_ValueChanged(object sender, EventArgs e)
		{
			ChangeAutoRefreshSettings();
		}

		public class TableDelegate : NSTableViewDelegate
		{
			private SuperLoggingBrowserWindowController _controller;
			public TableDelegate (SuperLoggingBrowserWindowController controller)
			{
				_controller = controller;
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				UIErrorHelper.CheckedExec (delegate() {
					NSTextFieldCell textCell = cell as NSTextFieldCell;
					if (textCell != null) {
						var collection = ((SuperLoggingTableViewDataSource)(_controller.SuperLogsTableView.DataSource)).Entries;
						if (collection != null) {
							var item = collection [(int)row];
							textCell.TextColor = (item.ErrorCode != "Success")  ? NSColor.Red: NSColor.Black;
						}
					}
				});
			}
		}
	}

	class TimerState {
		public int counter = 0;
		public Timer timer;
	}

	public enum SuperLoggingColumn
	{
		None = 0,
		Port = 1,
		LoginDN = 2,
		Operation = 3,
		ErrorCode = 4,
		Duration = 5,
		ClientIP = 6,
		String = 7
	}

	public enum Operation
	{
		StartsWith = 0,
		Equals = 1,
		EndsWith = 2,
		GreaterThan = 3,
		LessThan = 4,
		Contains = 5
	}
}
