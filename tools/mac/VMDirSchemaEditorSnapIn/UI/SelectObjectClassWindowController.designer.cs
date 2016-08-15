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
    [Register("SelectObjectClassWindowController")]
    partial class SelectObjectClassWindowController
    {
        [Outlet]
        AppKit.NSTableView ObjectClassList { get; set; }

        [Outlet]
        AppKit.NSSearchField SearchField { get; set; }

        [Outlet]
        AppKit.NSSearchFieldCell SearchFieldCell { get; set; }

        [Action("SelectAction:")]
        partial void SelectAction(Foundation.NSObject sender);

        void ReleaseDesignerOutlets()
        {
            if (SearchField != null)
            {
                SearchField.Dispose();
                SearchField = null;
            }

            if (SearchFieldCell != null)
            {
                SearchFieldCell.Dispose();
                SearchFieldCell = null;
            }

            if (ObjectClassList != null)
            {
                ObjectClassList.Dispose();
                ObjectClassList = null;
            }
        }
    }
}
