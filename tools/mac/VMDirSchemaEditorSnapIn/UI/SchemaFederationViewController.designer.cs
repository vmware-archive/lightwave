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

// WARNING
//
// This file has been generated automatically by Xamarin Studio to store outlets and
// actions made in the UI designer. If it is removed, they will be lost.
// Manual changes to this file may not be handled correctly.
//
using Foundation;
using System.CodeDom.Compiler;

namespace VMDirSchemaEditorSnapIn
{
    [Register("SchemaFederationViewController")]
    partial class SchemaFederationViewController
    {
        [Outlet]
        AppKit.NSButton CompareButton { get; set; }

        [Outlet]
        AppKit.NSMatrix CompareRadioMatrix { get; set; }

        [Outlet]
        AppKit.NSTextView DetailsView { get; set; }

        [Outlet]
        AppKit.NSOutlineView FederationNodeView { get; set; }

        [Outlet]
        AppKit.NSTextField FederationViewMessage { get; set; }

        [Outlet]
        AppKit.NSTableView NodesTableView { get; set; }

        [Outlet]
        AppKit.NSTextField SchemaViewTitle { get; set; }

        [Outlet]
        AppKit.NSButton ViewAttributeTypeDiffButton { get; set; }

        [Outlet]
        AppKit.NSButton ViewObjectClassDiffButton { get; set; }

        [Action("ChooseCompareOption:")]
        partial void ChooseCompareOption(Foundation.NSObject sender);

        [Action("OnCompare:")]
        partial void OnCompare(Foundation.NSObject sender);

        void ReleaseDesignerOutlets()
        {
            if (CompareButton != null)
            {
                CompareButton.Dispose();
                CompareButton = null;
            }

            if (CompareRadioMatrix != null)
            {
                CompareRadioMatrix.Dispose();
                CompareRadioMatrix = null;
            }

            if (DetailsView != null)
            {
                DetailsView.Dispose();
                DetailsView = null;
            }

            if (FederationNodeView != null)
            {
                FederationNodeView.Dispose();
                FederationNodeView = null;
            }

            if (FederationViewMessage != null)
            {
                FederationViewMessage.Dispose();
                FederationViewMessage = null;
            }

            if (NodesTableView != null)
            {
                NodesTableView.Dispose();
                NodesTableView = null;
            }

            if (SchemaViewTitle != null)
            {
                SchemaViewTitle.Dispose();
                SchemaViewTitle = null;
            }

            if (ViewObjectClassDiffButton != null)
            {
                ViewObjectClassDiffButton.Dispose();
                ViewObjectClassDiffButton = null;
            }

            if (ViewAttributeTypeDiffButton != null)
            {
                ViewAttributeTypeDiffButton.Dispose();
                ViewAttributeTypeDiffButton = null;
            }
        }
    }
}
