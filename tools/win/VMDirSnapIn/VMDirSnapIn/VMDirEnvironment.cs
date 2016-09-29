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
using VMDir.Common.DTO;
using VMDirSnapIn.Persistence;
using System.IO;
using System.Reflection;
using System.Xml.Serialization;
using VMDir.Common.Schema;
using VMDirSnapIn.Utilities;
using System.Drawing;
using Microsoft.ManagementConsole;
using VMIdentity.CommonUtils.Log;
using VMwareMMCIDP.UI.Common.Utilities;
using System.Windows.Forms;
using VMDir.Common.VMDirUtilities;

namespace VMDirSnapIn
{
    public class VMDirEnvironment
    {
        private static string DATA_FILE_NAME = "VMDirData.xml";
        
        public LocalData LocalData { get; set; }
        
        string _applicationPath;
        protected static VMDirEnvironment _instance;
        public SnapIn SnapIn { get; set; }
        private ILogger _logger;
        public List<Image> ImageLst = new List<Image>();

        public Image GetToolbarImage()
        {
            return MiscUtilsService.GetResourceImage("VMDirSnapIn.Images.Toolbar.bmp");
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

        public static VMDirEnvironment Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new VMDirEnvironment();
                return _instance;
            }
        }

        public void LoadLocalData()
        {
            if (!File.Exists(StoreFileName))
            {
                LocalData = new LocalData();
                return;
            }

            MiscUtilsService.CheckedExec(delegate
           {
               using (var ms = new MemoryStream())
               {
                   var bytes = File.ReadAllBytes(StoreFileName);
                   ms.Write(bytes, 0, bytes.Length);
                   ms.Seek(0, SeekOrigin.Begin);

                   var xmlSerializer = new XmlSerializer(typeof(LocalData));
                   LocalData = xmlSerializer.Deserialize(ms) as LocalData;
               }
           });
        }

        public void SaveLocalData()
        {
            MiscUtilsService.CheckedExec(delegate
           {
               using (var ms = new MemoryStream())
               {
                   var xmlSerializer = new XmlSerializer(typeof(LocalData));
                   xmlSerializer.Serialize(ms, VMDirEnvironment.Instance.LocalData);

                   File.WriteAllBytes(StoreFileName, ms.ToArray());
               }
           });
        }

        public Icon GetIconResource(VMDirIconIndex indx)
        {
            object obj = Resource.ResourceManager.GetObject(indx.ToString(), Resource.Culture);
            return (Icon)(obj);
        }
        public Image GetImageResource(VMDirIconIndex indx)
        {
            return GetIconResource(indx).ToBitmap();
        }
        public ILogger Logger
        {
            get
            {
                if (_logger == null)
                {
                    var logFolder = MMCUIConstants.GetLogFolder(Environment.UserName);
                    var filePath = string.Format("{0}{1}", logFolder, MMCUIConstants.VMDIR_LOG_FILE);
                    _logger = new FileLogger(filePath);
                }
                return _logger;
            }
        }
    }
}
