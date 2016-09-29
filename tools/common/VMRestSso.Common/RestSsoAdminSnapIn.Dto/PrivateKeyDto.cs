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
using System.Runtime.Serialization;

namespace Vmware.Tools.RestSsoAdminSnapIn.Dto
{
    [DataContract]
    public class PrivateKeyDto : IDataContext
    {
        [DataMember]
        private string encoded;

        public string Encoded
        {
            get { return encoded; }
            set { encoded = value; }
        }

        [DataMember]
        private string algorithm;

        public EncrptionAlgorithm Algorithm
        {
            get
            {
                EncrptionAlgorithm algo;
                return Enum.TryParse(algorithm, false, out algo) ? algo : EncrptionAlgorithm.RSA;
            }
            set
            {
                algorithm = value.ToString();
            }
        }
    }
}
