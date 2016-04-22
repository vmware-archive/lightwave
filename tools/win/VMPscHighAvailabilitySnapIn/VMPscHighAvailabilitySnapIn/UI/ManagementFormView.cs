﻿/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

using Microsoft.ManagementConsole;
using VMPscHighAvailabilitySnapIn.ScopeNodes;

namespace VMPscHighAvailabilitySnapIn.UI{    /// <summary>
    /// Management form view
    /// </summary>    public class ManagementFormView : FormView    {        /// <summary>
        /// User control for management view
        /// </summary>        private ManagementViewControl _control;                /// <summary>
        /// On initialization of the view
        /// </summary>
        /// <param name="status">Async status</param>        protected override void OnInitialize(AsyncStatus status)        {            base.OnInitialize(status);
            _control = (ManagementViewControl)Control;
            var node = ScopeNode as ManagementNode;
            if (node != null) node.UserControl = _control;
        }
        protected override void OnShow()
        {
            base.OnShow();
            _control.RefreshDataSource();
        }    }}