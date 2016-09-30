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
using AppKit;

namespace VmIdentity.UI.Common.Utilities
{
    public static class UIErrorHelper
    {
		public static nint ShowWarning(String text)
		{
			var alert = new NSAlert();
			alert.MessageText = "Warning";
			alert.AlertStyle = NSAlertStyle.Warning;
			alert.InformativeText = text;
			return alert.RunModal();
		}

		public static nint ShowInformation(String text)
		{
			var alert = new NSAlert();
			alert.MessageText = "Information";
			alert.AlertStyle = NSAlertStyle.Warning;
			alert.InformativeText = text;
			return alert.RunModal();
		}

		public static nint ShowError(String text)
		{
			var alert = new NSAlert();
			alert.MessageText = "Error";
			alert.AlertStyle = NSAlertStyle.Critical;
			alert.InformativeText = text;
			return alert.RunModal();
		}

        public static nint ShowAlert(String text, String caption)
        {
            var alert = new NSAlert();
            alert.MessageText = caption;
            alert.InformativeText = text;
            return alert.RunModal();
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
                ShowAlert(e.Message, "Warning");
                ret = -1;
            }
            catch (Exception exp)
            {
                ShowAlert(exp.Message, "Operation could not complete successfully.");
                ret = -1;
            }
            return ret;
        }

        public static void CatchAndThrow(Action fn)
        {
            try
            {
                fn();
            }
            catch (Exception exp)
            {
                throw;
            }
        }

        public static bool ConfirmDeleteOperation(string confirmMessage)
        {
            try
            {
                var oAlert = new NSAlert();

                // Set the buttons
                oAlert.AddButton("Yes");
                oAlert.AddButton("No");

                // Show the message box and capture
                oAlert.MessageText = confirmMessage;
                oAlert.InformativeText = "";
                oAlert.AlertStyle = NSAlertStyle.Warning;
                oAlert.Icon = NSImage.ImageNamed(NSImageName.Caution);
                var responseAlert = oAlert.RunModal();
                return (responseAlert == 1000); //returns 1001 for No and 1000 for Yes in this case
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        public static bool ConfirmOperation(string confirmMessage)
        {
            try
            {
                ConfirmationDialogController cwc = new ConfirmationDialogController(confirmMessage);
                nint result = NSApplication.SharedApplication.RunModalForWindow(cwc.Window);
                if (result == (nint)VMIdentityConstants.DIALOGOK)
                    return true;
                else
                    return false;
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }
}

