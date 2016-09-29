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
using Foundation;
using AppKit;
using VMDNS.Client;
using VmIdentity.UI.Common.Utilities;
using VMDNS.Common;

namespace VMDNS
{
    public partial class AddNewRecordController : NSWindowController
    {
        public VmDnsRecord Record { get; set; }

        RecordType recordType;
        RecordControllerBase recordControllerObject;
        bool isViewMode = false;

        public AddNewRecordController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public AddNewRecordController(NSCoder coder)
            : base(coder)
        {
        }

        public AddNewRecordController(RecordType rtype)
            : base("AddNewRecord")
        {
            recordType = rtype;
            Record = null;
        }

        public AddNewRecordController(VmDnsRecord  record)
            : base("AddNewRecord")
        {
            Record = record;
            recordType = (RecordType)record.Type;
            isViewMode = true;
        }

        void InitialiseViewControllers(RecordType rtype)
        {
            //TODO - move this to dictionary instead of switch case.
            switch (rtype)
            {
                case RecordType.VMDNS_RR_TYPE_A:
                    recordControllerObject = new ARecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_AAAA:
                    recordControllerObject = new AAAARecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_CNAME:
                    recordControllerObject = new CNameRecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_NS:
                    recordControllerObject = new NSRecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_PTR:
                    recordControllerObject = new PtrRecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_SRV:
                    recordControllerObject = new SRVRecordController();
                    break;
                case RecordType.VMDNS_RR_TYPE_SOA:
                    recordControllerObject = new SOARecordController();
                    break;
                default:
                    UIErrorHelper.ShowAlert("Unknown record Type", "");
                    break;
            }
            if (recordControllerObject != null)
                this.RecordContentView.AddSubview(recordControllerObject.View);
        }

        partial void OnClose(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        partial void OnAdd(Foundation.NSObject sender)
        {
            this.Record = recordControllerObject.GetRecordDataFromUIFields();
            if (this.Record != null)
            {
                this.Close();
                NSApplication.SharedApplication.StopModalWithCode(1);
            }
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseViewControllers(recordType);
            this.Window.Title += " - " + VMDNSUtilityService.GetRecordNameFromType(recordType);

            //if view properties mode, set ui controls to initialise with record data and remove add button. also make the ui controls not editable.
            if (isViewMode && recordControllerObject != null)
            {
                this.Window.Title = "Record Properties";
                recordControllerObject.SetUIFieldsFromRecordData(Record);
                AddButton.Hidden = true;
                recordControllerObject.SetUIFieldsEditability(false);
            }
            else
            {
                this.Close();
                NSApplication.SharedApplication.StopModalWithCode(0);
            }
        }

        public new AddNewRecord Window
        {
            get { return (AddNewRecord)base.Window; }
        }
    }
}
