using System;

using Foundation;
using AppKit;
using VMDirSchema;

namespace VMDirSchemaEditorSnapIn
{
    public partial class DiffDetailViewerController : NSWindowController
    {
        private string baseText;
        private string currentText;
        private string baseServer;
        private string currentServer;

        public DiffDetailViewerController(IntPtr handle)
            : base(handle)
        {
        }

        [Export("initWithCoder:")]
        public DiffDetailViewerController(NSCoder coder)
            : base(coder)
        {
        }

        public DiffDetailViewerController(string baseServer, string currentServer, string baseText, string currentText)
            : base("DiffDetailViewer")
        {
            this.baseText = baseText;
            this.currentText = currentText;
            this.baseServer = VMDirSchemaConstants.BASE_TITLE + baseServer;
            this.currentServer = VMDirSchemaConstants.CURRENT_TITLE + currentServer;
        }

        public override void AwakeFromNib()
        {
            base.AwakeFromNib();
            //Break the Metadata by ':' as delimiter and show in different lines.
            baseText = baseText.Replace(":", Environment.NewLine);
            currentText = currentText.Replace(":", Environment.NewLine);
            BaseTextView.Value = baseText;
            CurrentTextView.Value = currentText;
            BaseLabel.StringValue = baseServer;
            CurrentLabel.StringValue = currentServer;
        }

        public new DiffDetailViewer Window
        {
            get { return (DiffDetailViewer)base.Window; }
        }

        [Export("windowWillClose:")]
        public void WindowWillClose(NSNotification notification)
        {
            NSApplication.SharedApplication.StopModal();
        }
    }
}
