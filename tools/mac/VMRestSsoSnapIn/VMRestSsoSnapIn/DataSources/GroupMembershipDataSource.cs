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
using AppKit;
using System.Collections.Generic;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace Vmware.Tools.RestSsoAdminSnapIn.DataSource
{
	public class GroupMembershipDataSource : NSTableViewDataSource
	{
		public List<GroupDto> Groups { get; set; }
		public List<UserDto> Users { get; set; }
		public List<SolutionUserDto> SolutionUsers { get; set; }

		public GroupMembershipDataSource ()
		{
			Groups = new List<GroupDto> ();
			Users = new List<UserDto> ();
			SolutionUsers = new List<SolutionUserDto> ();
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export ("numberOfRowsInTableView:")]
		public int NumberOfRowsInTableView (NSTableView table)
		{
			var groups = (Groups != null) ? Groups.Count : 0;
			var users = (Users != null) ? Users.Count : 0;
			var solutionUsers = (SolutionUsers != null) ? SolutionUsers.Count : 0;
			return groups + users + solutionUsers;
		}

		// This method will be called by the control for each column and each row.
		[Export ("tableView:objectValueForTableColumn:row:")]
		public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
		{
			var value = (NSString)string.Empty;
			ActionHelper.Execute (delegate() {
				switch (col.Identifier) {
				case "Name":
					if (row < Groups.Count) {
						var obj = (this.Groups [row]) as GroupDto;
						value = (NSString)obj.GroupName;
					} else if (row >= Groups.Count && row <= (Groups.Count + Users.Count) - 1) {
						var obj = (this.Users [row - Groups.Count]) as UserDto;
						value = (NSString)obj.Name;
					} else if (row >= Groups.Count + Users.Count && row <= (Groups.Count + Users.Count + SolutionUsers.Count) - 1) {
						var obj = (this.SolutionUsers [row - Groups.Count - Users.Count]) as SolutionUserDto;
						value = (NSString)obj.Name;
					}
					break;
				}
			});
			return value;
		}

		public NSImage GetRowImage(int row)
		{
			NSImage image = NSImage.ImageNamed ("NSUserGroup");
			if(row < Groups.Count)
			{
				image = NSImage.ImageNamed ("NSUserGroup");
			} else if(row >= Groups.Count && row <= (Groups.Count + Users.Count) - 1)
			{
				image = NSImage.ImageNamed ("NSUser");
			} else if(row >= Groups.Count + Users.Count && row <= (Groups.Count + Users.Count + SolutionUsers.Count) - 1)
			{
				image = NSImage.ImageNamed ("NSUserGuest");
			}
			return image;
		}
	}
}

