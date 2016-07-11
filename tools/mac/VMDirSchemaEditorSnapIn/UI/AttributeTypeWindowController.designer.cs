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
    [Register("AttributeTypeWindowController")]
    partial class AttributeTypeWindowController
    {
        [Outlet]
        public AppKit.NSButton ActionButton { get; set; }

        [Outlet]
        public AppKit.NSPopUpButton AttribtueSyntaxPopUp { get; set; }

        [Outlet]
        public AppKit.NSTextField AttributeDescription { get; set; }

        [Outlet]
        public AppKit.NSTextField AttributeName { get; set; }

        [Outlet]
        public AppKit.NSTextField AttributeX500ID { get; set; }

        [Outlet]
        public AppKit.NSButton MultiValuedCheckBox { get; set; }

        [Action("OnClickActionButton:")]
        partial void OnClickActionButton(Foundation.NSObject sender);

        void ReleaseDesignerOutlets()
        {
            if (AttribtueSyntaxPopUp != null)
            {
                AttribtueSyntaxPopUp.Dispose();
                AttribtueSyntaxPopUp = null;
            }

            if (ActionButton != null)
            {
                ActionButton.Dispose();
                ActionButton = null;
            }

            if (AttributeDescription != null)
            {
                AttributeDescription.Dispose();
                AttributeDescription = null;
            }

            if (AttributeName != null)
            {
                AttributeName.Dispose();
                AttributeName = null;
            }

            if (MultiValuedCheckBox != null)
            {
                MultiValuedCheckBox.Dispose();
                MultiValuedCheckBox = null;
            }

            if (AttributeX500ID != null)
            {
                AttributeX500ID.Dispose();
                AttributeX500ID = null;
            }
        }
    }
}
