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
using System.Xml.Serialization;
using VMCASnapIn.Persistence;
using AppKit;
using Foundation;

namespace VMCASnapIn
{
    public class VMCAAppEnvironment
    {
        private const string DATA_FILE_NAME = "VMCAData.xml";

        protected static VMCAAppEnvironment _instance;

        string _applicationPath;

        public VMCALocalData LocalData { get; set; }

        public NSWindow MainWindow { get; set; }

        public static VMCAAppEnvironment Instance {
            get {
                if (_instance == null)
                    _instance = new VMCAAppEnvironment ();
                return _instance;
            }
        }

        public string StoreFileName {
            get {
                return Path.Combine (getApplicationPath (), DATA_FILE_NAME);
            }
        }

        public string getApplicationPath ()
        {
            if (string.IsNullOrWhiteSpace (_applicationPath)) {
                string[] paths = NSSearchPath.GetDirectories (NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
                if (paths.Length > 0) {
                    _applicationPath = paths [0] + "/LightwaveTools";
                    if (!Directory.Exists (_applicationPath)) {
                        Directory.CreateDirectory (_applicationPath);
                    }

                }
            }
            return _applicationPath;
        }

        public void LoadLocalData ()
        {
            if (!File.Exists (StoreFileName)) {
                LocalData = new VMCALocalData ();
                return;
            }

            try {
                using (var ms = new MemoryStream ()) {
                    var bytes = File.ReadAllBytes (StoreFileName);
                    ms.Write (bytes, 0, bytes.Length);
                    ms.Seek (0, SeekOrigin.Begin);

                    var xmlSerializer = new XmlSerializer (typeof(VMCALocalData));
                    LocalData = xmlSerializer.Deserialize (ms) as VMCALocalData;
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine (string.Concat ("Failed To Load LocalData : ", e.Message));
            }
        }

        public void SaveLocalData ()
        {
            try {
                using (var ms = new MemoryStream ()) {
                    var xmlSerializer = new XmlSerializer (typeof(VMCALocalData));
                    xmlSerializer.Serialize (ms, LocalData);

                    File.WriteAllBytes (StoreFileName, ms.ToArray ());
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine (string.Concat ("Failed To Save LocalData", e.Message));
            }
        }
    }
}
