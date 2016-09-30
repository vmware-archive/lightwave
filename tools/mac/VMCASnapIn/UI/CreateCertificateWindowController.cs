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
using System.IO;
using AppKit;
using Foundation;
using VMCASnapIn.DTO;
using VMCASnapIn.Services;
using VMCASnapIn.UI;
using VmIdentity.UI.Common.Utilities;
using VMIdentity.CommonUtils.Utilities;

namespace VMCASnapIn.UI
{
    public partial class CreateCertificateWindowController : AppKit.NSWindowController
    {
        CertRequestDTO dto;

        #region Constructors

        // Called when created from unmanaged code
        public CreateCertificateWindowController (IntPtr handle) : base (handle)
        {
        }

        // Called when created directly from a XIB file
        [Export ("initWithCoder:")]
        public CreateCertificateWindowController (NSCoder coder) : base (coder)
        {
        }

        // Call to load from the XIB/NIB file
        public CreateCertificateWindowController (CertRequestDTO dto) : base ("CreateCertificateWindow")
        {
            this.dto = dto;
        }

        #endregion

        public override void AwakeFromNib ()
        {
            base.AwakeFromNib ();

            //Prepopulate fields
            NotBefore.DateValue = DateTimeToNSDate (DateTime.UtcNow);
            NotAfter.DateValue = DateTimeToNSDate (DateTime.UtcNow.AddYears (1));

            string[] countries = NSLocale.ISOCountryCodes;
            CountryPopUpButton.AddItems (countries);
            CountryPopUpButton.SelectItem (Array.FindIndex (countries, x => x.Contains ("US")));

            State.StringValue = "WA";
            Locality.StringValue = "Bellevue";
            KeyUSageContraints.StringValue = "0";

            //Events
            CreateButton.Activated += OnClickCreateButton;
            SelectPriKey.Activated += OnClickSelectPriKey;
            CancelButton.Activated += (object sender, EventArgs e) => {
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (0);
            };
        }

        public void ValidateControls ()
        {
            if (string.IsNullOrWhiteSpace (dto.Country))
                throw new Exception ("Please enter a country");
            if (string.IsNullOrWhiteSpace (dto.Name))
                throw new Exception ("Please enter a name");
            if (!dto.PrivateKey.HasData)
                throw new Exception ("Please enter a private key");
            if (!string.IsNullOrWhiteSpace (dto.Email) && !Network.IsValidEmail (dto.Email))
                throw new Exception ("Please enter a valid Email");
            if (!string.IsNullOrWhiteSpace (dto.IPAddress) && !Network.IsValidIP (dto.IPAddress))
                throw new Exception ("Please enter a valid IP");
        }

        public void OnClickSelectPriKey (object sender, EventArgs eventargs)
        {
            PrivateKeyEditorWindowController pwc = new PrivateKeyEditorWindowController (dto.PrivateKey);
            nint result = NSApplication.SharedApplication.RunModalForWindow (pwc.Window);
            try {
                if (result == (nint)Constants.DIALOGOK) {
                    if (!string.IsNullOrEmpty (dto.PrivateKey.PrivateKeyFileName)) {
                        dto.PrivateKey.PrivateKeyString = File.ReadAllText (dto.PrivateKey.PrivateKeyFileName);
                    }
                    PrivateKey.StringValue = dto.PrivateKey.PrivateKeyString;
                }
            } catch (Exception e) {
                UIErrorHelper.ShowAlert (string.Empty, e.Message);
            }
        }


        public void OnClickCreateButton (object sender, EventArgs eventargs)
        {
            UIErrorHelper.CheckedExec (delegate() {
                dto.Country = CheckNullAndReturnString (CountryPopUpButton.TitleOfSelectedItem);
                dto.DNSName = CheckNullAndReturnString (DNSName.StringValue);
                dto.Email = CheckNullAndReturnString (Email.StringValue);
                dto.IPAddress = CheckNullAndReturnString (IPAddress.StringValue);
                dto.KeyUsageConstraints = Convert.ToUInt32 (KeyUSageContraints.IntValue);
                dto.Locality = CheckNullAndReturnString (Locality.StringValue);
                dto.Name = CheckNullAndReturnString (Name.StringValue);
                dto.NotAfter = this.NSDateToDateTime (NotAfter.DateValue);
                dto.NotBefore = this.NSDateToDateTime (NotBefore.DateValue);
                dto.Organization = CheckNullAndReturnString (Organization.StringValue);
                dto.OU = CheckNullAndReturnString (OU.StringValue);
                dto.State = CheckNullAndReturnString (State.StringValue);
                dto.URIName = CheckNullAndReturnString (URIName.StringValue);
                ValidateControls ();
                this.Close ();
                NSApplication.SharedApplication.StopModalWithCode (Constants.DIALOGOK);
            });
        }


        //Utility functions
        public string CheckNullAndReturnString (string str)
        {
            return string.IsNullOrWhiteSpace (str) ? null : str;
        }

        public  DateTime NSDateToDateTime (NSDate date)
        {
            //NSDate has a wider range than DateTime, so clip
            // the converted date to DateTime.Min|MaxValue.
            double secs = date.SecondsSinceReferenceDate;
            if (secs < -63113904000)
                return DateTime.MinValue;
            if (secs > 252423993599)
                return DateTime.MaxValue;
            return ((DateTime)date).ToUniversalTime ();
        }

        public  NSDate DateTimeToNSDate (DateTime date)
        {
            if (date.Kind == DateTimeKind.Unspecified)
                date = DateTime.SpecifyKind (date, DateTimeKind.Utc);
            return (NSDate)date.ToUniversalTime ();
        }

        //strongly typed window accessor
        public new CreateCertificateWindow Window {
            get {
                return (CreateCertificateWindow)base.Window;
            }
        }
    }
}

