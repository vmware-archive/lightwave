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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.ManagementConsole.Advanced;
using System.Windows.Forms;
using Microsoft.ManagementConsole;
using System;

namespace VMwareMMCIDP.UI.Common.Utilities
{
    public static class MMCDlgHelper
    {
        public static SnapIn snapIn { get; set; }
        public static void ShowError(string msg)
        {
            MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
        public static void ShowInformation(string msg)
        {
            MessageBox.Show(msg, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
        public static void ShowWarning(string msg)
        {
            MessageBox.Show(msg, "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
        public static bool ShowQuestion(string msg)
        {
            return MessageBox.Show(msg, "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes;
        }
        public static void ShowException(Exception exp)
        {
            var msgParams = new MessageBoxParameters
            {
                Caption = "Error",
                Buttons = MessageBoxButtons.OK,
                Icon = MessageBoxIcon.Error,
                Text = exp.Message
            };
            snapIn.Console.ShowDialog(msgParams);
        }
        public static bool ShowConfirm(string msg)
        {
            var msgParams = new MessageBoxParameters
            {
                Caption = "Confirm",
                Buttons = MessageBoxButtons.YesNo,
                Icon = MessageBoxIcon.Question,
                Text = msg
            };
            return snapIn.Console.ShowDialog(msgParams) == DialogResult.Yes;
        }
        public static bool ShowForm(Form frm)
        {
            return snapIn.Console.ShowDialog(frm) == DialogResult.OK;
        }
        public static void ShowMessage(string msg)
        {
            var msgParams = new MessageBoxParameters
            {
                Caption = "Message",
                Buttons = MessageBoxButtons.OK,
                Icon = MessageBoxIcon.None,
                Text = msg
            };
            snapIn.Console.ShowDialog(msgParams);
        }
    }
}

