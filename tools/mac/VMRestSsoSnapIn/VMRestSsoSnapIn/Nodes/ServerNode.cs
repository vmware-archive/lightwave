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
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.IO;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using RestSsoAdminSnapIn;
using System.Threading.Tasks;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Service;

namespace Vmware.Tools.RestSsoAdminSnapIn.Nodes
{
	public class ServerNode : ScopeNode
	{
		public LoginDto LoginDto;
		private ServerDto _serverDto;
		private ServerInfoDto _serverInfo;
		private NSObject notificationObject;

		public bool IsLoggedIn{ get; set; }

		public ServerNode (ServerDto dto)
		{
			_serverDto = dto;
			this.DisplayName = dto.ServerName;
			Tag = new AuthTokenDto { ServerDto = dto };
			notificationObject = NSNotificationCenter.DefaultCenter.AddObserver ((NSString)"CloseApplication", OnCloseConnectionNotificationReceived);
		}

		public void DeleteServer (object sender, EventArgs e)
		{
			SnapInContext.Instance.AuthTokenManager.RemoveAuthToken (((AuthTokenDto)Tag).ServerDto.ServerName);
			ScopeNode parent = this.Parent;
			if (parent != null)
				parent.Children.Remove (this);
			NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", parent);
		}

		public void Login ()
		{
			LoginAsync ();
		}

		public void LoginAsync ()
		{	
			try {
				var service = SnapInContext.Instance.ServiceGateway;
				var authToken = service.Authentication.Login (((AuthTokenDto)Tag).ServerDto, LoginDto, Constants.ClientId);
				Tag = authToken;
				AddServiceGatewayForServer(service, authToken);
				var key = authToken.ServerDto.ServerName;// + "-" + _loginDto.TenantName;
				SnapInContext.Instance.AuthTokenManager.SetAuthToken (authToken, key);
				AddTenantNode (authToken, new TenantDto{ Name = LoginDto.TenantName });
				IsLoggedIn = true;
			}
			catch (WebException exp)
			{
				if (((AuthTokenDto)Tag).ServerDto.TokenType == TokenType.SAML) {
					if (exp != null && exp.Response != null) {
						var response = exp.Response as HttpWebResponse;
						var resp = new StreamReader (exp.Response.GetResponseStream ()).ReadToEnd ();
						UIErrorHelper.ShowAlert (resp, "Error");
						return;
					} else {
						UIErrorHelper.ShowAlert (exp.Message, "Error");
						return;
					}
				} else {
					if (exp.Response is HttpWebResponse) {
						var response = exp.Response as HttpWebResponse;
						if (response != null && response.StatusCode == HttpStatusCode.Unauthorized) {
							var resp = new StreamReader (exp.Response.GetResponseStream ()).ReadToEnd ();
							var error = JsonConvert.Deserialize<AuthErrorDto> (resp);
							if (error != null) {
								if (error.Error == AuthError.InvalidToken) {
									UIErrorHelper.ShowAlert ("Token Expired", "Error");
								} else {
									UIErrorHelper.ShowAlert (error.Details, "Error");
								}
							}
						} else {
							if (response != null && response.StatusCode == HttpStatusCode.BadRequest && response.ContentType == "application/json;charset=UTF-8") {
								var resp = new StreamReader (response.GetResponseStream ()).ReadToEnd ();
								var error = JsonConvert.Deserialize<AuthErrorDto> (resp);
								if (resp.Contains (AuthError.InvalidGrant)) {                               
									if (error != null) {                                  
										UIErrorHelper.ShowAlert ("Invalid username or password", "Error");
									} else {
										UIErrorHelper.ShowAlert (exp.Message + " Details: " + resp, "Error");
									}
								} else {
									UIErrorHelper.ShowAlert (exp.Message + " Details: " + resp, "Error");
								}
							} else if (response != null && response.ContentType == "application/json") {
								var resp = new StreamReader (response.GetResponseStream ()).ReadToEnd ();
								UIErrorHelper.ShowAlert (exp.Message + " Details: " + resp, "Error");
							} else {
								UIErrorHelper.ShowAlert (exp.Message, "Error");
							}
						}
					} else {
						UIErrorHelper.ShowAlert (exp.Message, "Error");
					}
				}
			}
			catch (Exception exp)
			{
				UIErrorHelper.ShowAlert(exp.Message, "Error");
			}
		}

		private void AddServiceGatewayForServer(ServiceGateway service, AuthTokenDto authToken)
		{
			try
			{
				_serverInfo = service.Server.GetServerInfo(authToken.ServerDto, authToken.Token);
			}
			catch (Exception exc)
			{
				// default the configuration to vsphere
				_serverInfo = new ServerInfoDto
				{
					Release = "Vsphere",
					ProductName = "idm"
				};
			}

			var serviceConfigManager = new ServiceConfigManager(_serverInfo.Release);
			var serviceGateway = new ServiceGateway(serviceConfigManager);
			SnapInContext.Instance.ServiceGateway = serviceGateway;
		}

		private void AddTenantNode (AuthTokenDto tokenDto, TenantDto tenant)
		{
			var node = new TenantNode (tokenDto.ServerDto, tenant.Name) {
				DisplayName = tenant.Name,
				Tag = tenant.Guid,
				Parent = this
			};
			node.Refresh (this, EventArgs.Empty);
			AddTenantNode (node);
		}

		private void AddTenantNode (TenantNode node)
		{
			var nodeWithSameNameExists = false;
			var index = 0;
			while (index < Children.Count) {
				var tenantNode = (TenantNode)Children [index++];
				if (tenantNode.DisplayName == node.DisplayName) {
					nodeWithSameNameExists = true;
					break;
				}
			}

			if (nodeWithSameNameExists)
				Children.RemoveAt (index - 1);
			Children.Add (node);
			NSNotificationCenter.DefaultCenter.PostNotificationName ("ReloadOutlineView", this);
		}

		public void OnAddNewTenant (object sender, EventArgs e)
		{	
			var form = new AddNewTenantController ();
			var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			if (result == VMIdentityConstants.DIALOGOK) {
				var rp = AddTenant (form.TenantDto);
				if (rp != null) {
					UIErrorHelper.ShowAlert ("Tenant " + rp.Name + " created successfully", "Information");
				}
			}
		}

		public void OnAddExistingTenant (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AddExistingTenantController ();
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
				if (result == VMIdentityConstants.DIALOGOK) {
					var service = SnapInContext.Instance.ServiceGateway;
					var tenantDto = form.TenantDto;
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (DisplayName);
					tenantDto = service.Tenant.Get (auth.ServerDto, tenantDto.Name, auth.Token);
					AddTenantNode (auth, new TenantDto{ Name = tenantDto.Name });
				}
			});
		}

		public void OnShowAbout (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new AboutServerController (_serverInfo);
				var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
			});
		}

		public void OnShowActiveDirectory (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (DisplayName);
				var adJoinInfoDto = SnapInContext.Instance.ServiceGateway.Adf.GetActiveDirectory (auth.ServerDto, auth.Token);
				if (adJoinInfoDto == null || (adJoinInfoDto != null && adJoinInfoDto.JoinStatus != "DOMAIN")) {
					var form = new JoinActiveDirectoryController ();
					var result = NSApplication.SharedApplication.RunModalForWindow (form.Window);
					if (result == VMIdentityConstants.DIALOGOK) {
						var user = JoinActiveDirectory (form.ActiveDirectoryJoinRequestDto);
						if (user != null) {
							UIErrorHelper.ShowAlert ("AD join operation was successful. Please reboot the node.", "Information");
						}
					}
				} else {
					var form = new LeaveActiveDirectoryController (){ ActiveDirectoryJoinInfoDto = adJoinInfoDto, Server = DisplayName };
					NSApplication.SharedApplication.RunModalForWindow (form.Window);
				}
			});
		}
		public void ShowTokenWizard (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var form = new TokenWizardController (){ServerDto = _serverDto};
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
			});
		}

		public void ShowHttpTransport (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				UIErrorHelper.ShowAlert ("ShowHttpTransport", "Alert");
			});
		}

		private ActiveDirectoryJoinInfoDto JoinActiveDirectory (ActiveDirectoryJoinRequestDto dto)
		{
			var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(DisplayName);
			return SnapInContext.Instance.ServiceGateway.Adf.JoinActiveDirectory(auth.ServerDto, dto, auth.Token);
		}


		public void OnShowComputers (object sender, EventArgs e)
		{
			ActionHelper.Execute (delegate() {
				var serverDto = ((AuthTokenDto)Tag).ServerDto;
				var form = new ShowComputersController ();
				form.ServerDto = serverDto;
				NSApplication.SharedApplication.RunModalForWindow (form.Window);
			});
		}

		public void OnLogout (object sender, EventArgs e)
		{
			Logout ();
		}

		public void Logout ()
		{
			ActionHelper.Execute (delegate() {
				
				var auth = (AuthTokenDto)Tag;
				this.Children.Clear ();
				auth.Token = null;
				auth.Login = null;
				IsLoggedIn = false;
				SnapInContext.Instance.AuthTokenManager.SetAuthToken (auth, auth.ServerDto.ServerName);
			});
		}

		public void OnCloseConnectionNotificationReceived (NSNotification notification)
		{
			Logout ();
			NSNotificationCenter.DefaultCenter.RemoveObserver (notificationObject);
		}

		public string DisplayTitle {
			get{ return string.Format ("{0}", DisplayName); }
		}

		public override string GetDisplayTitle ()
		{
			return string.Format ("{0}", DisplayName);
		}

		~ServerNode ()
		{
			Console.WriteLine ("destructor called");
		}

		private TenantDto AddTenant(TenantDto tenantDto)
		{
			var succcess = false;
			ActionHelper.Execute (delegate() {
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (DisplayName);
				tenantDto = SnapInContext.Instance.ServiceGateway.Tenant.Create (auth.ServerDto, tenantDto, auth.Token);
				succcess = true;
			});
			return succcess ? tenantDto: null;
		}
	}
}

