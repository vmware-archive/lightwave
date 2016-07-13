/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
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

/*
 * NOTE:
 * This is manual implementation of Tuple class.
 * Remove this when interop is upgraded to .Net Framework 4.0 or above.
 */
namespace VmDirInterop.Schema.Utils
{
    public class Tuple<T1, T2>
    {
        public T1 item1 { get; private set; }
        public T2 item2 { get; private set; }
        public Tuple(T1 t1, T2 t2)
        {
            item1 = t1;
            item2 = t2;
        }
    }
}
