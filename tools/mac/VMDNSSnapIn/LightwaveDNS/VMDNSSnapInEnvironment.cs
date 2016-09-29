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
using VmIdentity.CommonUtils;
using Foundation;
using System.IO;
using VmIdentity.CommonUtils.Persistance;
using System.Xml.Serialization;
using VMDNS.Common;
using VmIdentity.UI.Common;

namespace VMDNS
{
    public class VMDNSSnapInEnvironment : VMBaseSnapInEnvironment
    {
        public static VMBaseSnapInEnvironment Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new VMDNSSnapInEnvironment();
                    instance.SetSnapInDataFileName(VMDNSConstants.VMDNS_DATA_FILENAME);
                }
                return instance;
            }
        }

        public  override string FileNamePath
        {
            get
            {
                return Path.Combine(GetApplicationPath(), DataFileName);
            }
        }

        public  override string GetApplicationPath()
        {
            if (string.IsNullOrEmpty(ApplicationPath))
            {
                string[] paths = NSSearchPath.GetDirectories(NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
                if (paths.Length > 0)
                {
                    ApplicationPath = paths[0] + @"/" + VMIdentityConstants.APPLICATION_DATA_FOLDER_NAME;
                    if (!Directory.Exists(ApplicationPath))
                    {
                        Directory.CreateDirectory(ApplicationPath);
                    }
                }
            }
            return ApplicationPath;
        }

    }
}

