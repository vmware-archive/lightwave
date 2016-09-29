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
 
using Foundation;
using System.CodeDom.Compiler;

namespace RestSsoAdminSnapIn
{
	[Register ("ExternalDomainAdvancedSettingsController")]
	partial class ExternalDomainAdvancedSettingsController
	{
		[Outlet]
		AppKit.NSTableView AttributeMapTableView { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddDomainSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddGroupSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddPasswordSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnAddUserSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnApply { get; set; }

		[Outlet]
		AppKit.NSButton BtnBaseDnForNestedGroups { get; set; }

		[Outlet]
		AppKit.NSButton BtnClose { get; set; }

		[Outlet]
		AppKit.NSButton BtnGroupSearch { get; set; }

		[Outlet]
		AppKit.NSButton BtnMatchRuleInChain { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveDomainSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveGroupSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemovePasswordSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSButton BtnRemoveUserSchemaAttribute { get; set; }

		[Outlet]
		AppKit.NSTableView DomainAttributesTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox DomainList { get; set; }

		[Outlet]
		AppKit.NSComboBox GroupAttributes { get; set; }

		[Outlet]
		AppKit.NSTableView GroupAttributesTableView { get; set; }

		[Outlet]
		AppKit.NSComboBox PasswordAttributeList { get; set; }

		[Outlet]
		AppKit.NSTableView PasswordTableView { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAttributeName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtAttributeValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomainClassName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtDomainValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtGroupClassName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtGroupValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordClassName { get; set; }

		[Outlet]
		AppKit.NSTextField TxtPasswordValue { get; set; }

		[Outlet]
		AppKit.NSTextField TxtUserAttributeValue { get; set; }

		[Outlet]
		AppKit.NSTextField UserClassName { get; set; }

		[Outlet]
		AppKit.NSComboBox UsersAttributeList { get; set; }

		[Outlet]
		AppKit.NSTableView UsersMapTableView { get; set; }
		
		void ReleaseDesignerOutlets ()
		{
			if (UserClassName != null) {
				UserClassName.Dispose ();
				UserClassName = null;
			}

			if (TxtPasswordClassName != null) {
				TxtPasswordClassName.Dispose ();
				TxtPasswordClassName = null;
			}

			if (TxtDomainClassName != null) {
				TxtDomainClassName.Dispose ();
				TxtDomainClassName = null;
			}

			if (TxtGroupClassName != null) {
				TxtGroupClassName.Dispose ();
				TxtGroupClassName = null;
			}

			if (AttributeMapTableView != null) {
				AttributeMapTableView.Dispose ();
				AttributeMapTableView = null;
			}

			if (BtnAddAttribute != null) {
				BtnAddAttribute.Dispose ();
				BtnAddAttribute = null;
			}

			if (BtnAddDomainSchemaAttribute != null) {
				BtnAddDomainSchemaAttribute.Dispose ();
				BtnAddDomainSchemaAttribute = null;
			}

			if (BtnAddGroupSchemaAttribute != null) {
				BtnAddGroupSchemaAttribute.Dispose ();
				BtnAddGroupSchemaAttribute = null;
			}

			if (BtnAddPasswordSchemaAttribute != null) {
				BtnAddPasswordSchemaAttribute.Dispose ();
				BtnAddPasswordSchemaAttribute = null;
			}

			if (BtnAddUserSchemaAttribute != null) {
				BtnAddUserSchemaAttribute.Dispose ();
				BtnAddUserSchemaAttribute = null;
			}

			if (BtnApply != null) {
				BtnApply.Dispose ();
				BtnApply = null;
			}

			if (BtnBaseDnForNestedGroups != null) {
				BtnBaseDnForNestedGroups.Dispose ();
				BtnBaseDnForNestedGroups = null;
			}

			if (BtnClose != null) {
				BtnClose.Dispose ();
				BtnClose = null;
			}

			if (BtnGroupSearch != null) {
				BtnGroupSearch.Dispose ();
				BtnGroupSearch = null;
			}

			if (BtnMatchRuleInChain != null) {
				BtnMatchRuleInChain.Dispose ();
				BtnMatchRuleInChain = null;
			}

			if (BtnRemoveAttribute != null) {
				BtnRemoveAttribute.Dispose ();
				BtnRemoveAttribute = null;
			}

			if (BtnRemoveDomainSchemaAttribute != null) {
				BtnRemoveDomainSchemaAttribute.Dispose ();
				BtnRemoveDomainSchemaAttribute = null;
			}

			if (BtnRemoveGroupSchemaAttribute != null) {
				BtnRemoveGroupSchemaAttribute.Dispose ();
				BtnRemoveGroupSchemaAttribute = null;
			}

			if (BtnRemovePasswordSchemaAttribute != null) {
				BtnRemovePasswordSchemaAttribute.Dispose ();
				BtnRemovePasswordSchemaAttribute = null;
			}

			if (BtnRemoveUserSchemaAttribute != null) {
				BtnRemoveUserSchemaAttribute.Dispose ();
				BtnRemoveUserSchemaAttribute = null;
			}

			if (DomainAttributesTableView != null) {
				DomainAttributesTableView.Dispose ();
				DomainAttributesTableView = null;
			}

			if (DomainList != null) {
				DomainList.Dispose ();
				DomainList = null;
			}

			if (GroupAttributes != null) {
				GroupAttributes.Dispose ();
				GroupAttributes = null;
			}

			if (GroupAttributesTableView != null) {
				GroupAttributesTableView.Dispose ();
				GroupAttributesTableView = null;
			}

			if (PasswordAttributeList != null) {
				PasswordAttributeList.Dispose ();
				PasswordAttributeList = null;
			}

			if (PasswordTableView != null) {
				PasswordTableView.Dispose ();
				PasswordTableView = null;
			}

			if (TxtAttributeName != null) {
				TxtAttributeName.Dispose ();
				TxtAttributeName = null;
			}

			if (TxtAttributeValue != null) {
				TxtAttributeValue.Dispose ();
				TxtAttributeValue = null;
			}

			if (TxtDomainValue != null) {
				TxtDomainValue.Dispose ();
				TxtDomainValue = null;
			}

			if (TxtGroupValue != null) {
				TxtGroupValue.Dispose ();
				TxtGroupValue = null;
			}

			if (TxtPasswordValue != null) {
				TxtPasswordValue.Dispose ();
				TxtPasswordValue = null;
			}

			if (TxtUserAttributeValue != null) {
				TxtUserAttributeValue.Dispose ();
				TxtUserAttributeValue = null;
			}

			if (UsersAttributeList != null) {
				UsersAttributeList.Dispose ();
				UsersAttributeList = null;
			}

			if (UsersMapTableView != null) {
				UsersMapTableView.Dispose ();
				UsersMapTableView = null;
			}
		}
	}
}
