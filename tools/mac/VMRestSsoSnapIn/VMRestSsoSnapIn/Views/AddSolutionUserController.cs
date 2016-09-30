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
using VmIdentity.CommonUtils.Utilities;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class AddSolutionUserController : AppKit.NSWindowController
	{
		public SolutionUserDto SolutionUserDto;

		#region Constructors

		// Called when created from unmanaged code
		public AddSolutionUserController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public AddSolutionUserController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public AddSolutionUserController () : base ("AddSolutionUser")
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
			this.BtnAddNew.Activated += OnClickAddButton;
			this.BtnBrowseCertificate.Activated += OnClickBrowseButton;
			this.BtnCloseNew.Activated += (object sender, EventArgs e) => {
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (0);
			};
		}

		public void OnClickAddButton (object sender, EventArgs e)
		{
			if (string.IsNullOrEmpty (TxtUsername.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid username", "Alert");
			} else if (string.IsNullOrEmpty (TxtDescription.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid Description", "Alert");
			} else if (string.IsNullOrEmpty (TxtCertificatePath.StringValue)) {
				UIErrorHelper.ShowAlert ("Please enter valid Certificate", "Alert");
			} else {
				var cert = new X509Certificate2 ();
				ActionHelper.Execute (delegate() {
					cert.Import (TxtCertificatePath.StringValue.Replace ("file://", string.Empty));
				});

				SolutionUserDto = new SolutionUserDto () {
					Name = TxtUsername.StringValue,
					Description = TxtDescription.StringValue,
					Certificate = new CertificateDto { Encoded = cert.ExportToPem() }
				};
				this.Close ();
				NSApplication.SharedApplication.StopModalWithCode (1);
			}
		}

		public void OnClickBrowseButton (object sender, EventArgs e)
		{
			var openPanel = new NSOpenPanel();
			openPanel.ReleasedWhenClosed = true;
			openPanel.Prompt = "Select file";

			var result = openPanel.RunModal();
			if (result == 1)
			{
				var filePath = openPanel.Url.AbsoluteString.Replace("file://",string.Empty);
				var cert = new X509Certificate2 ();
				ActionHelper.Execute (delegate() {
					
					cert.Import (filePath);
				});
				TxtCertificatePath.StringValue = filePath;
			}
		}

		#endregion

		//strongly typed window accessor
		public new AddSolutionUser Window {
			get {
				return (AddSolutionUser)base.Window;
			}
		}
	}
}

