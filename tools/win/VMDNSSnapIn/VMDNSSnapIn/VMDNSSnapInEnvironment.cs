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

using Microsoft.ManagementConsole;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using VMDNS.Common;
using VmIdentity.CommonUtils;

/*
 * @author Sumalatha Abhishek
 */

namespace VMDNSSnapIn
{
    public class VMDNSSnapInEnvironment : VMBaseSnapInEnvironment
    {
        public SnapIn SnapIn { get; set; }
        private new static VMDNSSnapInEnvironment instance;

        public static VMDNSSnapInEnvironment Instance
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

        public override string FileNamePath
        {
            get
            {
                return Path.Combine(GetApplicationPath(), DataFileName);
            }
        }

        public override string GetApplicationPath()
        {
                if (string.IsNullOrEmpty(ApplicationPath))
                    ApplicationPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                return ApplicationPath;
        }

    }
}
