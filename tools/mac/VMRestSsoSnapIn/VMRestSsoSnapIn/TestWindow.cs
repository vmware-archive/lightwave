using System;

using Foundation;
using AppKit;

namespace VMRestSsoSnapIn
{
	public partial class TestWindow : NSWindow
	{
		public TestWindow (IntPtr handle) : base (handle)
		{
		}

		[Export ("initWithCoder:")]
		public TestWindow (NSCoder coder) : base (coder)
		{
		}

		public override void AwakeFromNib ()
		{
			base.AwakeFromNib ();
		}
	}
}
