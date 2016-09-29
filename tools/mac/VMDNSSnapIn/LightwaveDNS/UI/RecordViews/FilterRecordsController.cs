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
using VmIdentity.UI.Common.Utilities;
using VMDNS.Common;
using VmIdentity.UI.Common;
using VMDNS.Client;

/*
 * Search Records UI Controller Class
 *
 * @author Sumalatha Abhishek
 */

namespace VMDNS
{
    public partial class FilterRecordsController : AppKit.NSViewController
    {
        NSPopover parent;

        public string RecordName{ get; set; }

        public RecordType RecordType { get; set; }

        public string ZoneName { get; set; }

        #region Constructors

        // Called when created from unmanaged code
        public FilterRecordsController(IntPtr handle)
            : base(handle)
        {
            Initialize();
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public FilterRecordsController(NSCoder coder)
            : base(coder)
        {
            Initialize();
        }

        // Call to load from the XIB/NIB file
        public FilterRecordsController(string zoneName, string searchFieldValue, NSPopover parentPopover)
            : base("FilterRecords", NSBundle.MainBundle)
        {
            parent = parentPopover;
            RecordName = searchFieldValue;
            ZoneName = zoneName;
            Initialize();
        }

        // Shared initialization code
        void Initialize()
        {
        }

        #endregion

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            ZoneNameField.StringValue = ZoneName;
            RecordNameField.StringValue = RecordName;
        }

        void DoValidateControls()
        {
            if (string.IsNullOrWhiteSpace(RecordNameField.StringValue) || string.IsNullOrWhiteSpace(RecordTypeOptionsField.TitleOfSelectedItem) ||
                string.IsNullOrWhiteSpace(ZoneNameField.StringValue))
                throw new ArgumentNullException(VMIdentityConstants.EMPTY_FIELD);
        }

        partial void Search(Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec(delegate()
                {
                    DoValidateControls();
                    RecordName = RecordNameField.StringValue;
                    RecordType = VMDNSUtilityService.GetRecordType(RecordTypeOptionsField.TitleOfSelectedItem);
                    ZoneName = ZoneNameField.StringValue;
                    parent.Close();
                });
        }

        //strongly typed view accessor
        public new FilterRecords View
        {
            get
            {
                return (FilterRecords)base.View;
            }
        }
    }
}
