
using System;
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;

namespace RestSsoAdminSnapIn
{
	public partial class ViewGroupDetailsController : AppKit.NSWindowController
	{
		#region Constructors

		// Called when created from unmanaged code
		public ViewGroupDetailsController (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ViewGroupDetailsController (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Call to load from the XIB/NIB file
		public ViewGroupDetailsController () : base ("ViewGroupDetails")
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion

		//strongly typed window accessor
		public new ViewGroupDetails Window {
			get {
				return (ViewGroupDetails)base.Window;
			}
		}
	}
}

