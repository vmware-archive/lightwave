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
using VmIdentity.CommonUtils.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class SolutionUserDetailsViewController : AppKit.NSViewController
	{
		public SolutionUserDto SolutionUserDtoOriginal;
		public ServerDto ServerDto;
		public string TenantName;
		public SolutionUserDto SolutionUserDto;
		public bool IsSystemDomain;
		private X509Certificate2 _certificate;

		#region Constructors

		// Called when created from unmanaged code
		public SolutionUserDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public SolutionUserDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public SolutionUserDetailsViewController () : base ("SolutionUserDetailsView", NSBundle.MainBundle)
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
			SolutionUserDto = SolutionUserDtoOriginal.DeepCopy();
			TxtName.StringValue = SolutionUserDto.Name;
			TxtDescription.StringValue = string.IsNullOrEmpty(SolutionUserDto.Description)?string.Empty:SolutionUserDto.Description;
			cbDisabled.StringValue = SolutionUserDto.Disabled ? "0" : "1";
			_certificate = new X509Certificate2 (Encoding.ASCII.GetBytes(SolutionUserDto.Certificate.Encoded));
			ActionHelper.Execute (delegate() {
				TxtIssuer.StringValue = _certificate.Issuer;
				TxtValidFrom.StringValue = _certificate.NotBefore.ToShortDateString ();
				TxtValidTo.StringValue = _certificate.NotAfter.ToShortDateString ();
				TxtDn.StringValue = _certificate.Subject;
			});

			//Events
			this.BtnSave.Activated += OnClickSaveButton;

			this.BtnChangeCertificate.Activated += (object sender, EventArgs e) => {
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
						TxtIssuer.StringValue =  cert1.Issuer;
						TxtValidFrom.StringValue = cert1.NotBefore.ToShortDateString();
						TxtValidTo.StringValue = cert1.NotAfter.ToShortDateString();
						TxtDn.StringValue = cert1.Subject;
						_certificate = cert1;
					});
					SolutionUserDto.Certificate.Encoded = _certificate.ExportToPem();
				}
			};

			BtnViewCertificate.Activated += (object sender, EventArgs e) => 
			{
				CertificateService.DisplayX509Certificate2(this, _certificate );
			};

			BtnSave.Hidden = !IsSystemDomain;
		}

		public void OnClickSaveButton (object sender, EventArgs e)
		{
			SolutionUserDto.Description = TxtDescription.StringValue;
			SolutionUserDto.Disabled = cbDisabled.StringValue == "1";

			ActionHelper.Execute (delegate() {
				if (SolutionUserDtoOriginal.Description != SolutionUserDto.Description ||
				    SolutionUserDtoOriginal.Disabled != SolutionUserDto.Disabled ||
				    SolutionUserDtoOriginal.Certificate.Encoded != SolutionUserDto.Certificate.Encoded) {
					var service = SnapInContext.Instance.ServiceGateway;
					var auth = SnapInContext.Instance.AuthTokenManager.GetAuthToken (ServerDto.ServerName);
					service.SolutionUser.Update (ServerDto, TenantName, SolutionUserDto, auth.Token);
					NSNotificationCenter.DefaultCenter.PostNotificationName ("RefreshTableView", this);
				}
			});
		}

		//strongly typed view accessor
		public new SolutionUserDetailsView View {
			get {
				return (SolutionUserDetailsView)base.View;
			}
		}
	}
}
