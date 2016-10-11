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
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;

namespace RestSsoAdminSnapIn
{
	public partial class ShowComputersController : AppKit.NSWindowController
	{
		private NSTableView TableView { get; set; }

		public ServerDto ServerDto;

		#region Constructors


		// Called when created from unmanaged code
		public ShowComputersController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ShowComputersController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public ShowComputersController () : base ("ShowComputers")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			TableView = new NSTableView ();
			TableView.Delegate = new TableDelegate ();
			TableView.Frame = new CoreGraphics.CGRect (20, 20, 450, 450);
			this.MainTableView.AddSubview (TableView);
			IList<ComputerDto> computers = new List<ComputerDto> ();
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				computers = SnapInContext.Instance.ServiceGateway.Server.GetComputers (ServerDto, auth.Token);
			});
			var listView = new ComputersDataSource { Entries = computers };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Computer Name", DisplayOrder = 1, Width = 300 },
				new ColumnOptions{ Id = "DomainController", DisplayName = "Domain Controller", DisplayOrder = 2, Width = 130 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				TableView.AddColumn (column);
			}
			TableView.DataSource = listView;
			TableView.ReloadData ();
		}

		#endregion

		//strongly typed window accessor
		public new ShowComputers Window {
			get {
				return (ShowComputers)base.Window;
			}
		}

		public class TableDelegate : NSTableViewDelegate
		{
			private NSImage icon;

			public TableDelegate ()
			{
				icon = NSImage.ImageNamed ("NSComputer");
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if (tableColumn.Identifier == "Name")
							browserCell.Image = icon;
					}
				});
			}
		}
	}
}

