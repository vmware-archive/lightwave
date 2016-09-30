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

using Microsoft.ManagementConsole.Advanced;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace VMDNSSnapIn
{
    class UIErrorHelper
    {
        public static void ShowError(Exception exp)
        {
            ShowError(exp.Message);
        }

        public static void ShowError(string error)
        {
            var msgParams = new MessageBoxParameters
            {
                Buttons = MessageBoxButtons.OK,
                Icon = MessageBoxIcon.Error,
                Text = error
            };
            MessageBox.Show(error);
        }

        public static void ShowMessage(string msg)
        {
            var msgParams = new MessageBoxParameters
            {
                Buttons = MessageBoxButtons.OK,
                Icon = MessageBoxIcon.Information,
                Text = msg
            };
            MessageBox.Show(msg);
        }

        public static int CheckedExec(Action fn)
        {
            int ret = -1;
            try
            {
                fn();
                ret = 1;
            }
            catch (ArgumentNullException e)
            {
                ShowError(e.Message);
                ret = -1;
            }
            catch (Exception exp)
            {
                ShowError(exp.Message);
                ret = -1;
            }
            return ret;
        }

        public static bool Confirm(string message)
        {
            var msgParams = new MessageBoxParameters
            {
                Buttons = MessageBoxButtons.YesNo,
                Icon = MessageBoxIcon.Information,
                Text = message
            };
            return VMDNSSnapInEnvironment.Instance.SnapIn.Console.ShowDialog(msgParams) == DialogResult.Yes;
        }
    }
}
