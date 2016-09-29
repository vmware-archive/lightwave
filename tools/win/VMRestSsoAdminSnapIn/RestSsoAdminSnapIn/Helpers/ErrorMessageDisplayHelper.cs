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
using System.IO;
using System.Net;
using System.Windows.Forms;
using Microsoft.ManagementConsole.Advanced;
using RestSsoAdminSnapIn;
using Vmware.Tools.RestSsoAdminSnapIn.Views;
using VMwareMMCIDP.UI.Common.Utilities;

namespace Vmware.Tools.RestSsoAdminSnapIn.Helpers
{
    public static class ErrorMessageDisplayHelper
    {
        public static void ShowError(Exception exp)
        {
            ShowError(exp.Message);
        }

        public static void ShowError(string error)
        {
            MMCDlgHelper.ShowError(error);
        }

        public static void ShowInfo(string message)
        {
            MMCDlgHelper.ShowInformation(message);
        }

        public static void ShowException(Exception exp)
        {
            string data = "";
            try
            {
                var we = exp as WebException;
                if (we != null && we.Response != null)
                {
                    using (var stream = we.Response.GetResponseStream())
                    {
                        if (stream != null)
                        {
                            using (var rdr = new StreamReader(stream))
                            {
                                data = rdr.ReadToEnd();
                            }
                        }
                    }
                }
                else
                {
                    var ssoWebExp = exp as SsoWebException;
                    if (ssoWebExp != null)
                        data = ssoWebExp.Message;
                }

                if (!string.IsNullOrEmpty(data))
                {
                    var frm = new GenericWebForm("Error", data);
                    SnapInContext.Instance.SnapIn.Console.Show(frm);
                }
                else
                    ShowError(exp.Message);
            }
            catch (Exception exp2)
            {
                ShowError(exp2.Message);
            }
        }


        public static bool Confirm(string message)
        {
            return MMCDlgHelper.ShowConfirm(message);
        }

        public static bool ConfirmMessage(IWin32Window owner, string message)
        {
           var result = MessageBox.Show(owner, message, "Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
           return result == DialogResult.Yes;
        }
    }
}