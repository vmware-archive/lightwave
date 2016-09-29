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

using System.Collections.Generic;
using System.Xml.Serialization;
using System.Linq;
using System;

namespace VmIdentity.CommonUtils.Persistance
{
    public class LocalData
    {
        [XmlIgnoreAttribute]
        Queue<string> _servers = new Queue<string>();

        [XmlIgnoreAttribute]
        public Queue<string> ServerQueue { get { return _servers; } set { _servers = value; } }

        public List<string> SerializableList = new List<string>();

        // Currently upto 3 recent servers are supported.
        public int CacheSize = 3;
        private bool isQueueLoaded = false;

        public LocalData()
        {
        }

        public void AddServer(string server)
        {
            try
            {
                var match = SerializableList.FirstOrDefault(stringToCheck => stringToCheck.Contains(server));
                if (match == null)
                {
                    if (ServerQueue.Count >= CacheSize)
                    {
                        string item = ServerQueue.Dequeue();
                        SerializableList.Remove(item);
                    }
                    ServerQueue.Enqueue(server);
                    SerializableList.Add(server);
                }
            }
            catch (Exception e)
            {
                throw e;
            }
        }

        public string[] GetServerArray()
        {
            if (!isQueueLoaded)
            {
                FillServerQueue();
                isQueueLoaded = true;
            }

            return ServerQueue.ToArray();

        }

        public void FillServerQueue()
        {
            foreach (var item in SerializableList)
            {
                ServerQueue.Enqueue(item);
            }
        }
    }
}
