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

using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using System.Collections;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service
{
    public class SuperLoggingHelper
    {
        public string GetHtml(EventLogDto dto)
        {
            var body = string.Empty;

            if (dto.Metadata != null)
            {
                foreach (var keyValuePair in dto.Metadata)
                {
                    var value = ToCamelCaseWithSpaces(keyValuePair.Key);
                    var header = string.Format("<strong>{0}</strong><hr>", value);
                    var content = string.Empty;
                    if (keyValuePair.Value.GetType() == typeof(Dictionary<string, object>))
                    {
                        var dict = keyValuePair.Value as Dictionary<string, object>;
                        if (dict != null)
                        {
                            foreach (var kvp in dict)
                            {
                                value = ToCamelCaseWithSpaces(kvp.Key);
                                content += string.Format("<span style='width:300px;margin-bottom:5px;'>{0}</span>:", value);

                                if (kvp.Value.GetType() == typeof(bool))
                                {
                                    var bVal = (bool)kvp.Value;
                                    var bStr = bVal ? "YES" : "NO";
                                    content += string.Format("<span style='color:blue;padding-left:5px;'>{0}</span><br/><br/>", bStr);
                                }
                                else if (kvp.Value.GetType() == typeof(string))
                                {
                                    var bStr = kvp.Value as string;
                                    content += string.Format("<span style='color:blue;padding-left:5px;'>{0}</span><br/><br/>", bStr);
                                }
                            }
                        }
                    }
                    else if (keyValuePair.Value.GetType() == typeof(string))
                    {
                        content = keyValuePair.Value.ToString();
                    }
                    body += string.Format("{0}{1}<br/><br/><br/>", header, content);
                }
            }
            var container = string.Format("<div style='font-family: Arial, Helvetica;font-size:0.7em;letter-spacing:2px;'>{0}</div>", body);
            return container;
        }

		public string GetText(EventLogDto dto)
		{
			var content = new StringBuilder();

			if (dto.Metadata != null)
			{
				foreach (var keyValuePair in dto.Metadata)
				{
					var value = ToCamelCaseWithSpaces(keyValuePair.Key);

					for (int i = 0; i < value.Length + 4; i++)
						content.Append ("=");
					content.AppendLine ();
					content.AppendFormat ("  {0}",value.ToUpper());
					content.AppendLine ();
					for (int i = 0; i < value.Length + 4; i++)
						content.Append ("=");
					content.AppendLine ();
					content.AppendLine ();


					if (keyValuePair.Value.GetType() == typeof(Dictionary<string, object>))
					{
						var dict = keyValuePair.Value as Dictionary<string, object>;
						if (dict != null)
						{
							foreach (var kvp in dict)
							{
								var val = ToCamelCaseWithSpaces(kvp.Key);
								content.AppendFormat ("{0}",val);

								if (kvp.Value.GetType() == typeof(bool))
								{
									var bVal = (bool)kvp.Value;
									var bStr = bVal ? "YES" : "NO";
									content.AppendFormat (" : {0}",bStr);
								}
								else if (kvp.Value.GetType() == typeof(string))
								{
									var bStr = kvp.Value as string;
									content.AppendFormat (" : {0}",bStr);
								}
								content.AppendLine ();
								content.AppendLine ();
							}
						}
					}
					else if (keyValuePair.Value.GetType() == typeof(string))
					{
						var bStr = keyValuePair.Value.ToString();
						content.AppendFormat ("{0}",bStr);
						content.AppendLine ();
						content.AppendLine ();
					}
				}
			}
			return content.ToString();
		}

        private string ToCamelCaseWithSpaces(string wordInCamelCase)
        {
            const string regEx = "([a-z](?=[A-Z])|[A-Z](?=[A-Z][a-z]))";
            var result = Regex.Replace(wordInCamelCase, regEx, "$1 ");
            result = char.ToUpper(result[0]) + (result.Length > 1 ? result.Substring(1) : string.Empty);
            return result;
        }

        public string GetJsonText(StringBuilder output, ArrayList eventLogs)
        {
            JsonConvert.JsonSerialize(eventLogs, output);
            return output.ToString();
        }
        public List<EventLogDto> ApplyFilter(List<EventLogDto> eventLogs, List<FilterCriteriaDto> filters)
        {
            if (eventLogs == null)
                return new List<EventLogDto>();
            return  eventLogs.Where(x => Filter(x, filters)).ToList();
        }

        private bool Filter(EventLogDto x, List<FilterCriteriaDto> filters)
        {
            var include = true;
            foreach (var filter in filters)
            {
                var isMatch = filter.Apply(x);
                if (!isMatch)
                {
                    include = false;
                    break;
                }
            }
            return include;
        }
    }
}
