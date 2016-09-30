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
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using Foundation;
using AppKit;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using VmIdentity.UI.Common.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace RestSsoAdminSnapIn
{
	public partial class ViewSolutionUserDetailsController : AppKit.NSWindowController
	{
		public SolutionUserDto SolutionUserDtoOriginal;
		public ServerDto ServerDto;
		public string TenantName;
		public SolutionUserDto SolutionUserDto;

		#region Constructors

		// Called when created from unmanaged code
		public ViewSolutionUserDetailsController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ViewSolutionUserDetailsController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public ViewSolutionUserDetailsController () : base ("ViewSolutionUserDetails")
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
			SolutionUserDto = SolutionUserDtoOriginal.DeepCopy();
			TxtName.StringValue = SolutionUserDto.Name;
			TxtDescription.StringValue = string.IsNullOrEmpty(SolutionUserDto.Description)?string.Empty:SolutionUserDto.Description;
			CbDisabled.StringValue = SolutionUserDto.Disabled ? "1" : "0";
			Window.Title = SolutionUserDto.Name + " Properties";
			var cert = new X509Certificate2 (Encoding.ASCII.GetBytes(SolutionUserDto.Certificate.Encoded));
			try {
				TxtIssuer.StringValue =  cert.Issuer;
				TxtValidFrom.StringValue = cert.NotBefore.ToShortDateString();
				TxtValidTo.StringValue = cert.NotAfter.ToShortDateString();
				TxtDC.StringValue = cert.IssuerName.Format(true);
			} catch (Exception) {
				UtilityService.ShowAlert ("Invalid X509 certificate", "Alert");
			}

			//Events
			this.BtnSave.Activated += OnClickSaveButton;
			this.BtnClose.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};

			this.BtnChangeCertificate.Activated += (object sender, EventArgs e) => {
				var openPanel = new NSOpenPanel();
				openPanel.ReleasedWhenClosed = true;
				openPanel.Prompt = "Select file";

				var result = openPanel.RunModal();
				if (result == 1)
				{
					var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);
					var cert1 = new X509Certificate2 ();
					try {

						cert1.Import (filePath);
						TxtIssuer.StringValue =  cert1.Issuer;
						TxtValidFrom.StringValue = cert1.NotBefore.ToShortDateString();
						TxtValidTo.StringValue = cert1.NotAfter.ToShortDateString();
						TxtDC.StringValue = cert1.IssuerName.Format(true);
					} catch (Exception) {
						UtilityService.ShowAlert ("Invalid X509 certificate", "Alert");
					}
					SolutionUserDto.Certificate.Encoded = cert.ToPem();
				}
			};
		}

		public void OnClickSaveButton (object sender, EventArgs e)
		{
			SolutionUserDto.Description = TxtDescription.StringValue;
			SolutionUserDto.Disabled = CbDisabled.StringValue == "1";

			try {
				if (SolutionUserDtoOriginal.Description != SolutionUserDto.Description || 
					SolutionUserDtoOriginal.Disabled != SolutionUserDto.Disabled ||
					SolutionUserDtoOriginal.Certificate.Encoded != SolutionUserDto.Certificate.Encoded)
				{
					var service = SnapInContext.Instance.ServiceGateway;
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken(ServerDto.ServerName);
					service.SolutionUser.Update(ServerDto, TenantName, SolutionUserDto, auth.Token);
				}
			} catch (Exception) {
				
			}
			this.Close ();
			NSApplication.SharedApplication.StopModalWithCode (1);

		}
		#endregion

		//strongly typed window accessor
		public new ViewSolutionUserDetails Window {
			get {
				return (ViewSolutionUserDetails)base.Window;
			}
		}
	}
}

