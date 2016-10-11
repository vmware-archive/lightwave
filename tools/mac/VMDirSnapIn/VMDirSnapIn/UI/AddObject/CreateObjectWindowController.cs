/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using AppKit;
using Foundation;
using VMDirSnapIn.DataSource;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VMDir.Common;
using VMDirInterop.LDAP;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn.UI
{
	public partial class CreateObjectWindowController : AppKit.NSWindowController
	{
		private string _objectClass;
		private VMDirServerDTO _serverDTO;
		public Dictionary<string, VMDirAttributeDTO> _properties;
		private CreateObjectTableViewDataSource ds;
		private string _parentDn;
		public string Rdn;

		#region Constructors

		// Called when created from unmanaged code
		public CreateObjectWindowController(IntPtr handle)
			: base(handle)
		{
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public CreateObjectWindowController(NSCoder coder)
			: base(coder)
		{
		}

		// Call to load from the XIB/NIB file
		public CreateObjectWindowController()
			: base("CreateObjectWindow")
		{
		}

		// Call to load from the XIB/NIB file
		public CreateObjectWindowController(string objectClass, VMDirServerDTO serverDTO, string parentDn)
			: base("CreateObjectWindow")
		{
			_objectClass = objectClass;
			_serverDTO = serverDTO;
			_parentDn = parentDn;
			Bind();
		}

		#endregion

		private void Bind()
		{
			var requiredProps = _serverDTO.Connection.SchemaManager.GetRequiredAttributes(_objectClass);
			_properties = new Dictionary<string, VMDirAttributeDTO>();
			foreach (var prop in requiredProps)
			{
				VMDirAttributeDTO dto = new VMDirAttributeDTO(prop.Name, new List<VMDirInterop.LDAP.LdapValue>(), prop);
				_properties.Add(prop.Name, dto);

			}
			var oc = _properties[VMDirConstants.ATTR_OBJECT_CLASS];
			LdapValue val = new LdapValue(_objectClass);
			oc.Values = new List<LdapValue>() { val };
			Utilities.RemoveDontShowAttributes(_properties);
		}

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();
			try
			{
				this.ParentDnTextField.StringValue = _parentDn;
				ds = new CreateObjectTableViewDataSource(_properties);
				this.PropertiesTableView.DataSource = ds;
				NSTableColumn col;
				col = this.PropertiesTableView.TableColumns()[0];
				if (col != null)
					col.DataCell = new NSBrowserCell();
				this.PropertiesTableView.Delegate = new NSTableViewDelegate();
				this.CreateButton.Activated += OnClickCreateButton;
				this.CancelButton.Activated += OnClickCancelButton;
			}
			catch (Exception e)
			{
				System.Diagnostics.Debug.WriteLine("Error " + e.Message);
			}
		}

		private void OnClickCancelButton(object sender, EventArgs e)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		private void OnClickCreateButton(object sender, EventArgs e)
		{
			if (DoValidate())
			{
				Rdn = this.RdnTextField.StringValue;
				this.Close();
				NSApplication.SharedApplication.StopModalWithCode(1);
			}
		}

		private bool DoValidate()
		{
			if (string.IsNullOrWhiteSpace(ParentDnTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_DN_ENT);
				return false;
			}
			if (string.IsNullOrWhiteSpace(RdnTextField.StringValue))
			{
				UIErrorHelper.ShowWarning(VMDirConstants.WRN_RDN_ENT);
				return false;
			}
			var requiredPropsNotFilled = _properties.Where(x =>
				{
					var val = x.Value.Values;
					if (val == null)
						return true;
					else if (val.Count <= 0)
						return true;
					else {
						var flag = false;
						foreach (LdapValue item in val)
							if (string.IsNullOrEmpty(item.StringValue))
								flag = true;
						return flag;
					}
				});
			if (requiredPropsNotFilled.Count() > 0)
			{
				string error = string.Format("{0} is empty", requiredPropsNotFilled.First().Key);
				UIErrorHelper.ShowWarning(error);
				return false;
			}
			return true;
		}

		[Export("windowWillClose:")]
		public void WindowWillClose(NSNotification notification)
		{
			NSApplication.SharedApplication.StopModal();
		}

		//strongly typed window accessor
		public new CreateObjectWindow Window
		{
			get
			{
				return (CreateObjectWindow)base.Window;
			}
		}
	}
}

