/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


using System;
using System.Runtime.InteropServices;
using System.Security.Cryptography.X509Certificates;
using AppKit;
using Foundation;
using ObjCRuntime;
using Security;

namespace RestSsoAdminSnapIn
{
    public class CertificateService
    {
       [DllImport ("/usr/lib/libobjc.dylib", EntryPoint = "objc_msgSend")]
        public extern static IntPtr IntPtr_objc_msgSend (IntPtr receiver, IntPtr selector);

        [DllImport ("/usr/lib/libobjc.dylib", EntryPoint = "objc_msgSend")]
        public extern static global::System.nint nint_objc_msgSend_IntPtr_bool (IntPtr receiver, IntPtr selector, IntPtr arg1, bool arg2);

        [DllImport ("/usr/lib/libobjc.dylib", EntryPoint = "objc_msgSend")]
        public extern static void void_objc_msgSend_IntPtr_IntPtr_IntPtr_IntPtr_bool (IntPtr receiver, IntPtr selector, IntPtr arg1, IntPtr arg2, IntPtr arg3, IntPtr arg4, IntPtr arg5, bool arg6);

        [Export ("createPanelDidEnd:returnCode:contextInfo:")]

        public void CreatePanelDidEnd (NSWindow w, int returnCode, IntPtr context)
        {
        }

        public static void DisplayX509Certificate2 (NSObject sender, X509Certificate2 cert)
        {
            Class sfCertificatePanelClass = new Class ("SFCertificatePanel");
            Selector sharedCertificatePanelSelector = new Selector ("sharedCertificatePanel");            
            IntPtr panel = IntPtr_objc_msgSend (sfCertificatePanelClass.Handle, sharedCertificatePanelSelector.Handle);

            using (var sc = new SecCertificate (cert)) {
                NSArray a = NSArray.FromNSObjects (sc);
                void_objc_msgSend_IntPtr_IntPtr_IntPtr_IntPtr_bool (panel, Selector.GetHandle ("beginSheetForWindow:modalDelegate:didEndSelector:contextInfo:certificates:showGroup:"), 
                    NSApplication.SharedApplication.MainWindow.Handle, sender.Handle, Selector.GetHandle ("createPanelDidEnd:returnCode:contextInfo:"), IntPtr.Zero, a.Handle, false);
            }
        }
    }
}

