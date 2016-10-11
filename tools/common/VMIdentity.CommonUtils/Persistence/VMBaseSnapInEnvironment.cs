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
using VmIdentity.CommonUtils.Persistance;


namespace VmIdentity.CommonUtils
{
    public abstract class VMBaseSnapInEnvironment
    {
        protected  string DataFileName { get; set; }

        public LocalData LocalData { get; set; }

        protected string ApplicationPath;
        public   static VMBaseSnapInEnvironment instance;

        public abstract string GetApplicationPath();

        public void SetSnapInDataFileName(string name)
        {
            DataFileName = name;
        }

        public  abstract string FileNamePath{ get; }

        public void LoadLocalData()
        {
            if (FileNamePath != null)
            {
                if (!File.Exists(FileNamePath))
                    LocalData = new LocalData();
                else
                {
                    try
                    {
                        using (var ms = new MemoryStream())
                        {
                            var bytes = File.ReadAllBytes(FileNamePath);
                            ms.Write(bytes, 0, bytes.Length);
                            ms.Seek(0, SeekOrigin.Begin);
                            var xmlSerializer = new XmlSerializer(typeof(LocalData));
                            LocalData = xmlSerializer.Deserialize(ms) as LocalData;
                        }
                    }
                    catch (Exception)
                    {
                        //Exception during reading or deserializing file - ignore and continue.
                    }
                }
            }
        }

		/// <summary>
		/// Saves the local data.
		/// </summary>
		public void SaveLocalData ()
		{
			try {
				using (var ms = new MemoryStream ()) {
					var xmlSerializer = new XmlSerializer (typeof(LocalData));
					xmlSerializer.Serialize (ms, LocalData);
					File.WriteAllBytes (FileNamePath, ms.ToArray ());
				}
			} catch (Exception) {
			}
		}
    }
}


