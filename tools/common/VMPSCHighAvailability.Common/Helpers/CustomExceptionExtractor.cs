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
using VMAFD.Client;
using VMDIR.Client;
namespace VMPSCHighAvailability.Common.Helpers
{
    public class CustomExceptionExtractor
    {

        public string GetCustomMessage(Exception exp)
        {
             string custom = string.Empty;

                if (exp is VmAfdException)
                {
                   var exception = exp as VmAfdException;

                    if(exception != null)
                        custom = string.Format("VMAFD Exception: ErrorCode: {0} Message: {1}  StackTrace: {2}", exception.ErrorCode, exception.Message, exception.StackTrace);
                } else if (exp is VmDirException)
                    {
                       var exception = exp as VmDirException;

                        if(exception != null)
                            custom = string.Format("VMDIR Exception: ErrorCode: {0} Message: {1}  StackTrace: {2}", exception.ErrorCode, exception.Message, exception.StackTrace);
                    }
            return custom;
        }
    }
}
