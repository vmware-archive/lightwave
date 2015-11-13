/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace RestSsoAdminSnapIn
{
	public partial class ShowAllGroupsController : AppKit.NSWindowController
	{
		public bool IsSystemDomain { get; set; }
		//private NSTableView TableView { get; set; }
		public List<GroupDto> SelectedGroups { get; set;}
		public ServerDto ServerDto;
		public string TenantName;
		public string DomainName;

		#region Constructors

		// Called when created from unmanaged code
		public ShowAllGroupsController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ShowAllGroupsController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public ShowAllGroupsController () : base ("ShowAllGroups")
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
			this.BtnAdd.Activated += OnClickAddGroupButton;

			foreach(NSTableColumn column in GroupsTableView.TableColumns())
			{
				GroupsTableView.RemoveColumn (column);
			}
			GroupsTableView.Delegate = new TableDelegate ();
			var groupInfo = new GroupMembershipDto (){ Groups = new List<GroupDto> () };
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				groupInfo = SnapInContext.Instance.ServiceGateway.Tenant.Search (ServerDto, TenantName, DomainName, MemberType.GROUP, SearchType.NAME, auth.Token);
			});
				
			var listView = new GroupsDataSource { Entries = groupInfo.Groups };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Group Name", DisplayOrder = 1, Width = 500 },
				};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				GroupsTableView.AddColumn (column);
			}
			GroupsTableView.DataSource = listView;
			GroupsTableView.ReloadData ();
		}

		public void OnClickAddGroupButton (object sender, EventArgs e)
		{
			if (GroupsTableView.SelectedRows.Count > 0) {
				SelectedGroups = new List<GroupDto> ();
				var dataSource = (GroupsDataSource)GroupsTableView.DataSource;

				foreach (var groupId in GroupsTableView.SelectedRows) {
					SelectedGroups.Add (dataSource.Entries [(int)groupId]);
				}
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
			}
		}
		#endregion

		//strongly typed window accessor
		public new ShowAllGroups Window {
			get {
				return (ShowAllGroups)base.Window;
			}
		}

		public class TableDelegate : NSTableViewDelegate
		{
			private NSImage icon;

			public TableDelegate ()
			{
				icon = NSImage.ImageNamed ("NSUserGroup");
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

