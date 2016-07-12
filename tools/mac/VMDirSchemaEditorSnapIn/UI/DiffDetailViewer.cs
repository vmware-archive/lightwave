using System;

using Foundation;
using AppKit;

namespace VMDirSchemaEditorSnapIn
{
    public partial class DiffDetailViewer : NSWindow
    {
        public DiffDetailViewer(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public DiffDetailViewer(NSCoder coder)
            : base(coder)
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
        }
    }
}
