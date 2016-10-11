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
using System.Linq;
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;
using VMDirSnapIn.DataSource;
using VMDir.Common.Schema;
using VMDirSnapIn.Delegate;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
	public partial class SelectObjectClassWindowController : NSWindowController
	{
		private List<ObjectClassDTO> _list;
		private ObjectClassDTO _selectedObject;
		private List<KeyValuePair<string, string>> dataSourceList;

		public ObjectClassDTO SelectedObject { get { return _selectedObject; } }

		#region Constructors

		// Called when created from unmanaged code
		public SelectObjectClassWindowController(IntPtr handle) : base(handle)
		{
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public SelectObjectClassWindowController(NSCoder coder) : base(coder)
		{
		}

		public SelectObjectClassWindowController(SchemaManager mgr) : base("SelectObjectClassWindow")
		{
			BindList(mgr);
		}

		// Call to load from the XIB/NIB file
		public SelectObjectClassWindowController() : base("SelectObjectClassWindow")
		{
		}

		int SortObjectClassDTO(ObjectClassDTO lhs, ObjectClassDTO rhs)
		{
			return lhs.Name.CompareTo(rhs.Name);
		}

		void BindList(SchemaManager mgr)
		{
			var om = mgr.GetObjectClassManager();
			_list = om.Data.Values.ToList();
			_list.Sort(SortObjectClassDTO);
			dataSourceList = new List<KeyValuePair<string, string>>();
			foreach (var obj in _list)
			{
				dataSourceList.Add(new KeyValuePair<string, string>(obj.Name, obj.Description));
			}
		}

		#endregion


		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			this.AddObjectTableView.DataSource = new GenericTableViewDataSource(dataSourceList);

			NSTableColumn col = this.AddObjectTableView.TableColumns()[0];
			if (col != null)
				col.DataCell = new NSBrowserCell();
			this.AddObjectTableView.Delegate = new GenericTableDelegate();

			this.SelectButton.Activated += OnClickSelectButton;
			this.CancelButton.Activated += OnClickCancelButton;
		}

		public void OnClickSelectButton(object sender, EventArgs e)
		{
			nint row = AddObjectTableView.SelectedRow;
			if (row >= (nint)0)
			{
				_selectedObject = _list[(int)row];
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			}
			else {
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_OC_SEL);
			}
		}

		public void OnClickCancelButton(object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModal();
			this.Close();
		}

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
			NSApplication.SharedApplication.StopModal();
		}

		//strongly typed window accessor
		public new SelectObjectClassWindow Window
		{
			get
			{
				return (SelectObjectClassWindow)base.Window;
			}
		}
	}
}

