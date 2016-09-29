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
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace RestSsoAdminSnapIn
{
	public partial class GroupDetailsViewController : AppKit.NSViewController
	{
		public GroupDto GroupDtoOriginal;
		public GroupMembershipDto GroupsMembershipDtoOriginal;
		public ServerDto ServerDto;
		public string TenantName;
		private GroupDto GroupDto;
		private GroupMembershipDto GroupMembershipDto;
		public bool IsSystemDomain;

		#region Constructors

		// Called when created from unmanaged code
		public GroupDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public GroupDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public GroupDetailsViewController () : base ("GroupDetailsView", NSBundle.MainBundle)
		{
			Initialize ();
		}

		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		//strongly typed view accessor
		public new GroupDetailsView View {
			get {
				return (GroupDetailsView)base.View;
			}
		}

		void SetGroupDataSource (GroupMembershipDto members)
		{
			foreach(NSTableColumn column in GroupMembersTableView.TableColumns())
			{
				GroupMembersTableView.RemoveColumn (column);
			}
			var listView = new GroupMembershipDataSource {
				Groups = members.Groups,
				Users = members.Users,
				SolutionUsers = members.SolutionUsers
			};
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions {
					Id = "Name",
					DisplayName = "Member",
					DisplayOrder = 1,
					Width = 180
				}
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				GroupMembersTableView.AddColumn (column);
			}
			GroupMembersTableView.DataSource = listView;
			GroupMembersTableView.ReloadData ();
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			GroupDto = new GroupDto {
				GroupName = GroupDtoOriginal.GroupName,
				GroupDomain = GroupDtoOriginal.GroupDomain,
				GroupDetails = new GroupDetailsDto { Description = GroupDtoOriginal.GroupDetails.Description }
			};
			TxtGroupName.StringValue = GroupDto.GroupName == null ? string.Empty : GroupDto.GroupName;
			TxtGroupDescription.StringValue = GroupDto.GroupDetails.Description == null ? string.Empty : GroupDto.GroupDetails.Description;
			GroupMembersTableView.Delegate = new TableDelegate ();
			GroupMembershipDto = new GroupMembershipDto {
				Users = (GroupsMembershipDtoOriginal.Users == null ? new List<UserDto> () : new List<UserDto> (GroupsMembershipDtoOriginal.Users)),
				SolutionUsers = (GroupsMembershipDtoOriginal.SolutionUsers == null ? new List<SolutionUserDto> () : new List<SolutionUserDto> (GroupsMembershipDtoOriginal.SolutionUsers)),
				Groups = (GroupsMembershipDtoOriginal.Groups == null ? new List<GroupDto> () : new List<GroupDto> (GroupsMembershipDtoOriginal.Groups))
			};
			SetGroupDataSource (GroupMembershipDto);

			BtnSave.Hidden = !IsSystemDomain;

			this.BtnSave.Activated += OnClickSaveButton;
			this.BtnAddMember.Activated += OnClickAddMember;
			this.BtnRemoveMember.Activated += OnClickRemoveMember;
			BtnAddMember.Enabled = IsSystemDomain;
			BtnRemoveMember.Enabled = IsSystemDomain;
		}

		public void OnClickAddMember (object sender, EventArgs e)
		{
			var form = new ShowAllGroupsController (){IsSystemDomain = IsSystemDomain};
			form.ServerDto = ServerDto;
			form.TenantName = TenantName;
			form.DomainName = GroupDto.GroupDomain;
			var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (result == VMIdentityConstants.DIALOGOK) {
				if (GroupsMembershipDtoOriginal.Groups == null)
					this.GroupMembershipDto.Groups = new List<GroupDto> ();
				foreach (var group in form.SelectedMembers.Groups) {
					var principalName = group.GroupName + "@" + group.GroupDomain;
					if (!GroupsMembershipDtoOriginal.Groups.Exists (x => (x.GroupName + "@" + x.GroupDomain) == principalName))
						this.GroupMembershipDto.Groups.Add (group);
				}
				foreach (var user in form.SelectedMembers.Users) {
					var principalName = user.Name + "@" + user.Domain;
					if (!GroupsMembershipDtoOriginal.Users.Exists (x => (x.Name + "@" + x.Domain) == principalName))
						this.GroupMembershipDto.Users.Add (user);
				}
				SetGroupDataSource (GroupMembershipDto);
			}
		}

		public void OnClickRemoveMember (object sender, EventArgs e)
		{
			if (GroupMembersTableView.SelectedRows.Count > 0) {
				var list = GroupMembersTableView.SelectedRows.OrderByDescending (x=>x).ToList();
				foreach (var selectedRow in list) {
					var row = (int)selectedRow;
					if (row < GroupMembershipDto.Groups.Count) {
						GroupMembershipDto.Groups.RemoveAt (row);
					} else if (row >= GroupMembershipDto.Groups.Count && row <= (GroupMembershipDto.Groups.Count + GroupMembershipDto.Users.Count) - 1) {
						GroupMembershipDto.Users.RemoveAt (row - GroupMembershipDto.Groups.Count);
					} else if (row >= GroupMembershipDto.Groups.Count + GroupMembershipDto.Users.Count &&
					         row < (GroupMembershipDto.Groups.Count + GroupMembershipDto.Users.Count + GroupMembershipDto.SolutionUsers.Count) - 1) {
						GroupMembershipDto.SolutionUsers.RemoveAt (row - GroupMembershipDto.Groups.Count - GroupMembershipDto.Users.Count);
					}
				}
				SetGroupDataSource (GroupMembershipDto);
			}
		}

		public void OnClickSaveButton (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				GroupDto.GroupName = TxtGroupName.StringValue;
				GroupDto.GroupDomain = GroupDtoOriginal.GroupDomain;
				GroupDto.GroupDetails.Description = TxtGroupDescription.StringValue;
				if (GroupDto.GroupDetails.Description != GroupDtoOriginal.GroupDetails.Description) {

					var service = SnapInContext.Instance.ServiceGateway;
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					service.Group.Update (ServerDto, TenantName, GroupDto, auth.Token);
				}
				UpdateGroupMembership (GroupsMembershipDtoOriginal, GroupMembershipDto);
				NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
			});
		}

		private void UpdateGroupMembership(GroupMembershipDto left, GroupMembershipDto right)
		{
			if (left.Groups == null && right.Groups != null) {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				SnapInContext.Instance.ServiceGateway.Group.AddGroups (ServerDto, TenantName, GroupDto, right.Groups, auth.Token);
			} else if (left.Groups != null && right.Groups != null) {
				var groups = new List<GroupDto> ();
				foreach (var group in left.Groups) {
					var fullName = group.GroupName + "@" + group.GroupDomain;
					if (right.Groups.FirstOrDefault (x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null) {
						groups.Add (group);
					}
					if(groups.Count() > 0)
					{
						// remove
						var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
						SnapInContext.Instance.ServiceGateway.Group.RemoveGroups (ServerDto, TenantName, GroupDto,groups, auth.Token);
					}
				}
				groups = new List<GroupDto> ();
				foreach (var group in right.Groups) {
					var fullName = group.GroupName + "@" + group.GroupDomain;
					if (left.Groups.FirstOrDefault (x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null) {
						groups.Add (group);
					}
				}

				if(groups.Count() > 0)
				{
					// add
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					SnapInContext.Instance.ServiceGateway.Group.AddGroups (ServerDto, TenantName, GroupDto,groups, auth.Token);
				}
			}

			if (left.Users == null && right.Users != null) {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				SnapInContext.Instance.ServiceGateway.Group.AddUsers (ServerDto, TenantName, GroupDto, right.Users, auth.Token);
			} else if (left.Users != null && right.Users != null) {
				var users = new List<UserDto> ();
				foreach (var user in left.Users) {
					var fullName = user.Name + "@" + user.Domain;
					if (right.Users.FirstOrDefault (x => (x.Name + "@" + x.Domain) == fullName) == null) {
						users.Add (user);
					}
					if(users.Count() > 0)
					{
						// remove
						var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
						SnapInContext.Instance.ServiceGateway.Group.RemoveUsers (ServerDto, TenantName, GroupDto,users, auth.Token);
					}
				}
				users = new List<UserDto> ();
				foreach (var user in right.Users) {
					var fullName = user.Name + "@" + user.Domain;
					if (left.Users.FirstOrDefault (x => (x.Name + "@" + x.Domain) == fullName) == null) {
						users.Add (user);
					}
				}

				if(users.Count() > 0)
				{
					// add
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					SnapInContext.Instance.ServiceGateway.Group.AddUsers (ServerDto, TenantName, GroupDto,users, auth.Token);
				}
			}
		}

		public class TableDelegate : NSTableViewDelegate
		{
			public TableDelegate ()
			{
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
				NSTableColumn tableColumn, nint row)
			{
				ActionHelper.Execute (delegate() {
					var datasource = tableView.DataSource as GroupMembershipDataSource;
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if (tableColumn.Identifier == "Name") {
							browserCell.Image = datasource.GetRowImage ((int)row);
							browserCell.Image.Size = new CoreGraphics.CGSize{ Width = (float)16.0, Height = (float)16.0 };
						}
					}
				});
			}
		}
	}
}
