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
using AppKit;
using Foundation;
using VMDir.Common;
using VMDir.Common.DTO;
using VMDirInterop.LDAP;
using VMDirSnapIn.UI;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;

namespace VMDirSnapIn.Nodes
{
	public class DirectoryNonExpandableNode : DirectoryNode
	{
		public DirectoryNonExpandableNode(string dn, List<string> oc, VMDirServerDTO dto) : base(dn,oc,dto,null)
		{
		}
		public override void AddUserToGroup(object sender, EventArgs e)
		{
			AddGroupByCNWindowController gwc = new AddGroupByCNWindowController(ServerDTO);
			nint result = NSApplication.SharedApplication.RunModalForWindow(gwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					string[] values = new string[2];
					values[1] = null;
					values[0] = Dn;
					LdapMod[] ldapVal = new LdapMod[1];
					ldapVal[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_MEMBER, values);
					ServerDTO.Connection.ModifyObject(gwc.DNText, ldapVal);
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_MEMBER_ADD_SUCC);
					ReloadChildren();
					RefreshProperties();
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadResultTableView", this);
				});
			}
		}
		public override void PerformDelete()
		{
			ConfirmationDialogController cwc = new ConfirmationDialogController("Are you sure?");
			nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					ServerDTO.Connection.DeleteObject(Dn);
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadResultOutlineView", this);
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_OBJ_DEL_SUCC);
				});
			}
		}
		public override void Delete(object sender, EventArgs e)
		{
			PerformDelete();
		}
		public void PerformRefreshNode()
		{
			_properties.Clear();
			FillProperties();
			NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadResultTableView", this);
		}
		public override void RefreshNode(object sender, EventArgs e)
		{
			PerformRefreshNode();
		}
	}
}