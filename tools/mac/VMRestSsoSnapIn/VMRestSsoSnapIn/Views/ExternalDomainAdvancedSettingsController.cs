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
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.DataSource;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;

namespace RestSsoAdminSnapIn
{
	public partial class ExternalDomainAdvancedSettingsController : NSWindowController
	{
		public IdentityProviderDto IdentityProviderDto; 
		public ExternalDomainAdvancedSettingsController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public ExternalDomainAdvancedSettingsController (NSCoder coder) : base (coder)
		{
		}

		public ExternalDomainAdvancedSettingsController () : base ("ExternalDomainAdvancedSettings")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};


			// Attributes
			BtnAddAttribute.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtAttributeName.StringValue))
				{
					UIErrorHelper.ShowAlert ("Attribute name cannot be empty", "Alert");
					return;
				} else if(string.IsNullOrEmpty(TxtAttributeValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("Attribute value cannot be empty", "Alert");
					return;
				}
				IdentityProviderDto.AttributesMap.Add(TxtAttributeName.StringValue, TxtAttributeValue.StringValue);
				ReloadTableView(AttributeMapTableView, IdentityProviderDto.AttributesMap);
				TxtAttributeName.StringValue = (NSString)string.Empty;
				TxtAttributeValue.StringValue = (NSString)string.Empty;
			};

			BtnRemoveAttribute.Activated += (object sender, EventArgs e) => {
				if (AttributeMapTableView.SelectedRows.Count > 0) {
					foreach (var index in AttributeMapTableView.SelectedRows) {
						var ds = (AttributeMapTableView.DataSource) as DictionaryDataSource;
						if(ds != null)
						{
							var entry = ds.Entries[(int)index];
							ds.Datasource.Remove(entry);
							ds.Entries.RemoveAt((int)index);
						}
					}
					ReloadTableView(AttributeMapTableView, IdentityProviderDto.AttributesMap);
				}
			};

			if (IdentityProviderDto.AttributesMap == null)
				IdentityProviderDto.AttributesMap = new Dictionary<string, string> ();

			if (IdentityProviderDto.Schema == null)
				IdentityProviderDto.Schema = new Dictionary<string, SchemaObjectMappingDto> ();
			
			ReloadTableView(AttributeMapTableView, IdentityProviderDto.AttributesMap);

			// User Schema
			BtnAddUserSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtUserAttributeValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("User schema attribute name cannot be empty", "Alert");
					return;
				} else if(((int)UsersAttributeList.SelectedIndex) < 0)
				{
					UIErrorHelper.ShowAlert ("User schema attribute value cannot be empty", "Alert");
					return;
				}
				var key = ObjectId.ObjectIdUser.ToString();
				var ds = (UsersMapTableView.DataSource) as DictionaryDataSource;
				if(ds != null && ds.Entries.Contains(UsersAttributeList.SelectedValue.ToString()))
				{
					UIErrorHelper.ShowAlert ("User schema attribute by this name already exists.", "Alert");
					return;
				}
				IdentityProviderDto.Schema[key].AttributeMappings.Add(UsersAttributeList.SelectedValue.ToString(), TxtUserAttributeValue.StringValue);

				ReloadTableView(UsersMapTableView, IdentityProviderDto.Schema[key].AttributeMappings);
				TxtUserAttributeValue.StringValue = (NSString)string.Empty;
				UsersAttributeList.SelectItem((nint) (-1));
			};

			BtnRemoveUserSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if (UsersMapTableView.SelectedRows.Count > 0) {
					var ds = (UsersMapTableView.DataSource) as DictionaryDataSource;
					var index  =  UsersMapTableView.SelectedRows.First();
					var entry = ds.Entries[(int)index];
					var d = ObjectId.ObjectIdUser.ToString();
					IdentityProviderDto.Schema[d].AttributeMappings.Remove(entry);
					ReloadTableView(UsersMapTableView, IdentityProviderDto.Schema[d].AttributeMappings);
				}
			};
			var desc = ObjectId.ObjectIdUser.ToString();
			if(!IdentityProviderDto.Schema.ContainsKey(desc))
			{
				IdentityProviderDto.Schema.Add(desc, new SchemaObjectMappingDto { 
					AttributeMappings =  new Dictionary<string,string>()
				});
			}
			else {

				var attribMap = new Dictionary<string,string> ();
				foreach (var item in IdentityProviderDto.Schema[desc].AttributeMappings) {
					UserAttributeId p;
					if(Enum.TryParse (item.Key, out p))
					{
						attribMap.Add (p.GetDescription (), item.Value);
					}
				}
				IdentityProviderDto.Schema[desc].AttributeMappings = attribMap;
			}
			ReloadTableView(UsersMapTableView, IdentityProviderDto.Schema[desc].AttributeMappings);

			// Password Schema
			BtnAddPasswordSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtPasswordValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("Password schema attribute name cannot be empty", "Alert");
					return;
				} else if(((int)PasswordAttributeList.SelectedIndex) < 0)
				{
					UIErrorHelper.ShowAlert ("Password schema attribute value cannot be empty", "Alert");
					return;
				}
				var key = ObjectId.ObjectIdPasswordSettings.ToString();
				var ds = (PasswordTableView.DataSource) as DictionaryDataSource;
				if(ds != null && ds.Entries.Contains(PasswordAttributeList.SelectedValue.ToString()))
				{
					UIErrorHelper.ShowAlert ("Password schema attribute by this name already exists.", "Alert");
					return;
				}
				IdentityProviderDto.Schema[key].AttributeMappings.Add(PasswordAttributeList.SelectedValue.ToString(), TxtPasswordValue.StringValue);

				ReloadTableView(PasswordTableView, IdentityProviderDto.Schema[key].AttributeMappings);
				TxtPasswordValue.StringValue = (NSString)string.Empty;
				PasswordAttributeList.SelectItem((nint) (-1));
			};

			BtnRemovePasswordSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if (PasswordTableView.SelectedRows.Count > 0) {
					var ds = (PasswordTableView.DataSource) as DictionaryDataSource;
					var index  =  PasswordTableView.SelectedRows.First();
					var entry = ds.Entries[(int)index];
					var d = ObjectId.ObjectIdPasswordSettings.ToString();
					IdentityProviderDto.Schema[d].AttributeMappings.Remove(entry);
					ReloadTableView(PasswordTableView, IdentityProviderDto.Schema[d].AttributeMappings);
				}
			};
			var desc1 = ObjectId.ObjectIdPasswordSettings.ToString();
			if (!IdentityProviderDto.Schema.ContainsKey (desc1)) {
				IdentityProviderDto.Schema.Add (desc1, new SchemaObjectMappingDto { 
					AttributeMappings = new Dictionary<string,string> ()
				});
			} else {

				var attribMap = new Dictionary<string,string> ();
				foreach (var item in IdentityProviderDto.Schema[desc1].AttributeMappings) {
					PasswordAttributeId p;
					if(Enum.TryParse (item.Key, out p))
					{
						attribMap.Add (p.GetDescription (), item.Value);
					}
				}
				IdentityProviderDto.Schema [desc1].AttributeMappings = attribMap;
			}
			ReloadTableView(PasswordTableView, IdentityProviderDto.Schema[desc1].AttributeMappings);


			// Group Schema
			BtnAddGroupSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtGroupValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("Group schema attribute name cannot be empty", "Alert");
					return;
				} else if(((int)GroupAttributes.SelectedIndex) < 0)
				{
					UIErrorHelper.ShowAlert ("Group schema attribute value cannot be empty", "Alert");
					return;
				}
				var key = ObjectId.ObjectIdGroup.ToString();
				var ds = (GroupAttributesTableView.DataSource) as DictionaryDataSource;
				if(ds != null && ds.Entries.Contains(GroupAttributes.SelectedValue.ToString()))
				{
					UIErrorHelper.ShowAlert ("Group schema attribute by this name already exists.", "Alert");
					return;
				}
				IdentityProviderDto.Schema[key].AttributeMappings.Add(GroupAttributes.SelectedValue.ToString(), TxtGroupValue.StringValue);

				ReloadTableView(GroupAttributesTableView, IdentityProviderDto.Schema[key].AttributeMappings);
				TxtGroupValue.StringValue = (NSString)string.Empty;
				GroupAttributes.SelectItem((nint) (-1));
			};

			BtnRemoveGroupSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if (GroupAttributesTableView.SelectedRows.Count > 0) {
					var ds = (GroupAttributesTableView.DataSource) as DictionaryDataSource;
					var index  =  GroupAttributesTableView.SelectedRows.First();
					var entry = ds.Entries[(int)index];
					var d = ObjectId.ObjectIdGroup.ToString();
					IdentityProviderDto.Schema[d].AttributeMappings.Remove(entry);
					ReloadTableView(GroupAttributesTableView, IdentityProviderDto.Schema[d].AttributeMappings);
				}
			};
			var desc2 = ObjectId.ObjectIdGroup.ToString();
			if(!IdentityProviderDto.Schema.ContainsKey(desc2))
			{
				IdentityProviderDto.Schema.Add(desc2, new SchemaObjectMappingDto { 
					AttributeMappings =  new Dictionary<string,string>()
				});
			}
			else {

				var attribMap = new Dictionary<string,string> ();
				foreach (var item in IdentityProviderDto.Schema[desc2].AttributeMappings) {
					GroupAttributeId p;
					if(Enum.TryParse (item.Key, out p))
					{
						attribMap.Add (p.GetDescription (), item.Value);
					}
				}
				IdentityProviderDto.Schema [desc2].AttributeMappings = attribMap;
			}
			ReloadTableView(GroupAttributesTableView, IdentityProviderDto.Schema[desc2].AttributeMappings);

			// Domain Schema
			BtnAddDomainSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if(string.IsNullOrEmpty(TxtDomainValue.StringValue))
				{
					UIErrorHelper.ShowAlert ("Domain schema attribute name cannot be empty", "Alert");
					return;
				} else if(((int)DomainList.SelectedIndex) < 0)
				{
					UIErrorHelper.ShowAlert ("Domain schema attribute value cannot be empty", "Alert");
					return;
				}
				var key = ObjectId.ObjectIdDomain.ToString();
				var ds = (DomainAttributesTableView.DataSource) as DictionaryDataSource;
				if(ds != null && ds.Entries.Contains(DomainList.SelectedValue.ToString()))
				{
					UIErrorHelper.ShowAlert ("Domain schema attribute by this name already exists.", "Alert");
					return;
				}
				IdentityProviderDto.Schema[key].AttributeMappings.Add(DomainList.SelectedValue.ToString(), TxtDomainValue.StringValue);
				ReloadTableView(DomainAttributesTableView, IdentityProviderDto.Schema[key].AttributeMappings);
				TxtDomainValue.StringValue = (NSString)string.Empty;
				DomainList.SelectItem((nint) (-1));
			};

			BtnRemoveDomainSchemaAttribute.Activated += (object sender, EventArgs e) => {
				if (DomainAttributesTableView.SelectedRows.Count > 0) {
					var ds = (DomainAttributesTableView.DataSource) as DictionaryDataSource;
					var index  =  DomainAttributesTableView.SelectedRows.First();
					var entry = ds.Entries[(int)index];
					var d = ObjectId.ObjectIdDomain.ToString();
					IdentityProviderDto.Schema[d].AttributeMappings.Remove(entry);
					ReloadTableView(DomainAttributesTableView, IdentityProviderDto.Schema[d].AttributeMappings);
				}
			};
			var desc3 = ObjectId.ObjectIdDomain.ToString();
			if(!IdentityProviderDto.Schema.ContainsKey(desc3))
			{
				IdentityProviderDto.Schema.Add(desc3, new SchemaObjectMappingDto { 
					AttributeMappings =  new Dictionary<string,string>()
				});
			}
			else {

				var attribMap = new Dictionary<string,string> ();
				foreach (var item in IdentityProviderDto.Schema[desc3].AttributeMappings) {
					DomainAttributeId p;
					if(Enum.TryParse (item.Key, out p))
					{
						attribMap.Add (p.GetDescription (), item.Value);
					}
				}
				IdentityProviderDto.Schema [desc3].AttributeMappings = attribMap;
			}
			ReloadTableView(DomainAttributesTableView, IdentityProviderDto.Schema[desc3].AttributeMappings);

			this.BtnApply.Activated += (object sender, EventArgs e) => {

				if(IsValid())
				{
					IdentityProviderDto.BaseDnForNestedGroupsEnabled = BtnBaseDnForNestedGroups.StringValue == "1";
					IdentityProviderDto.DirectGroupsSearchEnabled = BtnGroupSearch.StringValue == "1";
					IdentityProviderDto.MatchingRuleInChainEnabled = BtnMatchRuleInChain.StringValue == "1";

					var user = ObjectId.ObjectIdUser.ToString ();
					var pass = ObjectId.ObjectIdPasswordSettings.ToString();
					var grp = ObjectId.ObjectIdGroup.ToString();
					var dmn = ObjectId.ObjectIdDomain.ToString();
					IdentityProviderDto.Schema [user].ObjectClass = UserClassName.StringValue;
					IdentityProviderDto.Schema[pass].ObjectClass = TxtPasswordClassName.StringValue;
					IdentityProviderDto.Schema[grp].ObjectClass = TxtGroupClassName.StringValue;
					IdentityProviderDto.Schema[dmn].ObjectClass = TxtDomainClassName.StringValue;

					var schema = new Dictionary<string,SchemaObjectMappingDto>();

					if(IdentityProviderDto.Schema[user].AttributeMappings.Count > 0)
						schema.Add(user,IdentityProviderDto.Schema[user]);
					if(IdentityProviderDto.Schema[pass].AttributeMappings.Count > 0)
						schema.Add(pass,IdentityProviderDto.Schema[pass]);
					if(IdentityProviderDto.Schema[grp].AttributeMappings.Count > 0)
						schema.Add(grp,IdentityProviderDto.Schema[grp]);
					if(IdentityProviderDto.Schema[dmn].AttributeMappings.Count > 0)
						schema.Add(dmn,IdentityProviderDto.Schema[dmn]);

					IdentityProviderDto.Schema = new Dictionary<string, SchemaObjectMappingDto>(schema);

					this.Close ();
					NSApplication.SharedApplication.StopModalWithCode (1);
				}
			};

			if(IdentityProviderDto.AttributesMap == null)
				IdentityProviderDto.AttributesMap = new Dictionary<string, string>(); 
			if(IdentityProviderDto.Schema == null)
				IdentityProviderDto.Schema = new Dictionary<string, SchemaObjectMappingDto>();
			DtoToView ();
		}

		private bool IsValid()
		{
			if(IdentityProviderDto.Schema[ObjectId.ObjectIdUser.ToString()].AttributeMappings.Count > 0 && 
				string.IsNullOrEmpty(UserClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("User class name cannot be empty", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdPasswordSettings.ToString()].AttributeMappings.Count > 0 && 
				string.IsNullOrEmpty(TxtPasswordClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Password class name cannot be empty", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdDomain.ToString()].AttributeMappings.Count > 0 && 
				string.IsNullOrEmpty(TxtDomainClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Domain class name cannot be empty", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdGroup.ToString()].AttributeMappings.Count > 0 && 
				string.IsNullOrEmpty(TxtGroupClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Group class name cannot be empty", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdUser.ToString()].AttributeMappings.Count == 0 && 
				!string.IsNullOrEmpty(UserClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("User class name cannot exist without any schema mapping", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdPasswordSettings.ToString()].AttributeMappings.Count == 0 && 
				!string.IsNullOrEmpty(TxtPasswordClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Password class name cannot exist without any schema mapping", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdDomain.ToString()].AttributeMappings.Count == 0 && 
				!string.IsNullOrEmpty(TxtDomainClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Domain class name cannot exist without any schema mapping", "Alert");
				return false;
			} else if(IdentityProviderDto.Schema[ObjectId.ObjectIdGroup.ToString()].AttributeMappings.Count == 0 && 
				!string.IsNullOrEmpty(TxtGroupClassName.StringValue))
			{
				UIErrorHelper.ShowAlert ("Group class name cannot exist without any schema mapping", "Alert");
				return false;
			}
			return true;
		}

		private void ReloadTableView(NSTableView tableView, Dictionary<string,string> datasource)
		{
			foreach(NSTableColumn column in tableView.TableColumns())
			{
				tableView.RemoveColumn (column);
			}
			tableView.Delegate = new TableDelegate ();
			var columnNames = new List<ColumnOptions> {
				new ColumnOptions{ Id = "Name", DisplayName = "Name", DisplayOrder = 1, Width = 150 },
				new ColumnOptions{ Id = "Value", DisplayName = "Value", DisplayOrder = 2, Width = 210 }
			};
			var columns = ListViewHelper.ToNSTableColumns (columnNames);
			foreach (var column in columns) {
				tableView.AddColumn (column);
			}
			var listView = new DictionaryDataSource { Entries = datasource.Keys.ToList() , Datasource = datasource};
			tableView.DataSource = listView;
			tableView.ReloadData ();
		}

		private void DtoToView()
		{
			BtnBaseDnForNestedGroups.StringValue = IdentityProviderDto.BaseDnForNestedGroupsEnabled ? "1" : "0";
			BtnGroupSearch.StringValue = IdentityProviderDto.DirectGroupsSearchEnabled ? "1" : "0";
			BtnMatchRuleInChain.StringValue = IdentityProviderDto.MatchingRuleInChainEnabled ? "1" : "0";

			UserClassName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Schema [ObjectId.ObjectIdUser.ToString()].ObjectClass) ? string.Empty : IdentityProviderDto.Schema [ObjectId.ObjectIdUser.ToString()].ObjectClass;
			TxtGroupClassName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Schema [ObjectId.ObjectIdGroup.ToString()].ObjectClass) ? string.Empty : IdentityProviderDto.Schema [ObjectId.ObjectIdGroup.ToString()].ObjectClass;
			TxtDomainClassName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Schema [ObjectId.ObjectIdDomain.ToString()].ObjectClass) ? string.Empty : IdentityProviderDto.Schema [ObjectId.ObjectIdDomain.ToString()].ObjectClass;
			TxtPasswordClassName.StringValue = string.IsNullOrEmpty (IdentityProviderDto.Schema [ObjectId.ObjectIdPasswordSettings.ToString()].ObjectClass) ? string.Empty : IdentityProviderDto.Schema [ObjectId.ObjectIdPasswordSettings.ToString()].ObjectClass;
		}

		public new ExternalDomainAdvancedSettings Window {
			get { return (ExternalDomainAdvancedSettings)base.Window; }
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
					NSBrowserCell browserCell = cell as NSBrowserCell;
					if (browserCell != null) {
						browserCell.Leaf = true;
					}
				});
			}
		}
	}
}
