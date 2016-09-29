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
using AppKit;
using Foundation;
using VMDNS.Nodes;
using VmIdentity.UI.Common;

namespace VMDNS
{
    public class OutlineViewDataSource : NSOutlineViewDataSource
    {
        public ScopeNodeBase RootNode { get; set; }

        public OutlineViewDataSource(ScopeNodeBase node)
            : base()
        {
            RootNode = node;
        }

        public override nint GetChildrenCount(NSOutlineView outlineView, NSObject item)
        {
            // if null, it's asking about the root element
            if (item == null)
            {
                return 1;
            }
            else
            {
                ScopeNodeBase passedNode = item as ScopeNodeBase;
                if (passedNode != null)
                {
                    return passedNode.NumberOfChildren();
                }
                else
                {
                    return 0;
                }
            }
        }

        public override bool ItemExpandable(NSOutlineView outlineView, NSObject item)
        {
            if (item != null)
            {
                ScopeNodeBase passedNode = item as ScopeNodeBase; // cast to appropriate type of node
                if (passedNode != null)
                {
                    return (passedNode.NumberOfChildren() != 0);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                // if null, it's asking about the root element
                return true;
            }
        }

        public override NSObject GetObjectValue(NSOutlineView outlineView, NSTableColumn tableColumn, NSObject item)
        {
            if (item == null)
            {
                return new NSString(" ");
            }
            else
            {
                ScopeNodeBase passedNode = item as ScopeNodeBase;
                if (passedNode != null)
                {
                    return (NSString)passedNode.DisplayName;
                }
                else
                {
                    return new NSString();
                }
            }
        }

        public override NSObject GetChild(NSOutlineView outlineView, nint childIndex, NSObject item)
        {
            // null means it's asking for the root
            if (item == null)
            {
                return (NSObject)RootNode;
            }
            else
            {
                return (NSObject)((item as ScopeNodeBase).ChildAtIndex((int)childIndex));
            }
        }
    }
}


