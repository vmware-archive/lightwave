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
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using Microsoft.ManagementConsole.Advanced;
using VMCASnapIn.DTO;
using VMwareMMCIDP.UI.Common.Utilities;
using System.Net;
using System.Text.RegularExpressions;

namespace VMCASnapIn.Utilities
{
    public static class MiscUtilsService
    {
        public static Image GetToolbarImage()
        {
            return GetResourceImage("VMCASnapIn.Images.Toolbar.bmp");
        }

        public static Image GetResourceImage(string name)
        {
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                return new Bitmap(stream);
            }
        }
        public static bool ApproveCertRequestHandler(object obj, object objOld)
        {
            var dto = obj as CertRequestDTO;
            if (dto == null) return false;

            string msg = null;
            if (string.IsNullOrEmpty(dto.Country))
                msg = MMCUIConstants.COUNTRY_ENT;
            else if (dto.Country.Length > 2)
                msg = MMCUIConstants.COUNTRY_LEN_VAL;
            else if (string.IsNullOrEmpty(dto.Name))
                msg = MMCUIConstants.NAME_ENT;
            else if (!dto.PrivateKey.HasData)
                msg = MMCUIConstants.PRI_KEY_SEL;

            if (msg==null && !string.IsNullOrWhiteSpace(dto.Email))
            {
                if (!Regex.IsMatch(dto.Email, MMCUIConstants.EmailPattern))
                    msg = MMCUIConstants.INVALID_EMAIL;
            }
            if (msg == null && !string.IsNullOrWhiteSpace(dto.IPAddress))
            {
                IPAddress address;
                if (!IPAddress.TryParse(dto.IPAddress, out address))
                    msg = MMCUIConstants.INVALID_IP;
            }


            if (msg != null)
            {
                MMCDlgHelper.ShowWarning(msg);
                return false;
            }
            return true;
        }
    }
}
