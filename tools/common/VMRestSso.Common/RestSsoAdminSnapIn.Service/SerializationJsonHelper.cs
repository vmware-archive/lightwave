/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

using System.Text.RegularExpressions;
using System.Linq;
using System;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
	public class SerializationJsonHelper
	{
		public static string Cleanup(string json)
		{
			int start = 0;
			var legnth = "RestSsoAdminSnapIn".Length;
			do {
				start = FirstIndexOf (json, "__type");
				if (start != -1) {
					var end = FirstIndexOf (json, "RestSsoAdminSnapIn");
					if (end != -1) {
						var left = json.Substring (0, start - 1);
						var right = (((end + legnth + 6) < json.Length) ? json.Substring (end + legnth + 6) : string.Empty);
						json =  left + right;
					}
				} else
					break;
			} while (start != -1);

			json = CleanupKeyValue (json);
			return json;
		}

		private static int FirstIndexOf( string str, string value) {
			Match match = Regex.Matches(str, value).Cast<Match>().FirstOrDefault();
			return match != null ? match.Index : -1;
		}

		private static string CleanupKeyValue(string json)
		{
			return json.Replace ("\"Key\":", string.Empty).Replace (",\"Value\"", string.Empty);
		}

		public static string JsonToDictionary(string attribute, string json)
		{
			var startIndex = json.IndexOf (attribute);
			if (startIndex > -1) {
				var substr = json.Substring (startIndex);
				var endIndex = substr.IndexOf ('}');
				var original = json.Substring (startIndex, endIndex + 1);
				startIndex = original.IndexOf ('{');
				endIndex = original.IndexOf ('}');
				var prefix = original.Substring (0, startIndex);
				var jsonPayload = original.Substring (startIndex + 1, endIndex - startIndex - 1);
				var keyValuePairs = jsonPayload.Split (':');
				var result = new System.Text.StringBuilder ();
				for (int i = 0; i < keyValuePairs.Length; i += 2) {
					result.Append ("{");
					result.AppendFormat ("\"Key\":{0},\"Value\":{1}", keyValuePairs [i], keyValuePairs [i + 1]);
					result.Append ("}");
					if (i + 1 < keyValuePairs.Length - 1) {
						result.Append (",");
					}
				}
				var final = prefix + "[" + result.ToString () + "]";
				return json.Replace (original, final);
			}
			return json;
		}
	}
}

