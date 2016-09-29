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
 
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using Vmware.Tools.RestSsoAdminSnapIn;

namespace Vmware.Tools.RestSsoAdminSnapIn.Controller
{
    public class NavigationController
    {
        public IDataContext NavigateToView(ScopeNode node, Form view)
        {
            return node.SnapIn.Console.ShowDialog(view) == DialogResult.OK ? ((IView)view).DataContext : null;
        }

        public IDataContext NavigateToView(SnapIn snapIn, Form view)
        {
            return snapIn.Console.ShowDialog(view) == DialogResult.OK ? ((IView)view).DataContext : null;
        }

        public IDataContext NavigateToView(PropertySheet sheet, Form view)
        {
            return sheet.ShowDialog(view) == DialogResult.OK ? ((IView)view).DataContext : null;
        }
    }
}