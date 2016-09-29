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
using Microsoft.ManagementConsole;
using System.ComponentModel;

namespace VMwareMMCIDP.UI.Common
{
    public class VMPropertyPage : PropertyPage
    {
        public event CancelEventHandler Apply;
        public event EventHandler Initialize;

        protected override void OnInitialize()
        {
            if (Initialize != null)
            {
                Initialize(this, new EventArgs());
            }
        }

        protected override bool OnApply()
        {
            if (!Dirty)
                return true;

            if (Apply != null)
            {
                var eventArgs = new CancelEventArgs();
                Apply(this, eventArgs);
                return eventArgs.Cancel == false;
            }
            return false;
        }

        protected override bool OnOK()
        {
            return OnApply();
        }
    }
}

