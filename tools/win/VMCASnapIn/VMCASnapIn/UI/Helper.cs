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
using System.IO;
using VMCA.Client;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMCASnapIn.UI
{
    public static class Helper
    {
        public static void SaveKeyData(KeyPairData data)
        {
            MMCMiscUtil.SaveDataToFile(data.PrivateKey,"Save private key",MMCUIConstants.PRI_KEY_FILTER);
            MMCMiscUtil.SaveDataToFile(data.PublicKey, "Save public key", MMCUIConstants.PUB_KEY_FILTER);
        }
    }
}
