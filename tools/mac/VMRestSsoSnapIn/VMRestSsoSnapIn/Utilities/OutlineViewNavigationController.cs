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

namespace RestSsoAdminSnapIn
{
    // Caches the recently selected rows in the outlineview to be used in back/forward buttons in the toolbar.
    public class OutlineViewNavigationController
    {
        
        public int CurrentSelection { get; set; }

        private Stack<int> previousStack = new Stack<int> ();
        private Stack<int> forwardStack = new Stack<int> ();

        public OutlineViewNavigationController ()
        {
            CurrentSelection = -1;
        }

        public void AddPreviousSelectedRow (int row)
        {
            if (CurrentSelection != -1)
                previousStack.Push (CurrentSelection);
            CurrentSelection = row;
        }

        public int GetPreviousSelectedRow ()
        {
            if (CurrentSelection != -1) {
                forwardStack.Push (CurrentSelection);
                CurrentSelection = -1;
            }
            if (previousStack.Count > 0) {
                int previous = previousStack.Pop ();
                if (!((forwardStack.Count > 0) && (forwardStack.Peek () == previous)))
                    forwardStack.Push (previous);
                return previous;
            }
            return -1;
        }

        public int GetForwardSelectedRow ()
        {
            if (CurrentSelection != -1) {
                previousStack.Push (CurrentSelection);
                CurrentSelection = -1;
            }
            if (forwardStack.Count > 0) {
                int next = forwardStack.Pop ();
                if (!((previousStack.Count > 0) && (previousStack.Peek () == next)))
                    previousStack.Push (next);
                return next;
            }
            return -1;
        }
    }
}

