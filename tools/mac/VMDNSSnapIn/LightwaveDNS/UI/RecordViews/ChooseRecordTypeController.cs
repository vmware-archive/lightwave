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
using VmIdentity.UI.Common.Utilities;

namespace VMDNS
{
    public partial class ChooseRecordTypeController : NSWindowController
    {
        public String ZoneNameString { get; set; }

        public String RecordType { get; set; }

        public ChooseRecordTypeController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ChooseRecordTypeController(NSCoder coder)
            : base(coder)
        {
        }

        public ChooseRecordTypeController(string zoneName)
            : base("ChooseRecordType")
        {
            ZoneNameString = zoneName;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            this.ZoneName.StringValue = ZoneNameString;
        }

        void DoValidateControls()
        {
            if (string.IsNullOrEmpty(ZoneName.StringValue) || string.IsNullOrEmpty(ZoneTypeOptions.StringValue))
                throw new Exception("One or more required values are empty.");
        }


        partial void OnApply(Foundation.NSObject sender)
        {
            UIErrorHelper.CheckedExec(delegate ()
                {
                    DoValidateControls();
                    RecordType = this.ZoneTypeOptions.TitleOfSelectedItem;
                    this.Close();
                    NSApplication.SharedApplication.StopModalWithCode(1);
                });

        }


        partial void OnCancel(Foundation.NSObject sender)
        {
            this.Close();
            NSApplication.SharedApplication.StopModalWithCode(0);
        }

        public new ChooseRecordType Window
        {
            get { return (ChooseRecordType)base.Window; }
        }
    }
}
