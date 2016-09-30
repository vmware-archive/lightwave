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
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace VMwareMMCIDP.UI.Common.Utilities
{
    public class MMCUIConstants
    {
        // Confirmation messages
        public const string VALUES_EMPTY = "One or more required values are empty";
        public const string CONFIRM = "Are you sure ?";
        public const string CONFIRM_DELETE = "Are you sure, you want to delete? ";
        public const string UNABLE_TO_LOGIN = "Unable to login! One or more errors occured.";
        public const string EDIT = "Edit";
        public const string UPDATE = "Update";
        public const string LOGIN = "Login";
        public const string LOGOUT = "Logout";
        public const string INCOMPATIBLE_SERVER = "Server is unsupported or not reachable";

        // Error messages
        public const string SERVER_ENT = "Please enter server";
        public const string TENANT_ENT = "Please enter the domain on the server to which the username belongs";
        public const string USERNAME_ENT = "Please enter your username";
        public const string PASSWORD_ENT = "Please enter your password";
        public const string ALIAS_ENT = "Please enter an alias";
        public const string PRI_KEY_SEL = "Please select a private key file";
        public const string SEC_KEY_SEL = "Please select a private key file";
        public const string CERT_SEL = "Please select a certificate file";
        public const string TOKEN_SEL = "Please select a token file";
        public const string STORE_ENT = "Please enter a store name";
        public const string COUNTRY_ENT = "Please enter a country";
        public const string COUNTRY_LEN_VAL = "Country must not exceed 2 characters";
        public const string NAME_ENT = "Please enter a name";
        public const string CERT_CHAIN_SEL = "Please select atleast 2 chain certificates";
        public const string INVALID_CURRENT_PASSWORD = "Please enter a valid current password.";
        public const string INVALID_NEW_PASSWORD = "Please enter a valid new password.";
        public const string PASSWORD_MISMATCH = "New and confirm password do not match. Please re-enter.";
        public const string NEW_TENANT_ENT = "Please enter new Tenant";
        public const string VALID_TENANT_ENT = "Please enter valid tenant name";
        public const string ADMIN_USER_ENT = "Please enter admin user";
        public const string ADMIN_PWD_ENT = "Please enter admin password";
        public const string IDP_ADMIN_USER_ENT = "Please enter IDP admin user";
        public const string IDP_ADMIN_PWD_ENT = "Please enter IDP admin password";
        public const string PRI_KEY_NOT_FOUND = "Private key file not found";
        public const string UPN_ENT = "Please enter UPN";
        public const string DN_ENT = "Please enter DN";
        public const string SERVICE_NOT_FOUND = "Service not found.";

        //Invalid data messages
        public const string INVALID_IP = "Please enter a valid IP address.";
        public const string INVALID_EMAIL = "Please enter a valid email address.";

        // File type filters
        public const string PRI_KEY_FILTER = "Private Key Files (*.pem)|*.pem|All Files (*.*)|*.*";
        public const string PUB_KEY_FILTER = "Public Key Files (*.pem)|*.pem|All Files (*.*)|*.*";
        public const string CSR_FILTER = "CSR Files (*.csr)|*.csr|All Files (*.*)|*.*";
        public const string CERT_FILTER = "Certificate Files (*.crt)|*.crt|All Files (*.*)|*.*";
        public const string SAML_TOKEN_FILTER = "SAML Token Files (*.xml)|*.xml|All Files (*.*)|*.*";
        public const string XML_FILTER = "XML Files (*.xml)|*.xml|All Files (*.*)|*.*";
        public const string TXT_FILTER = "TXT Files (*.txt)|*.txt|All Files (*.*)|*.*";
        public const string CSV_FILTER = "CSV Files (*.csv)|*.csv|All Files (*.*)|*.*";

        // Log constants
        private const string LOG_FOLDER = @"C:\Users\{0}\AppData\Local\VMware, Inc\VMware Identity Tools\Logs\";
        public const string PSC_LOG_FILE = "VMPscSiteManagement.log";
        public const string VMDIR_LOG_FILE = "VMDir.log";

        //Pattern constants
        public const string EmailPattern = @"^(?("")("".+?(?<!\\)""@)|(([0-9a-z]((\.(?!\.))|[-!#\$%&'\*\+/=\?\^`\{\}\|~\w])*)(?<=[0-9a-z])@))"
            + @"(?(\[)(\[(\d{1,3}\.){3}\d{1,3}\])|(([0-9a-z][-\w]*[0-9a-z]*\.)+[a-z0-9][\-a-z0-9]{0,22}[a-z0-9]))$";

        public static string GetLogFolder(string windowsLogonName)
        {
            return string.Format(LOG_FOLDER, windowsLogonName);
        }
    }
}
