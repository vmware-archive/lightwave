using System;
using VMCA.Client;
using AppKit;
using System.IO;

namespace UI
{
    public static class UIHelper
    {
        public static void SaveKeyData (KeyPairData data)
        {
            var save = NSSavePanel.SavePanel;
            save.AllowedFileTypes = new string[] { "pem" };
            nint result = save.RunModal ();
            if (result == (int)1) {
                string path = save.Url.Path;
                File.WriteAllText (path, data.PrivateKey);
            }
                
            result = save.RunModal ();
            if (result == (int)1) {
                string path = save.Url.Path;
                File.WriteAllText (path, data.PublicKey);
            }
        }
    }
}

