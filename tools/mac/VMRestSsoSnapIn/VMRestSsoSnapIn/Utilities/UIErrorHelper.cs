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

namespace RestSsoAdminSnapIn
{
	public static class UIErrorHelper
    {
        public static nint ShowAlert (String text, String caption)
        {
            var alert = new NSAlert ();
            alert.MessageText = caption;
			alert.InformativeText = text == null ? "Error" : text;
			return alert.RunModal ();
        }

		public static bool ShowConfirm (String text, String caption)
		{
			var oAlert = new NSAlert();

			// Set the buttons
			oAlert.AddButton("Yes");
			oAlert.AddButton("No");

			// Show the message box and capture
			oAlert.MessageText = caption;
			oAlert.InformativeText = text;
			oAlert.AlertStyle = NSAlertStyle.Warning;
			oAlert.Icon = NSImage.ImageNamed (NSImageName.Caution);
			var responseAlert = oAlert.RunModal();
			return (responseAlert == 1000);
		}


        public static int CheckedExec (Action fn)
        {
            try {
                fn ();
                return 1;
            } catch (Exception exp) {
                ShowAlert (exp.Message, "Alert");
                return -1;
            }
        }
    }
}

