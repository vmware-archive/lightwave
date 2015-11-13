
using System;
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;

namespace VmIdentity.UI.Common
{
    public partial class ServerView : AppKit.NSView
    {
        #region Constructors

        // Called when created from unmanaged code
        public ServerView (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public ServerView (NSCoder coder) : base (coder)
        {
        }

        #endregion
    }
}

