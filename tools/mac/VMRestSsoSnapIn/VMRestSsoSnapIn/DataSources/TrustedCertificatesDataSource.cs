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
using AppKit;
using System.Collections.Generic;
using Foundation;
using Vmware.Tools.RestSsoAdminSnapIn.Nodes;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Security.Cryptography.X509Certificates;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Extensions;
using Vmware.Tools.RestSsoAdminSnapIn.Helpers;
using VmIdentity.CommonUtils.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.DataSource
{
	public class TrustedCertificatesDataSource : NSTableViewDataSource
	{
		public List<CertificateDto> Entries { get; set; }

		public TrustedCertificatesDataSource ()
		{
			Entries = new List<CertificateDto> ();
		}

		// This method will be called by the NSTableView control to learn the number of rows to display.
		[Export ("numberOfRowsInTableView:")]
		public int NumberOfRowsInTableView (NSTableView table)
		{
			if (Entries != null)
				return Entries.Count;
			else
				return 0;
		}

		// This method will be called by the control for each column and each row.
		[Export ("tableView:objectValueForTableColumn:row:")]
		public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
		{
			var value = (NSString)string.Empty;
			ActionHelper.Execute (delegate() {
				if (Entries != null) {
					var obj = (this.Entries [row]) as CertificateDto;
					X509Certificate2 cert;
					switch (col.Identifier) {
					case "Name":
						value = (NSString)obj.Chain;
						break;
					case "Status": 
						value = (NSString)(obj.IsSigner ? "ACTIVE" : "IN-ACTIVE");
						break;
					case "IssuedBy": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.Issuer;
						break;
					case "IssuedOn": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.NotBefore.ToShortDateString ();
						break;
					case "Expiration": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.NotAfter.ToShortDateString ();
						break;
					case "Purpose": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.GetKeyUsage ();
						break;
					case "SubjectDn": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.Subject;
						break;
					case "Fingerprint": 
						cert = new X509Certificate2 (Encoding.ASCII.GetBytes (obj.Encoded));
						value = (NSString)cert.GetFormattedThumbPrint ();
						break;
					default:
						break;
					}
				}
			});
			return value;
		}

	}
}

