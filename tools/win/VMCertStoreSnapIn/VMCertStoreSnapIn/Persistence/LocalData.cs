/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */using System;using System.Collections.Generic;using System.Linq;using System.Text;
using VMCertStore.Common.DTO;
namespace VMCertStoreSnapIn.Persistence{    public class LocalData    {        List<VMCertStoreServerDTO> _servers = new List<VMCertStoreServerDTO>();        public List<VMCertStoreServerDTO> ServerList { get { return _servers; } set { _servers = value; } }
        public LocalData()        {        }
        public void AddServer(VMCertStoreServerDTO dto)        {            ServerList.Add(dto);        }
        public bool RemoveServer(string guid)        {            int index = ServerList.FindIndex(x => x.GUID == guid);            if (index > -1)                ServerList.RemoveAt(index);            return index > -1;        }
        public VMCertStoreServerDTO GetServerByGuid(string guid)        {            return ServerList.Find(x => x.GUID == guid);        }    }}