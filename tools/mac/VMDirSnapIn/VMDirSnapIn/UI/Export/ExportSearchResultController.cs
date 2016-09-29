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
using Foundation;
using AppKit;
using VMDirSnapIn.DataSource;
using System.Collections.Generic;
using VMDirSnapIn.Nodes;
using VMDir.Common;
using VmIdentity.UI.Common.Utilities;
using System.Text;

namespace VMDirSnapIn.UI
{
	public partial class ExportSearchResultController : NSWindowController
	{
		private AttributeTableViewDataSource _attrToExportDs;
		private List<DirectoryNonExpandableNode> _resultList;
		private List<string> _returnedAttrList;
		private int _currPage;
		private int _pageSize;
		enum ExportScope
		{
			CURR_PAGE = 0,
			FETCHED_PAGE
		}

		public ExportSearchResultController(IntPtr handle) : base(handle)
		{
		}

		[Export("initWithCoder:")]
		public ExportSearchResultController(NSCoder coder) : base(coder)
		{
		}

		public ExportSearchResultController(List<DirectoryNonExpandableNode> _resultList, List<string> _returnedAttrList, int _currPage, int _pageSize) 
			: base("ExportSearchResult")
		{
			this._resultList = _resultList;
			this._returnedAttrList = _returnedAttrList;
			this._currPage = _currPage;
			this._pageSize = _pageSize;
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			_attrToExportDs = new AttributeTableViewDataSource();
			AttributeToExportTableView.DataSource = _attrToExportDs;
			foreach (var item in _returnedAttrList)
				AttributeToExportComboBox.Add((NSString)item);
			foreach (var item in VMDirConstants.ResultExportFormatList)
				FormatComboBox.Add((NSString)item);
			foreach (var item in VMDirConstants.ResultExportScopeList)
				ScopeComboBox.Add((NSString)item);
			AttributeToExportComboBox.SelectItem(0);
			FormatComboBox.SelectItem(0);
			ScopeComboBox.SelectItem(0);
			SetVisibility(false);
		}

		public new ExportSearchResult Window
		{
			get { return (ExportSearchResult)base.Window; }
		}

		bool ValidateExport()
		{
			if (FormatComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_FILE_FORMAT);
				return false;
			}
			if (ScopeComboBox.SelectedValue == null)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_SCOPE);
				return false;
			}
			if (AllReturnAttrCheckBox.State == NSCellStateValue.Off && _attrToExportDs.attrList.Count <= 0)
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_ATTR);
				return false;
			}
			return true;
		}

		void SetVisibility(bool v)
		{
			AttributeToExportComboBox.Enabled = v;
			AttributeToExportTableView.Enabled = v;
			AddButton.Enabled = v;
			RemoveButton.Enabled = v;
			RemoveAllButton.Enabled = v;
		}

		partial void OnAdd(NSObject sender)
		{
			_attrToExportDs.attrList.Add(AttributeToExportComboBox.SelectedValue.ToString());
			AttributeToExportTableView.ReloadData();
		}
		partial void OnRemove(NSObject sender)
		{
			nint row = AttributeToExportTableView.SelectedRow;
			if (row >= (nint)0)
			{
				_attrToExportDs.attrList.RemoveAt((int)row);
				AttributeToExportTableView.ReloadData();
			}
		}
		partial void OnRemoveAll(NSObject sender)
		{
			_attrToExportDs.attrList.Clear();
			AttributeToExportTableView.ReloadData();
		}
		partial void OnCancel(NSObject sender)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
		partial void OnExport(NSObject sender)
		{
			if (!ValidateExport())
				return;
			UIErrorHelper.CheckedExec(delegate
			{
				StringBuilder sb = new StringBuilder();
				var start = 0;
				var end = _resultList.Count;
				if (ScopeComboBox.SelectedIndex == (int)ExportScope.CURR_PAGE)
				{
					start = (_currPage - 1) * _pageSize;
					end = _currPage * _pageSize > _resultList.Count ? _resultList.Count : _currPage * _pageSize;
				}
				HashSet<string> attrToExport = new HashSet<string>();
				if (AllReturnAttrCheckBox.State == NSCellStateValue.On)
				{
					foreach (var item in _returnedAttrList)
					{
						attrToExport.Add(item);
					}
				}
				else
				{
					foreach (var item in _attrToExportDs.attrList)
					{
						attrToExport.Add(item);
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
						if (_resultList[i].NodeProperties.ContainsKey(item))
						{
							foreach (var val in _resultList[i].NodeProperties[item].Values)
								sb.Append(val.StringValue + " ");
						}
						sb.Append("\"");
						sb.Append(",");
					}
					sb.Append(Environment.NewLine);
				}
				if (FileIOUtil.WriteAllTextToFile(sb.ToString(), "Export Result", new string[] { "csv" }))
				{
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_RES_EXPO_SUCC);
				}
			});
		}

		partial void OnAllAttrCheckBoxClick(NSObject sender)
		{
			if (AllReturnAttrCheckBox.State == NSCellStateValue.On)
				SetVisibility(false);
			else
				SetVisibility(true);
		}

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
			NSApplication.SharedApplication.StopModalWithCode(0);
		}
	}
}
