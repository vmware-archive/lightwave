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
using Foundation;
using AppKit;
using VMDirSchemaEditorSnapIn.Nodes;
using VMDirSchemaEditorSnapIn.ListViews;
using VmDirInterop.Schema.Interfaces;
using VmDirInterop.Schema.Diffs;
using VmIdentity.UI.Common.Utilities;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Metadata;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SchemaFederationViewController : AppKit.NSViewController
    {
        private OutlineViewDataSource outlineViserverewDataSource;

        private FederationNodeBase rootNode;
        private StringItemsListView nodesDataSource;
        NSImage NodeUpIcon, NodeDownIcon, BaseIcon, CompareIcon;
        IDictionary<string,SchemaDefinitionDiff> attrTypediff;
        IDictionary<String, SchemaMetadataDiff> schemaDiff;
        List<KeyValuePair<string,string>> ObjectClassDiff = new List<KeyValuePair<string,string>>();
        List<KeyValuePair<string,string>> AttrDiff = new List<KeyValuePair<string,string>>();
        List<KeyValuePair<string,string>> MetaDataDiff = new List<KeyValuePair<string,string>>();
        OutlineViewDataSource outlineViewDataSource;

        public  VMDirSchemaServerNode ServerNode { get; set; }

        public string CurrentNode { get; set; }

        #region Constructors


        // Called when created from unmanaged code
        public SchemaFederationViewController(IntPtr handle)
            : base(handle)
        {
            Initialize();
        }

        // Called when created directly from a XIB file
        [Export("initWithCoder:")]
        public SchemaFederationViewController(NSCoder coder)
            : base(coder)
        {
            Initialize();
        }

        // Call to load from the XIB/NIB file
        public SchemaFederationViewController()
            : base("SchemaFederationView", NSBundle.MainBundle)
        {
            Initialize();
        }

        public void Initialize()
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            InitialiseWindow();
        }

        private void LoadIcons()
        {
            NodeUpIcon = NSImage.ImageNamed("nodeupicon.png");
            BaseIcon = NSImage.ImageNamed("directoryObject.png");
            NodeDownIcon = NSImage.ImageNamed("nodedownicon");
            CompareIcon = NSImage.ImageNamed("comparison.png_48x48.png");

        }

        partial void OnCompare(Foundation.NSObject sender)
        {
            try
            {
                switch (this.CompareRadioMatrix.SelectedTag)
                {

                    case 0:
                        ViewAttributeTypeDiffButton.Hidden = false;
                        ViewObjectClassDiffButton.Hidden = false;
                        ViewObjectClassDiffButton.Title = VMDirSchemaConstants.DIFF_OBJECTCLASS;
                        attrTypediff = ServerNode.ServerDTO.Connection.SchemaConnection.GetAllSchemaDefinitionDiffs();
                        SetNodesDataSource(attrTypediff);
                        break;
                    case 1:
                        ViewAttributeTypeDiffButton.Hidden = true;
                        ViewObjectClassDiffButton.Hidden = false;
                        ViewObjectClassDiffButton.Title = VMDirSchemaConstants.DIFF_METADATA;
                        schemaDiff = ServerNode.ServerDTO.Connection.SchemaConnection.GetAllSchemaMetadataDiffs();
                        SetNodesDataSourceForMetaData(schemaDiff);
                        break;
                    default:
                        break;
                }
            }
            catch (Exception e)
            {
                UIErrorHelper.ShowAlert(e.Message, String.Empty);
            }
        }

        public void SetNodesDataSourceForMetaData(IDictionary<string,SchemaMetadataDiff> diff)
        {
            nodesDataSource = new StringItemsListView(diff.Keys.ToList());
            NodesTableView.DataSource = nodesDataSource;
        }

        public void SetNodesDataSource(IDictionary<string,SchemaDefinitionDiff> diff)
        {
            nodesDataSource = new StringItemsListView(diff.Keys.ToList());
            NodesTableView.DataSource = nodesDataSource;
        }

        // Shared initialization code
        void InitialiseWindow()
        {
            rootNode = new FederationNodeBase("Federation Nodes", ServerNode);
            outlineViewDataSource = new OutlineViewDataSource(rootNode);
            FederationNodeView.DataSource = outlineViewDataSource;
            FederationNodeView.OutlineTableColumn.DataCell = new NSBrowserCell();
            LoadIcons();
            CompareButton.Image = CompareIcon;


            FederationNodeView.Delegate = new OutlineDelegate(this);
            ViewAttributeTypeDiffButton.Activated += ViewDiffButtonClicked;
            ViewObjectClassDiffButton.Activated += ViewDiffButtonClicked;
            NodesTableView.Enabled = true;

        }


        #endregion

        private void ParseAttrType()
        {
            int row = (int)NodesTableView.SelectedRow;
            AttrDiff.Clear();
            if (row >= 0)
            {

                KeyValuePair<String, SchemaDefinitionDiff> p = attrTypediff.ElementAt(row);
                {
                    CurrentNode = p.Key;
                    SchemaDefinitionDiff diff = p.Value;

                    if (diff != null)
                    {
                        foreach (VmDirInterop.Schema.Utils.Tuple<AttributeType, AttributeType> t in diff.GetAttributeTypeDiff())
                        {
                            string baseAttr = (t.item1 != null) ? t.item1.ToString() : VMDirSchemaConstants.MISSING_ATTRIBUTETYPE;
                            string currentAttr = (t.item2 != null) ? t.item2.ToString() : VMDirSchemaConstants.MISSING_ATTRIBUTETYPE;
                            AttrDiff.Add(new KeyValuePair<string,string>(baseAttr, currentAttr));
                        }
                    }
                    else
                        throw new Exception(VMDirSchemaConstants.NO_DATA_FOUND);
                }
            }
        }

        private void ParseObjectClass()
        {
            int row = (int)NodesTableView.SelectedRow;
            ObjectClassDiff.Clear();
            if (row >= 0)
            {

                KeyValuePair<String, SchemaDefinitionDiff> p = attrTypediff.ElementAt(row);
                {
                    CurrentNode = p.Key;
                    SchemaDefinitionDiff diff = p.Value;

                    if (diff != null)
                    {
                        foreach (VmDirInterop.Schema.Utils.Tuple<ObjectClass, ObjectClass> t in diff.GetObjectClassDiff())
                        {
                            string baseObject = (t.item1 != null) ? t.item1.ToString() : VMDirSchemaConstants.MISING_OBJECTCLASS;
                            string currentObject = (t.item2 != null) ? t.item2.ToString() : VMDirSchemaConstants.MISING_OBJECTCLASS;
                            ObjectClassDiff.Add(new KeyValuePair<string,string>(baseObject, currentObject));
                        } 
                    }
                    else
                        throw new Exception(VMDirSchemaConstants.NO_DATA_FOUND);
                }
            }
        }

        private void ParseMetaData()
        {
            int row = (int)NodesTableView.SelectedRow;
            MetaDataDiff.Clear();
            if (row >= 0)
            {
                KeyValuePair<String, SchemaMetadataDiff> p = schemaDiff.ElementAt(row);
                {
                    CurrentNode = p.Key;
                    SchemaMetadataDiff diff = p.Value;
                    if (diff != null)
                    {
                        foreach (VmDirInterop.Schema.Utils.Tuple<SchemaEntry, SchemaEntry> t in diff.GetAttributeTypeDiff())
                        {
                            listSchemaMetadataDiffBreakdown(t.item1, t.item2);
                        }
                        foreach (VmDirInterop.Schema.Utils.Tuple<SchemaEntry, SchemaEntry> t in diff.GetObjectClassDiff())
                        {
                            listSchemaMetadataDiffBreakdown(t.item1, t.item2);
                        }
                    }
                    else
                        throw new Exception(VMDirSchemaConstants.NO_DATA_FOUND);
                }
            }
        }



        private  void listSchemaMetadataDiffBreakdown(SchemaEntry e1, SchemaEntry e2)
        {
            SchemaComparableList<AttributeMetadata> mdList1 = null;
            SchemaComparableList<AttributeMetadata> mdList2 = null;
            if (e1 != null && e2 != null)
            {
                
                mdList1 = e1.GetMetadataList();
                mdList2 = e2.GetMetadataList();

                VmDirInterop.Schema.Utils.TupleList<AttributeMetadata, AttributeMetadata> diff = mdList1.GetDiff(mdList2);

                foreach (VmDirInterop.Schema.Utils.Tuple<AttributeMetadata, AttributeMetadata> t in diff)
                {
                    //baseMetaData
                    string baseData = (t.item1 != null) ? e1.defName + " : " + t.item1.ToString() : VMDirSchemaConstants.MISING_METADATA;
                    string currentData = (t.item2 != null) ? e1.defName + " : " + t.item2.ToString() : VMDirSchemaConstants.MISING_METADATA;
                    MetaDataDiff.Add(new KeyValuePair<string, string>(baseData, currentData));
                }
            }
            else if (e1 != null)
            {
                mdList1 = e1.GetMetadataList();
                foreach (AttributeMetadata md in mdList1)
                {
                    string baseData = e1.defName + " : " + md;
                    string currentData = VMDirSchemaConstants.MISING_METADATA;
                    MetaDataDiff.Add(new KeyValuePair<string, string>(baseData, currentData));
                }
            }
            else
            {
                mdList2 = e2.GetMetadataList();
                foreach (AttributeMetadata md in mdList2)
                {
                    string currentData = e1.defName + " : " + md;
                    string baseData = VMDirSchemaConstants.MISING_METADATA;
                    MetaDataDiff.Add(new KeyValuePair<string, string>(baseData, currentData));
                }
            }
        }

        public void ViewDiffButtonClicked(object sender, EventArgs e)
        {
            
            UIErrorHelper.CheckedExec(delegate()
                {

                    NSButton button = sender as NSButton;
                    if (button.Title == VMDirSchemaConstants.DIFF_ATTRIBUTETYPE)
                    {
                        ParseAttrType();
                        if (AttrDiff == null || AttrDiff.Count == 0)
                        {
                            UIErrorHelper.ShowAlert(VMDirSchemaConstants.NO_DIFF_FOUND, string.Empty);
                        }
                        else
                        {
                            ViewDiffController vdc = new ViewDiffController(this.ServerNode.ServerDTO.Server, this.CurrentNode, AttrDiff);
                            NSApplication.SharedApplication.RunModalForWindow(vdc.Window);
                        }
                    }
                    else if (button.Title == VMDirSchemaConstants.DIFF_OBJECTCLASS)
                    {
                        ParseObjectClass();
                        if (ObjectClassDiff == null || ObjectClassDiff.Count == 0)
                        {
                            UIErrorHelper.ShowAlert(VMDirSchemaConstants.NO_DIFF_FOUND, string.Empty);
                        }
                        else
                        {
                            ViewDiffController vc = new ViewDiffController(this.ServerNode.ServerDTO.Server, this.CurrentNode, ObjectClassDiff);
                            NSApplication.SharedApplication.RunModalForWindow(vc.Window);
                        }
                    }
                    else
                    {
                        ParseMetaData();
                        if (MetaDataDiff == null || MetaDataDiff.Count == 0)
                        {
                            UIErrorHelper.ShowAlert(VMDirSchemaConstants.NO_DIFF_FOUND, string.Empty);
                        }
                        else
                        {
                            ViewDiffController mvc = new ViewDiffController(this.ServerNode.ServerDTO.Server, this.CurrentNode, MetaDataDiff);
                            NSApplication.SharedApplication.RunModalForWindow(mvc.Window);
                        }
                    }

                });
        }

        //strongly typed view accessor
        public new SchemaFederationView View
        {
            get
            {
                return (SchemaFederationView)base.View;
            }
        }

        //Delegate classes

        class OutlineDelegate : NSOutlineViewDelegate
        {
            SchemaFederationViewController ob;

            public OutlineDelegate(SchemaFederationViewController ob)
            {
                this.ob = ob;
            }

            public override void WillDisplayCell(NSOutlineView outlineView, NSObject cell,
                                                 NSTableColumn tableColumn, NSObject item)
            {
                try
                {
                    NSBrowserCell browserCell = cell as NSBrowserCell;
                    if (browserCell != null)
                    {
                        browserCell.Leaf = true;
                        if (item is FederationNode)
                        {
                            if ((item as FederationNode).isNodeUp)
                                browserCell.Image = ob.NodeUpIcon;
                            else
                                browserCell.Image = ob.NodeDownIcon;
                        }
                        else
                            browserCell.Image = ob.BaseIcon;
                    }

                }
                catch (Exception e)
                {
                }
            }

        }

    }
}
