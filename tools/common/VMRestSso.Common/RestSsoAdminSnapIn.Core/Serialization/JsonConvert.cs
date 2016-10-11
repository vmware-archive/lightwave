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
using System.IO;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Web.Script.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization
{
    /// <summary>
    /// Generic Json serializer and deserializer
    /// </summary>
    public static class JsonConvert
    {
        /// <summary>
        /// Serialize object to json string
        /// </summary>
        /// <typeparam name="T">Any class or collection</typeparam>
        /// <param name="obj">Object</param>
        /// <returns>Serialized json string</returns>
        public static string Serialize<T>(T obj, bool dictionary=false) where T : class
        {
            DataContractJsonSerializer serializer;
            if (dictionary)
            {
                var settings = new DataContractJsonSerializerSettings();
                settings.UseSimpleDictionaryFormat = true;
                serializer = new DataContractJsonSerializer(obj.GetType(), settings);
            }
            else
            {
                serializer = new DataContractJsonSerializer(obj.GetType());
            }           
            using (var ms = new MemoryStream())
            {
                serializer.WriteObject(ms, obj);
                var retVal = Encoding.Default.GetString(ms.ToArray());
                return retVal;
            }
        }

		/// <summary>
		/// Serialize object to json string
		/// </summary>
		/// <typeparam name="T">Any class or collection</typeparam>
		/// <param name="obj">Object</param>
		/// <returns>Serialized json string</returns>
		public static string Serialize<T>(T obj, string rootName, Type[] types, bool dictionary=false) where T : class
		{
			DataContractJsonSerializer serializer;
			if (dictionary)
			{
				var settings = new DataContractJsonSerializerSettings();
				settings.UseSimpleDictionaryFormat = true;
				settings.RootName = rootName;
				settings.MaxItemsInObjectGraph = 1000;
				settings.KnownTypes = types;
				settings.EmitTypeInformation = System.Runtime.Serialization.EmitTypeInformation.AsNeeded;
				serializer = new DataContractJsonSerializer(obj.GetType(), settings);
			}
			else
			{
				serializer = new DataContractJsonSerializer(obj.GetType());
			}           
			using (var ms = new MemoryStream())
			{
				serializer.WriteObject(ms, obj);
				var retVal = Encoding.Default.GetString(ms.ToArray());
				return retVal;
			}
		}

        /// <summary>
        /// Deserialize from json string to object
        /// </summary>
        /// <typeparam name="T">Any class or collection</typeparam>
        /// <param name="json">Json string</param>
        /// <returns>Object of type T</returns>
        public static T Deserialize<T>(string json, bool dictionary = false) where T : class
        {
            var obj = Activator.CreateInstance<T>();
            using (var ms = new MemoryStream(Encoding.Unicode.GetBytes(json)))
            {
                DataContractJsonSerializer serializer;
                if(dictionary)
                {
                    var settings = new DataContractJsonSerializerSettings();
                    settings.UseSimpleDictionaryFormat = true;
                    serializer = new DataContractJsonSerializer(obj.GetType(), settings);
                }
                else
                {
                    serializer = new DataContractJsonSerializer(obj.GetType());
                }
                obj = (T) serializer.ReadObject(ms);
                ms.Close();
                return obj;
            }
        }

		/// <summary>
		/// Deserialize from json string to object
		/// </summary>
		/// <typeparam name="T">Any class or collection</typeparam>
		/// <param name="json">Json string</param>
		/// <returns>Object of type T</returns>
		public static T Deserialize<T>(string json, string rootName, Type[] types, bool dictionary = false) where T : class
		{
			var obj = Activator.CreateInstance<T> ();
			using (var ms = new MemoryStream (Encoding.Unicode.GetBytes (json))) {
				DataContractJsonSerializer serializer;
				if (dictionary) {
					var settings = new DataContractJsonSerializerSettings ();
					settings.UseSimpleDictionaryFormat = true;
					settings.RootName = rootName;
					settings.MaxItemsInObjectGraph = 1000;
					settings.KnownTypes = types;
					settings.EmitTypeInformation = System.Runtime.Serialization.EmitTypeInformation.AsNeeded;
					serializer = new DataContractJsonSerializer (obj.GetType (), settings);
				} else {
					serializer = new DataContractJsonSerializer (obj.GetType ());
				}
				obj = (T)serializer.ReadObject (ms);
				ms.Close ();
				return obj;
			}
		}

        /// <summary>
        /// Deserialize using Javascript serializer
        /// </summary>
        /// <typeparam name="T">Dto</typeparam>
        /// <param name="json">json string</param>
        /// <returns>Instance of Dto</returns>
        public static T JsonDeserialize<T>(string json)
        {
            var serializer = new JavaScriptSerializer();
            var result = serializer.Deserialize<T>(json);
            return result;
        }

        /// <summary>
        /// Serialize using Javascript serializer
        /// </summary>
        public static string JsonSerialize(Object obj)
        {
            var serializer = new JavaScriptSerializer();
            var result = serializer.Serialize(obj);
            return result;
        }

        /// <summary>
        /// Serialize using Javascript serializer
        /// </summary>
        public static void JsonSerialize(Object obj, StringBuilder output)
        {
            var serializer = new JavaScriptSerializer();
            serializer.Serialize(obj, output);
        }
    }
}