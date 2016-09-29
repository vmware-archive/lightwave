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
using VMCA.Client;
using AppKit;
using System.IO;

namespace UI
{
    public static class UIHelper
    {
        public static void SaveKeyData (KeyPairData data)
        {
            var save = NSSavePanel.SavePanel;
            save.AllowedFileTypes = new string[] { "pem" };
            nint result = save.RunModal ();
            if (result == (int)1) {
                string path = save.Url.Path;
                File.WriteAllText (path, data.PrivateKey);
            }
                
            result = save.RunModal ();
            if (result == (int)1) {
                string path = save.Url.Path;
                File.WriteAllText (path, data.PublicKey);
            }
        }
    }
}

