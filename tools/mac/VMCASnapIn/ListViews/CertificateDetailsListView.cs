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
using AppKit;
using Foundation;
using VMCA.Utilities;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;

namespace VMCASnapIn.ListViews
{
    public class CertificateDetailsListView : NSTableViewDataSource
    {
        public  List<X509Certificate2> Entries { get; set; }

        public  VMCAServerDTO ServerDto { get; set; }

        public int CertificateState { get; set; }

        public CertificateDetailsListView ()
        {
            Entries = null;
        }

        public CertificateDetailsListView (List<X509Certificate2> certList, VMCAServerDTO dto, int state)
        {
            Entries = certList;
            ServerDto = dto;
            CertificateState = state;
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
                    switch (col.Identifier) {
                    case Constants.CERT_ISSUED_BY:
                        return (NSString)this.Entries [row].Issuer;
                    case Constants.CERT_ISSUED_DATE:
                        return (NSString)this.Entries [row].NotBefore.ToShortDateString ();
                    case Constants.CERT_EXPIRATION_DATE:
                        return (NSString)this.Entries [row].NotAfter.ToShortDateString ();
                    case Constants.CERT_INTENDED_PURPOSES:
                        return (NSString)CertificateExtensions.GetKeyUsage (this.Entries [row]);
                    case Constants.CERT_STATUS:
                        break;
                    case Constants.CERT_ISSUED_TO:
                        return (NSString)this.Entries [row].Subject;
                    }
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine ("Error in fetching data : " + e.Message);
            }
            return null;
        }
    }
}

