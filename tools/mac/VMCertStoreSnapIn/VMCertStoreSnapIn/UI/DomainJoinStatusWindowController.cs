
using System;
using System.Collections.Generic;
using System.Linq;
using MonoMac.Foundation;
using MonoMac.AppKit;

namespace VMCertStoreSnapIn
{
    public partial class DomainJoinStatusWindowController : MonoMac.AppKit.NSWindowController
    {
        #region Constructors

        // Called when created from unmanaged code
        public DomainJoinStatusWindowController (IntPtr handle) : base (handle)
        {
            Initialize ();
        }
		
        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public DomainJoinStatusWindowController (NSCoder coder) : base (coder)
        {
            Initialize ();
        }
		
        // Call to load from the XIB/NIB file
        public DomainJoinStatusWindowController () : base ("DomainJoinStatusWindow")
        {
            Initialize ();
        }
		
        // Shared initialization code
        void Initialize ()
        {
        }

        #endregion

        //strongly typed window accessor
        public new DomainJoinStatusWindow Window {
            get {
                return (DomainJoinStatusWindow)base.Window;
            }
        }
    }
}

