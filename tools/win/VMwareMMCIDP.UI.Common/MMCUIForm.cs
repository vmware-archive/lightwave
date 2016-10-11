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
using System.Windows.Forms;
using System.ComponentModel;

namespace VMwareMMCIDP.UI.Common
{
    public delegate void FormApplyEventHandler(object sender, FormApplyEventArgs args);

    public class MMCUIForm : Form
    {
        public event FormApplyEventHandler Apply;

        protected virtual void OnApply(FormApplyEventArgs args)
        {
            var handler = Apply;
            if (handler != null)
                handler(this, args);
        }
    }

    public class FormApplyEventArgs : CancelEventArgs
    {
        public object Modified { get; set; }
        public object Original { get; set; }
    }
}
