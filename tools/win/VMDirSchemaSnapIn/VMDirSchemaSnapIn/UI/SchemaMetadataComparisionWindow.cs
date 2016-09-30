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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using VmDirInterop.Schema;
using VmDirInterop.Schema.Definitions;
using VmDirInterop.Schema.Diffs;
using VmDirInterop.Schema.Entries;
using VmDirInterop.Schema.Metadata;
using VMDirSchemaEditorSnapIn.Nodes;
using VMwareMMCIDP.UI.Common.Utilities;

namespace VMDirSchemaSnapIn.UI
{
    public partial class SchemaMetadataComparisionWindow : Form
    {
        IDictionary<string, SchemaDefinitionDiff> attrTypediff;
        IDictionary<String, SchemaMetadataDiff> schemaDiff;
        List<KeyValuePair<string, string>> ObjectClassDiff = new List<KeyValuePair<string, string>>();
        List<KeyValuePair<string, string>> AttrDiff = new List<KeyValuePair<string, string>>();
        List<KeyValuePair<string, string>> MetaDataDiff = new List<KeyValuePair<string, string>>();

        public VMDirSchemaServerNode ServerNode { get; set; }

        public string CurrentNode { get; set; }

        public SchemaMetadataComparisionWindow(VMDirSchemaServerNode serverNode)
        {
            this.ServerNode = serverNode;
            InitializeComponent();
        }

        private void ParseAttrType()
        {
            int row = (int)this.NodesList.SelectedIndex;
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
                            AttrDiff.Add(new KeyValuePair<string, string>(baseAttr, currentAttr));
                        }
                    }
                    else
                        throw new Exception(VMDirSchemaConstants.NO_DATA_FOUND);
                }
            }
        }

        private void ParseObjectClass()
        {
            int row = (int)this.NodesList.SelectedIndex;
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
                            ObjectClassDiff.Add(new KeyValuePair<string, string>(baseObject, currentObject));
                        }
                    }
                    else
                        throw new Exception(VMDirSchemaConstants.NO_DATA_FOUND);
                }
            }
        }

        private void ParseMetaData()
        {
            int row = (int)this.NodesList.SelectedIndex;
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


        private void listSchemaMetadataDiffBreakdown(SchemaEntry e1, SchemaEntry e2)
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



            UIErrorHelper.CheckedExecNonModal(delegate()
            {
                Button button = sender as Button;
                if (button.Text == VMDirSchemaConstants.DIFF_ATTRIBUTETYPE)
                {
                    ParseAttrType();
                    if (AttrDiff == null || AttrDiff.Count == 0)
                    {
                        MMCDlgHelper.ShowInformation("No Diff Found");
                    }
                    else
                    {
                        var frm = new ViewDiffWindow(this.ServerNode.ServerDTO.Server, this.CurrentNode, MetaDataDiff);
                        frm.ShowDialog();
                       
                    }
                }
                else if (button.Text == VMDirSchemaConstants.DIFF_OBJECTCLASS)
                {
                    ParseObjectClass();
                    if (ObjectClassDiff == null || ObjectClassDiff.Count == 0)
                    {

                        MMCDlgHelper.ShowInformation("No Diff Found");
                    }
                    else
                    {
                        var frm = new ViewDiffWindow(this.ServerNode.ServerDTO.Server, this.CurrentNode, MetaDataDiff);
                        frm.ShowDialog();
                    }
                }
                else
                {
                    ParseMetaData();
                    if (MetaDataDiff == null || MetaDataDiff.Count == 0)
                    {
                        MMCDlgHelper.ShowInformation("No Diff Found");
                    }
                    else
                    {
                        var frm = new ViewDiffWindow(this.ServerNode.ServerDTO.Server, this.CurrentNode, MetaDataDiff);
                        frm.ShowDialog();
                    }
                }

            });
        }

        private void CompareButton_Click(object sender, EventArgs e)
        {
            try
            {
                if (this.MetaDataButton.Checked == true)
                {
                    ViewAttributeTypeDiffButton.Visible = true;
                    ViewObjectClassDiffButton.Visible = false;
                    ViewAttributeTypeDiffButton.Text = VMDirSchemaConstants.DIFF_METADATA;
                    schemaDiff = ServerNode.ServerDTO.Connection.SchemaConnection.GetAllSchemaMetadataDiffs();
                    NodesList.DataSource = attrTypediff.Keys.ToList();
                }
                else if (this.SchemaButton.Checked == true)
                {
                    ViewAttributeTypeDiffButton.Visible = true;
                    ViewObjectClassDiffButton.Visible = true;
                    ViewAttributeTypeDiffButton.Text = VMDirSchemaConstants.DIFF_ATTRIBUTETYPE;
                    attrTypediff = ServerNode.ServerDTO.Connection.SchemaConnection.GetAllSchemaDefinitionDiffs();
                    NodesList.DataSource = attrTypediff.Keys.ToList();
                }
            }
            catch (Exception ex)
            {
                MMCDlgHelper.ShowError(ex.Message);
            }
        }
    }
}
