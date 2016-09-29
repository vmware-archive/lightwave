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
using System.Windows.Forms;
using VMPSCHighAvailability.Common.Helpers;
using VMPscHighAvailabilitySnapIn.SnapIn;

namespace VMPscHighAvailabilitySnapIn.Utils
{
    /// <summary>
    /// Utility Services
    /// </summary>
    public static class MiscUtilsService
    {
        /// <summary>
        /// Checked exception handling
        /// </summary>
        /// <param name="fn">Action</param>
        public static void CheckedExec(System.Action fn)
        {
            try
            {
                fn();
            }
            catch (Exception exp)
            {
                var custom =  new CustomExceptionExtractor().GetCustomMessage(exp);
                PscHighAvailabilityAppEnvironment.Instance.Logger.LogException(exp, custom);
                ShowError(exp);
            }
        }

        /// <summary>
        /// Show error message for exception
        /// </summary>
        /// <param name="exp">Exception</param>
        /// <returns>Dialog result</returns>
        public static DialogResult ShowError(Exception exp)
        {
            string error = exp.Message;
            return MessageBox.Show(error);
        }

        /// <summary>
        /// Show confirmation messages
        /// </summary>
        /// <param name="text">Message text</param>
        /// <param name="caption">Caption</param>
        /// <param name="btn">buttons</param>
        /// <returns>Dialog result</returns>
        public static DialogResult ShowError(string text, string caption, MessageBoxButtons btn)
        {
            return MessageBox.Show(text, caption, btn);
        }
    }
}
