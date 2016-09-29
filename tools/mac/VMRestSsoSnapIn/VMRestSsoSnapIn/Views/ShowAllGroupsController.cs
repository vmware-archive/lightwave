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
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace RestSsoAdminSnapIn
{
	public partial class ShowAllGroupsController : AppKit.NSWindowController
	{
		private enum MemberTypeFilter{ All = 0, Users, Groups};
		public bool IsSystemDomain { get; set; }
		//private NSTableView TableView { get; set; }
		public GroupMembershipDto SelectedMembers { get; set;}
		public ServerDto ServerDto;
		public string TenantName;
		public string DomainName;
		public bool IsUserSearch { get; set; }

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
			this.SearchButton.Activated += (object sender, EventArgs e) => {

				if(!IsUserSearch)
				{
					var domain = ((NSString)DomainComboBox.SelectedValue).ToString();
					var filter = (MemberTypeFilter)((int)MemberTypeComboBox.SelectedIndex);
					Search(NameTextString.StringValue, domain, filter);
				}
				else
				{
					Search(NameTextString.StringValue, DomainName, MemberTypeFilter.Groups);
				}
			};
			foreach(NSTableColumn column in GroupsTableView.TableColumns())
			{
				GroupsTableView.RemoveColumn (column);
			}
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 500 },
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				GroupsTableView.AddColumn (column);
			}
			GroupsTableView.AllowsMultipleSelection = true;
			GroupsTableView.Delegate = new TableDelegate ();
			GetIdentitySources ();
		}

		private void GetIdentitySources(){
			if (!IsUserSearch) {
				
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				var service = SnapInContext.Instance.ServiceGateway;
				var identityProviders = service.IdentityProvider.GetAll (ServerDto, TenantName, auth.Token);

				if (identityProviders.Count > 0) {
					var items = identityProviders.Select (x => (NSString)x.Name).ToArray ();
					DomainComboBox.Add (items);
					DomainComboBox.SelectItem (0);
					MemberTypeComboBox.SelectItem (0);
					Search (null, items [0], MemberTypeFilter.All);
				}
			} else {
				DomainComboBox.Hidden = true;
				MemberTypeComboBox.Hidden = true;
				DomainLabel.Hidden = true;
				MemberTypeLabel.Hidden = true;
				Search (null, DomainName, MemberTypeFilter.Groups);
			}
		}

		private void Search(string name, string domain, MemberTypeFilter filter){

			var groupInfo = new GroupMembershipDto (){ Groups = new List<GroupDto> (), Users = new List<UserDto>() };
			if (filter != MemberTypeFilter.Users) {
				
				ActionHelper.Execute (delegate() {
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					var groupInfo1 = SnapInContext.Instance.ServiceGateway.Tenant.Search (ServerDto, TenantName, domain, MemberType.GROUP, SearchType.NAME, auth.Token, name);
					groupInfo.Groups = groupInfo1.Groups;
				});
			}

			if (filter != MemberTypeFilter.Groups) {

				ActionHelper.Execute (delegate() {
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					var groupInfo2 = SnapInContext.Instance.ServiceGateway.Tenant.Search (ServerDto, TenantName, domain, MemberType.USER, SearchType.NAME, auth.Token, name);
					groupInfo.Users = groupInfo2.Users;
				});
			}
			var listView = new GroupMembershipDataSource { Groups = groupInfo.Groups, Users = groupInfo.Users };
			if (listView.Groups == null)
				listView.Groups = new List<GroupDto> ();
			
			if (listView.Users == null)
				listView.Users = new List<UserDto> ();
			
			GroupsTableView.DataSource = listView;
			GroupsTableView.ReloadData ();
			WarningLabel.Hidden = (listView.Groups.Count + listView.Users.Count) < 100;
		}

		public void OnClickAddGroupButton (object sender, EventArgs e)
		{
			if (GroupsTableView.SelectedRows.Count > 0) {
				SelectedMembers = new GroupMembershipDto (){ Groups = new List<GroupDto> (), Users = new List<UserDto> () };
				var dataSource = (GroupMembershipDataSource)GroupsTableView.DataSource;

				foreach (var groupId in GroupsTableView.SelectedRows) {
					var gid = (int)groupId;
					if (gid < dataSource.Groups.Count) {
						SelectedMembers.Groups.Add (dataSource.Groups [gid]);
					} else {
						SelectedMembers.Users.Add (dataSource.Users [gid - dataSource.Groups.Count]);
					}
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
			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					var ds = tableView.DataSource as GroupMembershipDataSource;
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if (tableColumn.Identifier == "Name")
							browserCell.Image = ds.GetRowImage((int)row);
					}
				});
			}
		}
	}
}

