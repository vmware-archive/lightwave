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
using VMCASnapIn.DTO;
using System.Xml.Serialization;
using System.Linq;

namespace VMCASnapIn.Persistence
{
    public class VMCALocalData
    {
        [XmlIgnoreAttribute]
        Queue<VMCAServerDTO> _servers = new Queue<VMCAServerDTO> ();

        [XmlIgnoreAttribute]
        public Queue<VMCAServerDTO> ServerQueue { get { return _servers; } set { _servers = value; } }

        public List<VMCAServerDTO> SerializableList = new List<VMCAServerDTO> ();

        // Currently upto 3 recent servers are supported.
        public int CacheSize = 3;

        public VMCALocalData () : base ()
        {
            CacheSize = 3;
            FillServerQueue ();
        }

        public void AddServer (VMCAServerDTO dto)
        {
            var match = SerializableList.FirstOrDefault (listElement => string.Equals (listElement.Server, dto.Server));
            if (match == null) {
                if (ServerQueue.Count >= CacheSize) {
                    VMCAServerDTO item = ServerQueue.Dequeue ();
                    SerializableList.Remove (item);
                }
                ServerQueue.Enqueue (dto);
                SerializableList.Add (dto);
            }
        }

        public void RemoveServer(VMCAServerDTO dto)
        {
            for (int i = 0; i < SerializableList.Count; i++)
            {
                VMCAServerDTO item = ServerQueue.Dequeue();
                if (!string.Equals(item.Server, dto.Server))
                {
                    ServerQueue.Enqueue(item);
                }
            }
            SerializableList.Remove(dto);
        }

        public VMCAServerDTO GetServerDTO (string server)
        {
            return SerializableList.FirstOrDefault (listElement => string.Equals (listElement.Server, server));
        }

        public string[] GetServerArray ()
        {
            FillServerQueue ();
            List<string> servers = new List<string> ();
            foreach (var item in ServerQueue) {
                
                servers.Add (item.Server);
            }
            return servers.ToArray ();
        }

        public void FillServerQueue ()
        {
            foreach (var item in SerializableList) {
                ServerQueue.Enqueue (item);
            }
        }
    }
}

