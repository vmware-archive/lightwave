/* * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved. * * Licensed under the Apache License, Version 2.0 (the “License”); you may not * use this file except in compliance with the License.  You may obtain a copy * of the License at http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an “AS IS” BASIS, without * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the * License for the specific language governing permissions and limitations * under the License. */ namespace VMIdentity.CommonUtils{    public static class CommonConstants
    {        public const string VALUES_EMPTY = "One or more required values are empty";        public const string CONFIRM_DELETE = "Are you sure, you want to delete {0} {1} ?";        public const string UNABLE_TO_LOGIN = "Unable to login! One or more errors occured.";
        public const string CONFIRM_SELECTED_DELETE = "Are you sure, you want to delete selected {0} ({1}) ?";

        public static string GetDeleteMsg(string obj, string value)
        {
            return string.Format(CONFIRM_DELETE, obj, value);
        }

        public static string GetSelectedDeleteMsg(string obj, int count)
        {
            return string.Format(CONFIRM_SELECTED_DELETE, obj, count);
        }

    }
}
