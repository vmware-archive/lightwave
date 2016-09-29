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
using System.Net;
using System.Reflection;

namespace Vmware.Tools.RestSsoAdminSnapIn.Core.Web
{
    public class HttpWebRequestManager : IWebRequestManager
    {
        private bool _canPublish;
        private IList<Action<WebMessageTrace>> _subscribers;

        public HttpWebRequestManager(bool publish = true)
        {
            _canPublish = publish;
            _subscribers = new List<Action<WebMessageTrace>>();
        }

        private WebRequest Compose(string uri, RequestSettings settings, IDictionary<HttpRequestHeader, string> headers, string postData)
        {
            var request = Compose(uri, settings, headers, null, postData);
            HttpWebRequestHelper.WriteStringToRequestStream(request, postData);
            return request;
        }

        private WebRequest Compose(string uri, RequestSettings settings, IDictionary<HttpRequestHeader, string> headers, IDictionary<string, string> customHeaders, string postData)
        {
			//var url = new Uri (uri);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            if (settings != null)
            {
                request.Method = settings.Method.ToString().ToUpper();
            }

            if (headers != null)
            {
                foreach (var header in headers)
                {
                    switch (header.Key)
                    {
                        case HttpRequestHeader.UserAgent:
                            request.UserAgent = headers[HttpRequestHeader.UserAgent];
                            break;
                        case HttpRequestHeader.ContentType:
                            request.ContentType = headers[HttpRequestHeader.ContentType];
                            break;
                        case HttpRequestHeader.Authorization:
                            request.Headers.Add(HttpRequestHeader.Authorization, headers[HttpRequestHeader.Authorization]);
                            break;
                        case HttpRequestHeader.Accept:
                            request.Accept = headers[HttpRequestHeader.Accept];
                            break;
                    }
                }
            }

            if (customHeaders != null)
            {
                foreach (var header in customHeaders)
                {
                    request.Headers.Add(header.Key, header.Value);
                }
            }

            if (!string.IsNullOrEmpty(postData))
            {
                HttpWebRequestHelper.WriteStringToRequestStream(request, postData);
            }

            return request;
        }


        private HttpResponse GetResponse(WebRequest request)
        {
            return HttpWebRequestHelper.GetResponseFromServer(request);
        }

        private void Notify(Guid messageId, HttpResponse response)
        {
            var trace = new WebMessageTrace(messageId)
            {
                Messages = new List<WebMessage>
                {
                    new WebMessage(WebMessageType.Response)
                    {
                        Details = response.Data,
                        Headers = HttpWebRequestHelper.GetHeaders(response.Headers)
                    }
                }
            };
            Publish(trace);
        }
        private void Notify(Guid messageId, WebRequest request, string data)
        {
            var trace = new WebMessageTrace(messageId)
            {
                Messages = new List<WebMessage>
                {
                    new WebMessage(WebMessageType.Request)
                    {
                        Details = data,
                        Headers = HttpWebRequestHelper.GetHeaders(request.Headers),
                        Method = request.Method,
                        AbsolutePath = request.RequestUri.AbsolutePath
                    }
                }
            };
            Publish(trace);
        }
        private void Notify(Guid messageId, WebException exception)
        {
            var trace = new WebMessageTrace(messageId)
            {
                Messages = new List<WebMessage>
                {
                    new WebMessage(WebMessageType.Error)
                    {
                        Details = string.Format("Message - {0} Stack Trace - {1}", exception.Message , exception.StackTrace),
                        Headers = HttpWebRequestHelper.GetHeaders(exception.Response.Headers)
                    }
                }
            };
            Publish(trace);
        }

        private void Notify(Guid messageId, HttpRequest request, HttpResponse response, Exception exception)
        {
            var trace = new WebMessageTrace(messageId);

            if(response!= null)
                trace.Messages.Add(
                    new WebMessage(WebMessageType.Response)
                    {
                        Details = response.Data,
                        Headers = HttpWebRequestHelper.GetHeaders(response.Headers),
                        Timestamp = DateTime.UtcNow
                    });

            if (request != null)
            {
                trace.Server = request.Server;
                trace.Messages.Add(
                        new WebMessage(WebMessageType.Request)
                        {
                            Details = request.Data,
                            Headers = HttpWebRequestHelper.GetHeaders(request.Headers),
                            Method = request.Request.Method,
                            AbsolutePath = request.Request.RequestUri.AbsolutePath,
                            Timestamp = request.Timestamp
                        });
            }
            if(exception!= null)
               trace.Messages.Add(
                    new WebMessage(WebMessageType.Error)
                    {
                        Details = string.Format("Message - {0} Stack Trace - {1}", exception.Message , exception.StackTrace),
                        Headers = exception.GetType() == typeof(WebException) && ((WebException)exception).Response != null ? HttpWebRequestHelper.GetHeaders(((WebException)exception).Response.Headers) : new Dictionary<string, string>(),
                        Timestamp = DateTime.UtcNow
                    });

            Publish(trace);
        }

        public string GetResponse(string uri, RequestSettings settings, IDictionary<HttpRequestHeader, string> headers, IDictionary<string, string> customHeaders, string postData)
        {
            Exception error = null;
            HttpResponse response = null;

            var request = Compose(uri, settings, headers, customHeaders, postData);
            var httpRequest = new HttpRequest((HttpWebRequest)request) { Data = postData };

            try
            {
                response = GetResponse(request);
            }
            catch (Exception exception)
            {
                error = exception;
            }

            var messageId = Guid.NewGuid();
            Notify(messageId, httpRequest, response, error);

            if (error != null)
                throw error;
            else
                return response.Data;
        }

        public void Publish(WebMessageTrace message)
        {
            if (_canPublish)
            {
                foreach (var action in _subscribers)
                    action.Invoke(message);
            }
        }

        public void CanPublish(bool publish)
        {
            _canPublish = publish;
        }

        public void Subscribe(Action<WebMessageTrace> subscriber)
        {
            _subscribers.Add(subscriber);
        }
        public void UnSubscribe(Action<WebMessageTrace> subscriber)
        {
            if (_subscribers.Contains(subscriber))
                _subscribers.Remove(subscriber);
        }
    }
}