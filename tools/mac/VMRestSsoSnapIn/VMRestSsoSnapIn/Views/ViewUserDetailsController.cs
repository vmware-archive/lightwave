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
using VmIdentity.UI.Common.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn;
using VmIdentity.UI.Common;

namespace RestSsoAdminSnapIn
{
	public partial class ViewUserDetailsController : AppKit.NSViewController
	{
		public UserDto UserDtoOriginal;
		public List<GroupDto> GroupsOriginal;
		public ServerDto ServerDto;
		public string TenantName;
		private UserDto UserDto;
		private List<GroupDto> Groups;
		private NSTableView lstGroups;

		#region Constructors

		// Called when created from unmanaged code
		public ViewUserDetailsController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ViewUserDetailsController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public ViewUserDetailsController () : base ("ViewUserDetails", NSBundle.MainBundle)
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		public new ViewUserDetails View {
			get {
				return (ViewUserDetails)base.View;
			}
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
			UserDto = UserDtoOriginal.DeepCopy ();
			Groups = new List<GroupDto> (GroupsOriginal);

			TxtUsername.StringValue = UserDto.Name == null ? string.Empty : UserDto.Name;
			TxtFirstName.StringValue = UserDto.PersonDetails.FirstName == null ? string.Empty : UserDto.PersonDetails.FirstName;
			TxtLastName.StringValue = UserDto.PersonDetails.LastName == null ? string.Empty : UserDto.PersonDetails.LastName;
			TxtEmail.StringValue = UserDto.PersonDetails.EmailAddress == null ? string.Empty : UserDto.PersonDetails.EmailAddress;
			TxtDescription.StringValue = UserDto.PersonDetails.Description == null ? string.Empty : UserDto.PersonDetails.Description;
			if (UserDto.PasswordDetails != null) {
				TxtPasswordLastChanged.StringValue = DateTimeHelper.UnixToWindows ((long)UserDto.PasswordDetails.LastSet).ToString ("dd-MMM-yyyy hh:mm:ss");
				TxtPasswordDaysUntilExpiry.StringValue = SecondsToDaysAndHours (UserDto.PasswordDetails.Lifetime);
			}
			ChDisabled.StringValue = UserDto.Disabled ? "0" : "1";
			ChActAsUser.StringValue = UserDto.ActAsUsers ? "1" : "0";
			ChIdpAdmin.StringValue = UserDto.IsIdpAdmin ? "1" : "0";
			RdoRoleGroup.SelectCell (new NSCell (UserDto.Role.ToString ()));

			lstGroups = new NSTableView ();
			lstGroups.Delegate = new TableDelegate ();
			this.LstGroupMembershipView.AddSubview (lstGroups);
			var listView = new GroupsDataSource { Entries = Groups };
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Group Name", DisplayOrder = 1, Width = 300 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				lstGroups.AddColumn (column);
			}
			lstGroups.DataSource = listView;
			lstGroups.ReloadData ();

			//Events
			this.BtnSave.Activated += OnClickSaveButton;
			this.BtnAddGroup.Activated += OnClickAddGroupButton;
			this.BtnRemoveGroup.Activated += OnClickRemoveGroupButton;
		}

		public void OnClickAddGroupButton (object sender, EventArgs e)
		{
			NSApplication.SharedApplication.StopModal ();
			var form = new ShowAllGroupsController ();
			form.ServerDto = ServerDto;
			form.TenantName = TenantName;
			form.DomainName = UserDto.Domain;
			var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (result == VMIdentityConstants.DIALOGOK) {
				foreach (var group in form.SelectedGroups) {
					var principalName = group.GroupName + "@" + group.GroupDomain;
					if (!Groups.Exists (x => (x.GroupName + "@" + x.GroupDomain) == principalName))
						Groups.Add (group);
				}
				var listView = new GroupsDataSource { Entries = Groups };
				lstGroups.DataSource = listView;
				lstGroups.ReloadData ();
			}
		}

		public void OnClickRemoveGroupButton (object sender, EventArgs e)
		{
			foreach (var row in lstGroups.SelectedRows) {
				Groups.RemoveAt ((int)row);
			}
			var listView = new GroupsDataSource { Entries = Groups };
			lstGroups.DataSource = listView;
			lstGroups.ReloadData ();
		}

		public void OnClickSaveButton (object sender, EventArgs e)
		{
			UserDto.PersonDetails.FirstName = TxtFirstName.StringValue;
			UserDto.PersonDetails.LastName = TxtLastName.StringValue;
			UserDto.PersonDetails.EmailAddress = TxtEmail.StringValue;
			UserDto.PersonDetails.Description = TxtDescription.StringValue;
			UserDto.IsIdpAdmin = ChIdpAdmin.StringValue == "1";
			UserDto.ActAsUsers = ChActAsUser.StringValue == "1";
			UserDto.Disabled = ChDisabled.StringValue == "0";
			var role = RdoRoleGroup.SelectedCell.Title.ToString ();
			UserRole roleType = UserRole.GuestUser;
			if (Enum.TryParse (role, out roleType))
				UserDto.Role = roleType;

			if (UserDtoOriginal.IsDifferentThan (UserDto)) {
				try {
					var service = SnapInContext.Instance.ServiceGateway;
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					service.User.Update (ServerDto, TenantName, UserDto, auth.Token);
				} catch (Exception) {
				}
			}

			UpdateGroupMembership ();

			UpdateUserRole ();

			//this.Close ();
			NSApplication.SharedApplication.StopModalWithCode (1);
		}

		private void UpdateGroupMembership ()
		{
			var users = new List<UserDto> { UserDto };
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
			foreach (var group in GroupsOriginal) {
				var fullName = group.GroupName + "@" + group.GroupDomain;
				if (Groups.FirstOrDefault (x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null) {
					// remove
					SnapInContext.Instance.ServiceGateway.Group.RemoveUsers (ServerDto, TenantName, group, users, auth.Token);
				}
			}

			foreach (var group in Groups) {
				var fullName = group.GroupName + "@" + group.GroupDomain;
				if (GroupsOriginal.FirstOrDefault (x => (x.GroupName + "@" + x.GroupDomain) == fullName) == null) {
					// add
					SnapInContext.Instance.ServiceGateway.Group.AddUsers (ServerDto, TenantName, group, users, auth.Token);
				}
			}
		}

		private void UpdateUserRole()
		{
			var result = true;
			var dto = UserDto;
			var users = new List<UserDto> { dto };
			var name = string.Empty;
			GroupDto group;
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
			if (result && dto.Role != UserDtoOriginal.Role)
			{
				// Remove group membership
				if (UserDtoOriginal.Role != UserRole.GuestUser)
				{
					name = (UserDtoOriginal.Role == UserRole.Administrator ? "Administrators" : "Users");
					group = new GroupDto { GroupName = name, GroupDomain = UserDtoOriginal.Domain };
					result = SnapInContext.Instance.ServiceGateway.Group.RemoveUsers(ServerDto, TenantName, group, users, auth.Token);
				}
				if (result && dto.Role != UserRole.GuestUser)
				{
					// Add group membership
					name = (dto.Role == UserRole.Administrator ? "Administrators" : "Users");
					group = new GroupDto { GroupName = name, GroupDomain = dto.Domain };
					result = SnapInContext.Instance.ServiceGateway.Group.AddUsers(ServerDto, TenantName, group, users, auth.Token);
				}
			}

			if (result && dto.ActAsUsers != UserDtoOriginal.ActAsUsers)
			{
				name = "ActAsUsers";
				group = new GroupDto { GroupName = name, GroupDomain = dto.Domain };
				result = (dto.ActAsUsers)
					? SnapInContext.Instance.ServiceGateway.Group.AddUsers(ServerDto, TenantName, group, users, auth.Token)
					: SnapInContext.Instance.ServiceGateway.Group.RemoveUsers(ServerDto, TenantName, group, users, auth.Token);
			}

			if (result && dto.IsIdpAdmin != UserDtoOriginal.IsIdpAdmin)
			{
				name = "IdpProvisioningAdmin";
				group = new GroupDto { GroupName = name, GroupDomain = dto.Domain };
				result = (dto.IsIdpAdmin)
					? SnapInContext.Instance.ServiceGateway.Group.AddUsers(ServerDto, TenantName, group, users, auth.Token)
					: SnapInContext.Instance.ServiceGateway.Group.RemoveUsers(ServerDto, TenantName, group, users, auth.Token);
			}
		}

		#endregion

		//strongly typed window accessor
//		public new ViewUserDetails Window {
//			get {
//				return (ViewUserDetails)base.Window;
//			}
//		}

		private string SecondsToDaysAndHours (long seconds)
		{
			var hours = seconds / (60 * 60);
			return string.Format ("{0} days {1} hours", hours / 24, hours % 24);
		}

		public class TableDelegate : NSTableViewDelegate
		{
			public TableDelegate ()
			{
			}

			public override void WillDisplayCell (NSTableView tableView, NSObject cell,
			                                      NSTableColumn tableColumn, nint row)
			{
				try {
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
						if (tableColumn.Identifier == "Name") {
							var image = NSImage.ImageNamed ("NSUserGroup");
							browserCell.Image = image;
							browserCell.Image.Size = new CoreGraphics.CGSize{ Width = (float)16.0, Height = (float)16.0 };
						}
					}
				} catch (Exception e) {
					System.Diagnostics.Debug.WriteLine ("Exception in casting : " + e.Message);
				}
			}
		}
	}
}

