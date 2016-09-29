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
using System.Reflection;
using System.Xml.Serialization;
using Microsoft.ManagementConsole;
using VMCASnapIn.Persistence;
using System.Drawing;

namespace VMCASnapIn
{
    public class VMCASnapInEnvironment
    {
        private static string DATA_FILE_NAME = "VMCAData.xml";
        
        protected static VMCASnapInEnvironment _instance;

        string _applicationPath;        
        public SnapIn SnapIn { get; set; }
        public VMCALocalData LocalData { get; set; }

        public static VMCASnapInEnvironment Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new VMCASnapInEnvironment();
                return _instance;
            }
        }

        public string StoreFileName
        {
            get
            {
                return Path.Combine(ApplicationPath, DATA_FILE_NAME);
            }
        }

        public string ApplicationPath
        {
            get
            {
                if (string.IsNullOrEmpty(_applicationPath))
                    _applicationPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                return _applicationPath;
            }
        }

        public void LoadLocalData()
        {
            if (!File.Exists(StoreFileName))
            {
                LocalData = new VMCALocalData();
                return;
            }

            try
            {
                using (var ms = new MemoryStream())
                {
                    var bytes = File.ReadAllBytes(StoreFileName);
                    ms.Write(bytes, 0, bytes.Length);
                    ms.Seek(0, SeekOrigin.Begin);

                    var xmlSerializer = new XmlSerializer(typeof(VMCALocalData));
                    LocalData = xmlSerializer.Deserialize(ms) as VMCALocalData;
                    LocalData.FillServerQueue();
                }
            }
            catch (Exception)
            {
            }
        }

        public void SaveLocalData()
        {
            try
            {
                using (var ms = new MemoryStream())
                {
                    var xmlSerializer = new XmlSerializer(typeof(VMCALocalData));
                    xmlSerializer.Serialize(ms, LocalData);

                    File.WriteAllBytes(StoreFileName, ms.ToArray());
                }
            }
            catch (Exception)
            {
            }
        }
        public Icon GetIconResource(VMCAIconIndex indx)
        {
            object obj = global::VMCASnapIn.Resources.ResourceManager.GetObject(indx.ToString(), global::VMCASnapIn.Resources.Culture);
            return (Icon)(obj);
        }
    }
}
