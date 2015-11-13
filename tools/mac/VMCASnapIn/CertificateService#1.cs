using System;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.IO;
using NVelocity.App;
using NVelocity;

namespace VmIdentity.UI.Common.Utilities
{
    public static class CertificateService
    {
        //Todo- move to commonUtils and remove UI code reference
        public static void DisplayCertificate (X509Certificate2 cert)
        {
            if (cert != null) {
                String data = ConstructHTMLFromX509 (cert);
                X509CertViewerController xwc = new X509CertViewerController (data);
                xwc.Window.MakeKeyAndOrderFront (xwc);
            } else {
                UtilityService.ShowAlert ("Certificate is null", "Alert");
            }
        }

        //Todo - Fill the certification details for all fields and add icon once available from Tony
        public static  String ConstructHTMLFromX509 (X509Certificate2 cert)
        {
            string htmlString = null;
            UtilityService.CheckedExec (delegate() {
                Velocity.Init ();
                var model = new
                {
                    Version = cert.Version,
                    SerialNumber = cert.SerialNumber,
                    SignatureAlgorithm = cert.SignatureAlgorithm,
                    PublicKey = cert.PublicKey
                };
                var velocityContext = new VelocityContext ();
                velocityContext.Put ("model", model);
                string template = string.Join (Environment.NewLine, new [] {
                    "<html><head></head>",
                    " <body><h3>Certificate Details&nbsp;</h3>",
                    "<p><b><font color=\"grey\"> Subject Name </font></b></p>",
                    "<hr>",
                    "<p ><font color=\"808080\">Issued To:</font></p>",

                    "<p ><font color=\"808080\">Issued by:</font></p>",

                    "<p><font color=\"808080\">Version :</font $model.Version </p>",

                    "<p ><font color=\"808080\">Serial Number :</font> $model.SerialNumber</p>",

                    "<p><font color=\"808080\">Signature algorithm :</font>$model.SignatureAlgorithm</p>",

                    "<p ><font color=\"808080\">Signature hash algorithm :</font></p>",
                    "<p><b><font color=\"grey\"> Issuer Name </font></b></p> <hr>",

                    "<p ><font color=\"808080\">Issuer :&nbsp;</font></p>",

                    "<p ><font color=\"808080\">Valid from :</font></p>",

                    "<p ><font color=\"808080\">Valid to :</font></p>",

                    "<p ><font color=\"808080\">Subject :</font></p>",
                    "<p><b><font color=\"grey\"> Public Key Info </font></b></p><hr>",
                    "<p ><font color=\"808080\">Public key :</font> $model.PublicKey</p>",

                    "<p ><font color=\"808080\">Key Usage :&nbsp;</font></p>",

                    "<p ><font color=\"808080\">Subject Alternative Name :</font></p>",

                    "<p ><font color=\"808080\">Subject Key Identifier :</font></p>",

                    "<p ><font color=\"808080\">Authority Key Identifier :&nbsp;</font></p>",

                    "<p ><font color=\"808080\">Thumbprint algorithm :</font></p>",

                    "<p ><font color=\"808080\">Thumbprint :</font></p></body></html>"
                });

                var sb = new StringBuilder ();
                Velocity.Evaluate (
                    velocityContext,
                    new StringWriter (sb),
                    "Certificate template",
                    new StringReader (template));
                htmlString = sb.ToString ();
            });
            return htmlString;
        }
    }
}

