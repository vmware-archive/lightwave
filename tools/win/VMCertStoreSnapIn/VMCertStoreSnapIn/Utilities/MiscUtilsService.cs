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
using System.Windows.Forms;
using System.Reflection;
using System.Drawing;
using Microsoft.ManagementConsole.Advanced;

namespace VMCertStoreSnapIn.Utilities
{
    public static class MiscUtilsService
    {
        public static Image GetToolbarImage()
        {
            return GetResourceImage("VMCertStoreSnapIn.Images.Toolbar.bmp");
        }

        public static Image GetResourceImage(string name)
        {
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(name))
            {
                return new Bitmap(stream);
            }
        }

        public static bool IsSystemStore(string storeName)
        {
            string[] storeNames = new string[] { "MACHINE_SSL_CERT", "TRUSTED_ROOTS", "TRUSTED_ROOT_CRLS" };
            return (storeNames.Contains(storeName, StringComparer.InvariantCultureIgnoreCase));
        }
    }
}
