
using System;
using System.Collections.Generic;
using System.Linq;
using Foundation;
using AppKit;

namespace RestSsoAdminSnapIn
{
	public partial class ViewSolutionUserDetails : AppKit.NSWindow
	{
		#region Constructors

		// Called when created from unmanaged code
		public ViewSolutionUserDetails (IntPtr handle) : base (handle)
		{
			Initialize ();
		}
		
		// Called when created directly from a XIB file
		[Export ("initWithCoder:")]
		public ViewSolutionUserDetails (NSCoder coder) : base (coder)
		{
			Initialize ();
		}
		
		// Shared initialization code
		void Initialize ()
		{
		}

		#endregion
	}
}

