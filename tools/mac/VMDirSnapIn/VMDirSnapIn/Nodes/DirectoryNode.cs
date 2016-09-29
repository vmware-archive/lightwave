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
using System.Linq;
using AppKit;
using Foundation;
using VMDirSnapIn.UI;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VMDir.Common.VMDirUtilities;
using VMDirInterop.Interfaces;
using VMDirInterop.LDAP;
using System.Collections.Generic;
using VMDir.Common;
using UI.Password;
using VMIdentity.CommonUtils;

namespace VMDirSnapIn.Nodes
{
	public class DirectoryNode : ChildScopeNode
	{
		public string Dn { get; private set; }
		public List<string> ObjectClass { get; private set; }

		private QueryDTO qdto;
		private IntPtr cookie;
		private int totalCount;
		public bool morePages { get; private set; }
		private int pageNumber;

		public bool isChildrenLoaded { get; private set; }
		public bool IsBaseNode { get; set; }
		protected Dictionary<string, VMDirAttributeDTO> _properties;

		public Dictionary<string, VMDirAttributeDTO> NodeProperties
		{
			get
			{
				if(_properties==null)
					FillProperties();
				return _properties;
			}
			set
			{
				_properties = value;
			}
		}

		public DirectoryNode(string dn, List<string> ocSet, VMDirServerDTO dto, ScopeNode parent) : base(dto)
		{
			Dn = dn;
			ObjectClass = ocSet;
			DisplayName = VMDirServerDTO.DN2CN(Dn);
			Parent = parent;
			IsBaseNode = false;
			InitPageSearch();
		}

		public void FillProperties()
		{
			UIErrorHelper.CheckedExec(delegate
			{
				TextQueryDTO dto = new TextQueryDTO(Dn, LdapScope.SCOPE_BASE, VMDirConstants.SEARCH_ALL_OC, null, 0, IntPtr.Zero, 0);

				ServerDTO.Connection.Search(dto,
					(l, e) =>
					{
					if(e.Count>0)
						_properties = ServerDTO.Connection.GetEntryProperties(e[0]);
					});
			});
		}

		public void RefreshProperties()
		{
			_properties.Clear();
			FillProperties();
			NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
		}
		private void InitPageSearch()
		{
			qdto = new TextQueryDTO(Dn, LdapScope.SCOPE_ONE_LEVEL, VMDirConstants.SEARCH_ALL_OC,
			                        new string[] { VMDirConstants.ATTR_DN, VMDirConstants.ATTR_OBJECT_CLASS }, 0, IntPtr.Zero, 0);
			cookie = IntPtr.Zero;
			totalCount = 0;
			pageNumber = 1;
			morePages = true;
		}

		public void Expand(string itemDN)
		{
			if (!isChildrenLoaded)
			{
				InitPageSearch();
				GetPage();
			}
		}

		private void GetPage()
		{
			List<DirectoryNode> lst = new List<DirectoryNode>();
			UIErrorHelper.CheckedExec(delegate
			{
				ServerDTO.Connection.PagedSearch(qdto, ServerDTO.PageSize, cookie, morePages,
					delegate (ILdapMessage ldMsg, IntPtr ck, bool moreP, List<ILdapEntry> entries)
					{
						cookie = ck;
						morePages = moreP;
						totalCount += entries.Count();
						pageNumber++;
						foreach (var entry in entries)
						{
							var ocList = new List<string>(entry.getAttributeValues(VMDirConstants.ATTR_OBJECT_CLASS).Select(x => x.StringValue).ToArray());
							var node = new DirectoryNode(entry.getDN(), ocList, ServerDTO, this);
							//node.NodeProperties = ServerDTO.Connection.GetEntryProperties(entry);
							lst.Add(node);
						}
					});
				isChildrenLoaded = true;
				this.Children.AddRange(lst.ToArray());
				NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
			});
		}

		public void ReloadChildren()
		{
			UIErrorHelper.CheckedExec(delegate ()
			{

				if (this.Children != null)
				{
					this.Children.Clear();
					isChildrenLoaded = false;
					InitPageSearch();
					Expand(Dn);
				}
			});
		}

		//Events
		public virtual void RefreshNode(object sender, EventArgs e)
		{
			RefreshProperties();
			ReloadChildren();
		}

		public void Add(object sender, EventArgs e)
		{
			ShowAddWindow();
		}

		public virtual void Delete(object sender, EventArgs e)
		{
			PerformDelete();
		}

		public void AddUser(object sender, EventArgs e)
		{
			ShowAddUser();
		}

		public void AddGroup(object sender, EventArgs e)
		{
			ShowAddGroup();
		}
		public void Search(object sender, EventArgs e)
		{
			ShowSearch();
		}

		public void FetchNextPage(object sender, EventArgs e)
		{
			GetNextPage();
		}

		public void VerifyUserPassword(object sender, EventArgs e)
		{
			ShowVerifyUserPassword();
		}

		public void ShowSearch()
		{
			SearchWindowController swc = new SearchWindowController(Dn, ServerDTO);
			NSApplication.SharedApplication.RunModalForWindow(swc.Window);
			//swc.Window.MakeKeyAndOrderFront(this);
		}

		public void GetNextPage()
		{
			if (morePages)
				GetPage();
			else
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_NO_MORE_PAGES);
		}

		void ShowVerifyUserPassword()
		{
			var upn = Utilities.GetAttrLastVal(_properties, VMDirConstants.ATTR_KRB_UPN);

			VerifyPasswordController cwc = new VerifyPasswordController(upn);
			nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				try
				{
					VMDirServerDTO ser = new VMDirServerDTO();
					ser.Server = ServerDTO.Server;
					ser.Password = cwc.Password;
					ser.BindDN = cwc.Upn;
					ser.Connection = new LdapConnectionService(ser.Server, ser.BindDN, ser.Password);
					if (ser.Connection.CheckCredentials())
						UIErrorHelper.ShowAlert(CommonConstants.CORRECT_PWD, "Success");
				}
				catch (Exception)
				{
					UIErrorHelper.ShowAlert(CommonConstants.INVALID_PWD, "Failure");
				}
			}
		}

		//Launch Dialogs
		public virtual void AddUserToGroup(object sender, EventArgs e)
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
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
				});
			}
		}

		public void ShowAddWindow()
		{
			SelectObjectClassWindowController swc = new SelectObjectClassWindowController(ServerDTO.Connection.SchemaManager);
			nint result = NSApplication.SharedApplication.RunModalForWindow(swc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				CreateObjectWindowController cwc = new CreateObjectWindowController(swc.SelectedObject.Name, ServerDTO,Dn);
				nint res = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
				if (res == (nint)VMIdentityConstants.DIALOGOK)
				{
					UIErrorHelper.CheckedExec(delegate ()
					{
						var attr = cwc._properties.Select(x => Utilities.MakeAttribute(x)).ToArray();
						string newdn = cwc.Rdn + "," + Dn;
						ServerDTO.Connection.AddObject(newdn, attr);
						UIErrorHelper.ShowInformation(VMDirConstants.STAT_OBJ_ADD_SUCC);
						var oc = Utilities.GetObjectClassList(ServerDTO,newdn,LdapScope.SCOPE_BASE);
						this.Children.Insert(0,new DirectoryNode(newdn,oc,ServerDTO,this));
						//ReloadChildren();
						RefreshProperties();
						NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
						NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
					});

				}
				swc.Dispose();
			}
		}

		public void ShowAddGroup()
		{
			GroupDTO dto = new GroupDTO();
			AddNewGroupController agc = new AddNewGroupController(dto);
			nint res = NSApplication.SharedApplication.RunModalForWindow(agc.Window);
			if (res == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					LdapMod[] user = new LdapMod[4];
					user[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
						dto.cn,
						null
					});
					user[1] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GROUPTYPE, new string[] {
						dto.groupType.ToString (),
						null
					});
					user[2] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
						dto.sAMAccountName,
						null
					});
					user[3] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
						dto.objectClass,
						null
					});
					string dn = string.Format("cn={0},{1}", dto.cn, Dn);
					ServerDTO.Connection.AddObject(dn, user);
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_GRP_ADD_SUCC);
					var oc = Utilities.GetObjectClassList(ServerDTO, dn, LdapScope.SCOPE_BASE);
					this.Children.Insert(0, new DirectoryNode(dn, oc, ServerDTO, this));
					//ReloadChildren();
					RefreshProperties();
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
				});
			}
		}

		public void ShowAddUser()
		{
            UserDTO userDTO = new UserDTO();
			AddNewUserController awc = new AddNewUserController(userDTO);
			nint res = NSApplication.SharedApplication.RunModalForWindow(awc.Window);
			if (res == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					LdapMod[] user = new LdapMod[6];
					user[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_GIVEN_NAME, new string[] {
						userDTO.FirstName,
						null
					});
					user[1] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SN, new string[] {
						userDTO.LastName,
						null
					});
					user[2] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_CN, new string[] {
						userDTO.Cn,
						null
					});
					user[3] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_KRB_UPN, new string[] {
						userDTO.UPN,
						null
					});
					user[4] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_SAM_ACCOUNT_NAME, new string[] {
						userDTO.SAMAccountName,
						null
					});
					user[5] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_ADD, VMDirConstants.ATTR_OBJECT_CLASS, new string[] {
						VMDirConstants.USER_OC,
						null
					});
					string dn = string.Format("cn={0},{1}", userDTO.Cn, Dn);
					ServerDTO.Connection.AddObject(dn, user);
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_USR_ADD_SUCC);
					var oc = Utilities.GetObjectClassList(ServerDTO, dn, LdapScope.SCOPE_BASE);
					this.Children.Insert(0, new DirectoryNode(dn, oc, ServerDTO, this));
					//ReloadChildren();
					RefreshProperties();
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", this);
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
				});

			}
		}

		public virtual void PerformDelete()
		{
			ConfirmationDialogController cwc = new ConfirmationDialogController("Are you sure?");
			nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					ServerDTO.Connection.DeleteObject(Dn);
					ScopeNode node = this.Parent;
					if (node != null)
					{
						node.Children.Remove(this);
						if (node is DirectoryNode)
							(node as DirectoryNode).ReloadChildren();
						NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadOutlineView", node);
						NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", node);
						UIErrorHelper.ShowInformation(VMDirConstants.STAT_OBJ_DEL_SUCC);
					}
					else {
						UIErrorHelper.ShowInformation(VMDirConstants.STAT_BASE_OBJ_DEL_SUCC);
					}
				});
			}
		}

		public void RestUserPassword(object sender, EventArgs e)
		{
			ResetPasswordWindowController cwc = new ResetPasswordWindowController(Dn);
			nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
			if (result == (nint)VMIdentityConstants.DIALOGOK)
			{
				UIErrorHelper.CheckedExec(delegate ()
				{
					LdapMod[] mod = new LdapMod[1];
					mod[0] = new LdapMod((int)LdapMod.mod_ops.LDAP_MOD_REPLACE, VMDirConstants.ATTR_USER_PASSWORD, new string[] {
						cwc.Password,
						null
					});
					ServerDTO.Connection.ModifyObject(cwc.Dn, mod);
					UIErrorHelper.ShowInformation(VMDirConstants.STAT_PWD_RESET_SUCC);
					ReloadChildren();
					NSNotificationCenter.DefaultCenter.PostNotificationName("ReloadTableView", this);
				});
			}
		}
	}
}

