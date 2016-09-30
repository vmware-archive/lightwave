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
using System.IO;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Crypto;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;

namespace RestSsoAdminSnapIn
{
	public partial class TokenWizardController : AppKit.NSWindowController
	{
		public ServerDto ServerDto;

		#region Constructors

		// Called when created from unmanaged code
		public TokenWizardController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public TokenWizardController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public TokenWizardController () : base ("TokenWizard")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();

			//Events
			this.BtnAcquireToken.Activated += OnClickAddButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
			this.BtnSelectCertificate.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";

				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);
					var cert1 = new X509Certificate2 ();
					ActionHelper.Execute (delegate() {

						cert1.Import (filePath);
						TxtCertificate.StringValue =  filePath;
					});
				}
			};
			this.BtnSelectPrivateKey.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";
				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);

					ActionHelper.Execute (delegate() {
						TxtPrivateKey.StringValue = filePath;
					});
				}
			};
			this.BtnBrowseTokenFile.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";
				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);
					ActionHelper.Execute (delegate() {
						TxtTokenFile.StringValue = filePath;
					});
				}
			};
			this.CbSaml.Activated += (object sender, EventArgs e) => {
				var legacy = this.CbSaml.StringValue == "1";
				this.TxtStsUrl.Hidden = !legacy;
				this.LblStsUrl.Hidden = !legacy;
				SetUrl(this,EventArgs.Empty);
				PnlJwt.Hidden = legacy;
				PnlSaml.Hidden = !legacy;
			};
			this.RdoTypeGroup.Activated += (object sender, EventArgs e) => {
				SetGroupControls();
			};
			this.TxtServer.Changed += SetUrl;
			this.TxtPort.Changed += SetUrl;
			this.TxtStsUrl.Changed += SetUrl;
			this.TxtTenant.Changed += SetUrl;
			this.CbSsl.Activated += SetUrl;
			var saml = false;
			this.RdoTypeGroup.SelectCellWithTag (1);
			SetGroupControls ();

			if (ServerDto != null) {
				this.TxtServer.StringValue = ServerDto.ServerName;
				this.TxtStsUrl.StringValue = ServerDto.TokenType == TokenType.SAML ? (string.IsNullOrEmpty(ServerDto.StsUrl) ? "sts/STSService" : ServerDto.StsUrl) : "sts/STSService" ;
				this.TxtPort.StringValue = ServerDto.Port;
				this.TxtTenant.StringValue = ServerDto.Tenant;
				saml = ServerDto.TokenType == TokenType.SAML;
				var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
				if (auth != null && auth.Login != null) {
					TxtUsername.StringValue = auth.Login.User;
					TxtDomain.StringValue = auth.Login.DomainName;
					this.TxtTenant.StringValue = auth.Login.DomainName;
				}
			}
			this.CbSaml.StringValue = saml ? "1" : "0";
			PnlJwt.Hidden = saml;
			PnlSaml.Hidden = !saml;
			TxtStsUrl.Hidden = !saml;
			LblStsUrl.Hidden = !saml;
			SetUrl (this,EventArgs.Empty);
		}

		private void SetGroupControls()
		{	
			var userCredentialOption = RdoTypeGroup.SelectedTag == 1;
			var certificateOption = RdoTypeGroup.SelectedTag == 2;
			var tokenOption = CbSaml.StringValue == "1" && RdoTypeGroup.SelectedTag == 3;
			var certOrtokenOption = certificateOption || tokenOption;
			TxtUsername.Enabled = userCredentialOption;
			TxtPassword.Enabled = userCredentialOption;
			TxtDomain.Enabled = userCredentialOption;
			TxtCertificate.Enabled =  certOrtokenOption;
			BtnSelectCertificate.Enabled = certOrtokenOption ;
			BtnSelectPrivateKey.Enabled = certOrtokenOption;
			TxtPrivateKey.Enabled = certOrtokenOption;
			TxtTokenFile.Enabled = tokenOption;
			BtnBrowseTokenFile.Enabled = tokenOption;
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
				StsUrl = TxtStsUrl.StringValue
			};
			LblServerUrl.StringValue = legacy ?
				SnapInContext.Instance.ServiceGateway.GetTenantEndpoint (legacy, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.StsUrl + "/" + serverDto.Tenant)
				:SnapInContext.Instance.ServiceGateway.GetTenantEndpoint (legacy, serverDto.Protocol, serverDto.ServerName, serverDto.Port, serverDto.Tenant);
		}
		private bool IsValid()
		{
			if (string.IsNullOrEmpty (TxtServer.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid server", "Alert");
				return false;
			} else if (!WebUtil.PingHost (TxtServer.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid server name or ip address", "Alert");
				return false;
			} else if (string.IsNullOrEmpty (TxtTenant.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid tenant name", "Alert");
				return false;
			} else if (string.IsNullOrEmpty (TxtPort.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid port", "Alert");
				return false;
			} else if (this.CbSaml.StringValue == "1" && string.IsNullOrEmpty (TxtStsUrl.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid STS endpoint", "Alert");
				return false;
			} else {
				var userCredentialOption = RdoTypeGroup.SelectedTag == 1;
				var certificateOption = RdoTypeGroup.SelectedTag == 2;
				var tokenOption = CbSaml.StringValue == "1" && RdoTypeGroup.SelectedTag == 3;
				var certOrtokenOption = certificateOption || tokenOption;

				if (userCredentialOption && string.IsNullOrEmpty (TxtUsername.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
					return false;
				} else if (userCredentialOption && string.IsNullOrEmpty (TxtDomain.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid domain name", "Alert");
					return false;
				} else if (userCredentialOption && string.IsNullOrEmpty (TxtPassword.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid password", "Alert");
					return false;
				} else if (certOrtokenOption && string.IsNullOrEmpty (TxtPrivateKey.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid private key", "Alert");
					return false;
				} else if (certOrtokenOption && !string.IsNullOrEmpty (TxtPrivateKey.StringValue)) {
					try {
						if (!System.IO.File.Exists (TxtPrivateKey.StringValue)) {
							UIErrorHelper.ShowAlert ("Private key file not found", "Alert");
							return false;
						}
					} catch (Exception exc) {
						UIErrorHelper.ShowAlert ("Private key file - " + exc.Message, "Alert");
						return false;
					}
				} else if (certOrtokenOption && string.IsNullOrEmpty (TxtCertificate.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid certificate", "Alert");
					return false;
				} else if (certOrtokenOption && !string.IsNullOrEmpty (TxtCertificate.StringValue)) {
					try {
						if (!System.IO.File.Exists (TxtCertificate.StringValue)) {
							UIErrorHelper.ShowAlert ("Certificate file not found", "Alert");
							return false;
						} else {
							var cert = new X509Certificate2 ();
							cert.Import (TxtCertificate.StringValue);
						}	
					} catch (Exception exc) {
						UIErrorHelper.ShowAlert ("Certificate file - " + exc.Message, "Alert");
						return false;
					}
				} else if (tokenOption && string.IsNullOrEmpty (TxtTokenFile.StringValue)) {
					UIErrorHelper.ShowAlert ("Please enter valid token file", "Alert");
				} else if (tokenOption && !string.IsNullOrEmpty (TxtTokenFile.StringValue)) {
					try {
						if (!System.IO.File.Exists (TxtTokenFile.StringValue)) {
							UIErrorHelper.ShowAlert ("Token file not found", "Alert");
							return false;
						}
					} catch (Exception exc) {
						UIErrorHelper.ShowAlert ("Token file - " + exc.Message, "Alert");
						return false;
					}
				} 
			}
			return true;
		} 
		public void OnClickAddButton (object sender, EventArgs e)
		{
			if(IsValid())
			{
				var serverDto = new ServerDto {
					ServerName = TxtServer.StringValue,
					Tenant = TxtTenant.StringValue,
					Port = TxtPort.StringValue,
					Protocol = CbSsl.StringValue == "1" ? "https" : "http",
					TokenType = CbSaml.StringValue == "1" ? TokenType.SAML : TokenType.Bearer,
					Url = LblServerUrl.StringValue,
					StsUrl = string.IsNullOrEmpty(TxtStsUrl.StringValue) ? string.Empty : TxtStsUrl.StringValue
				};
				var login = new LoginDto {
					User = TxtUsername.StringValue,
					Pass = TxtPassword.StringValue,
					DomainName = TxtDomain.StringValue,
					TenantName = TxtTenant.StringValue
				};
				try {
					TxtIDTokenString.StringValue = string.Empty;
					TxtAccessTokenString.StringValue = string.Empty;
					TxtRefreshTokenString.StringValue = string.Empty;
					TxtSamlToken.StringValue = string.Empty;

					if (CbSaml.StringValue == "0") {
						if (RdoTypeGroup.SelectedTag == 1) {
							var auth = SnapInContext.Instance.ServiceGateway.Authentication.Login (serverDto, login, Constants.ClientId);
							PopulateToken (auth);
						} else if (RdoTypeGroup.SelectedTag == 2) {
							var cert = new X509Certificate2 (TxtCertificate.StringValue);
							var rsa = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider (TxtPrivateKey.StringValue);
							var auth = SnapInContext.Instance.ServiceGateway.JwtTokenService.GetTokenFromCertificate (serverDto, cert, rsa);
							PopulateToken (auth);
						}
					} else {
						if (RdoTypeGroup.SelectedTag == 1) {
							var auth = SnapInContext.Instance.ServiceGateway.SamlTokenService.Authenticate (serverDto, login, Constants.ClientId);
							var bytes = Convert.FromBase64String (auth.Token.AccessToken);
							var token = System.Text.Encoding.Default.GetString (bytes);
							TxtSamlToken.StringValue = token;
						} else if (RdoTypeGroup.SelectedTag == 2) {
							var cert = new X509Certificate2 (TxtCertificate.StringValue);
							var rsa = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider (TxtPrivateKey.StringValue);
							var token = SnapInContext.Instance.ServiceGateway.SamlTokenService.GetSamlTokenFromCertificate (serverDto, cert, rsa);
							TxtSamlToken.StringValue = token;
						} else if (RdoTypeGroup.SelectedTag == 3) {
							var cert = new X509Certificate2 (TxtCertificate.StringValue);
							var rsa = ShaWithRsaSigner.PrivatePemKeyToRSACryptoServiceProvider (TxtPrivateKey.StringValue);
							var tokenText = System.IO.File.ReadAllText (TxtTokenFile.StringValue);
							var token = SnapInContext.Instance.ServiceGateway.SamlTokenService.GetSamlTokenFromToken (serverDto, tokenText, cert, rsa);
							TxtSamlToken.StringValue = token;
						}
					}
				}

				catch (WebException exp)
				{
					if (CbSaml.StringValue == "1") {
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
		}

		private void PopulateToken(AuthTokenDto authToken)
		{
			TxtIDTokenString.StringValue = JwtHelper.Decode(authToken.Token.IdToken);
			TxtAccessTokenString.StringValue = JwtHelper.Decode(authToken.Token.AccessToken);
			TxtRefreshTokenString.StringValue = JwtHelper.Decode(authToken.Token.RefreshToken);
		}
		#endregion

		//strongly typed window accessor
		public new TokenWizard Window {
			get {
				return (TokenWizard)base.Window;
			}
		}
	}
}

