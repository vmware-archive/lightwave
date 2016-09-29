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
using System.Security.Cryptography.X509Certificates;
using VmIdentity.UI.Common;
using VmIdentity.UI.Common.Utilities;
using Foundation;
using AppKit;
using VMCertStore.Utilities;
using VMCertStore.Common.DTO;
using Vecs;

namespace VMCertStoreSnapIn.ListViews
{
    public class CertificateDetailsListView : NSTableViewDataSource
    {
        public  List<CertDTO> Entries { get; set; }

        public  VMCertStoreServerDTO ServerDto { get; set; }

        public string Store { get; set; }

        public CertificateDetailsListView ()
        {
            Entries = null;
        }

        public CertificateDetailsListView (List<CertDTO> certList, VMCertStoreServerDTO dto, String store)
        {
            Entries = certList;
            ServerDto = dto;
            Store = store;
        }

        // This method will be called by the NSTableView control to learn the number of rows to display.
        [Export ("numberOfRowsInTableView:")]
        public int NumberOfRowsInTableView (NSTableView table)
        {
            if (Entries != null)
                return this.Entries.Count;
            else
                return 0;
        }

        // This method will be called by the control for each column and each row.
        [Export ("tableView:objectValueForTableColumn:row:")]
        public NSObject ObjectValueForTableColumn (NSTableView table, NSTableColumn col, int row)
        {
            try {
                if (Entries != null) {
                    X509Certificate2 cert = Entries [row].Cert;
                    switch (col.Identifier) {
                    case "Alias":
                        return (NSString)Entries [row].Alias;
                    case VMIdentityConstants.CERT_ISSUED_BY:
                        return (NSString)cert.Issuer;
                    case VMIdentityConstants.CERT_ISSUED_DATE:
                        return (NSString)cert.NotBefore.ToShortDateString ();
                    case VMIdentityConstants.CERT_EXPIRATION_DATE:
                        return (NSString)cert.NotAfter.ToShortDateString ();
                    case VMIdentityConstants.CERT_INTENDED_PURPOSES:
                        return (NSString)cert.GetKeyUsage ();
                    case VMIdentityConstants.CERT_STATUS:
                        break;
                    case VMIdentityConstants.CERT_ISSUED_TO:
                        return (NSString)cert.Subject;
                    }
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error in fetching data : " + e.Message);
            }
            return null;
        }
    }
}

