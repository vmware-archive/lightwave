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

namespace VmIdentity.UI.Common
{
    public  static class VMIdentityConstants
    {
        /* dialog result codes */
        public const int DIALOGOK = 1;
        public const int DIALOGCANCEL = -1;

        /* UI Control labels */
        public const string CONNECT = "Connect";
        public const string CONNECT_TO = "Connected to ";
        public const string DISCONNECT = "Disconnect";
        public const string NAME_FIELD = "Name";
        public const string LOGGED_IN = "Logged in : ";
        public const string EDIT = "Edit";
        public const string UPDATE = "Update";
        public const string BINDUPN_VSPHERE = "Administrator@vsphere.local";

        /* Certificate Constants */
        public const string CERT_ISSUED_BY = "IssuedBy";
        public const string CERT_ISSUED_DATE = "IssuedDate";
        public const string CERT_EXPIRATION_DATE = "ExpirationDate";
        public const string CERT_INTENDED_PURPOSES = "IntendedPurposes";
        public const string CERT_STATUS = "Status";
        public const string CERT_ISSUED_TO = "IssuedTo";

        /* UI Error Messages */
        /* Error Messages */
        public const string SERVER_CONNECT_ERROR = "Unable to connect to the Server. Please check the login details.";
        public const string CONNECTION_NOT_SUCCESSFUL = "Connection is not successful.";
        public const string SERVER_TIMED_OUT = "Server timed out";
        public const string SERVER_CONNECT = "Connect to Server";
        public const string EMPTY_FIELD = "One or more fields are empty. Please make sure to provide values.";
        public const string SUCCESS = "Success";

        public const string HOST_NOT_REACHABLE = "Host not reachable";
        public const string HOST_OR_IP_ADDRESS_NOT_REACHABLE = "Host or IP address is not reachable. Please enter a valid server name of ip address and try again.";
        public const string CONFIRM_DELETE = "Are you sure to Delete?";
        public const string CONFIRM_MSG = "Are you sure?";

        /* File Path/Names */
        public const string APPLICATION_DATA_FOLDER_NAME = "LightwaveTools";

        /* General constants */
        public const int DefaultColumnWidth = 200;
        public const int ServerTimeoutInSeconds = 30;
        public const int LOGIN_TIMEOUT_IN_MILLI = 30000;

        /* UI Event Constants */
        public const string CLOSE_MAIN_WINDOW = "CloseMainWindow";
        public const string RELOAD_OUTLINEVIEW = "ReloadOutlineView";
        public const string RELOAD_TABLEVIEW = "ReloadTableView";
        public const string REFRESH_UI = "RefreshUI";
    }
}

