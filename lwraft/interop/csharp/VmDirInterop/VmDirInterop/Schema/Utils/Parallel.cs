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
using System.Threading;

/*
 * NOTE:
 * This is manual implementation of Parallel class.
 * Remove this when interop is upgraded to .Net Framework 4.0 or above.
 */
namespace VmDirInterop.Schema.Utils
{
    class Parallel
    {
        public static void ForEach<T>(IEnumerable<T> items, Action<T> action)
        {
            if (items == null)
                throw new ArgumentNullException("enumerable");
            if (action == null)
                throw new ArgumentNullException("action");

            var resetEvents = new List<ManualResetEvent>();

            foreach (var item in items)
            {
                var evt = new ManualResetEvent(false);
                ThreadPool.QueueUserWorkItem((i) =>
                {
                    action((T)i);
                    evt.Set();
                }, item);
                resetEvents.Add(evt);
            }

            foreach (ManualResetEvent e in resetEvents)
            {
                e.WaitOne();
            }
        }
    }
}
