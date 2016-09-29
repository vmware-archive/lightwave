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

using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;

namespace RestSsoAdminSnapIn
{
	public partial class CertificateDetailsViewController : AppKit.NSViewController
	{
		private X509Certificate2 _certificate;
		public CertificateDto CertificateDto { get; set;}

		#region Constructors

		// Called when created from unmanaged code
		public CertificateDetailsViewController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}

		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public CertificateDetailsViewController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}

		// Call to load from the XIB/NIB file
		public CertificateDetailsViewController () : base ("CertificateDetailsView", NSBundle.MainBundle)
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
			ChkActive.StringValue = CertificateDto.IsSigner ? "1" : "0";
			BtnViewCertificate.Activated += (object sender, EventArgs e) => 
			{
				CertificateService.DisplayX509Certificate2(this, _certificate);
			};
			ActionHelper.Execute (delegate() {
				_certificate = new X509Certificate2 (Encoding.ASCII.GetBytes (CertificateDto.Encoded));
				TxtIssuer.StringValue = _certificate.Issuer;
				TxtValidFrom.StringValue = _certificate.NotBefore.ToShortDateString ();
				TxtValidTo.StringValue = _certificate.NotAfter.ToShortDateString ();
				TxtDn.StringValue = _certificate.Subject;
			});
		}
		#endregion

		//strongly typed view accessor
		public new CertificateDetailsView View {
			get {
				return (CertificateDetailsView)base.View;
			}
		}
	}
}
