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
using VMCertStore.Client.Exceptions;

namespace VMAfd.Client
{
    public static class VMAfdClientError
    {
        public static void Check (UInt32 resultCode)
        {
            if (resultCode == VMAfdAdaptor.SUCCESS)
                return;

            if (resultCode == 40700)
                throw new VMAfdValueNotSetException (resultCode, "Value not set");

            if (resultCode == 382312491)
                throw new VMAfdException (resultCode, "The network address is not valid");

            string errorString = "unknown afd error";
            throw new VMAfdException (resultCode, errorString);
        }
    }
}
