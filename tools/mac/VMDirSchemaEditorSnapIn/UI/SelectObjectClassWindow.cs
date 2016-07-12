using System;

using Foundation;
using AppKit;

namespace VMDirSchemaEditorSnapIn
{
    public partial class SelectObjectClassWindow : NSWindow
    {
        public SelectObjectClassWindow(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public SelectObjectClassWindow(NSCoder coder)
            : base(coder)
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
        }
    }
}
