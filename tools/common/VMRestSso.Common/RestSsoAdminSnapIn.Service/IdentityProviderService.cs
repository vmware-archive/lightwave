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

using System;
using System.Collections.Generic;
using System.Net;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Serialization;
using Vmware.Tools.RestSsoAdminSnapIn.Core.Web;
using Vmware.Tools.RestSsoAdminSnapIn.Dto;
using Vmware.Tools.RestSsoAdminSnapIn.Dto.Attributes;
using S=System.Runtime.Serialization.Formatters;

namespace Vmware.Tools.RestSsoAdminSnapIn.Service.IdentityProvider
{
	public class IdentityProviderService
	{
		private readonly IWebRequestManager _webRequestManager;
        private readonly IServiceConfigManager _serviceConfigManager;
        public IdentityProviderService(IWebRequestManager webRequestManager, IServiceConfigManager serviceConfigManager)
		{
			_webRequestManager = webRequestManager;
            _serviceConfigManager = serviceConfigManager;
		}

		static string CleanupJson (string response)
		{
			var startIndex = 0;
			var subStr = response;
			while (startIndex >= 0) {
				startIndex = subStr.IndexOf ("\"schema\":{");
				var endIndex = subStr.IndexOf ("\"matchingRuleInChainEnabled");
				if (startIndex < 0 || endIndex < 0)
					break;
				var subStr1 = subStr.Substring (startIndex, endIndex - startIndex);
				subStr = subStr.Replace (subStr1, string.Empty);
			}
			startIndex = 0;
			while (startIndex >= 0) {
				startIndex = subStr.IndexOf ("\"attributesMap\":{");
				var endIndex = subStr.IndexOf ("\"matchingRuleInChainEnabled");
				if (startIndex < 0 || endIndex < 0)
					break;
				var subStr1 = subStr.Substring (startIndex, endIndex - startIndex);
				subStr = subStr.Replace (subStr1, string.Empty);
			}
			return subStr;
		}

		static string CleanupSchemaJson (string response)
		{
			var startIndex = 0;
			var subStr = response;
			while (startIndex >= 0) {
				startIndex = subStr.IndexOf ("\"schema\":{");
				var endIndex = subStr.IndexOf ("\"matchingRuleInChainEnabled");
				if (startIndex < 0 || endIndex < 0)
					break;
				var subStr1 = subStr.Substring (startIndex, endIndex - startIndex);
				subStr = subStr.Replace (subStr1, string.Empty);
			}
			return subStr;
		}

		static string SchemaJsonToDictionary (string response)
		{
			var startIndex = 0;
			var subStr = response;
			var result = string.Empty;
				startIndex = subStr.IndexOf ("schema");
				var endIndex = subStr.IndexOf ("matchingRuleInChainEnabled");
			var original = subStr.Substring (startIndex - 1, endIndex - startIndex - 1);
			result = original;
			result = result.Replace ("\"schema\":{", "\"schema\":[");
			result = result.Replace ("\"ObjectIdUser\"", "{\"Key\":\"ObjectIdUser\",\"Value\"");
			result = result.Replace ("\"ObjectIdPasswordSettings\"", "{\"Key\":\"ObjectIdPasswordSettings\",\"Value\"");
			result = result.Replace ("\"ObjectIdDomain\"", "{\"Key\":\"ObjectIdDomain\",\"Value\"");
			result = result.Replace ("\"ObjectIdGroup\"", "{\"Key\":\"ObjectIdGroup\",\"Value\"");
			result = result.Replace ("\"attributeMappings\":{", "\"attributeMappings\":[");
			result = result.Replace ("}}", "}]}");

			foreach (UserAttributeId uAttr in Enum.GetValues(typeof(UserAttributeId)))
			{
				var attributeName = "\"" + uAttr.ToString () + "\"";
				result = result.Replace (attributeName, "{\"Key\":" + attributeName + ",\"Value\"");
			}
			foreach (DomainAttributeId uAttr in Enum.GetValues(typeof(DomainAttributeId)))
			{
				var attributeName = "\"" + uAttr.ToString () + "\"";
				result = result.Replace (attributeName, "{\"Key\":" + attributeName + ",\"Value\"");
			}
			foreach (GroupAttributeId uAttr in Enum.GetValues(typeof(GroupAttributeId)))
			{
				var attributeName = "\"" + uAttr.ToString () + "\"";
				result = result.Replace (attributeName, "{\"Key\":" + attributeName + ",\"Value\"");
			}
			foreach (PasswordAttributeId uAttr in Enum.GetValues(typeof(PasswordAttributeId)))
			{
				var attributeName = "\"" + uAttr.ToString () + "\"";
				result = result.Replace (attributeName, "{\"Key\":" + attributeName + ",\"Value\"");
			}
			result = result.Substring (0, result.Length - 1);
			result += "]";
			return response.Replace (original, result);
		}

		public List<IdentityProviderDto> GetAll(ServerDto server, string tenant, Token token)
		{
			tenant = Uri.EscapeDataString(tenant);
			var url = string.Format(_serviceConfigManager.GetIdentityProvidersPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
			return JsonConvert.JsonDeserialize<List<IdentityProviderDto>>(response);
		}

		public IdentityProviderDto Get(ServerDto server, string tenant, string provider, Token token)
		{
			tenant = Uri.EscapeDataString(tenant);
			provider = Uri.EscapeDataString(provider);
			var url = string.Format(_serviceConfigManager.GetIdentityProviderPostEndPoint(), server.Protocol, server.ServerName, server.Port, tenant, provider);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var postData = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, postData);
            return JsonConvert.JsonDeserialize <IdentityProviderDto>(response);
		}

		public IdentityProviderDto Create(ServerDto server, string tenant, IdentityProviderDto provider, Token token)
		{
			var schemaSerialized = SerializeSchema(provider.Schema);
			var attributeSerailized = SerializeAttributes (provider.AttributesMap, "attributesMap");

			provider.Schema = null; 
			provider.AttributesMap = null;

			tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetIdentityProvidersEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
			var dto = typeof(IdentityProviderDto).Assembly;
			var json = JsonConvert.Serialize(provider,"root", dto.GetTypes(), true);
			json = SerializationJsonHelper.Cleanup (json);
			json = json.Substring (0, json.Length - 1);

			var attributeString = "\"attributesMap\":null,";
			if (json.Contains (attributeString))
				json = json.Replace (attributeString, attributeSerailized + (string.IsNullOrEmpty(attributeSerailized)? string.Empty : ","));
			else
				json += attributeSerailized;

			var schemaString = "\"schema\":null,";
			if (json.Contains (schemaString))
				json = json.Replace (schemaString, schemaSerialized + (string.IsNullOrEmpty(schemaSerialized)? string.Empty : ","));
			else
				json += schemaSerialized;
			json += "}";

			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.JsonDeserialize <IdentityProviderDto>(response);
		}

		public bool Delete(ServerDto server, string tenant, string provider, Token token)
		{
			tenant = Uri.EscapeDataString(tenant);
			provider = Uri.EscapeDataString(provider);
            var url = string.Format(_serviceConfigManager.GetIdentityProviderEndPoint(), server.Protocol, server.ServerName, server.Port, tenant, provider);
			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Delete,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			var json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower();
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			return string.IsNullOrEmpty(response);
		}

		public IdentityProviderDto Update(ServerDto server, string tenant, IdentityProviderDto provider, Token token)
		{
			var schemaSerialized = SerializeSchema(provider.Schema);
			var attributeSerailized = SerializeAttributes (provider.AttributesMap, "attributesMap");

			provider.Schema = null; 
			provider.AttributesMap = null;

			tenant = Uri.EscapeDataString(tenant);
			var name = Uri.EscapeDataString(provider.Name);
            var url = string.Format(_serviceConfigManager.GetIdentityProviderEndPoint(), server.Protocol, server.ServerName, server.Port, tenant, name);
			var dto = typeof(IdentityProviderDto).Assembly;
			var json = JsonConvert.Serialize(provider,"root", dto.GetTypes(), true);
			json = SerializationJsonHelper.Cleanup (json);
			json = json.Substring (0, json.Length - 1);

			var attributeString = "\"attributesMap\":null,";
			if (json.Contains (attributeString))
				json = json.Replace (attributeString, attributeSerailized + (string.IsNullOrEmpty(attributeSerailized)? string.Empty : ","));
			else
				json += attributeSerailized;

			var schemaString = "\"schema\":null,";
			if (json.Contains (schemaString))
				json = json.Replace (schemaString, schemaSerialized + (string.IsNullOrEmpty(schemaSerialized)? string.Empty : ","));
			else
				json += schemaSerialized;
			json += "}";


			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Put,
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
            return JsonConvert.JsonDeserialize <IdentityProviderDto>(response);
		}

		public IdentityProviderDto Probe(ServerDto server, string tenant, IdentityProviderDto provider, Token token)
		{
			var schemaSerialized = SerializeSchema(provider.Schema);
			var attributeSerailized = SerializeAttributes (provider.AttributesMap, "attributesMap");

			provider.Schema = null; 
			provider.AttributesMap = null;

			tenant = Uri.EscapeDataString(tenant);
            var url = string.Format(_serviceConfigManager.GetIdentityProvidersEndPoint(), server.Protocol, server.ServerName, server.Port, tenant);
			url += "?probe=true";
			var dto = typeof(IdentityProviderDto).Assembly;
			var json = JsonConvert.Serialize(provider,"root", dto.GetTypes(), true);
			json = SerializationJsonHelper.Cleanup (json);

			json = json.Substring (0, json.Length - 1);

			var attributeString = "\"attributesMap\":null,";
			if (json.Contains (attributeString))
				json = json.Replace (attributeString, attributeSerailized + (string.IsNullOrEmpty(attributeSerailized)? string.Empty : ","));
			else
				json += attributeSerailized;

			var schemaString = "\"schema\":null,";
			if (json.Contains (schemaString))
				json = json.Replace (schemaString, schemaSerialized + (string.IsNullOrEmpty(schemaSerialized)? string.Empty : ","));
			else
				json += schemaSerialized;
			json += "}";

			ServicePointManager.ServerCertificateValidationCallback = delegate { return true; };
			var requestConfig = new RequestSettings
			{
				Method = HttpMethod.Post
			};
			var headers = ServiceHelper.AddHeaders(ServiceConstants.JsonContentType);
			json = "access_token=" + token.AccessToken + "&token_type=" + token.TokenType.ToString().ToLower() + "&" + json;
			var response = _webRequestManager.GetResponse(url, requestConfig, headers, null, json);
			response = SerializationJsonHelper.JsonToDictionary("attributesMap",response);
			response = CleanupSchemaJson (response);
			return JsonConvert.Deserialize<IdentityProviderDto>(response,"root", dto.GetTypes(), true);
		}

		private string SerializeAttributes(Dictionary<string,string> attributes, string key)
		{
			if (attributes == null)
				return string.Empty;
			var attributesSerialized = "\"" + key + "\":{";

			foreach (var item in attributes) {
				attributesSerialized += "\"" + item.Key + "\":\"" + item.Value + "\",";
			}
			attributesSerialized = attributesSerialized.Substring (0, attributesSerialized.Length - 1);
			attributesSerialized += "}";
			return attributesSerialized;
		}

		private string SerializeSchema(Dictionary<string,SchemaObjectMappingDto> schema)
		{
			if (schema != null) {
				var schemaSerialized = "\"schema\":{";
				foreach (var item in schema) {
					var value = item.Value;
					schemaSerialized += "\"" + item.Key +"\":{" + SerializeAttributes(value.AttributeMappings, "attributeMappings") + ",\"objectClass\":\"" +  value.ObjectClass + "\"},";
				}
				schemaSerialized = schemaSerialized.Substring (0, schemaSerialized.Length - 1);
				schemaSerialized += "}";
				return schemaSerialized;
			}
			return string.Empty;
		}
	}
}