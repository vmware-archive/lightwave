using System;

using Foundation;
using AppKit;

namespace VmIdentity.UI.Common
{
    [Register ("ActivatableToolBarItem")]
    public class ActivatableToolBarItem : NSToolbarItem
    {
        public bool Active { get; set; } = false;

        public ActivatableToolBarItem ()
        {
        }

        public ActivatableToolBarItem (IntPtr handle) : base (handle)
        {
        }

        public ActivatableToolBarItem (NSObjectFlag  t) : base (t)
        {
        }

        public ActivatableToolBarItem (string title) : base (title)
        {
        }

        public override void Validate ()
        {
            base.Validate ();

            Enabled = Active;
        }
    }
}
