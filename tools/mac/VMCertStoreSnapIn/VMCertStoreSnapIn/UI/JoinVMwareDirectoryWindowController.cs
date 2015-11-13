
using System;
using System.Collections.Generic;
using System.Linq;
using MonoMac.Foundation;
using MonoMac.AppKit;

namespace VMCertStoreSnapIn
{
    public partial class JoinVMwareDirectoryWindowController : MonoMac.AppKit.NSWindowController
    {
        #region Constructors

        // Called when created from unmanaged code
        public JoinVMwareDirectoryWindowController (IntPtr handle) : base (handle)
        {
            Initialize ();
        }
		
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public JoinVMwareDirectoryWindowController (NSCoder coder) : base (coder)
        {
            Initialize ();
        }
		
        // Call to load from the XIB/NIB file
        public JoinVMwareDirectoryWindowController () : base ("JoinVMwareDirectoryWindow")
        {
            Initialize ();
        }
		
        // Shared initialization code
        void Initialize ()
        {
        }

        #endregion

        //strongly typed window accessor
        public new JoinVMwareDirectoryWindow Window {
            get {
                return (JoinVMwareDirectoryWindow)base.Window;
            }
        }
    }
}

