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
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using VmIdentity.UI.Common;
using VMIdentity.CommonUtils;

namespace RestSsoAdminSnapIn
{
	public partial class AddNewServerController : NSWindowController
	{
		public ServerDto ServerDto { get; set; }
		public LoginDto LoginDto { get; private set; }
		private bool _changeServer;

		#region Constructors

		// Called when created from unmanaged code
		public AddNewServerController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddNewServerController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddNewServerController () : base ("AddNewServer")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//Events
			this.BtnAdd.Activated += OnClickAddButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			this.CbSaml.Activated += (object sender, EventArgs e) => {
				var legacy = this.CbSaml.StringValue == "1";
				this.TxtStsEndpoint.Hidden = !legacy;
				this.LblStsEndpoint.Hidden = !legacy;
				SetUrl(this,EventArgs.Empty);
			};
			this.CbSsl.Activated += SetUrl;
			this.TxtServer.Changed += SetUrl;
			this.TxtPort.Changed += SetUrl;
			this.TxtStsEndpoint.Changed += SetUrl;
			this.TxtTenant.Changed += SetUrl;
			this.TxtStsEndpoint.Changed += SetUrl;

			_changeServer = ServerDto != null && !string.IsNullOrEmpty(ServerDto.ServerName);
			if (_changeServer) {
				this.TxtServer.StringValue = ServerDto.ServerName;
				this.TxtPort.StringValue = ServerDto.Port;
				this.TxtTenant.StringValue = ServerDto.Tenant;
				this.TxtStsEndpoint.StringValue = string.IsNullOrEmpty(ServerDto.StsUrl) ?  "sts/STSService" : ServerDto.StsUrl;
				this.CbSaml.StringValue = ServerDto.TokenType == TokenType.SAML ? "1" : "0";
				this.LblStsEndpoint.Hidden = ServerDto.TokenType == TokenType.Bearer;
				this.TxtStsEndpoint.Hidden = ServerDto.TokenType == TokenType.Bearer;
			} else {
				this.TxtStsEndpoint.StringValue = "sts/STSService";
				this.TxtPort.StringValue = "443";
				this.TxtTenant.StringValue = MiscUtil.GetBrandConfig(CommonConstants.TENANT);
				this.CbSaml.StringValue = "0";
				this.LblStsEndpoint.Hidden = true;
				this.TxtStsEndpoint.Hidden = true;
			}
			SetUrl (this, EventArgs.Empty);
		}

		public void SetUrl (object sender, EventArgs e)
		{
			var legacy = this.CbSaml.StringValue == "1";
			var serverDto = new ServerDto () {
				ServerName = TxtServer.StringValue,
				Port = TxtPort.StringValue,
				Tenant = TxtTenant.StringValue,
				Protocol = CbSsl.StringValue == "1" ? "https" : "http",
				TokenType =  legacy ? TokenType.SAML : TokenType.Bearer,
				StsUrl = TxtStsEndpoint.StringValue
			};
			LblUrl.StringValue = legacy ?
				SnapInContext.Instance.ServiceGateway.GetTenantEndpoint (legacy, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.StsUrl + "/" + serverDto.Tenant)
				:SnapInContext.Instance.ServiceGateway.GetTenantEndpoint (legacy, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
		}

		public void OnClickAddButton (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				if (string.IsNullOrEmpty (TxtServer.StringValue)) {
					UIErrorHelper.ShowAlert ("Server name cannot be empty", "Alert");
				} else if (!WebUtil.PingHost (TxtServer.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid server name or ip address", "Alert");
				} else if (string.IsNullOrEmpty (TxtPort.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid server STS port", "Alert");
				} else if (string.IsNullOrEmpty (TxtTenant.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid tenant name", "Alert");
				} else if (this.CbSaml.StringValue == "1" && string.IsNullOrEmpty (TxtStsEndpoint.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid STS endpoint", "Alert");
				} else if (string.IsNullOrEmpty (TxtUsername.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
				} else if (string.IsNullOrEmpty (TxtPassword.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid password", "Alert");
				} else {
					var legacy = this.CbSaml.StringValue == "1";
					ServerDto = new ServerDto () {
						ServerName = TxtServer.StringValue,
						Port = TxtPort.StringValue,
						Tenant = TxtTenant.StringValue,
						Protocol = CbSsl.StringValue == "1" ? "https" : "http",
						TokenType = legacy ? TokenType.SAML : TokenType.Bearer,
						Url = LblUrl.StringValue,
						StsUrl = string.IsNullOrEmpty(TxtStsEndpoint.StringValue) ? string.Empty : TxtStsEndpoint.StringValue
					};

					LoginDto = new LoginDto {
						User = TxtUsername.StringValue,
						Pass = TxtPassword.StringValue,
						DomainName = TxtTenant.StringValue,
						TenantName = TxtTenant.StringValue
					};
					NSApplication.SharedApplication.StopModalWithCode (1);
				}
			});
		}

		//strongly typed window accessor
		public new AddNewServer Window {
			get {
				return (AddNewServer)base.Window;
			}
		}
	}
}

