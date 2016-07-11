using System;

using Foundation;
using AppKit;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SelectListItemsWindow : NSWindow
    {
        public SelectListItemsWindow(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public SelectListItemsWindow(NSCoder coder)
            : base(coder)
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
        }
    }
}
