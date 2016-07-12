using System;

using Foundation;
using AppKit;

namespace VMDirSchemaEditorSnapIn
{
    public partial class ViewDiff : NSWindow
    {
        public ViewDiff(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public ViewDiff(NSCoder coder)
            : base(coder)
        {
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
        }
    }
}
