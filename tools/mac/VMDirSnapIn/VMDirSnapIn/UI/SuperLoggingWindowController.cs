using System;

using Foundation;
using AppKit;

namespace UI
{
    public partial class SuperLoggingWindowController : NSWindowController
    {
        public SuperLoggingWindowController (IntPtr handle) : base (handle)
        {
        }

        [Export ("initWithCoder:")]
        public SuperLoggingWindowController (NSCoder coder) : base (coder)
        {
        }

        public SuperLoggingWindowController () : base ("SuperLoggingWindow")
        {
        }

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();
        }

        public new SuperLoggingWindow Window {
            get { return (SuperLoggingWindow)base.Window; }
        }
    }
}
