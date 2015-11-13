
using System;
using System.Collections.Generic;
using System.Linq;
using MonoMac.Foundation;
using MonoMac.AppKit;

namespace VMCertStoreSnapIn
{
    public partial class JoinActiveDirectoryWindowController : MonoMac.AppKit.NSWindowController
    {
        #region Constructors

        // Called when created from unmanaged code
        public JoinActiveDirectoryWindowController (IntPtr handle) : base (handle)
        {
            Initialize ();
        }
		
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public JoinActiveDirectoryWindowController (NSCoder coder) : base (coder)
        {
            Initialize ();
        }
		
        // Call to load from the XIB/NIB file
        public JoinActiveDirectoryWindowController () : base ("JoinActiveDirectoryWindow")
        {
            Initialize ();
        }
		
        // Shared initialization code
        void Initialize ()
        {
        }

        #endregion

        //strongly typed window accessor
        public new JoinActiveDirectoryWindow Window {
            get {
                return (JoinActiveDirectoryWindow)base.Window;
            }
        }
    }
}

