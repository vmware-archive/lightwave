using System;

using Foundation;
using AppKit;

namespace UI
{
    public partial class SuperLoggingWindow : NSWindow
    {
        public SuperLoggingWindow (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public SuperLoggingWindow (NSCoder coder) : base (coder)
        {
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
        }
    }
}
