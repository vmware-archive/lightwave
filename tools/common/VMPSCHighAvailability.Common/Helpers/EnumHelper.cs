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
using System.ComponentModel;
using System.Reflection;

namespace VMPSCHighAvailability.Common.Helpers
{
	/// <summary>
	/// Enum helper.
	/// </summary>
	public class EnumHelper
	{
		/// <summary>
		/// Enum to the dictionary.
		/// </summary>
		/// <returns>The dictionary.</returns>
		/// <param name="enumerator">Enumerator.</param>
		public static Dictionary<string, string> ToDictionary(Enum enumerator)
		{ 
			var dictionary = new Dictionary<string, string> ();

			Type type = enumerator.GetType ();
			foreach (Enum item in Enum.GetValues(type)) 
            {
				var description = GetDescription (item);
				dictionary.Add (item.ToString (), description);
			}
			return dictionary;
		}

		/// <summary>
		/// Enum to the list.
		/// </summary>
		/// <returns>The list.</returns>
		/// <param name="enumerator">Enumerator.</param>
		public static List<string> ToList(Enum enumerator)
		{
			var list = new List<string> ();

			var type = enumerator.GetType ();
			foreach (Enum item in Enum.GetValues(type))
            {
				var description = GetDescription (item);
				list.Add (description);
			}
			return list;
		}

		/// <summary>
		/// Gets the description.
		/// </summary>
		/// <returns>The description.</returns>
		/// <param name="enumerator">Enumerator.</param>
		public static string GetDescription(Enum enumerator)
		{
			Type type = enumerator.GetType ();
			MemberInfo[] memberInfo = type.GetMember (enumerator.ToString ());

			if (memberInfo != null && memberInfo.Length > 0) 
            {
				object[] attributes = memberInfo [0].GetCustomAttributes (typeof(DescriptionAttribute), false);

				if (attributes != null && attributes.Length > 0) 
                {
					return ((DescriptionAttribute)attributes [0]).Description;
				}
			}

			return enumerator.ToString ();
		}

		/// <summary>
		/// Gets the by description.
		/// </summary>
		/// <returns>The by description.</returns>
		/// <param name="enumerator">Enumerator.</param>
		/// <param name="description">Description.</param>
		public static Enum GetByDescription(Enum enumerator, string description)
		{
			Type type = enumerator.GetType();
			Enum value = enumerator;
			foreach (Enum item in Enum.GetValues(type))
			{
				var desc = GetDescription(item);
				if (desc == description)
				{
					value = item;
					break;
				}
			}
			return value;
		}
	}
}

