/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using AppKit;
using Foundation;
using VmIdentity.UI.Common.Utilities;
using VMDir.Common.DTO;
using VmIdentity.UI.Common;
using VMIdentity.CommonUtils;
using VMDir.Common;

namespace VMDirSnapIn.UI
{
	public partial class ConnectToLdapWindowController : AppKit.NSWindowController
	{
		VMDirServerDTO _dto;

		public VMDirServerDTO ServerDTO { get { return _dto; } }

		#region Constructors

		// Called when created from unmanaged code
		public ConnectToLdapWindowController(IntPtr handle)
			: base(handle)
		{
		}

		// Called when created directly from a XIB file
		[Export("initWithCoder:")]
		public ConnectToLdapWindowController(NSCoder coder)
			: base(coder)
		{
		}

		// Call to load from the XIB/NIB file
		public ConnectToLdapWindowController(VMDirServerDTO dto)
			: base("ConnectToLdapWindow")
		{
			Initialize(dto);
		}

		// Shared initialization code
		void Initialize(VMDirServerDTO dto)
		{
			_dto = dto;
		}

		#endregion

		public override void AwakeFromNib()
		{
			base.AwakeFromNib();

			PrePopulateFields();
			AddEventListeners();
		}

		private void PrePopulateFields()
		{
			ServerName.StringValue = _dto.Server ?? "";
			var tenant = MiscUtil.GetBrandConfig(CommonConstants.TENANT);
			BindDN.StringValue = string.IsNullOrWhiteSpace(_dto.BindDN)?"Administrator@" + tenant:_dto.BindDN;
			BaseDN.StringValue = string.IsNullOrWhiteSpace(_dto.BaseDN)?CommonConstants.GetDNFormat(tenant):_dto.BaseDN;
		}

		private void AddEventListeners()
		{
			OKButton.Activated += OnClickOKButton;
			CancelButton.Activated += OnClickCancelButton;
		}

		//Event Handlers
		private void OnClickCancelButton(object sender, EventArgs e)
		{
			this.Close();
			NSApplication.SharedApplication.StopModalWithCode(0);
		}

		private void OnClickOKButton(object sender, EventArgs e)
		{
			FillDtoFromUIFields();
			UIErrorHelper.CheckedExec(delegate ()
				{
					if (!ValidateDto())
						return;
					this.Close();
					NSApplication.SharedApplication.StopModalWithCode(VmIdentity.UI.Common.VMIdentityConstants.DIALOGOK);
				});
		}

		private bool ValidateDto()
		{
			string msg = string.Empty;

			if (string.IsNullOrEmpty(_dto.Server))
				msg = VMDirConstants.WRN_SERVER_ENT;
			else if (string.IsNullOrEmpty(_dto.BaseDN))
				msg = VMDirConstants.WRN_BASE_DN_ENT;
			else if (string.IsNullOrEmpty(_dto.BindDN))
				msg = VMDirConstants.WRN_UPN_ENT;
			else if (string.IsNullOrEmpty(_dto.Password))
				msg = VMDirConstants.WRN_PWD_ENT;

			if (!string.IsNullOrWhiteSpace(msg))
			{
				UIErrorHelper.ShowWarning(msg);
				return false;
			}
			return true;
		}

		private void FillDtoFromUIFields()
		{
			_dto.BaseDN = BaseDN.StringValue;
			_dto.BindDN = BindDN.StringValue;
			_dto.Server = ServerName.StringValue;
			_dto.Password = Password.StringValue;
		}

		//strongly typed window accessor
		public new ConnectToLdapWindow Window
		{
			get
			{
				return (ConnectToLdapWindow)base.Window;
			}
		}
	}
}

