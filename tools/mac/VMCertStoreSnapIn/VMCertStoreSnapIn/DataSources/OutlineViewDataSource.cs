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
using VMCertStoreSnapIn.Nodes;
using AppKit;
using Foundation;

namespace VMCertStoreSnapIn.DataSource
{
    public class OutlineViewDataSource : NSOutlineViewDataSource
    {
        public ScopeNode RootNode { get; set; }

        public OutlineViewDataSource (ChildScopeNode node) : base ()
        {
            RootNode = node;
        }

        public override nint GetChildrenCount (NSOutlineView outlineView, NSObject item)
        {
            // if null, it's asking about the root element
            if (item == null) {
                return 1;
            } else {
                ScopeNode passedNode = item as ScopeNode;
                if (passedNode != null) {
                    return passedNode.NumberOfChildren ();
                } else {
                    System.Diagnostics.Debug.WriteLine ("could not cast, there is a problem here");
                    return 0;
                }
            }
        }

        public override bool ItemExpandable (NSOutlineView outlineView, NSObject item)
        {
            if (item != null) {
                try {

                    ScopeNode passedNode = item as ScopeNode; // cast to appropriate type of node
                    if (passedNode != null) {
                        return (passedNode.NumberOfChildren () != 0);
                    } else {
                        System.Diagnostics.Debug.WriteLine ("passedNode cast failed.");
                        return false;
                    }
                } catch (Exception e) {
                    System.Diagnostics.Debug.WriteLine (e);
                    return false;
                }
            } else {
                // if null, it's asking about the root element
                return true;
            }
        }

        public override NSObject GetObjectValue (NSOutlineView outlineView, NSTableColumn tableColumn, NSObject item)
        {
            if (item == null) {
                System.Diagnostics.Debug.WriteLine ("passed null, returning empty String");
                return new NSString (" ");
            } else {
                ScopeNode passedNode = item as ScopeNode;
                if (passedNode != null) {
                    return (NSString)passedNode.DisplayName;
                } else {
                    System.Diagnostics.Debug.WriteLine ("returning an empty string, cast failed.");
                    return new NSString ();
                }
            }
        }

        public override NSObject GetChild (NSOutlineView outlineView, nint childIndex, NSObject item)
        {
            // null means it's asking for the root
            if (item == null) {
                return (NSObject)RootNode;
            } else {
                return (NSObject)((item as ScopeNode).ChildAtIndex ((int)childIndex));
            }
        }
    }
}


