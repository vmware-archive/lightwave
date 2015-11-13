/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
using VmIdentity.CommonUtils.Persistance;


namespace VmIdentity.CommonUtils
{
    public abstract class VMBaseSnapInEnvironment
    {
        private string DATA_FILE_NAME;

        public LocalData LocalData { get; set; }

        protected string ApplicationPath;
        public  static VMBaseSnapInEnvironment _instance;

        // public NSWindow MainWindow { get; set; } - define in derived class

        public abstract string GetApplicationPath ();


        public abstract void SaveLocalData ();


        public void SetSnapInDataFileName (string name)
        {
            DATA_FILE_NAME = name;
        }

        public string StoreFileName {
            get {
                return Path.Combine (ApplicationPath, DATA_FILE_NAME);
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


 
    }
}


