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
using AppKit;
using VMDir.Common.Persistence;
using Foundation;

namespace VMDirSnapIn
{
    public class VMDirSnapInEnvironment
    {
        private static string DATA_FILE_NAME = "VMDirData.xml";

        public LocalData LocalData { get; set; }

        string _applicationPath;
        protected static VMDirSnapInEnvironment _instance;

        public NSWindow MainWindow { get; set; }

        public string StoreFileName {
            get {
                return Path.Combine (ApplicationPath, DATA_FILE_NAME);
            }
        }

        public string ApplicationPath {
            get {
                if (string.IsNullOrEmpty (_applicationPath)) {
                    NSFileManager fileManager = NSFileManager.DefaultManager;
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
        }

        public static VMDirSnapInEnvironment Instance {
            get {
                if (_instance == null)
                    _instance = new VMDirSnapInEnvironment ();
                return _instance;
            }
        }

        public void LoadLocalData ()
        {
            if (!File.Exists (StoreFileName)) {
                LocalData = new LocalData ();
                return;
            }
            try {
                using (var ms = new MemoryStream ()) {
                    var bytes = File.ReadAllBytes (StoreFileName);
                    ms.Write (bytes, 0, bytes.Length);
                    ms.Seek (0, SeekOrigin.Begin);
                    var xmlSerializer = new XmlSerializer (typeof(LocalData));
                    LocalData = xmlSerializer.Deserialize (ms) as LocalData;
                }
            } catch (Exception) {
            }
        }

        public void SaveLocalData ()
        {
            try {
                using (var ms = new MemoryStream ()) {
                    var xmlSerializer = new XmlSerializer (typeof(LocalData));
                    xmlSerializer.Serialize (ms, VMDirSnapInEnvironment.Instance.LocalData);
                    File.WriteAllBytes (StoreFileName, ms.ToArray ());
                }
            } catch (Exception e) {
                System.Diagnostics.Debug.WriteLine (e.Message);
            }
        }
    }
}

