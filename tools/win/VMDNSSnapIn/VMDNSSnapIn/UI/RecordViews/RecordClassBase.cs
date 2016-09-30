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
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VMDNS.Client;

namespace VMDNSSnapIn.UI.RecordViews
{
    abstract class RecordClassBase
    {
        public Panel RecordPanel { get; set; }
        public AddNewRecord AddRecordFrm { get; set; }

        internal RecordClassBase()
        {
        }

        internal RecordClassBase(Panel recordPanel, AddNewRecord record)
        {
            this.RecordPanel = recordPanel;
            this.AddRecordFrm = record;
        }

        public abstract VmDnsRecord GetRecordDataFromUIFields();

        public abstract void SetUIFieldsFromRecordData(VmDnsRecord record);

        public abstract void SetUIFieldsEditability(bool state);

        protected abstract void DoValidateControls();
    }
}
