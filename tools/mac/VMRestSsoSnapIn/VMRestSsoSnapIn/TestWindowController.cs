using System;

using Foundation;
using AppKit;

using VmIdentity.UI.Common;

namespace VMRestSsoSnapIn
{
	public partial class TestWindowController : NSWindowController
	{
		public TestWindowController (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public TestWindowController (NSCoder coder) : base (coder)
		{
		}

		public TestWindowController () : base ("TestWindow")
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
//			var controller = new ThreePaneSplitViewController();
//			this.CustomView.AddSubview(controller.View); 
		}

		public new TestWindow Window {
			get { return (TestWindow)base.Window; }
		}
	}
}
